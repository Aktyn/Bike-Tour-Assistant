import { useEffect, useState } from 'react'
import Slider from '@react-native-community/slider'
import { ScrollView, StyleSheet, View } from 'react-native'
import { Text, useTheme } from 'react-native-paper'
import { useCore } from '../context/coreContext'
import { MessageType } from '../core/message'
import { useDebounce } from '../hooks/useDebounce'

export const Main = () => {
  const theme = useTheme()
  const { bluetooth } = useCore()

  //TODO: store some settings locally
  const [lightness, setLightness] = useState(100)

  const synchronizeLightness = useDebounce(
    (value: number) => {
      bluetooth
        .sendMessage({
          type: MessageType.SET_LIGHTNESS,
          data: { lightness: value },
        })
        .catch(console.error)
    },
    200,
    [bluetooth],
  )

  useEffect(() => {
    synchronizeLightness(lightness)
  }, [lightness, synchronizeLightness])

  return (
    <View style={styles.container}>
      <Text variant="titleLarge">Bike Tour Assistant</Text>
      <ScrollView style={styles.scrollView}>
        <Text>Display lightness: {Math.round(lightness)}%</Text>
        <Slider
          style={{ width: '100%', height: 40 }}
          minimumValue={0}
          maximumValue={100}
          value={lightness}
          onValueChange={setLightness}
        />
      </ScrollView>
      <Text
        variant="labelSmall"
        style={{ ...styles.footer, color: theme.colors.onSurfaceVariant }}
      >
        Made by Aktyn
      </Text>
    </View>
  )
}

const styles = StyleSheet.create({
  container: {
    paddingVertical: 16,
    alignItems: 'center',
    justifyContent: 'space-between',
    rowGap: 16,
    flex: 1,
  },
  scrollView: {
    width: '100%',
    paddingHorizontal: 24,
  },
  footer: {
    paddingHorizontal: 24,
    width: '100%',
    textAlign: 'right',
  },
})
