import EventEmitter from 'events'
import { BleManager, State, type Device } from 'react-native-ble-plx'
import { requestBluetoothPermission } from './common'

declare interface BluetoothEventEmitter {
  on(event: 'stateChange', listener: (isEnabled: boolean) => void): this
  off(event: 'stateChange', listener: (isEnabled: boolean) => void): this
  emit(event: 'stateChange', isEnabled: boolean): boolean

  on(event: 'isScanning', listener: (isScanning: boolean) => void): this
  off(event: 'isScanning', listener: (isScanning: boolean) => void): this
  emit(event: 'isScanning', isScanning: boolean): boolean
}

class BluetoothEventEmitter extends EventEmitter {}

export class Bluetooth extends BluetoothEventEmitter {
  private bleManager: BleManager | null = null
  private scannedDevices: Device[] = []
  private state = State.Unknown

  constructor() {
    super()
  }

  destroy() {
    console.info('Destroying bluetooth core...')
    this.bleManager?.destroy()
  }

  async init() {
    const granted = await requestBluetoothPermission()
    if (!granted) {
      console.error('Bluetooth permission not granted')
      return
    }

    this.bleManager = new BleManager()
    this.state = await this.bleManager.state()
    this.emit('stateChange', this.state === State.PoweredOn)

    this.bleManager.onStateChange((state) => {
      this.state = state
      this.emit('stateChange', state === State.PoweredOn)
    })

    console.info('Bluetooth module initialized')
  }

  isEnabled() {
    return this.state === State.PoweredOn
  }

  async enable() {
    try {
      await this.bleManager!.enable()
    } catch (error) {
      return false
    }
    return true
  }

  startScanning() {
    if (!this.bleManager) {
      throw new Error('Bluetooth not initialized')
    }

    this.scannedDevices = []
    this.emit('isScanning', true)
    this.bleManager.startDeviceScan(null, null, (error, scannedDevice) => {
      if (error) {
        throw new Error(error.message)
      }

      if (
        scannedDevice &&
        !this.scannedDevices.some((device) => device.id === scannedDevice.id)
      ) {
        console.info('Scanned device:', scannedDevice.id, scannedDevice.name)
        this.scannedDevices.push(scannedDevice)
      }
    })
  }

  stopScanning() {
    if (!this.bleManager) {
      throw new Error('Bluetooth not initialized')
    }

    this.emit('isScanning', false)
    this.bleManager?.stopDeviceScan()
  }
}
