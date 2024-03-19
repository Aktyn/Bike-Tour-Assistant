import { useCallback, useState } from 'react'
import Slider from '@react-native-community/slider'
import * as DocumentPicker from 'expo-document-picker'
import { ScrollView, StyleSheet, View } from 'react-native'
import {
  Button,
  Divider,
  IconButton,
  Text,
  TextInput,
  useTheme,
} from 'react-native-paper'
import { commonStyles } from '../common/styles'
import { removeNonNumericCharacters } from '../common/utils'
import { useCore } from '../context/coreContext'
import { MessageType } from '../core/message'
import { useCoreEvent } from '../hooks/useCoreEvent'

const minMapZoom = 2
const maxMapZoom = 19

export const GeneralOptions = () => {
  const theme = useTheme()
  const { deviceSettings, bluetooth } = useCore()

  const [lightness, setLightness] = useState(deviceSettings.get('lightness'))
  const [distancePerPhoto, setDistancePerPhoto] = useState(
    deviceSettings.get('distancePerPhoto'),
  )
  const [gpxFile, setGpxFile] = useState(deviceSettings.get('gpxFile'))
  const [mapZoom, setMapZoom] = useState(deviceSettings.get('mapZoom'))

  useCoreEvent(deviceSettings, 'change', (settings, key) => {
    switch (key) {
      case 'lightness':
        setLightness(settings.lightness)
        break
      case 'distancePerPhoto':
        setDistancePerPhoto(settings.distancePerPhoto)
        break
      case 'mapZoom':
        setMapZoom(settings.mapZoom)
        break
      case 'gpxFile':
        setGpxFile(settings.gpxFile)
        break
    }
  })

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
    <ScrollView
      style={commonStyles.scrollView}
      contentContainerStyle={commonStyles.scrollViewContent}
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
      <TextInput
        mode="outlined"
        label="Distance per photo (meters)"
        value={distancePerPhoto.toString()}
        right={<TextInput.Affix text="meters" />}
        left={<TextInput.Icon icon="map-marker-distance" />}
        maxLength={4}
        keyboardType="numeric"
        onChangeText={(value) => {
          const parsedValue = parseInt(removeNonNumericCharacters(value), 10)
          if (isNaN(parsedValue) || parsedValue < 1) return
          deviceSettings.set('distancePerPhoto', parsedValue)
        }}
      />

      <Divider />
      {gpxFile ? (
        <View style={styles.horizontalView}>
          <Text variant="bodyLarge">{gpxFile.assets.at(0)?.name ?? '-'}</Text>
          <IconButton
            icon="delete"
            iconColor={theme.colors.onSurface}
            size={24}
            onPress={() => deviceSettings.set('gpxFile', null)}
            style={{ marginLeft: 'auto' }}
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
  )
}

const styles = StyleSheet.create({
  horizontalView: {
    alignSelf: 'stretch',
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
  },
})
