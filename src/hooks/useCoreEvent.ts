import type { DependencyList } from 'react'
import { useEffect } from 'react'

type EmitterBase<EventName extends string> = {
  on: (
    eventName: EventName,
    listener: (...args: unknown[]) => void,
  ) => EmitterBase<EventName>
  off: (
    eventName: EventName,
    listener: (...args: unknown[]) => void,
  ) => EmitterBase<EventName>
}

export function useCoreEvent<
  EventNameType extends string,
  EmitterType extends EmitterBase<EventNameType>,
>(
  emitter: EmitterType,
  event: EventNameType,
  listener: ExtractListenerType<EventNameType, EmitterType>,
  deps: DependencyList = [],
) {
  useEffect(() => {
    const handler = (...args: unknown[]) => {
      listener(...args)
    }
    emitter.on(event, handler)

    return () => {
      emitter.off(event, handler)
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, deps)
}

type ExtractListenerType<
  E extends string,
  T extends EmitterBase<E>,
> = T extends {
  on(event: E, listener: infer L): T
}
  ? L
  : never
