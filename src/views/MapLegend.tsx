import { useAssets } from 'expo-asset'
import { Dimensions, Image, ScrollView } from 'react-native'

export const MapLegend = () => {
  const [assets] = useAssets([
    require('../img/cyclosm-legend-1.png'),
    require('../img/cyclosm-legend-2.png'),
    require('../img/cyclosm-legend-3.png'),
    require('../img/cyclosm-legend-4.png'),
  ])

  if (!assets?.length) {
    return null
  }

  const windowWidth = Dimensions.get('window').width

  return (
    <ScrollView
      style={{
        display: 'flex',
        flexDirection: 'column',
      }}
      contentContainerStyle={{
        justifyContent: 'flex-start',
        alignItems: 'flex-start',
        backgroundColor: '#fff',
      }}
    >
      {assets.map(
        (asset) =>
          asset.downloaded && (
            <Image
              key={asset.uri}
              source={{ uri: asset.uri }}
              width={windowWidth}
              height={(windowWidth / (asset.width ?? 1)) * (asset.height ?? 0)}
              resizeMode="stretch"
            />
          ),
      )}
    </ScrollView>
  )
}
