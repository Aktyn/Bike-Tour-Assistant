import { Bluetooth } from './bluetooth'
import { DeviceSettings } from './deviceSettings'
import { MessageType } from './message'

export class Core {
  public readonly deviceSettings = new DeviceSettings()
  public readonly bluetooth = new Bluetooth()

  constructor() {
    console.info('Initializing core...')

    this.deviceSettings.init().catch(console.error)
    Promise.all([this.bluetooth.init()])
      .then(() => this.postInit())
      .catch(console.error)
  }

  private postInit() {
    const broadcastLightnessUpdate = (value: number) =>
      this.bluetooth
        .sendMessage({
          type: MessageType.SET_LIGHTNESS,
          data: { lightness: value },
        })
        .catch(console.error)

    this.bluetooth.on('deviceConnected', () => {
      broadcastLightnessUpdate(this.deviceSettings.get('lightness'))
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

    this.bluetooth.destroy()
  }
}
