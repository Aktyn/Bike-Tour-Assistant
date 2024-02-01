import { BleManager } from 'react-native-ble-plx'
import { requestBluetoothPermission } from './common'

export class Bluetooth {
  private bleManager: BleManager | null = null

  constructor() {
    console.log('test')
  }

  destroy() {
    console.log('Destroying bluetooth core...')
    this.bleManager?.destroy()
  }

  async init() {
    console.log('hm')
    const granted = await requestBluetoothPermission()
    if (!granted) {
      console.error('Bluetooth permission not granted')
      return
    } else {
      console.log('Bluetooth permission granted')
    }
    console.log('Bluetooth module initializing...')
    this.bleManager = new BleManager()
    console.log('Bluetooth module initialized', this.bleManager)
  }
}
