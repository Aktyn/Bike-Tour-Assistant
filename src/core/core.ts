import { Bluetooth } from './bluetooth'
import { DeviceSettings } from './deviceSettings'
import { MessageType } from './message'
import { Config } from '../config'

export class Core {
  public readonly deviceSettings = new DeviceSettings()
  public readonly bluetooth = new Bluetooth()

  private pingMessageInterval: NodeJS.Timeout | null = null

  constructor() {
    console.info('Initializing core...')

    Promise.all([this.deviceSettings.init(), this.bluetooth.init()])
      .then(() => this.postInit())
      .catch(console.error)
  }

  private postInit() {
    //TODO: debounce this function
    const broadcastLightnessUpdate = (value: number) =>
      this.bluetooth
        .sendMessage({
          type: MessageType.SET_LIGHTNESS,
          data: { lightness: value },
        })
        .catch(console.error)

    this.bluetooth.on('deviceConnected', () => {
      this.pingMessageInterval = setInterval(() => {
        this.bluetooth
          .sendMessage({ type: MessageType.PING, data: null })
          .catch(console.error)
      }, Config.PING_INTERVAL_MS)

      broadcastLightnessUpdate(this.deviceSettings.get('lightness'))
    })

    this.bluetooth.on('deviceDisconnected', () => {
      if (this.pingMessageInterval) {
        clearInterval(this.pingMessageInterval)
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
