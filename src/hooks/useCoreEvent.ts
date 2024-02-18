import { useEffect, type DependencyList } from 'react'

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

//TODO better typing
export function useCoreEvent<
  EventNameType extends string,
  EmitterType extends EmitterBase<EventNameType>,
>(
  emitter: EmitterType,
  event: EventNameType,
  listener: Parameters<EmitterType['on']>[1],
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
