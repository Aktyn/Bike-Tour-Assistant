import { useAssets } from 'expo-asset'
import { Dimensions, Image, ScrollView } from 'react-native'

/* eslint-disable @typescript-eslint/no-var-requires */
const legend1 = require('../img/cyclosm-legend-1.png')
const legend2 = require('../img/cyclosm-legend-2.png')
const legend3 = require('../img/cyclosm-legend-3.png')
const legend4 = require('../img/cyclosm-legend-4.png')

export const MapLegend = () => {
  const [assets] = useAssets([legend1, legend2, legend3, legend4])

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
