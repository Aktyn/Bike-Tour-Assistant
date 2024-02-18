import type { LocationObject } from 'expo-location'
import * as TaskManager from 'expo-task-manager'
import { Core } from './core'
import { Config } from '../config'

TaskManager.defineTask(
  Config.GPS_TASK_NAME,
  async ({ data, error }: TaskManager.TaskManagerTaskBody) => {
    if (error) {
      console.error(error)
      return
    }
    if (data) {
      const { locations } = data as { locations: LocationObject[] }
      const location = locations[0]

      if (location) {
        Core.instances.forEach((instance) =>
          instance.gps.updateLocation(location),
        )
      }
    }
  },
)
