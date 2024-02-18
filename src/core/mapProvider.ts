import EventEmitter from 'events'
import { type DecodedPng, decode } from 'fast-png'
import { convertLatLongToTile } from '../utils'

const serverLetters = ['a', 'b', 'c']

type TileURL = string

export interface Tile {
  x: number
  y: number
  z: number
  image: DecodedPng
}

declare interface MapProviderEventEmitter {
  on(event: 'tileLoaded', listener: (tile: Tile) => void): this
  off(event: 'tileLoaded', listener: (tile: Tile) => void): this
  emit(event: 'tileLoaded', tile: Tile): boolean
}

class MapProviderEventEmitter extends EventEmitter {}

export class MapProvider extends MapProviderEventEmitter {
  private serverLetter = getRandomServerLetter()
  private tileResolution = 256
  //TODO: option for selecting tiles provider
  private tilesProvider =
    'https://{s}.tile-cyclosm.openstreetmap.fr/cyclosm/{z}/{x}/{y}.png'
  // 'https://tile.openstreetmap.org/{z}/{x}/{y}.png'
  // https://wiki.openstreetmap.org/wiki/Raster_tile_providers
  private zoom = 16

  private loadedTiles = new Map<TileURL, Tile>()
  private tilesToLoad = new Map<TileURL, Omit<Tile, 'image'>>()
  private loadingTile = false

  setZoom(zoom: number) {
    if (zoom === this.zoom) {
      return
    }

    this.reset()
    this.zoom = zoom
  }

  destroy() {
    this.removeAllListeners()
    this.reset()
  }

  reset() {
    this.serverLetter = getRandomServerLetter()
    this.loadedTiles.clear()
    this.tilesToLoad.clear()
  }

  update(latitude: number, longitude: number) {
    const { x, y } = convertLatLongToTile(latitude, longitude, this.zoom)

    const url = this.tilesProvider
      .replace(/\{s\}/gi, this.serverLetter)
      .replace(/\{z\}/gi, this.zoom.toString())
      .replace(/\{x\}/gi, x.toString())
      .replace(/\{y\}/gi, y.toString())

    if (this.loadedTiles.has(url) || this.tilesToLoad.has(url)) {
      return
    }

    this.tilesToLoad.set(url, { x, y, z: this.zoom })
    this.fetchNextTile()
  }

  private fetchNextTile() {
    if (this.loadingTile) {
      return
    }
    const [url, partialTile]: [TileURL, Omit<Tile, 'image'>] =
      this.tilesToLoad.entries().next().value ?? []
    if (!url) {
      return
    }

    this.loadingTile = true
    console.info('Fetching tile:', url)

    fetch(url, { method: 'GET' })
      .then((res) => res.arrayBuffer())
      .then((buffer) => {
        this.tilesToLoad.delete(url)
        const image = decode(buffer, { checkCrc: true })
        if (
          image.width !== this.tileResolution ||
          image.height !== this.tileResolution
        ) {
          console.error('Invalid tile resolution')
          this.loadingTile = false
          this.fetchNextTile()
          return
        }

        const tile: Tile = { ...partialTile, image }
        this.loadedTiles.set(url, tile)
        this.emit('tileLoaded', tile)
        this.loadingTile = false
        this.fetchNextTile()
      })
      .catch((error) => {
        console.error(
          `Error: ${error instanceof Error ? error.message : error}`,
        )
        this.loadingTile = false
      })
  }
}

function getRandomServerLetter() {
  return serverLetters[Math.floor(Math.random() * serverLetters.length)]
}
