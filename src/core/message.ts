import { Buffer } from 'buffer'
import type { LocationState } from './gps'
import type { TourPoint } from './gpxParser'
import type { Tile } from './mapProvider'

// NOTE: reordering this enum will need to be reflected in raspberry pi project code
export enum MessageType {
  PING = 1,
  SET_LIGHTNESS,
  TAKE_PHOTO,
  LOCATION_UPDATE,
  SEND_MAP_TILE_START,
  SEND_MAP_TILE_DATA_CHUNK,
  CLEAR_TOUR_DATA,
  SEND_TOUR_START,
  SEND_TOUR_DATA_CHUNK,
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
        tileIdentifier: Omit<Tile, 'fileData'>
        fileSize: number
      }
    >
  | MessageBase<
      MessageType.SEND_MAP_TILE_DATA_CHUNK,
      { chunkIndex: number; bytes: ArrayBuffer }
    >
  | MessageBase<MessageType.CLEAR_TOUR_DATA, null>
  | MessageBase<MessageType.SEND_TOUR_START, { pointsCount: number }>
  | MessageBase<MessageType.SEND_TOUR_DATA_CHUNK, TourPoint[]>

/** Returns base64 representation of buffer (224 bytes limit) */
export function parseMessageData(message: Message) {
  let buffer: Buffer | null = null

  switch (message.type) {
    default:
    case MessageType.PING:
    case MessageType.TAKE_PHOTO:
    case MessageType.CLEAR_TOUR_DATA:
      buffer = Buffer.alloc(1)
      break
    case MessageType.SET_LIGHTNESS:
      buffer = Buffer.alloc(2)
      buffer.writeUInt8(Math.floor(message.data.lightness), 1)
      break
    case MessageType.LOCATION_UPDATE:
      buffer = Buffer.alloc(65)

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
      buffer.writeDoubleLE(message.data.latitude, 1)
      buffer.writeDoubleLE(message.data.longitude, 9)
      buffer.writeDoubleLE(message.data.speed, 17)
      buffer.writeDoubleLE(message.data.heading, 25)
      buffer.writeDoubleLE(message.data.altitude, 33)
      buffer.writeDoubleLE(message.data.altitudeAccuracy, 41)
      buffer.writeDoubleLE(message.data.accuracy, 49)
      buffer.writeBigUInt64LE(BigInt(message.data.timestamp), 57)
      break
    case MessageType.SEND_MAP_TILE_START:
      buffer = Buffer.alloc(17)
      buffer.writeUint32LE(message.data.tileIdentifier.x, 1)
      buffer.writeUint32LE(message.data.tileIdentifier.y, 5)
      buffer.writeUint32LE(message.data.tileIdentifier.z, 9)
      buffer.writeUint32LE(message.data.fileSize, 13)
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
    case MessageType.SEND_TOUR_START:
      buffer = Buffer.alloc(3)
      buffer.writeUint16LE(message.data.pointsCount, 1)
      break
    case MessageType.SEND_TOUR_DATA_CHUNK:
      {
        const chunkSize = message.data.length
        buffer = Buffer.alloc(3 + chunkSize * 10)
        buffer.writeUint16LE(chunkSize, 1)

        for (let i = 0; i < message.data.length; i++) {
          const point = message.data[i]
          buffer.writeUint16LE(point.index, 3 + i * 10)
          buffer.writeFloatLE(point.latitude, 5 + i * 10)
          buffer.writeFloatLE(point.longitude, 9 + i * 10)
        }
      }
      break
  }

  if (!buffer) {
    throw new Error('Unknown message type')
  }
  buffer.writeUInt8(message.type, 0)
  return buffer.toString('base64')
}

export function parseBase64(value: string) {
  return Buffer.from(value, 'base64')
}
