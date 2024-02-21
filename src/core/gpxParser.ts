//@ts-expect-error No type definitions for react-native-dom-parser
import DomSelector from 'react-native-dom-parser'
import { float } from '../utils'

export interface TourPoint {
  index: number
  latitude: number
  longitude: number
}

export async function parseGpxFile(fileUri: string) {
  const gpxContent = await fetch(fileUri).then((res) => res.text())

  const gpxDOM = DomSelector(
    gpxContent
      .split(/\r*\n/)
      .join('')
      .match(/<trk>(.*)<\/trk>/gi)?.[0] ?? '',
  )
  const trkptArray = gpxDOM.getElementsByTagName('trkpt')
  //   const clusteredPoints: ClusteredTour = new Map()
  //   this.rawTour = []
  const tour: TourPoint[] = []
  let index = 0
  for (const trkpt of trkptArray) {
    if (
      trkpt.tagName !== 'trkpt' ||
      !trkpt.attributes.lat ||
      !trkpt.attributes.lon
    ) {
      continue
    }

    const latitude = float(trkpt.attributes.lat)
    const longitude = float(trkpt.attributes.lon)
    // const tilePos = convertLatLongToTile(latitude, longitude, this.mapZoom)

    // let timestamp = 0
    // try {
    //   timestamp = new Date(
    //     trkpt.getElementsByTagName('time')?.[0]?.children[0]?.text,
    //   ).getTime()
    // } catch (e) {}

    const point = {
      index,
      latitude,
      longitude,
      //   tilePos,
      //   timestamp,
    }
    index++
    // const maxI = 2 ** this.mapZoom
    // const tileKey = `${Math.floor(tilePos.x) % maxI}-${
    //   Math.floor(tilePos.y) % maxI
    // }` as const
    // const cluster = clusteredPoints.get(tileKey) ?? []
    // cluster.push(point)
    tour.push(point)

    // clusteredPoints.set(tileKey, cluster)
  }
  //   this.tour = clusteredPoints
  //   this.rawTour.sort((a, b) => a.timestamp - b.timestamp) //ASC
  //   this.reduceRawTour()
  //   this.emit('tourUpdate', this.tour, this.mapZoom)

  return tour
}
