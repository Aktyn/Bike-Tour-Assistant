import BackgroundTimer, { type TimeoutId } from 'react-native-background-timer'
import { Bluetooth } from './bluetooth'
import { DeviceSettings } from './deviceSettings'
import { MessageType } from './message'
import { Config } from '../config'
import { debounce } from '../utils'

export class Core {
  public readonly deviceSettings = new DeviceSettings()
  public readonly bluetooth = new Bluetooth()

  private pingMessageInterval: TimeoutId | null = null

  constructor() {
    console.info('Initializing core...')

    Promise.all([this.deviceSettings.init(), this.bluetooth.init()])
      .then(() => this.postInit())
      .catch(console.error)
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
    })

    this.deviceSettings.on('change', (settings, key) => {
      if (!this.bluetooth.isConnectedToDevice()) {
        return
      }

      if (!key || key === 'lightness') {
        broadcastLightnessUpdate(settings.lightness)
      }
    })
  }

  destroy() {
    console.info('Destroying core...')

    this.deviceSettings.destroy()
    this.bluetooth.destroy()
  }
}
