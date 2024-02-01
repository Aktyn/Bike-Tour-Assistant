import { PermissionsAndroid, Platform } from 'react-native'

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
