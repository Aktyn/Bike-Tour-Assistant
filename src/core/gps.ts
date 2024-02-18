import EventEmitter from 'events'
import * as Location from 'expo-location'
import { LocationAccuracy, type LocationObject } from 'expo-location'
import { requestBackgroundLocationPermissions } from './common'
import type { DeviceSettingsSchema } from './deviceSettings'
import { Config } from '../config'
import { pick } from '../utils'

export type LocationState = {
  timestamp: number
  latitude: number
  longitude: number
  /** The altitude in meters above the WGS 84 reference ellipsoid */
  // altitude: number
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

  // on(event: 'toggleGranted', listener: (granted: boolean) => void): this
  // off(event: 'toggleGranted', listener: (granted: boolean) => void): this
  //   emit(event: 'toggleGranted', granted: boolean): boolean
}

class GPSEventEmitter extends EventEmitter {}

export class GPS extends GPSEventEmitter {
  private granted = false
  private locationState: LocationState = {
    timestamp: 0,
    latitude: 0,
    longitude: 0,
    heading: 0,
    // altitude: -Number.MAX_SAFE_INTEGER,
    // slope: 0,
    speed: 0,
  }
  locationObservingOptions: {
    accuracy: number
    gpsTimeInterval: number
    gpsDistanceSensitivity: number
  } | null = null

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
      //TODO: use accuracy to show area of uncertainty
      //TODO: use altitude to calculate slope
      ...pick(location.coords, 'latitude', 'longitude'),
    }
    this.emit('locationUpdate', this.locationState)
  }

  async startObservingLocation(_settings: DeviceSettingsSchema) {
    //TODO
    // const accuracy = settings.gpsAccuracy
    // const gpsTimeInterval = settings.gpsTimeInterval
    // const gpsDistanceSensitivity = settings.gpsDistanceSensitivity
    const accuracy = LocationAccuracy.BestForNavigation
    const gpsTimeInterval = 1000
    const gpsDistanceSensitivity = 1

    this.locationObservingOptions = {
      accuracy,
      gpsTimeInterval,
      gpsDistanceSensitivity,
    }

    const granted = await this.requestPermissions()
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
  }

  async stopObservingLocation() {
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
