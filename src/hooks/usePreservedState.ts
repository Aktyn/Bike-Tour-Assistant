import { useEffect, useState } from 'react'
import { sessionStorage } from '../utils/storage'

export function usePreservedState<T>(key: string, value: T) {
  const [state, setState] = useState(value)
  useEffect(() => {
    const preservedState = sessionStorage.getItem(key)
    if (preservedState) {
      setState(JSON.parse(preservedState))
    }
  }, [key])
  useEffect(() => {
    sessionStorage.setItem(key, JSON.stringify(state))
  }, [key, state])
  return [state, setState] as const
}
