import BackgroundTimer, { type TimeoutId } from 'react-native-background-timer'
import { Bluetooth, MessagePriority } from './bluetooth'
import { DeviceSettings, type DeviceSettingsSchema } from './deviceSettings'
import { GPS, type LocationState } from './gps'
import { parseGpxFile, type TourPoint } from './gpxParser'
import { MapProvider, type Tile } from './mapProvider'
import { IncommingMessageType, MessageType } from './message'
import { Config } from '../config'
import { debounce, pick } from '../utils'

export class Core {
  public static readonly instances = new Set<Core>()

  public readonly deviceSettings = new DeviceSettings()
  public readonly bluetooth = new Bluetooth()
  public readonly mapProvider = new MapProvider()
  public readonly gps = new GPS()

  private pingMessageInterval: TimeoutId | null = null
  private sendingMapTile = false
  private tilesToSendQueue: Tile[] = []

  constructor() {
    console.info('Initializing core...')
    Core.instances.add(this)

    Promise.all([this.deviceSettings.init(), this.bluetooth.init()])
      .then(() => this.postInit())
      .catch(console.error)
  }

  async destroy() {
    console.info('Destroying core...')
    Core.instances.delete(this)

    this.deviceSettings.destroy()
    this.bluetooth.destroy()
    this.mapProvider.destroy()
    await this.gps.destroy()
  }

  private postInit() {
    const [broadcastLightnessUpdate] = debounce((value: number) => {
      this.bluetooth
        .sendMessage(
          {
            type: MessageType.SET_LIGHTNESS,
            data: { lightness: value },
          },
          MessagePriority.HIGH,
        )
        .catch(console.error)
    }, 200)
    const [broadcastDistancePerPhotoUpdate] = debounce((value: number) => {
      this.bluetooth
        .sendMessage(
          {
            type: MessageType.SET_DISTANCE_PER_PHOTO,
            data: { distance: value },
          },
          MessagePriority.HIGH,
        )
        .catch(console.error)
    }, 200)

    this.bluetooth.on('deviceConnected', () => {
      try {
        this.pingMessageInterval = BackgroundTimer.setInterval(() => {
          //Skip sending ping message if there are at least 2 messages in the queue
          if (this.bluetooth.getSendingMessageQueueLength() >= 2) {
            return
          }

          this.bluetooth
            .sendMessage(
              { type: MessageType.PING, data: null },
              MessagePriority.VERY_LOW,
            )
            .catch(console.error)
        }, Config.PING_INTERVAL_MS)
      } catch (error) {
        console.error(`Cannot set ping message interval. Error: ${error}`)
      }

      broadcastLightnessUpdate(this.deviceSettings.get('lightness'))
      broadcastDistancePerPhotoUpdate(
        this.deviceSettings.get('distancePerPhoto'),
      )

      this.gps
        .startObservingLocation(this.deviceSettings.getSettings())
        .catch(console.error)
      void this.synchronizeTour(this.deviceSettings.get('gpxFile'))
      void this.synchronizePointsOfInterest(
        this.deviceSettings.get('pointsOfInterest'),
      )
    })

    this.bluetooth.on('deviceDisconnected', () => {
      if (this.pingMessageInterval) {
        try {
          BackgroundTimer.clearInterval(this.pingMessageInterval)
        } catch (error) {
          console.error(`Cannot clear ping message interval. Error: ${error}`)
        }
        this.pingMessageInterval = null
      }

      this.mapProvider.reset()
      void this.gps.stopObservingLocation()
      this.sendingMapTile = false
      this.tilesToSendQueue = []
    })

    this.bluetooth.on('messageReceived', (message) => {
      const type: IncommingMessageType = message[6]
      switch (type) {
        default:
          console.warn('Unknown incomming message type:', type)
          break
        case IncommingMessageType.PONG:
          console.info('Pong')
          break
        case IncommingMessageType.MESSAGE_OUT_REQUEST_TILE:
          {
            const tileX = message.readUint32LE(7)
            const tileY = message.readUint32LE(11)
            const tileZ = message.readUint8(15)
            console.info('Requesting tile:', tileX, tileY, tileZ)
            this.mapProvider.requestTile(tileX, tileY, tileZ)
          }
          break
      }

      this.bluetooth
        .sendMessage(
          { type: MessageType.CONFIRM_RECEIVED_MESSAGE, data: null },
          MessagePriority.VERY_HIGH,
        )
        .catch(console.error)
    })

    this.deviceSettings.on('change', (settings, key) => {
      if (!this.bluetooth.isConnectedToDevice()) {
        return
      }

      if (!key || key === 'lightness') {
        broadcastLightnessUpdate(settings.lightness)
      }
      if (!key || key === 'distancePerPhoto') {
        broadcastDistancePerPhotoUpdate(settings.distancePerPhoto)
      }
      if (!key || key === 'gpxFile') {
        void this.synchronizeTour(settings.gpxFile)
      }
      if (!key || key === 'pointsOfInterest') {
        void this.synchronizePointsOfInterest(settings.pointsOfInterest)
      }

      if (
        key === 'gpsAccuracy' ||
        key === 'gpsTimeInterval' ||
        key === 'gpsDistanceSensitivity'
      ) {
        this.gps.startObservingLocation(settings).catch(console.error)
      }
    })

    this.gps.on('locationUpdate', this.handleLocationUpdate.bind(this))
    this.mapProvider.on('tileLoaded', async (tile) => {
      void this.sendMapTile(tile)
    })
  }

  private async handleLocationUpdate(locationState: LocationState) {
    console.info(
      'Coordinates:',
      locationState.latitude,
      locationState.longitude,
      'Heading:',
      locationState.heading,
      'Speed:',
      locationState.speed,
      'Altitude:',
      locationState.altitude,
    )

    if (!this.bluetooth.isConnectedToDevice()) {
      console.warn('No device connected. Stopping location updates.')
      await this.gps.stopObservingLocation()
      return
    }

    this.bluetooth
      .sendMessage(
        {
          type: MessageType.LOCATION_UPDATE,
          data: {
            location: locationState,
            mapZoom: this.deviceSettings.get('mapZoom'),
          },
        },
        MessagePriority.VERY_HIGH,
      )
      .catch(console.error)
  }

  private async sendMapTile(tile: Tile) {
    if (this.sendingMapTile) {
      this.tilesToSendQueue.push(tile)
      return
    }

    this.sendingMapTile = true

    try {
      const tileIdentifier = pick(tile, 'x', 'y', 'z')

      await this.bluetooth.sendMessage(
        {
          type: MessageType.SEND_MAP_TILE_START,
          data: {
            tileIdentifier,
            fileSize: tile.fileData.byteLength,
          },
        },
        MessagePriority.VERY_LOW,
      )

      const chunkSize = 224 //(max message length) - 1 byte for message type - 2 bytes for chunk index
      const chunksCount = Math.ceil(tile.fileData.byteLength / chunkSize)

      console.info(
        'Sending tile data sequentialy:',
        tile.x,
        tile.y,
        tile.z,
        'chunks:',
        chunksCount,
      )

      for (let c = 0; c < chunksCount; c++) {
        const start = c * chunkSize
        const end = start + chunkSize
        const bytes = tile.fileData.slice(start, end)

        try {
          await this.bluetooth.sendMessage(
            {
              type: MessageType.SEND_MAP_TILE_DATA_CHUNK,
              data: { chunkIndex: c, bytes },
            },
            MessagePriority.VERY_LOW,
          )
        } catch (error) {
          console.error('Failed to send map tile chunk:', c)
        }
      }
    } catch (error) {
      console.error('Failed to send map tile', error)
    }

    this.sendingMapTile = false

    if (this.tilesToSendQueue.length > 0) {
      const nextTile = this.tilesToSendQueue.shift()
      if (nextTile) {
        void this.sendMapTile(nextTile)
      }
    }
  }

  private async synchronizeTour(gpxFile: DeviceSettingsSchema['gpxFile']) {
    if (!gpxFile || !gpxFile.assets?.length) {
      await this.bluetooth.sendMessage(
        { type: MessageType.CLEAR_TOUR_DATA, data: null },
        MessagePriority.HIGH,
      )
      return
    }

    try {
      const tour = await parseGpxFile(gpxFile.assets[0].uri)

      await this.bluetooth.sendMessage(
        {
          type: MessageType.SEND_TOUR_START,
          data: { pointsCount: tour.length },
        },
        MessagePriority.HIGH,
      )

      const tourDataChunkSize = 22

      for (let i = 0; i < tour.length; i += tourDataChunkSize) {
        const data: TourPoint[] = []

        for (let j = 0; j < tourDataChunkSize; j++) {
          const point = tour[i + j]
          if (!point) {
            break
          }
          data.push(point)
        }

        await this.bluetooth.sendMessage(
          {
            type: MessageType.SEND_TOUR_DATA_CHUNK,
            data,
          },
          MessagePriority.HIGH,
        )
      }
    } catch (error) {
      console.error('Failed to synchronize tour:', error)
    }
  }

  private async synchronizePointsOfInterest(
    points: DeviceSettingsSchema['pointsOfInterest'],
  ) {
    try {
      await this.bluetooth.sendMessage(
        {
          type: MessageType.SEND_POINTS_OF_INTEREST_START,
          data: { pointsCount: points.length },
        },
        MessagePriority.HIGH,
      )

      const tourDataChunkSize = 26

      for (let i = 0; i < points.length; i += tourDataChunkSize) {
        const data: Pick<LocationState, 'latitude' | 'longitude'>[] = []

        for (let j = 0; j < tourDataChunkSize; j++) {
          const point = points[i + j]
          if (!point) {
            break
          }
          data.push(point)
        }

        await this.bluetooth.sendMessage(
          {
            type: MessageType.SEND_POINTS_OF_INTEREST_DATA_CHUNK,
            data,
          },
          MessagePriority.HIGH,
        )
      }
    } catch (error) {
      console.error('Failed to synchronize points of interest:', error)
    }
  }
}
