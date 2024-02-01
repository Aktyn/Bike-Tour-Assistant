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
    console.log('apiLevel', apiLevel)

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

      console.log('test 2')

      for (const permission of permissionsList) {
        console.log('test 3', permission)
        if (await PermissionsAndroid.check(permission)) {
          continue
        }
        console.log('test 4', permission)
        const status = await PermissionsAndroid.request(permission)
        if (status !== PermissionsAndroid.RESULTS.GRANTED) {
          console.error('Permission not granted', permission)
          return false
        }
        console.log('test 5', permission)
      }

      return true

      // return (
      //   result['android.permission.BLUETOOTH_CONNECT'] ===
      //     PermissionsAndroid.RESULTS.GRANTED &&
      //   result['android.permission.BLUETOOTH_SCAN'] ===
      //     PermissionsAndroid.RESULTS.GRANTED &&
      //   result['android.permission.ACCESS_FINE_LOCATION'] ===
      //     PermissionsAndroid.RESULTS.GRANTED
      // )
    }
  }

  console.error('Permission have not been granted')
  // this.showErrorToast('Permission have not been granted')

  return false
}
