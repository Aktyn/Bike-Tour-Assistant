import { useCallback, useState } from 'react'
import Slider from '@react-native-community/slider'
import { ScrollView, StyleSheet, View } from 'react-native'
import { Button, Text, useTheme } from 'react-native-paper'
import { useCore } from '../context/coreContext'
import { MessageType } from '../core/message'
import { useCoreEvent } from '../hooks/useCoreEvent'

export const Main = () => {
  const theme = useTheme()
  const { deviceSettings, bluetooth } = useCore()

  const [lightness, setLightness] = useState(deviceSettings.get('lightness'))

  useCoreEvent(deviceSettings, 'change', (settings, key) => {
    if (key === 'lightness') {
      setLightness(settings.lightness)
    }
  })

  const handleLightnessChange = useCallback(
    (value: number) => {
      deviceSettings.set('lightness', value)
    },
    [deviceSettings],
  )

  return (
    <View style={styles.container}>
      <Text variant="titleLarge">Bike Tour Assistant</Text>
      <ScrollView
        style={styles.scrollView}
        contentContainerStyle={styles.scrollViewContent}
      >
        <Text>Display lightness: {Math.floor(lightness)}%</Text>
        <Slider
          style={{ width: '100%', height: 40 }}
          minimumValue={0}
          maximumValue={100}
          value={lightness}
          onValueChange={handleLightnessChange}
        />
        <Text>TODO: loading tour file</Text>
        <Button
          dark
          mode="contained"
          icon="camera"
          onPress={() =>
            bluetooth
              .sendMessage({ type: MessageType.TAKE_PHOTO, data: null })
              .catch(console.error)
          }
        >
          Take photo
        </Button>
      </ScrollView>
      <Button
        dark
        mode="contained"
        icon="bluetooth-off"
        onPress={() => bluetooth.disconnectFromDevice().catch(console.error)}
      >
        Disconnect
      </Button>
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
  scrollViewContent: {
    rowGap: 16,
  },
  footer: {
    paddingHorizontal: 24,
    width: '100%',
    textAlign: 'right',
  },
})
