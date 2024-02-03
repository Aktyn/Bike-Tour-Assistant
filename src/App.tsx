import React, { useEffect, useState } from 'react'
import { registerRootComponent } from 'expo'
import { StatusBar } from 'expo-status-bar'
import { blueGrey } from 'material-ui-colors'
import { StyleSheet, useColorScheme } from 'react-native'
import { PaperProvider } from 'react-native-paper'
import { SafeAreaProvider, SafeAreaView } from 'react-native-safe-area-context'
import { CoreContext } from './context/coreContext'
import { Core } from './core/core'
import { darkTheme } from './themes/darkTheme'
import { ViewRouter } from './views/ViewRouter'

export default function App() {
  const colorScheme = useColorScheme() as 'light' | 'dark'

  const [core, setCore] = useState<Core | null>(null)

  useEffect(() => {
    const coreInstance = new Core()
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
      <PaperProvider theme={darkTheme}>
        <SafeAreaProvider>
          <SafeAreaView style={styles.container}>
            <ViewRouter />
            <StatusBar style={colorScheme === 'dark' ? 'light' : 'dark'} />
          </SafeAreaView>
        </SafeAreaProvider>
      </PaperProvider>
    </CoreContext.Provider>
  )
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: blueGrey[700],
    alignItems: 'stretch',
    justifyContent: 'flex-start',
  },
})

registerRootComponent(App)
