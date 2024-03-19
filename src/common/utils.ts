export const removeNonNumericCharacters = (value: string) =>
  value.replace(/[^\d,.]/g, '')
