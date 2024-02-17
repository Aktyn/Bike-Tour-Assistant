import EventEmitter from 'events'
import AsyncStorage from '@react-native-async-storage/async-storage'
import type { DocumentPickerSuccessResult } from 'expo-document-picker'
import { tryParseJSON } from '../utils'

const defaultSettings = {
  lightness: 100,
  gpxFile: null as null | DocumentPickerSuccessResult,
  // mapZoom: 16, //TODO
  // gpsAccuracy: LocationAccuracy.Highest, //TODO
  /** Minimum time to wait between each update in milliseconds. Default value may depend on accuracy option. */
  // gpsTimeInterval: 4000, //TODO
  /** Receive updates only when the location has changed by at least this distance in meters. Default value may depend on accuracy option. */
  // gpsDistanceSensitivity: 10, //TODO
}

export type DeviceSettingsSchema = typeof defaultSettings

declare interface DeviceSettingsEventEmitter {
  on(
    event: 'change',
    listener: (
      settings: DeviceSettingsSchema,
      key?: keyof DeviceSettingsSchema,
    ) => void,
  ): this
  off(
    event: 'change',
    listener: (
      settings: DeviceSettingsSchema,
      key?: keyof DeviceSettingsSchema,
    ) => void,
  ): this
  emit(
    event: 'change',
    settings: DeviceSettingsSchema,
    key?: keyof DeviceSettingsSchema,
  ): boolean
}

class DeviceSettingsEventEmitter extends EventEmitter {}

export class DeviceSettings extends DeviceSettingsEventEmitter {
  private settingsStore = defaultSettings

  constructor() {
    super()
  }

  destroy() {
    console.info('Destroying device settings core...')
    super.removeAllListeners()
  }

  async init() {
    AsyncStorage.getItem('@settings')
      .then((settingsString) => {
        this.settingsStore = {
          ...this.settingsStore,
          ...tryParseJSON(settingsString ?? '{}', {}),
        }
        this.emit('change', this.settingsStore)
      })
      .catch((error) => {
        console.error(
          `Cannot read settings: ${
            error instanceof Error ? error.message : String(error)
          }`,
        )
      })
  }

  get<Key extends keyof DeviceSettingsSchema>(
    key: Key,
  ): DeviceSettingsSchema[Key] {
    return this.settingsStore[key]
  }

  set<Key extends keyof DeviceSettingsSchema>(
    key: Key,
    value: DeviceSettingsSchema[Key],
  ) {
    console.info(`Setting "${key}" to "${value}"`)
    this.settingsStore[key] = value
    this.emit('change', this.settingsStore, key)

    AsyncStorage.setItem('@settings', JSON.stringify(this.settingsStore)).catch(
      (error) => {
        console.error(
          `Cannot update "${key}" setting: ${
            error instanceof Error ? error.message : String(error)
          }`,
        )
      },
    )
  }
}
