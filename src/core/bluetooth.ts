import EventEmitter from 'events'
import { BleManager, State, type Device } from 'react-native-ble-plx'
import { requestBluetoothPermission } from './common'
import { Config } from '../config'

declare interface BluetoothEventEmitter {
  on(event: 'deviceDiscovered', listener: (device: Device) => void): this
  off(event: 'deviceDiscovered', listener: (device: Device) => void): this
  emit(event: 'deviceDiscovered', device: Device): boolean

  on(event: 'deviceConnected', listener: (device: Device) => void): this
  off(event: 'deviceConnected', listener: (device: Device) => void): this
  emit(event: 'deviceConnected', device: Device): boolean

  on(event: 'deviceDisconnected', listener: () => void): this
  off(event: 'deviceDisconnected', listener: () => void): this
  emit(event: 'deviceDisconnected'): boolean

  on(event: 'stateChange', listener: (isEnabled: boolean) => void): this
  off(event: 'stateChange', listener: (isEnabled: boolean) => void): this
  emit(event: 'stateChange', isEnabled: boolean): boolean

  on(
    event: 'connectingToTargetDevice',
    listener: (connecting: boolean) => void,
  ): this
  off(
    event: 'connectingToTargetDevice',
    listener: (connecting: boolean) => void,
  ): this
  emit(event: 'connectingToTargetDevice', connecting: boolean): boolean

  on(event: 'isScanning', listener: (isScanning: boolean) => void): this
  off(event: 'isScanning', listener: (isScanning: boolean) => void): this
  emit(event: 'isScanning', isScanning: boolean): boolean
}

class BluetoothEventEmitter extends EventEmitter {}

export class Bluetooth extends BluetoothEventEmitter {
  private bleManager: BleManager | null = null
  private state = State.Unknown
  private scannedDevices: Device[] = []
  private connectedDevice: Device | null = null

  private scanning = false
  private connectingToTargetDevice = false

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
    this.scannedDevices = []
    this.connectedDevice = null
    this.state = await this.bleManager.state()
    this.emit('stateChange', this.state === State.PoweredOn)

    this.bleManager.onStateChange((state) => {
      this.state = state
      this.emit('stateChange', state === State.PoweredOn)
    })

    this.startScanning()

    console.info('Bluetooth module initialized')
  }

  isEnabled() {
    return this.state === State.PoweredOn
  }

  isScanning() {
    return this.scanning
  }

  isConnectingToTargetDevice() {
    return this.connectingToTargetDevice
  }

  getDiscoveredDevices() {
    return this.scannedDevices
  }

  async enable() {
    try {
      this.scannedDevices = []
      this.stopScanning()
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
    this.scanning = true
    this.emit('isScanning', true)
    this.bleManager.startDeviceScan(null, null, (error, scannedDevice) => {
      if (error) {
        throw new Error(error.message)
      }

      if (
        scannedDevice &&
        scannedDevice.name &&
        !this.scannedDevices.some((device) => device.id === scannedDevice.id)
      ) {
        console.info('Scanned device:', scannedDevice.id, scannedDevice.name)
        this.scannedDevices.push(scannedDevice)
        this.emit('deviceDiscovered', scannedDevice)

        if (scannedDevice.name === Config.TARGET_DEVICE_NAME) {
          this.connectToTargetDevice()
        }
      }
    })
  }

  stopScanning() {
    if (!this.bleManager) {
      throw new Error('Bluetooth not initialized')
    }

    this.scanning = false
    this.emit('isScanning', false)
    this.bleManager.stopDeviceScan()
  }

  async connectToTargetDevice() {
    const targetDevice = this.scannedDevices.find(
      (device) => device.name === Config.TARGET_DEVICE_NAME,
    )
    if (!targetDevice) {
      throw new Error('Target device not found')
    }

    this.stopScanning()

    this.connectingToTargetDevice = true
    this.emit('connectingToTargetDevice', true)

    console.info(
      'Connecting to target device:',
      targetDevice.id,
      targetDevice.name,
    )
    try {
      const device = await targetDevice
        .connect({
          autoConnect: false,
          timeout: 20_000,
        })
        .then((d) => d.discoverAllServicesAndCharacteristics())
      this.connectedDevice = device
      const subscription = this.connectedDevice.onDisconnected(() => {
        console.info('Disconnected from target device:', device.id, device.name)
        this.scannedDevices = []
        this.connectedDevice = null
        this.emit('deviceDisconnected')
        this.connectingToTargetDevice = false
        this.emit('connectingToTargetDevice', false)
        subscription.remove()
      })
      this.emit('deviceConnected', device)
      console.info('Connected to target device:', device.id, device.name)
    } catch (error) {
      console.error(
        'Failed to connect to target device:',
        error instanceof Error ? error.message : error,
      )
    }

    this.connectingToTargetDevice = false
    this.emit('connectingToTargetDevice', false)
  }

  //TODO: handle errors and develop generic data sending logic
  async sendMessage() {
    if (!this.connectedDevice) {
      throw new Error('Device not connected')
    }
    if (!(await this.connectedDevice.isConnected())) {
      this.connectedDevice = null
      this.emit('deviceDisconnected')
      throw new Error('Device not connected')
    }

    const services = await this.connectedDevice.services()
    const targetService = services.at(-1)
    if (!targetService) {
      throw new Error('No services found')
    }

    const characteristics =
      await this.connectedDevice.characteristicsForService(targetService.uuid)
    const writeableCharacteristic = characteristics.find(
      (char) => char.isWritableWithoutResponse,
    )
    if (!writeableCharacteristic) {
      throw new Error('No characteristics found')
    }

    await this.connectedDevice.writeCharacteristicWithoutResponseForService(
      writeableCharacteristic.serviceUUID,
      writeableCharacteristic.uuid,
      'aGVsbG8gbWlzcyB0YXBweQ==',
    )
  }
}
