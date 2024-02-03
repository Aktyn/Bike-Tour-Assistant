import { useEffect } from 'react'
import { View } from 'react-native'
import { Text } from 'react-native-paper'
import { useCore } from '../context/coreContext'

export const Main = () => {
  const { bluetooth } = useCore()

  useEffect(() => {
    //TODO
    bluetooth.sendMessage().catch(console.error)
  }, [bluetooth])

  return (
    <View>
      <Text variant="titleMedium">Main view</Text>
    </View>
  )
}
