import { useCallback, useEffect, useMemo, useState } from 'react'
import { deepOrange } from 'material-ui-colors'
import { ScrollView, StyleSheet, View } from 'react-native'
import {
  AnimationType,
  LeafletView,
  type MapShape,
  type MapMarker,
  type WebviewLeafletMessage,
  MapShapeType,
} from 'react-native-leaflet-view'
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
import { parseGpxFile } from '../core/gpxParser'
import { useCoreEvent } from '../hooks/useCoreEvent'

type Point = Pick<LocationState, 'latitude' | 'longitude'>
type LatLng = { lat: number; lng: number }

export const PointsOfInterest = () => {
  const theme = useTheme()
  const { deviceSettings, gps } = useCore()

  const [coordinatesInput, setCoordinatesInput] = useState('')
  const [points, setPoints] = useState<Point[]>(
    deviceSettings.get('pointsOfInterest'),
  )
  const [expandPointsList, setExpandPointsList] = useState(true)
  const [mapCenterPosition, setMapCenterPosition] = useState<LatLng | null>(
    null,
  )
  const [zoom, setZoom] = useState(15)
  const [touchPoint, setTouchPoint] = useState<LatLng | null>(null)
  const [gpxFile, setGpxFile] = useState(deviceSettings.get('gpxFile'))
  const [tourShapes, setTourShapes] = useState<MapShape[]>([])

  const parsedPoint = parseCoordinates(coordinatesInput)

  useCoreEvent(deviceSettings, 'change', (settings, key) => {
    switch (key) {
      case 'pointsOfInterest':
        setPoints(settings.pointsOfInterest)
        break
      case 'gpxFile':
        setGpxFile(settings.gpxFile)
        break
    }
  })

  useCoreEvent(gps, 'locationUpdate', (location) => {
    setMapCenterPosition(
      (current) =>
        current ?? {
          lat: (location as unknown as LocationState).latitude,
          lng: (location as unknown as LocationState).longitude,
        },
    )
  })

  useEffect(() => {
    if (!gpxFile) {
      setTourShapes([])
      return
    }

    let mounted = true

    parseGpxFile(gpxFile.assets[0].uri)
      .then((tour) => {
        if (!mounted) {
          return
        }

        setTourShapes([
          {
            color: deepOrange[400],
            positions: tour
              .map((point) => ({
                lat: point.latitude,
                lng: point.longitude,
                index: point.index,
              }))
              .sort((a, b) => a.index - b.index),
            shapeType: MapShapeType.POLYLINE,
          },
        ])
      })
      .catch(console.error)

    return () => {
      mounted = false
    }
  }, [gpxFile])

  const markers = useMemo<MapMarker[]>(() => {
    const markersOfInterest = points.map((point) => ({
      position: { lat: point.latitude, lng: point.longitude },
      icon: '🎯',
      size: [24, 24],
      iconAnchor: [12, 12],
    }))

    return touchPoint
      ? [
          ...markersOfInterest,
          {
            position: touchPoint,
            icon: '📌',
            size: [24, 24],
            iconAnchor: [0, 16],
            animation: { type: AnimationType.WAGGLE, iterationCount: 1 },
          },
        ]
      : markersOfInterest
  }, [points, touchPoint])

  const handleLeafletViewUpdate = useCallback(
    (message: WebviewLeafletMessage) => {
      if (
        message.event === 'onZoomEnd' &&
        typeof message.payload?.zoom === 'number'
      ) {
        setZoom(message.payload?.zoom)
      }

      if (message.event === 'onMapClicked' && message.payload?.touchLatLng) {
        setTouchPoint(message.payload.touchLatLng)
        setCoordinatesInput(
          `${message.payload.touchLatLng.lat}, ${message.payload.touchLatLng.lng}`,
        )
      }
    },
    [],
  )

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
          onChangeText={(text) => {
            setCoordinatesInput(text)
            setTouchPoint(null)
          }}
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
              setTouchPoint(null)
            }
          }}
        >
          Add point
        </Button>
      </View>
      <ScrollView style={{ maxHeight: 256, flexGrow: 0, marginTop: 16 }}>
        {points.length > 0 ? (
          <List.Accordion
            title={`Points (${points.length})`}
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
          mapShapes={tourShapes}
          mapCenterPosition={mapCenterPosition}
          zoom={zoom}
          onMessageReceived={handleLeafletViewUpdate}
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
