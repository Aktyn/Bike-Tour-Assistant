import { blueGrey, cyan, deepOrange, red } from 'material-ui-colors'
import type { MD3Theme } from 'react-native-paper'
import { MD3DarkTheme as DefaultTheme } from 'react-native-paper'

export const darkTheme: MD3Theme = {
  ...DefaultTheme,
  mode: 'exact',
  roundness: 4,
  colors: {
    ...DefaultTheme.colors,
    background: blueGrey[600],
    primary: cyan[400],
    secondary: deepOrange[200],
    error: red[400],
    outline: blueGrey[100],
    outlineVariant: blueGrey[400],

    // primaryContainer: '#f5f',
    // secondaryContainer: '#f5f',
    // tertiary: '#f5f',
    // tertiaryContainer: '#f5f',
    // surface: '#f5f',s
    // surfaceVariant: '#f5f',
    // surfaceDisabled: '#f5f',
    // errorContainer: '#f5f',
    // onPrimary: '#f5f',
    // onPrimaryContainer: '#f5f',
    // onSecondary: '#f5f',
    // onSecondaryContainer: '#f5f',
    // onTertiary: '#f5f',
    // onTertiaryContainer: '#f5f',
    // onSurface: '#f5f',
    // onSurfaceVariant: '#f5f',
    // onSurfaceDisabled: '#f5f',
    // onError: '#f5f',
    // onErrorContainer: '#f5f',
    // onBackground: '#f5f',
    // inverseSurface: '#f5f',
    // inverseOnSurface: '#f5f',
    // inversePrimary: '#f5f',
    // shadow: '#f5f',
    // scrim: '#f5f',
    // backdrop: '#f5f',
  },
}