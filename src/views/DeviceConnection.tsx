import { useState } from 'react'
import { Button, SafeAreaView, StyleSheet, Text, View } from 'react-native'
import { useCore } from '../context/coreContext'
import { useCoreEvent } from '../hooks/useCoreEvent'

export const DeviceConnection = () => {
  const { bluetooth } = useCore()

  const [isEnabled, setIsEnabled] = useState(bluetooth?.isEnabled())
  const [isScanning, setIsScanning] = useState(false)

  useCoreEvent(bluetooth, 'stateChange', setIsEnabled)
  useCoreEvent(bluetooth, 'isScanning', setIsScanning)

  return (
    <SafeAreaView style={styles.container}>
      {isEnabled ? (
        <View>
          <Text>Bluetooth is enabled</Text>
          {isScanning && <Text>Scanning...</Text>}
          <Button
            title={isScanning ? 'Stop scanning' : 'Scan for devices'}
            onPress={() =>
              isScanning ? bluetooth.stopScanning() : bluetooth.startScanning()
            }
          />
        </View>
      ) : (
        <Button title="Enable Bluetooth" onPress={() => bluetooth.enable()} />
      )}
    </SafeAreaView>
  )
}

const styles = StyleSheet.create({
  container: {
    alignItems: 'center',
    justifyContent: 'center',
  },
})
