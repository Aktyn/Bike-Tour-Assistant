import { useMemo, useState } from 'react'
import { ScrollView, StyleSheet, View } from 'react-native'
import { LeafletView, type MapMarker } from 'react-native-leaflet-view'
import {
  Button,
  Divider,
  IconButton,
  List,
  Text,
  TextInput,
  useTheme,
} from 'react-native-paper'
import { useCore } from '../context/coreContext'
import type { LocationState } from '../core/gps'
import { useCoreEvent } from '../hooks/useCoreEvent'

type Point = Pick<LocationState, 'latitude' | 'longitude'>

export const PointsOfInterest = () => {
  const theme = useTheme()
  const { deviceSettings, gps } = useCore()

  const [coordinatesInput, setCoordinatesInput] = useState('')
  const [points, setPoints] = useState<Point[]>(
    deviceSettings.get('pointsOfInterest'),
  )
  const [expandPointsList, setExpandPointsList] = useState(true)

  const parsedPoint = parseCoordinates(coordinatesInput)

  useCoreEvent(deviceSettings, 'change', (settings, key) => {
    switch (key) {
      case 'pointsOfInterest':
        setPoints(settings.pointsOfInterest)
        break
    }
  })

  useCoreEvent(gps, 'locationUpdate', (location) => {
    console.log(location) //TODO: use to set initial map position
  })

  const markers = useMemo<MapMarker[]>(() => {
    return points.map((point) => ({
      position: { lat: point.latitude, lng: point.longitude },
      icon: '🎯',
      size: [24, 24],
      iconAnchor: [12, 12],
    }))
  }, [points])

  return (
    <View style={styles.container}>
      <View
        style={{
          paddingHorizontal: 24,
          rowGap: 16,
          flexGrow: 0,
          flexShrink: 1,
        }}
      >
        <TextInput
          mode="outlined"
          label="Coordinates"
          placeholder="Latitude, Longitude"
          value={coordinatesInput}
          left={<TextInput.Icon icon="crosshairs-gps" />}
          keyboardType="numeric"
          onChangeText={setCoordinatesInput}
        />
        <Button
          dark
          mode="contained"
          icon="plus"
          disabled={!parsedPoint}
          onPress={() => {
            if (
              parsedPoint &&
              !points.some(
                ({ latitude, longitude }) =>
                  parsedPoint.latitude === latitude &&
                  parsedPoint.longitude === longitude,
              )
            ) {
              deviceSettings.set('pointsOfInterest', [...points, parsedPoint])
              setCoordinatesInput('')
            }
          }}
        >
          Add point
        </Button>
      </View>
      <ScrollView style={{ maxHeight: '50%', flexGrow: 0, marginTop: 16 }}>
        {points.length > 0 ? (
          <List.Accordion
            title="Points"
            expanded={expandPointsList}
            onPress={() => setExpandPointsList(!expandPointsList)}
            // eslint-disable-next-line react/no-unstable-nested-components
            left={(props) => <List.Icon {...props} icon="view-list" />}
          >
            {points.map((point) => (
              <List.Item
                key={`${point.latitude},${point.longitude}`}
                title={`Latitude: ${point.latitude.toFixed(4)}, Longitude: ${point.longitude.toFixed(4)}`}
                // eslint-disable-next-line react/no-unstable-nested-components
                right={(props) => (
                  <IconButton
                    {...props}
                    icon="delete"
                    iconColor={theme.colors.onSurface}
                    size={24}
                    onPress={() =>
                      deviceSettings.set(
                        'pointsOfInterest',
                        points.filter((p) => p !== point),
                      )
                    }
                  />
                )}
              />
            ))}
          </List.Accordion>
        ) : (
          <Text variant="bodyLarge" style={{ textAlign: 'center' }}>
            No points specified
          </Text>
        )}
      </ScrollView>
      <Divider />
      <View style={styles.mapContainer}>
        <LeafletView
          key="leaflet-view"
          doDebug={false}
          androidHardwareAccelerationDisabled={false}
          mapMarkers={markers}
          // mapShapes={tourShapes}
          // mapCenterPosition={latLng} //TODO: use first location update
          // zoom={zoom}
          // onMessageReceived={handleLeafletViewUpdate}
        />
      </View>
    </View>
  )
}

function parseCoordinates(input: string): Point | null {
  const [latitude, longitude] = input.replace(/\s/g, '').split(',')
  const parsedLatitude = parseFloat(latitude)
  const parsedLongitude = parseFloat(longitude)
  if (isNaN(parsedLatitude) || isNaN(parsedLongitude)) {
    return null
  }
  return { latitude: parsedLatitude, longitude: parsedLongitude }
}

export const styles = StyleSheet.create({
  container: {
    width: '100%',
    height: '100%',
    display: 'flex',
    flexDirection: 'column',
    justifyContent: 'space-between',
    alignItems: 'stretch',
    paddingTop: 16,
  },
  mapContainer: {
    width: '100%',
    flexGrow: 1,
    backgroundColor: '#afc4',
  },
})
