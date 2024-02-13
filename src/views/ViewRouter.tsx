import { useCallback, useEffect, useState } from 'react'
import { AppState } from 'react-native'
import DeviceInfo from 'react-native-device-info'
import { Debug } from './Debug'
import { DeviceConnection } from './DeviceConnection'
import { Main } from './Main'
import { useCore } from '../context/coreContext'
import { useCoreEvent } from '../hooks/useCoreEvent'

enum VIEW {
  DEVICE_CONNECTION,
  MAIN,
  DEBUG,
}

export const ViewRouter = () => {
  const { bluetooth } = useCore()

  const [view, setView] = useState(VIEW.DEVICE_CONNECTION)
  const [appState, setAppState] = useState(AppState.currentState)

  const handleDeviceConnected = useCallback(() => setView(VIEW.MAIN), [])
  const handleDeviceDisconnected = useCallback(
    () => setView(VIEW.DEVICE_CONNECTION),
    [],
  )

  useCoreEvent(bluetooth, 'deviceConnected', handleDeviceConnected)
  useCoreEvent(bluetooth, 'deviceDisconnected', handleDeviceDisconnected)

  useEffect(() => {
    const subscription = AppState.addEventListener('change', setAppState)
    return () => {
      subscription.remove()
    }
  }, [])

  useEffect(() => {
    if (view === VIEW.DEBUG) {
      return
    }
    DeviceInfo.isEmulator().then((emulator) => {
      if (emulator) {
        setView(VIEW.DEBUG)
      }
    })
  }, [view])

  useEffect(() => {
    console.info('AppState', appState)
  }, [appState])

  const isBackgroundState = !!appState.match(/inactive|background/)

  if (isBackgroundState) {
    return null
  }

  switch (view) {
    case VIEW.DEVICE_CONNECTION:
      return <DeviceConnection />
    case VIEW.MAIN:
      return <Main />
    case VIEW.DEBUG:
      return <Debug />
  }

  return null
}
