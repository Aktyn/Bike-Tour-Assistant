import { Bluetooth } from './bluetooth'

export class Core {
  public readonly bluetooth = new Bluetooth()

  destroy() {
    this.bluetooth.destroy()
  }
}
