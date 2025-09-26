# JSX to JavaScript Transformer

This project provides a simple, lightweight tool to transform JSX code into standard JavaScript calls using `React.createElement`. It is implemented in Go and designed with a TDD approach for clarity and maintainability.

## Features

- Parse JSX code embedded in JavaScript files.
- Transform JSX elements, including nested components, expressions, and props.
- Handle boolean props, inline event handlers, and expressions inside JSX.
- Segment JavaScript code and JSX to allow partial transformations.
- Multi-pass transformation to correctly handle nested JSX inside expressions.

## Usage

```bash
clang build.c -o builder && ./builder clean test
clang build.c -o builder && ./builder clean
./build/jsx_parser example.jsx
```

Or you can link to `libjsx_parser.a`, we do not provide dynamic lib.

## Notes

 - This transformer is designed for educational purposes and lightweight JSX parsing. It does not fully implement JSX/React syntax rules.
 - For production-grade JSX transformations, consider using established tools like Babel.
