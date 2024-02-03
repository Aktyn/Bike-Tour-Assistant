import { useCallback, useState } from 'react'
import { DeviceConnection } from './DeviceConnection'
import { Main } from './Main'
import { useCore } from '../context/coreContext'
import { useCoreEvent } from '../hooks/useCoreEvent'

enum VIEW {
  DEVICE_CONNECTION,
  MAIN,
}

export const ViewRouter = () => {
  const { bluetooth } = useCore()

  const [view, setView] = useState(VIEW.DEVICE_CONNECTION)

  const handleDeviceConnected = useCallback(() => setView(VIEW.MAIN), [])
  const handleDeviceDisconnected = useCallback(
    () => setView(VIEW.DEVICE_CONNECTION),
    [],
  )

  useCoreEvent(bluetooth, 'deviceConnected', handleDeviceConnected)
  useCoreEvent(bluetooth, 'deviceDisconnected', handleDeviceDisconnected)

  switch (view) {
    case VIEW.DEVICE_CONNECTION:
      return <DeviceConnection />
    case VIEW.MAIN:
      return <Main />
  }

  return null
}
