import { type ComponentType, useState } from 'react'
import { StyleSheet, View } from 'react-native'
import {
  BottomNavigation,
  Button,
  Divider,
  Text,
  useTheme,
} from 'react-native-paper'
import type { BaseRoute } from 'react-native-paper/lib/typescript/components/BottomNavigation/BottomNavigation'
import { GeneralOptions } from './GeneralOptions'
import { LocationOptions } from './LocationOptions'
import { MapLegend } from './MapLegend'
import { PointsOfInterest } from './PointsOfInterest'
import { useCore } from '../context/coreContext'
import { useCoreEvent } from '../hooks/useCoreEvent'

export const Main = () => {
  const theme = useTheme()
  const { deviceSettings, bluetooth, gps } = useCore()

  const [locationPermissionsGranted, setLocationPermissionsGranted] =
    useState(true)

  useCoreEvent(gps, 'toggleGranted', setLocationPermissionsGranted)

  const [index, setIndex] = useState(0)
  const renderScene = BottomNavigation.SceneMap(navigationScenes)

  return (
    <>
      <View style={styles.titleContainer}>
        <Text variant="titleLarge" style={{ fontWeight: 'bold' }}>
          Bike Tour Assistant
        </Text>
        <Button
          dark
          mode="contained"
          icon="bluetooth-off"
          onPress={() => bluetooth.disconnectFromDevice().catch(console.error)}
        >
          Disconnect
        </Button>
      </View>
      {!locationPermissionsGranted && (
        <>
          <Divider />
          <View style={{ padding: 16 }}>
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
          </View>
        </>
      )}
      <BottomNavigation
        navigationState={{ index, routes }}
        onIndexChange={setIndex}
        renderScene={renderScene}
        sceneAnimationType="shifting"
        barStyle={{
          backgroundColor: theme.colors.primaryContainer,
          marginBottom: -8,
        }}
        activeIndicatorStyle={{ backgroundColor: theme.colors.onPrimary }}
      />
      <Text
        variant="labelSmall"
        style={{
          ...styles.footer,
          color: theme.colors.onSurfaceVariant,
          backgroundColor: theme.colors.primaryContainer,
        }}
      >
        Made by Aktyn
      </Text>
    </>
  )
}

const routes = [
  {
    key: 'general',
    title: 'General',
    focusedIcon: 'cellphone-cog',
    unfocusedIcon: 'cellphone-cog',
  },
  {
    key: 'location',
    title: 'Location options',
    focusedIcon: 'crosshairs-gps',
    unfocusedIcon: 'crosshairs',
  },
  {
    key: 'points-of-interest',
    title: 'Points of interest',
    focusedIcon: 'map-marker-multiple',
    unfocusedIcon: 'map-marker-multiple-outline',
  },
  {
    key: 'legend',
    title: 'Map legend',
    focusedIcon: 'map-legend',
    unfocusedIcon: 'map-legend',
  },
] as const satisfies BaseRoute[]

const navigationScenes = {
  general: GeneralOptions,
  location: LocationOptions,
  'points-of-interest': PointsOfInterest,
  legend: MapLegend,
} satisfies {
  [key in (typeof routes)[number]['key']]: ComponentType<{
    route: BaseRoute
    jumpTo: (key: string) => void
  }>
}

const styles = StyleSheet.create({
  titleContainer: {
    display: 'flex',
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
    fontWeight: 'bold',
    padding: 16,
    textAlign: 'center',
  },
  footer: {
    paddingHorizontal: 24,
    width: '100%',
    textAlign: 'right',
  },
})
