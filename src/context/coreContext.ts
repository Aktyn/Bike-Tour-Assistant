import { createContext, useContext } from 'react'
import type { Core } from '../core/core'

export const CoreContext = createContext<Core>(null as never)

export function useCore() {
  const core = useContext(CoreContext)
  if (!core) {
    throw new Error('Core not found')
  }
  return core
}
