import { useState } from 'react'
import { LocationAccuracy } from 'expo-location'
import { ScrollView, StyleSheet, View } from 'react-native'
import { Divider, RadioButton, Text, TextInput } from 'react-native-paper'
import { commonStyles } from '../common/styles'
import { removeNonNumericCharacters } from '../common/utils'
import { useCore } from '../context/coreContext'
import { useCoreEvent } from '../hooks/useCoreEvent'

export const LocationOptions = () => {
  const { deviceSettings } = useCore()

  const [gpsAccuracy, setGpsAccuracy] = useState(
    deviceSettings.get('gpsAccuracy'),
  )
  const [gpsTimeInterval, setGpsTimeInterval] = useState(
    deviceSettings.get('gpsTimeInterval'),
  )
  const [gpsDistanceSensitivity, setGpsDistanceSensitivity] = useState(
    deviceSettings.get('gpsDistanceSensitivity'),
  )

  useCoreEvent(deviceSettings, 'change', (settings, key) => {
    switch (key) {
      case 'gpsAccuracy':
        setGpsAccuracy(settings.gpsAccuracy)
        break
      case 'gpsTimeInterval':
        setGpsTimeInterval(settings.gpsTimeInterval)
        break
      case 'gpsDistanceSensitivity':
        setGpsDistanceSensitivity(settings.gpsDistanceSensitivity)
        break
    }
  })

  return (
    <ScrollView
      style={commonStyles.scrollView}
      contentContainerStyle={commonStyles.scrollViewContent}
    >
      <TextInput
        mode="outlined"
        label="GPS location updates interval (milliseconds)"
        value={gpsTimeInterval.toString()}
        right={<TextInput.Affix text="milliseconds" />}
        left={<TextInput.Icon icon="map-clock" />}
        maxLength={9}
        keyboardType="numeric"
        onChangeText={(value) => {
          const parsedValue = parseInt(removeNonNumericCharacters(value), 10)
          if (isNaN(parsedValue) || parsedValue < 1) return
          deviceSettings.set('gpsTimeInterval', parsedValue)
        }}
      />

      <Divider />
      <TextInput
        mode="outlined"
        label="GPS distance sensitivity (meters)"
        value={gpsDistanceSensitivity.toString()}
        right={<TextInput.Affix text="meters" />}
        left={<TextInput.Icon icon="map-marker-distance" />}
        maxLength={3}
        keyboardType="numeric"
        onChangeText={(value) => {
          const parsedValue = parseInt(removeNonNumericCharacters(value), 10)
          if (isNaN(parsedValue) || parsedValue < 0) return
          deviceSettings.set('gpsDistanceSensitivity', parsedValue)
        }}
      />

      <Divider />
      <Text variant="bodyLarge">GPS accuracy</Text>
      {accuracies.map((accuracy) => (
        <View key={accuracy.value} style={styles.gpsAccuracyRadioRow}>
          <RadioButton
            key={accuracy.value}
            value={accuracy.value.toString()}
            status={gpsAccuracy === accuracy.value ? 'checked' : 'unchecked'}
            onPress={() => deviceSettings.set('gpsAccuracy', accuracy.value)}
          />
          <Text
            onPress={() => deviceSettings.set('gpsAccuracy', accuracy.value)}
          >
            {accuracy.label}
          </Text>
        </View>
      ))}
    </ScrollView>
  )
}

const accuracies = [
  { label: 'BestForNavigation', value: LocationAccuracy.BestForNavigation },
  { label: 'Highest', value: LocationAccuracy.Highest },
  { label: 'High', value: LocationAccuracy.High },
  { label: 'Balanced', value: LocationAccuracy.Balanced },
  { label: 'Low', value: LocationAccuracy.Low },
  { label: 'Lowest', value: LocationAccuracy.Lowest },
]

const styles = StyleSheet.create({
  gpsAccuracyRadioRow: {
    display: 'flex',
    flexDirection: 'row',
    alignItems: 'center',
  },
})
