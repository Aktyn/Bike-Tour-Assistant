import BackgroundTimer, { type TimeoutId } from 'react-native-background-timer'

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
