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
