import { Buffer } from 'buffer'
import type { LocationState } from './gps'
import { type Tile } from './mapProvider'

// NOTE: reordering this enum will need to be reflected in raspberry pi project code
export enum MessageType {
  PING = 1,
  SET_LIGHTNESS,
  TAKE_PHOTO,
  LOCATION_UPDATE,
  SEND_MAP_TILE_START,
  SEND_MAP_TILE_DATA_CHUNK,
  SEND_MAP_TILE_INDEXED_COLORS_BATCH_48,
}

type MessageBase<T extends MessageType, DataType> = {
  type: T
  data: DataType
}

export type Message =
  | MessageBase<MessageType.PING, null>
  | MessageBase<MessageType.SET_LIGHTNESS, { lightness: number }>
  | MessageBase<MessageType.TAKE_PHOTO, null>
  | MessageBase<MessageType.LOCATION_UPDATE, LocationState>
  | MessageBase<
      MessageType.SEND_MAP_TILE_START,
      {
        tileIdentifier: Omit<Tile, 'image'>
        tileWidth: number
        tileHeight: number
        dataByteLength: number
        paletteSize: number
      }
    >
  | MessageBase<
      MessageType.SEND_MAP_TILE_DATA_CHUNK,
      { chunkIndex: number; bytes: ArrayBuffer }
    >
  | MessageBase<
      MessageType.SEND_MAP_TILE_INDEXED_COLORS_BATCH_48,
      { colorIndex: number; red: number; green: number; blue: number }[]
    >

/** Returns base64 representation of buffer (224 bytes limit) */
export function parseMessageData(message: Message) {
  let buffer: Buffer | null = null

  switch (message.type) {
    default:
    case MessageType.PING:
    case MessageType.TAKE_PHOTO:
      buffer = Buffer.alloc(1)
      break
    case MessageType.SET_LIGHTNESS:
      buffer = Buffer.alloc(2)
      buffer.writeUInt8(Math.floor(message.data.lightness), 1)
      break
    case MessageType.LOCATION_UPDATE:
      buffer = Buffer.alloc(25)

      /** 
       * Example message.data:
       *  {
            heading: 261.33624267578125,
            latitude: 51.7770881,
            longitude: 19.4279,
            speed: 3.5133471488952637,
            timestamp: 1708199578000,
          }
        */
      buffer.writeFloatLE(message.data.latitude, 1)
      buffer.writeFloatLE(message.data.longitude, 5)
      buffer.writeFloatLE(message.data.speed, 9)
      buffer.writeFloatLE(message.data.heading, 13)
      buffer.writeBigUInt64LE(BigInt(message.data.timestamp), 17)
      break
    case MessageType.SEND_MAP_TILE_START:
      buffer = Buffer.alloc(23)
      buffer.writeUint32LE(message.data.tileIdentifier.x, 1)
      buffer.writeUint32LE(message.data.tileIdentifier.y, 5)
      buffer.writeUint32LE(message.data.tileIdentifier.z, 9)
      buffer.writeUint16LE(message.data.tileWidth, 13)
      buffer.writeUint16LE(message.data.tileHeight, 15)
      buffer.writeUint32LE(message.data.dataByteLength, 17)
      buffer.writeUint16LE(message.data.paletteSize, 21)
      break
    case MessageType.SEND_MAP_TILE_DATA_CHUNK:
      {
        const chunkSize = message.data.bytes.byteLength
        buffer = Buffer.alloc(3 + chunkSize)
        buffer.writeUint16LE(message.data.chunkIndex, 1)

        const uint8Array = new Uint8Array(message.data.bytes)
        for (let i = 0; i < chunkSize; i++) {
          buffer.writeUInt8(uint8Array[i], 3 + i)
        }
      }
      break
    case MessageType.SEND_MAP_TILE_INDEXED_COLORS_BATCH_48:
      buffer = Buffer.alloc(1 + message.data.length * 5)
      for (let ci = 0; ci < message.data.length; ci++) {
        const color = message.data[ci]
        buffer.writeUint16LE(color.colorIndex, 1 + 5 * ci)
        buffer.writeUInt8(color.red, 3 + 5 * ci)
        buffer.writeUInt8(color.green, 4 + 5 * ci)
        buffer.writeUInt8(color.blue, 5 + 5 * ci)
      }
      break
  }

  if (!buffer) {
    throw new Error('Unknown message type')
  }
  buffer.writeUInt8(message.type, 0)
  return buffer.toString('base64')
}
