import BackgroundTimer, { type TimeoutId } from 'react-native-background-timer'

export function clamp(value: number, min: number, max: number) {
  return Math.max(min, Math.min(max, value))
}

export const int = (value?: string) => parseInt(value ?? '', 10) || 0
export const float = (value?: string) => parseFloat(value ?? '') || 0

export function pick<ObjectType, Key extends Extract<keyof ObjectType, string>>(
  object: ObjectType,
  ...keys: Key[]
) {
  const picked = {} as Pick<ObjectType, Key>
  for (const key of keys) {
    picked[key] = object[key]
  }
  return picked
}

export function omit<ObjectType, Key extends Extract<keyof ObjectType, string>>(
  object: ObjectType,
  ...keys: Key[]
) {
  const omitted = {} as Omit<ObjectType, Key>
  const keysSet = new Set<Extract<keyof ObjectType, string>>(keys)
  for (const objectKey in object) {
    if (!keysSet.has(objectKey)) {
      omitted[objectKey as unknown as Exclude<keyof ObjectType, Key>] =
        object[objectKey as unknown as Exclude<keyof ObjectType, Key>]
    }
  }
  return omitted
}

export function tryParseJSON<FallbackType>(
  jsonString: string,
  fallbackValue?: FallbackType,
): FallbackType

export function tryParseJSON<FallbackType = undefined>(
  jsonString: string,
  fallbackValue?: FallbackType,
): unknown | null

export function tryParseJSON<FallbackType = undefined>(
  jsonString: string,
  fallbackValue?: FallbackType,
) {
  try {
    return JSON.parse(jsonString)
  } catch (e) {
    return fallbackValue ?? null
  }
}

type AnyFunction = (...args: never[]) => void
export type ArgumentTypes<F extends AnyFunction> = F extends (
  ...args: infer A
) => void
  ? A
  : never

export function debounce<FunctionType extends AnyFunction>(
  func: FunctionType,
  delay?: number,
  options: Partial<{ forceAfterNumberOfAttempts: number }> = {},
): [
  debouncedFunction: (...args: ArgumentTypes<typeof func>) => void,
  cancel: () => void,
] {
  let timeout: TimeoutId | null = null
  let attempts = 0

  const cancel = () => {
    if (timeout) {
      BackgroundTimer.clearTimeout(timeout)
      timeout = null
    }
  }

  return [
    (...args: ArgumentTypes<typeof func>) => {
      if (
        typeof options?.forceAfterNumberOfAttempts === 'number' &&
        options.forceAfterNumberOfAttempts >= attempts
      ) {
        func(...args)
        cancel()
        attempts = 0
        return
      }

      cancel()
      attempts++
      timeout = BackgroundTimer.setTimeout(() => {
        timeout = null
        attempts = 0
        func(...args)
      }, delay ?? 16) as never
    },
    cancel,
  ]
}
