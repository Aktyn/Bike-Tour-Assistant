import { useEffect } from 'react'
import { StyleSheet, View } from 'react-native'
import { Button, Text } from 'react-native-paper'
import { useCore } from '../context/coreContext'

export const Debug = () => {
  const { gps, deviceSettings } = useCore()

  useEffect(() => {
    gps.stopObservingLocation()
  }, [gps])

  return (
    <View style={styles.container}>
      <Text variant="titleLarge">Debug</Text>
      <Button
        dark
        mode="contained"
        icon="crosshairs-gps"
        onPress={() =>
          gps
            .startObservingLocation(deviceSettings.getSettings())
            .catch(console.error)
        }
      >
        Start GPS
      </Button>
    </View>
  )
}

const styles = StyleSheet.create({
  container: {
    alignItems: 'center',
    justifyContent: 'flex-start',
    rowGap: 16,
    flex: 1,
  },
})
