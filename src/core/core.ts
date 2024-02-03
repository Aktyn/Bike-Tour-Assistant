import { Bluetooth } from './bluetooth'

export class Core {
  public readonly bluetooth = new Bluetooth()

  constructor() {
    this.bluetooth.init().catch(console.error)
  }

  destroy() {
    this.bluetooth.destroy()
  }
}
