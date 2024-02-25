import EventEmitter from 'events'
import * as Location from 'expo-location'
import { type LocationObject } from 'expo-location'
import BackgroundTimer, { type TimeoutId } from 'react-native-background-timer'
import { requestBackgroundLocationPermissions } from './common'
import type { DeviceSettingsSchema } from './deviceSettings'
import { Config } from '../config'
import { pick } from '../utils'
import { mockTour } from '../utils/gpsTourMock'

export type LocationState = {
  timestamp: number
  latitude: number
  longitude: number
  /** The altitude in meters above the WGS 84 reference ellipsoid */
  altitude: number
  /** The accuracy of the altitude value, in meters */
  altitudeAccuracy: number
  /** The radius of uncertainty for the location, measured in meters */
  accuracy: number
  /** Horizontal direction of travel of this device, measured in degrees starting at due north and continuing clockwise around the compass. Thus, north is 0 degrees, east is 90 degrees, south is 180 degrees, and so on. */
  heading: number
  /** Meters per second */
  speed: number
  // slope: number
}

declare interface GPSEventEmitter {
  on(
    event: 'locationUpdate',
    listener: (coordinates: LocationState) => void,
  ): this
  off(
    event: 'locationUpdate',
    listener: (coordinates: LocationState) => void,
  ): this
  emit(event: 'locationUpdate', coordinates: LocationState): boolean

  on(event: 'toggleGranted', listener: (granted: boolean) => void): this
  off(event: 'toggleGranted', listener: (granted: boolean) => void): this
  emit(event: 'toggleGranted', granted: boolean): boolean
}

class GPSEventEmitter extends EventEmitter {}

export class GPS extends GPSEventEmitter {
  private granted = false
  private locationState: LocationState = {
    timestamp: 0,
    latitude: 0,
    longitude: 0,
    heading: 0,
    altitude: -Number.MAX_SAFE_INTEGER,
    altitudeAccuracy: 0,
    accuracy: 0,
    speed: 0,
  }
  locationObservingOptions: {
    accuracy: number
    gpsTimeInterval: number
    gpsDistanceSensitivity: number
  } | null = null
  private tourMockTimeout: TimeoutId | null = null

  async destroy() {
    super.removeAllListeners()
    await this.stopObservingLocation()
  }

  isGranted() {
    return this.granted
  }

  getCoordinates() {
    return this.locationState
  }

  /** Should be called only from TaskManager */
  updateLocation(location: LocationObject) {
    this.locationState = {
      timestamp: location.timestamp,
      heading: location.coords.heading ?? 0,
      speed: location.coords.speed ?? 0,
      altitude: location.coords.altitude ?? 0,
      altitudeAccuracy: location.coords.altitudeAccuracy ?? 0,
      accuracy: location.coords.accuracy ?? 0,
      ...pick(location.coords, 'latitude', 'longitude'),
    }
    this.emit('locationUpdate', this.locationState)
  }

  async startObservingLocation(settings: DeviceSettingsSchema) {
    const {
      gpsAccuracy: accuracy,
      gpsTimeInterval,
      gpsDistanceSensitivity,
    } = settings

    this.locationObservingOptions = {
      accuracy,
      gpsTimeInterval,
      gpsDistanceSensitivity,
    }

    const granted = await this.requestPermissions()
    this.emit('toggleGranted', granted)
    if (!granted) {
      console.error('GPS permission not granted')
      //TODO: show toast with error
      return
    }

    if (await Location.hasStartedLocationUpdatesAsync(Config.GPS_TASK_NAME)) {
      console.info('Task already registered, stopping and restarting')
      await Location.stopLocationUpdatesAsync(Config.GPS_TASK_NAME)
    }

    console.info('Starting location updates with options:', {
      accuracy,
      gpsTimeInterval,
      gpsDistanceSensitivity,
    })

    await Location.startLocationUpdatesAsync(Config.GPS_TASK_NAME, {
      accuracy,
      timeInterval: gpsTimeInterval,
      deferredUpdatesInterval: gpsTimeInterval,
      distanceInterval: gpsDistanceSensitivity,
      deferredUpdatesDistance: gpsDistanceSensitivity,

      showsBackgroundLocationIndicator: true,
      foregroundService: {
        notificationTitle: 'Bike Tour Assistant',
        notificationBody: 'Location is tracking in background',
        notificationColor: '#fff',
      },
    })

    if (Config.MOCK_TOUR) {
      let index = 0
      let lastMockTimestamp = mockTour[0].timestamp
      const sendNextMockLocation = () => {
        if (index >= mockTour.length) {
          this.tourMockTimeout = null
          return
        }
        const mockLocation = mockTour[index++]

        this.tourMockTimeout = BackgroundTimer.setTimeout(() => {
          this.emit('locationUpdate', mockLocation)
          sendNextMockLocation()
        }, mockLocation.timestamp - lastMockTimestamp)

        lastMockTimestamp = mockLocation.timestamp
      }
      sendNextMockLocation()
    }
  }

  async stopObservingLocation() {
    if (Config.MOCK_TOUR && this.tourMockTimeout) {
      BackgroundTimer.clearTimeout(this.tourMockTimeout)
      this.tourMockTimeout = null
    }

    try {
      console.info('Stopping location updates')
      this.locationObservingOptions = null
      await Location.stopLocationUpdatesAsync(Config.GPS_TASK_NAME)
    } catch (e) {
      console.warn(
        `Cannot stop location updates: ${
          e instanceof Error ? e.message : String(e)
        }`,
      )
    }
  }

  private async requestPermissions() {
    if ((await requestBackgroundLocationPermissions()) === false) {
      return false
    }

    const foregroundPermission =
      await Location.requestForegroundPermissionsAsync()
    if (!foregroundPermission.granted) {
      return false
    }

    const backgroundPermission = await Location.getBackgroundPermissionsAsync()
    if (!backgroundPermission.granted) {
      return false
    }

    this.granted = true
    return true
  }
}
