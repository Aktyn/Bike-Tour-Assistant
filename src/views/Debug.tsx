import { StyleSheet, View } from 'react-native'
import { Text } from 'react-native-paper'

export const Debug = () => {
  return (
    <View style={styles.container}>
      <Text variant="titleLarge">Debug</Text>
    </View>
  )
}

const styles = StyleSheet.create({
  container: {
    alignItems: 'center',
    justifyContent: 'space-between',
    rowGap: 16,
    flex: 1,
  },
})
