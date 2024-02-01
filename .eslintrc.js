module.exports = {
  parser: '@typescript-eslint/parser',
  extends: [
    'eslint:recommended',
    'plugin:jest/recommended',
    '@react-native-community',
    'plugin:react/recommended',
    'plugin:@typescript-eslint/recommended',
    'plugin:prettier/recommended',
    'plugin:import/recommended',
    'plugin:react-hooks/recommended',
    'plugin:import/typescript',
  ],
  plugins: [
    'react',
    'react-native',
    '@typescript-eslint',
    'prettier',
    'import',
  ],
  parserOptions: {
    ecmaVersion: 2018,
    sourceType: 'module',
    ecmaFeatures: {
      jsx: true,
    },
  },
  settings: {
    'import/parsers': {
      '@typescript-eslint/parser': ['.ts', '.tsx'],
    },
    'import/resolver': {
      typescript: {
        alwaysTryTypes: true,
      },
      node: {
        paths: ['src'],
        extensions: ['.js', '.jsx', '.ts', '.tsx'],
        moduleDirectory: ['node_modules', 'src/'],
      },
    },
    react: {
      version: 'detect',
      pragma: 'React',
    },
  },
  globals: {
    fetch: true,
  },
  env: {
    'react-native/react-native': true,
    es6: true,
    jest: true,
  },
  rules: {
    'no-shadow': 'off',
    'no-alert': ['off'],
    '@typescript-eslint/consistent-type-imports': 'error',
    '@typescript-eslint/no-shadow': ['error'],
    '@typescript-eslint/no-extra-semi': ['off'],
    '@typescript-eslint/no-empty-interface': ['off'],
    '@typescript-eslint/explicit-module-boundary-types': 0,
    '@typescript-eslint/explicit-function-return-type': 0,
    '@typescript-eslint/no-unused-vars': [
      'error',
      {
        args: 'after-used',
        ignoreRestSiblings: false,
        argsIgnorePattern: '^_',
        varsIgnorePattern: '^_',
      },
    ],
    '@typescript-eslint/indent': [0],
    'import/default': 0,
    'import/extensions': 0,
    'import/named': 0,
    'import/no-named-as-default-member': 0,
    'import/no-named-as-default': 0,
    'import/order': [
      'error',
      {
        groups: ['builtin', 'external', 'internal', 'unknown'],
        alphabetize: {
          order: 'asc',
        },
        pathGroups: [
          {
            pattern: 'react',
            group: 'external',
            position: 'before',
          },
          {
            pattern: 'types',
            group: 'internal',
            position: 'before',
          },
          {
            pattern: 'types/**',
            group: 'internal',
            position: 'before',
          },
        ],
        pathGroupsExcludedImportTypes: ['builtin'],
        'newlines-between': 'ignore',
      },
    ],
    'react/display-name': [0],
    'react/jsx-closing-bracket-location': [0],
    'react/jsx-closing-tag-location': [0],
    'react/jsx-curly-newline': [0],
    'react/jsx-curly-spacing': [0],
    'react/jsx-equals-spacing': [0],
    'react/jsx-filename-extension': [
      2,
      { extensions: ['.js', '.jsx', '.ts', '.tsx'] },
    ],
    'react/jsx-first-prop-new-line': [0],
    'react/jsx-indent': [0],
    'react/jsx-indent-props': [0],
    'react/jsx-max-props-per-line': [0],
    'react/jsx-one-expression-per-line': [0],
    'react/jsx-props-no-multi-spaces': [0],
    'react/jsx-tag-spacing': [0],
    'react/jsx-wrap-multilines': [0],
    'react/prop-types': [0],
    'react/react-in-jsx-scope': 'off',
    'no-console': [
      'warn',
      {
        allow: ['warn', 'error'],
      },
    ],
    'react-native/no-inline-styles': 'off',
    'no-bitwise': 'off',
  },
  overrides: [
    {
      files: ['*.test.tsx', '**/test-utils/**'],
      rules: {
        '@typescript-eslint/no-explicit-any': 'off',
        '@typescript-eslint/no-non-null-assertion': 'off',
      },
    },
  ],
}
