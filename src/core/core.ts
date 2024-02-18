import BackgroundTimer, { type TimeoutId } from 'react-native-background-timer'
import { Bluetooth } from './bluetooth'
import { DeviceSettings } from './deviceSettings'
import { GPS, type LocationState } from './gps'
import { MapProvider, type Tile } from './mapProvider'
import { MessageType } from './message'
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
        .sendMessage({
          type: MessageType.SET_LIGHTNESS,
          data: { lightness: value },
        })
        .catch(console.error)
    }, 200)

    this.bluetooth.on('deviceConnected', () => {
      try {
        this.pingMessageInterval = BackgroundTimer.setInterval(() => {
          this.bluetooth
            .sendMessage({ type: MessageType.PING, data: null })
            .catch(console.error)
        }, Config.PING_INTERVAL_MS)
      } catch (error) {
        console.error(`Cannot set ping message interval. Error: ${error}`)
      }

      broadcastLightnessUpdate(this.deviceSettings.get('lightness'))

      this.gps
        .startObservingLocation(this.deviceSettings.getSettings())
        .catch(console.error)
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

    this.deviceSettings.on('change', (settings, key) => {
      if (!this.bluetooth.isConnectedToDevice()) {
        return
      }

      if (!key || key === 'lightness') {
        broadcastLightnessUpdate(settings.lightness)
      }
      if (!key || key === 'mapZoom') {
        this.mapProvider.setZoom(settings.mapZoom)
      }

      //TODO: gps.startObservingLocation(deviceSettings.getSettings()) when location settings change
    })

    this.gps.on('locationUpdate', this.handleLocationUpdate.bind(this))
    this.mapProvider.on('tileLoaded', async (tile) => {
      //TODO: remove
      console.log(
        'Tile loaded',
        tile.x,
        tile.y,
        tile.z,
        tile.image.width,
        tile.image.height,
        tile.image.data.buffer.byteLength,
        tile.image.depth,
        tile.image.channels,
        tile.image.palette?.length,
      )

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
    )

    if (!this.bluetooth.isConnectedToDevice()) {
      console.warn('No device connected. Stopping location updates.')
      await this.gps.stopObservingLocation()
      return
    }

    this.bluetooth
      .sendMessage({
        type: MessageType.LOCATION_UPDATE,
        data: locationState,
      })
      .catch(console.error)

    this.mapProvider.update(locationState.latitude, locationState.longitude)
  }

  private async sendMapTile(tile: Tile) {
    if (this.sendingMapTile) {
      this.tilesToSendQueue.push(tile)
      return
    }

    this.sendingMapTile = true

    try {
      const tileIdentifier = pick(tile, 'x', 'y', 'z')

      const palette = tile.image.palette ?? []

      await this.bluetooth.sendMessage({
        type: MessageType.SEND_MAP_TILE_START,
        data: {
          tileIdentifier,
          tileWidth: tile.image.width,
          tileHeight: tile.image.height,
          dataByteLength: tile.image.data.buffer.byteLength,
          paletteSize: palette.length,
        },
      })

      const colorsBatchSize = 48

      for (let i = 0; i < palette.length; i += colorsBatchSize) {
        const data = []

        for (let j = 0; j < colorsBatchSize; j++) {
          const color = palette[i + j]
          if (!color) {
            break
          }
          data.push({
            colorIndex: i + j,
            red: color[0],
            green: color[1],
            blue: color[2],
          })
        }

        await this.bluetooth.sendMessage({
          type: MessageType.SEND_MAP_TILE_INDEXED_COLORS_BATCH_48,
          data,
        })
      }

      const chunkSize = 224 //224 (max message length) - 1 byte for message type - 2 bytes for chunk index
      const chunksCount = Math.ceil(
        tile.image.data.buffer.byteLength / chunkSize,
      )

      console.info(
        'Sending map data sequentialy:',
        tile.x,
        tile.y,
        tile.z,
        'chunks:',
        chunksCount,
      )

      for (let c = 0; c < chunksCount; c++) {
        const start = c * chunkSize
        const end = start + chunkSize
        const bytes = tile.image.data.buffer.slice(start, end)

        try {
          //TODO: send with lower priority
          await this.bluetooth.sendMessage({
            type: MessageType.SEND_MAP_TILE_DATA_CHUNK,
            data: { chunkIndex: c, bytes },
          })
        } catch (error) {
          console.error('Failed to send map tile chunk:', c)
        }
      }

      await this.bluetooth.sendMessage({
        type: MessageType.SEND_MAP_TILE_END,
        data: { tileIdentifier },
      })
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
}
