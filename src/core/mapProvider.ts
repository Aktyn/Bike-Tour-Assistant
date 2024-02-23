import EventEmitter from 'events'
import { convertLatLongToTile } from '../utils'

const serverLetters = ['a', 'b', 'c']

type TileURL = string

export interface Tile {
  x: number
  y: number
  z: number
  fileData: ArrayBuffer
}

declare interface MapProviderEventEmitter {
  on(event: 'tileLoaded', listener: (tile: Tile) => void): this
  off(event: 'tileLoaded', listener: (tile: Tile) => void): this
  emit(event: 'tileLoaded', tile: Tile): boolean
}

class MapProviderEventEmitter extends EventEmitter {}

export class MapProvider extends MapProviderEventEmitter {
  private readonly tilesRadius = 1
  private readonly tileResolution = 256
  //TODO: option for selecting tiles provider
  private readonly tilesProvider =
    'https://{s}.tile-cyclosm.openstreetmap.fr/cyclosm/{z}/{x}/{y}.png'
  // 'https://tile.openstreetmap.org/{z}/{x}/{y}.png'
  // https://wiki.openstreetmap.org/wiki/Raster_tile_providers

  private zoom = 16
  private readonly serverLetter = getRandomServerLetter()

  private loadedTiles = new Map<TileURL, Tile>()
  private tilesToLoad = new Map<TileURL, Omit<Tile, 'fileData'>>()
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
    this.loadedTiles.clear()
    this.tilesToLoad.clear()
  }

  update(latitude: number, longitude: number) {
    const { x, y } = convertLatLongToTile(latitude, longitude, this.zoom)

    this.loadTile(x, y)
    for (let i = -this.tilesRadius; i <= this.tilesRadius; i++) {
      for (let j = -this.tilesRadius; j <= this.tilesRadius; j++) {
        if (i === 0 && j === 0) {
          continue
        }
        this.loadTile(x + i, y + j)
      }
    }
  }

  private loadTile(x: number, y: number) {
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
    const [url, partialTile]: [TileURL, Omit<Tile, 'fileData'>] =
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

        const tile: Tile = { ...partialTile, fileData: buffer }
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
