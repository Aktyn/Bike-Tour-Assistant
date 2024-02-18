/** Converts latitude, longitude, zoom to tile coordinates */
export function convertLatLongToTile(
  latitude: number,
  longitude: number,
  zoom: number,
) {
  const latRad = (latitude / 180) * Math.PI
  const n = 2 ** zoom

  const x = ((longitude + 180.0) / 360.0) * n
  const y = ((1.0 - Math.asinh(Math.tan(latRad)) / Math.PI) / 2.0) * n

  return {
    x: Math.floor(x) % n,
    y: Math.floor(y) % n,
  }
}
