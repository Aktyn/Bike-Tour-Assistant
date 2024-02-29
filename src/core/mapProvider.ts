import EventEmitter from 'events'

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
  //TODO: option for selecting tiles provider
  private readonly tilesProvider =
    'https://{s}.tile-cyclosm.openstreetmap.fr/cyclosm/{z}/{x}/{y}.png'
  // 'https://tile.openstreetmap.org/{z}/{x}/{y}.png'
  // https://wiki.openstreetmap.org/wiki/Raster_tile_providers

  private readonly serverLetter = getRandomServerLetter()

  private loadedTiles = new Map<TileURL, Tile>()
  private tilesToLoad = new Map<TileURL, Omit<Tile, 'fileData'>>()
  private loadingTile = false

  destroy() {
    this.removeAllListeners()
    this.reset()
  }

  reset() {
    this.loadedTiles.clear()
    this.tilesToLoad.clear()
  }

  requestTile(x: number, y: number, z: number) {
    this.loadTile(x, y, z)
  }

  private loadTile(x: number, y: number, z: number) {
    const url = this.tilesProvider
      .replace(/\{s\}/gi, this.serverLetter)
      .replace(/\{x\}/gi, x.toString())
      .replace(/\{y\}/gi, y.toString())
      .replace(/\{z\}/gi, z.toString())

    if (this.loadedTiles.has(url) || this.tilesToLoad.has(url)) {
      return
    }

    this.tilesToLoad.set(url, { x, y, z })
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
