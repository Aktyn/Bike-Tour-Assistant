import { useCallback, useState } from 'react'
import Slider from '@react-native-community/slider'
import * as DocumentPicker from 'expo-document-picker'
import { LocationAccuracy } from 'expo-location'
import { ScrollView, StyleSheet, View } from 'react-native'
import {
  Button,
  Divider,
  IconButton,
  RadioButton,
  Text,
  TextInput,
  useTheme,
} from 'react-native-paper'
import { useCore } from '../context/coreContext'
import { MessageType } from '../core/message'
import { useCoreEvent } from '../hooks/useCoreEvent'

const minMapZoom = 2
const maxMapZoom = 19

export const Main = () => {
  const theme = useTheme()
  const { deviceSettings, bluetooth, gps } = useCore()

  const [lightness, setLightness] = useState(deviceSettings.get('lightness'))
  const [gpxFile, setGpxFile] = useState(deviceSettings.get('gpxFile'))
  const [mapZoom, setMapZoom] = useState(deviceSettings.get('mapZoom'))
  const [gpsAccuracy, setGpsAccuracy] = useState(
    deviceSettings.get('gpsAccuracy'),
  )
  const [gpsTimeInterval, setGpsTimeInterval] = useState(
    deviceSettings.get('gpsTimeInterval'),
  )
  const [gpsDistanceSensitivity, setGpsDistanceSensitivity] = useState(
    deviceSettings.get('gpsDistanceSensitivity'),
  )
  const [locationPermissionsGranted, setLocationPermissionsGranted] =
    useState(true)

  useCoreEvent(deviceSettings, 'change', (settings, key) => {
    switch (key) {
      case 'lightness':
        setLightness(settings.lightness)
        break
      case 'mapZoom':
        setMapZoom(settings.mapZoom)
        break
      case 'gpxFile':
        setGpxFile(settings.gpxFile)
        break
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

  useCoreEvent(gps, 'toggleGranted', setLocationPermissionsGranted)

  const selectTourFile = useCallback(() => {
    DocumentPicker.getDocumentAsync({
      type: ['application/gpx+xml', 'application/octet-stream'],
    })
      .then((data) => {
        if (data.canceled) {
          throw new Error('File selection canceled')
        }
        deviceSettings.set('gpxFile', data)
      })

      .catch((error) =>
        console.error(
          `Cannot load gpx file. Error: ${
            error instanceof Error ? error.message : String(error)
          }`,
        ),
      )
  }, [deviceSettings])

  return (
    <View style={styles.container}>
      <Text variant="titleLarge" style={{ fontWeight: 'bold' }}>
        Bike Tour Assistant
      </Text>
      <ScrollView
        style={styles.scrollView}
        contentContainerStyle={styles.scrollViewContent}
      >
        <Text variant="bodyLarge">
          Display lightness: {Math.floor(lightness)}%
        </Text>
        <Slider
          style={{ width: '100%', height: 40, marginTop: -24 }}
          step={1}
          minimumValue={0}
          maximumValue={100}
          value={lightness}
          onValueChange={(value) => deviceSettings.set('lightness', value)}
        />
        {/* TODO: switch to enable auto lightness (based on sunrise/sundawn time in current location) */}

        <Divider />
        {gpxFile ? (
          <View style={styles.horizontalView}>
            <Text variant="bodyLarge">{gpxFile.assets.at(0)?.name ?? '-'}</Text>
            <IconButton
              icon="delete"
              iconColor={theme.colors.onSurface}
              size={24}
              onPress={() => deviceSettings.set('gpxFile', null)}
            />
          </View>
        ) : (
          <Text variant="bodyLarge">No tour file selected</Text>
        )}
        <Button
          dark
          mode="contained"
          icon="map-marker-path"
          onPress={selectTourFile}
        >
          Select tour file
        </Button>

        <Divider />
        <View style={styles.horizontalView}>
          <Text variant="bodyLarge">Map zoom: {mapZoom}</Text>
          <View style={styles.horizontalView}>
            <IconButton
              icon="minus"
              iconColor={theme.colors.onSurface}
              size={24}
              disabled={mapZoom <= minMapZoom}
              onPress={() => deviceSettings.set('mapZoom', mapZoom - 1)}
            />
            <IconButton
              icon="plus"
              iconColor={theme.colors.onSurface}
              size={24}
              disabled={mapZoom >= maxMapZoom}
              onPress={() => deviceSettings.set('mapZoom', mapZoom + 1)}
            />
          </View>
        </View>
        <Slider
          style={{ width: '100%', height: 40, marginTop: -24 }}
          step={1}
          minimumValue={minMapZoom}
          maximumValue={maxMapZoom}
          value={mapZoom}
          onValueChange={(value) => deviceSettings.set('mapZoom', value)}
        />

        <Divider />
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

        <Divider />
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
        {!locationPermissionsGranted && (
          <>
            <Divider />
            <Text
              style={{
                color: theme.colors.onError,
                fontWeight: 'bold',
                textAlign: 'center',
              }}
              variant="bodyLarge"
            >
              Location permission has not been granted or GPS is disabled!!!
            </Text>
            <Button
              dark
              mode="contained"
              icon="restart"
              onPress={() =>
                gps
                  .startObservingLocation(deviceSettings.getSettings())
                  .catch(console.error)
              }
            >
              Restart observing location
            </Button>
          </>
        )}
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
  horizontalView: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
  },
  footer: {
    paddingHorizontal: 24,
    width: '100%',
    textAlign: 'right',
  },
  gpsAccuracyRadioRow: {
    display: 'flex',
    flexDirection: 'row',
    alignItems: 'center',
  },
})

const removeNonNumericCharacters = (value: string) =>
  value.replace(/[^\d,.]/g, '')

const accuracies = [
  { label: 'BestForNavigation', value: LocationAccuracy.BestForNavigation },
  { label: 'Highest', value: LocationAccuracy.Highest },
  { label: 'High', value: LocationAccuracy.High },
  { label: 'Balanced', value: LocationAccuracy.Balanced },
  { label: 'Low', value: LocationAccuracy.Low },
  { label: 'Lowest', value: LocationAccuracy.Lowest },
]
