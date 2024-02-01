import { useEffect, useState } from 'react'
import { registerRootComponent } from 'expo'
import { StatusBar } from 'expo-status-bar'
import { StyleSheet, View } from 'react-native'
import { CoreContext } from './context/coreContext'
import { Core } from './core/core'
import { DeviceConnection } from './views/DeviceConnection'

export default function App() {
  const [core, setCore] = useState<Core | null>(null)

  useEffect(() => {
    const coreInstance = new Core()
    coreInstance.bluetooth.init().catch(console.error)

    setCore(coreInstance)

    return () => {
      coreInstance.destroy()
    }
  }, [])

  if (!core) {
    return null
  }

  return (
    <CoreContext.Provider value={core}>
      <View style={styles.container}>
        <StatusBar style="auto" />
        <DeviceConnection />
      </View>
    </CoreContext.Provider>
  )
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#006064',
    alignItems: 'stretch',
    justifyContent: 'center',
  },
})

registerRootComponent(App)
