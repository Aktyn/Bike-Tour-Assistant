import {
  PermissionsAndroid,
  Platform,
  type Permission,
  type Rationale,
} from 'react-native'

export async function requestBluetoothPermission() {
  if (Platform.OS === 'ios') {
    return true
  }
  if (
    Platform.OS === 'android' &&
    PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION
  ) {
    const apiLevel = parseInt(Platform.Version.toString(), 10)

    if (apiLevel < 31) {
      if (
        await PermissionsAndroid.check(
          PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
        )
      ) {
        return true
      }
      const granted = await PermissionsAndroid.request(
        PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
      )
      return granted === PermissionsAndroid.RESULTS.GRANTED
    }
    if (
      PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN &&
      PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT
    ) {
      const permissionsList = [
        PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
        PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
        PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
      ]

      for (const permission of permissionsList) {
        if (await PermissionsAndroid.check(permission)) {
          continue
        }
        const status = await PermissionsAndroid.request(permission)
        if (status !== PermissionsAndroid.RESULTS.GRANTED) {
          console.error('Permission not granted', permission)
          return false
        }
      }

      return true
    }
  }

  console.error('Permission have not been granted')
  return false
}

async function requestPermission(
  permission: Permission,
  rationale: Partial<Rationale>,
) {
  try {
    const granted = await PermissionsAndroid.request(permission, {
      title: 'Permission',
      message: 'Permission is required',
      buttonNeutral: 'Ask Me Later',
      buttonNegative: 'Cancel',
      buttonPositive: 'OK',
      ...rationale,
    })
    if (granted === PermissionsAndroid.RESULTS.GRANTED) {
      return true
    } else {
      return false
    }
  } catch (err) {
    return false
  }
}

export function requestBackgroundLocationPermissions() {
  return requestPermission(
    PermissionsAndroid.PERMISSIONS.ACCESS_BACKGROUND_LOCATION,
    {
      title: 'Background location permission',
      message: 'Permission for tracking device location in background',
    },
  )
}
