import { Buffer } from 'buffer'
import { useCallback, useEffect, useMemo, useState } from 'react'
import * as DocumentPicker from 'expo-document-picker'
import * as FileSystem from 'expo-file-system'
import { deepOrange } from 'material-ui-colors'
import { Platform, ScrollView, StyleSheet, View } from 'react-native'
import {
  AnimationType,
  LeafletView,
  MapShapeType,
  type MapMarker,
  type MapShape,
  type WebviewLeafletMessage,
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

type Point = Pick<LocationState, 'latitude' | 'longitude'> & { name: string }
type LatLng = { lat: number; lng: number }

export const PointsOfInterest = () => {
  const theme = useTheme()
  const { deviceSettings, gps } = useCore()

  const [pointNameInput, setPointNameInput] = useState('')
  const [coordinatesInput, setCoordinatesInput] = useState('')
  const [points, setPoints] = useState<Point[]>(
    deviceSettings.get('pointsOfInterest'),
  )
  const [expandPointsList, setExpandPointsList] = useState(false)
  const [mapCenterPosition, setMapCenterPosition] = useState<LatLng | null>(
    null,
  )
  const [zoom, setZoom] = useState(15)
  const [touchPoint, setTouchPoint] = useState<LatLng | null>(null)
  const [gpxFile, setGpxFile] = useState(deviceSettings.get('gpxFile'))
  const [tourShapes, setTourShapes] = useState<MapShape[]>([])

  const parsedPoint = parseCoordinates(coordinatesInput, pointNameInput)

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

  const moveMapToLocation = useCallback((point: Point) => {
    setMapCenterPosition({
      lat: point.latitude,
      lng: point.longitude,
    })
  }, [])

  useEffect(() => {
    const last = points[points.length - 1]
    if (last) {
      moveMapToLocation(last)
    }
  }, [moveMapToLocation, points])

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
      // icon: `🎯${point.name}`,
      icon: `<div style="display: flex; flex-direction: column; align-items: center">
        <span>🎯</span>
        <span style="font-size: 16px; white-space: nowrap;">${point.name}</span>
      </div>`,
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

  const importPointsFile = useCallback(async () => {
    try {
      const data = await DocumentPicker.getDocumentAsync({
        type: ['application/json'],
      })

      if (data.canceled) {
        throw new Error('File selection canceled')
      }
      const fileUri = data.assets[0]?.uri
      const jsonContent = await fetch(fileUri).then((res) => res.text())
      const parsed = JSON.parse(jsonContent)
      if (!Array.isArray(parsed)) {
        throw new Error('Incorrect data')
      }
      deviceSettings.set('pointsOfInterest', parsed)
    } catch (error) {
      console.error(
        `Cannot load points file. Error: ${
          error instanceof Error ? error.message : String(error)
        }`,
      )
    }
  }, [deviceSettings])

  const exportPointsFile = useCallback(async () => {
    try {
      if (Platform.OS !== 'android') {
        return
      }
      const permissions =
        await FileSystem.StorageAccessFramework.requestDirectoryPermissionsAsync()

      if (!permissions.granted) {
        throw new Error('Permissions not granted')
      }
      const base64 = Buffer.from(JSON.stringify(points, null, 2)).toString(
        'base64',
      )

      const fileUri = await FileSystem.StorageAccessFramework.createFileAsync(
        permissions.directoryUri,
        'points.json',
        'application/json',
      )
      await FileSystem.writeAsStringAsync(fileUri, base64, {
        encoding: FileSystem.EncodingType.Base64,
      })
    } catch (error) {
      console.error(
        `Cannot save points file. Error: ${
          error instanceof Error ? error.message : String(error)
        }`,
      )
    }
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
          label="Name"
          placeholder="Name or short description of the point"
          value={pointNameInput}
          left={<TextInput.Icon icon="label" />}
          onChangeText={(text) => {
            setPointNameInput(text)
          }}
        />
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
              setPointNameInput('')
              setCoordinatesInput('')
              setTouchPoint(null)
            }
          }}
        >
          Add point
        </Button>
      </View>
      <ScrollView style={{ maxHeight: '100%', flexGrow: 0, marginTop: 16 }}>
        {points.length > 0 ? (
          <List.Accordion
            title={`Points (${points.length})`}
            expanded={expandPointsList}
            onPress={() => setExpandPointsList(!expandPointsList)}
            // eslint-disable-next-line react/no-unstable-nested-components
            left={(props) => <List.Icon {...props} icon="view-list" />}
          >
            <List.Item
              title={
                <View style={styles.pointsOptions}>
                  <Button
                    dark
                    textColor={theme.colors.onSurface}
                    mode="text"
                    icon="file-import"
                    onPress={importPointsFile}
                  >
                    Import
                  </Button>
                  <Button
                    dark
                    textColor={theme.colors.onSurface}
                    mode="text"
                    icon="file-export"
                    onPress={exportPointsFile}
                  >
                    Export
                  </Button>
                </View>
              }
            />
            {points.map((point) => (
              <List.Item
                key={`${point.latitude},${point.longitude}`}
                title={
                  <View>
                    {point.name && <Text>{point.name}</Text>}
                    <Text>
                      Latitude:&nbsp;{point.latitude.toFixed(4)}
                      ,&nbsp;Longitude:&nbsp;{point.longitude.toFixed(4)}
                    </Text>
                  </View>
                }
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
                onPress={() => moveMapToLocation(point)}
              />
            ))}
          </List.Accordion>
        ) : (
          <View
            style={{
              display: 'flex',
              flexDirection: 'column',
              alignItems: 'center',
              paddingBottom: 8,
              rowGap: 4,
            }}
          >
            <Text variant="bodyLarge" style={{ textAlign: 'center' }}>
              No points specified
            </Text>
            <Button
              dark
              mode="contained-tonal"
              icon="file-import"
              onPress={importPointsFile}
            >
              Import
            </Button>
          </View>
        )}
      </ScrollView>
      <Divider />
      {(!expandPointsList || points.length < 5) && (
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
      )}
    </View>
  )
}

function parseCoordinates(input: string, name: string): Point | null {
  const [latitude, longitude] = input.replace(/\s/g, '').split(',')
  const parsedLatitude = parseFloat(latitude)
  const parsedLongitude = parseFloat(longitude)
  if (isNaN(parsedLatitude) || isNaN(parsedLongitude)) {
    return null
  }
  return { latitude: parsedLatitude, longitude: parsedLongitude, name }
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
  pointsOptions: {
    width: '100%',
    flexGrow: 1,
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'flex-start',
    justifyContent: 'flex-start',
  },
  mapContainer: {
    width: '100%',
    flexGrow: 1,
    backgroundColor: '#afc4',
  },
})
