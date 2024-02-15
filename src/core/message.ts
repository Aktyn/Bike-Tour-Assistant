import { Buffer } from 'buffer'

export enum MessageType {
  PING = 1,
  SET_LIGHTNESS,
  TAKE_PHOTO, //TODO: use
}

type MessageBase<T extends MessageType, DataType> = {
  type: T
  data: DataType
}

export type Message =
  | MessageBase<MessageType.PING, null>
  | MessageBase<MessageType.SET_LIGHTNESS, { lightness: number }>
  | MessageBase<MessageType.TAKE_PHOTO, null>

/** Returns base64 representation of buffer (32 bytes limit) */
export function parseMessageData(message: Message) {
  let buffer: Buffer | null = null

  switch (message.type) {
    case MessageType.PING:
    case MessageType.TAKE_PHOTO:
      buffer = Buffer.alloc(1)
      break
    case MessageType.SET_LIGHTNESS:
      buffer = Buffer.alloc(2)
      buffer.writeUInt8(Math.floor(message.data.lightness), 1)
      break
  }

  if (!buffer) {
    throw new Error('Unknown message type')
  }
  buffer.writeUInt8(message.type, 0)
  return buffer.toString('base64')
}
