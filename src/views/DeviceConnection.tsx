import { useCallback, useEffect, useMemo, useRef, useState } from 'react'
import { lightGreen } from 'material-ui-colors'
import { Animated, ScrollView, StyleSheet, View } from 'react-native'
import { Button, Divider, Icon, List, Text, Title } from 'react-native-paper'
import { Config } from '../config'
import { useCore } from '../context/coreContext'
import { useCoreEvent } from '../hooks/useCoreEvent'

export const DeviceConnection = () => {
  const pulseAnimation = useRef(new Animated.Value(1)).current
  const blinkAnimation = useRef(new Animated.Value(1)).current

  const { bluetooth } = useCore()

  const [isEnabled, setIsEnabled] = useState(bluetooth.isEnabled())
  const [isScanning, setIsScanning] = useState(bluetooth.isScanning())
  const [isConnectingToTargetDevice, setIsConnectingToTargetDevice] = useState(
    bluetooth.isConnectingToTargetDevice(),
  )
  const [devicesList, setDevicesList] = useState<
    { id: string; name: string }[]
  >([])

  const isTargetDeviceDiscovered = useMemo(
    () =>
      devicesList.some((device) => device.name === Config.TARGET_DEVICE_NAME),
    [devicesList],
  )

  const handleDeviceDiscovered = useCallback(() => {
    setDevicesList(
      bluetooth.getDiscoveredDevices().reduce(
        (acc, device) => {
          if (device.name) {
            acc.push({ id: device.id, name: device.name })
          }
          return acc
        },
        [] as typeof devicesList,
      ),
    )
  }, [bluetooth])

  useCoreEvent(bluetooth, 'stateChange', (v) => setIsEnabled(v as never))
  useCoreEvent(bluetooth, 'isScanning', (v) => setIsScanning(v as never))
  useCoreEvent(bluetooth, 'connectingToTargetDevice', (v) =>
    setIsConnectingToTargetDevice(v as never),
  )
  useCoreEvent(bluetooth, 'deviceDiscovered', handleDeviceDiscovered)

  useEffect(() => {
    if (isScanning && isTargetDeviceDiscovered) {
      setIsScanning(false)
      bluetooth.stopScanning()
    }
  }, [bluetooth, isScanning, isTargetDeviceDiscovered])

  useEffect(() => {
    if (!isScanning) {
      pulseAnimation.setValue(1)
      blinkAnimation.setValue(1)
      return
    }

    const animation1 = Animated.loop(
      Animated.sequence([
        Animated.timing(pulseAnimation, {
          toValue: 1,
          duration: 750,
          useNativeDriver: true,
        }),
        Animated.timing(pulseAnimation, {
          toValue: 0.618,
          duration: 750,
          useNativeDriver: true,
        }),
        Animated.timing(pulseAnimation, {
          toValue: 1,
          duration: 750,
          useNativeDriver: true,
        }),
      ]),
    )
    animation1.start()

    const animation2 = Animated.loop(
      Animated.sequence([
        Animated.timing(blinkAnimation, {
          toValue: 1,
          duration: 750,
          useNativeDriver: true,
        }),
        Animated.timing(blinkAnimation, {
          toValue: 0.5,
          duration: 750,
          useNativeDriver: true,
        }),
        Animated.timing(blinkAnimation, {
          toValue: 1,
          duration: 750,
          useNativeDriver: true,
        }),
      ]),
    )
    animation2.start()

    return () => {
      animation1.stop()
      animation2.stop()
    }
  }, [blinkAnimation, isScanning, pulseAnimation])

  return (
    <View style={styles.container}>
      <Animated.View style={{ transform: [{ scale: pulseAnimation }] }}>
        <Icon
          source={
            isScanning
              ? 'bluetooth-connect'
              : isEnabled
                ? 'bluetooth'
                : 'bluetooth-off'
          }
          size={64}
        />
      </Animated.View>
      {isEnabled ? (
        <>
          <Button
            dark
            mode="contained"
            icon={isScanning ? 'pause' : 'radar'}
            disabled={isTargetDeviceDiscovered || isConnectingToTargetDevice}
            onPress={() =>
              isScanning ? bluetooth.stopScanning() : bluetooth.startScanning()
            }
          >
            {isScanning
              ? 'Stop scanning'
              : 'Scan for Bike Tour Assistant device'}
          </Button>
          <ScrollView>
            {devicesList.length > 0 && (
              <List.Section>
                <List.Subheader>Discovered devices</List.Subheader>
                {devicesList.map(({ id, name }) => (
                  <List.Item
                    key={id}
                    title={name}
                    left={DeviceIcon}
                    titleStyle={
                      name === Config.TARGET_DEVICE_NAME
                        ? styles.targetDeviceItemStyle
                        : undefined
                    }
                  />
                ))}
              </List.Section>
            )}
          </ScrollView>
          {isScanning ? (
            <Animated.View style={{ opacity: blinkAnimation }}>
              <Title>Scanning...</Title>
            </Animated.View>
          ) : (
            isTargetDeviceDiscovered && (
              <View style={styles.connectingInfoContainer}>
                <Divider style={styles.divider} />
                <Text variant="titleMedium">Target device found</Text>
                <Button
                  dark
                  mode="contained"
                  loading={isConnectingToTargetDevice}
                  onPress={() =>
                    bluetooth.connectToTargetDevice().catch(console.error)
                  }
                >
                  {isConnectingToTargetDevice ? 'Connecting' : 'Connect'}
                </Button>
              </View>
            )
          )}
        </>
      ) : (
        <Button dark mode="contained" onPress={() => bluetooth.enable()}>
          Enable Bluetooth
        </Button>
      )}
    </View>
  )
}

const DeviceIcon = () => <List.Icon icon="devices" />

const styles = StyleSheet.create({
  container: {
    paddingVertical: 32,
    alignItems: 'center',
    rowGap: 16,
    flex: 1,
  },
  divider: {
    width: '100%',
  },
  targetDeviceItemStyle: {
    color: lightGreen[400],
    fontWeight: 'bold',
  },
  connectingInfoContainer: {
    width: '100%',
    alignItems: 'center',
    rowGap: 16,
  },
})
