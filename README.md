# JuneLang


A simple programming language written in C++ inspired by C.

It is statically typed and compiled to native x86-64.

## Features

- Statically typed
- 3 primitive types: `int`, `char`, `bool`, and their list variants
- If blocks
- `while` and `for` loops
- Functions
- Type inference

## Example

```java
fun sample_function(list: [int], size: int): int [
    var s = 0;

    for (var i = 0; i < size; i += 1) [
        if (list[i] < 4) [
            s += list[i];
        ] else [
            s -= list[i];
        ]
    ]

    return s;
]

var int_list = {1, 2, 3 + 1, 4, 5};
var result = sample_function(int_list, 5);

var str: [char; 13] = "Hello World!";
print str;
```

## Prerequisites
- C++ compiler with C++23 support
- CMake 3.20+


## Building

```bash
git clone https://github.com/sreedevcv/JuneLang.git
cd JuneLang

mkdir build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Usage
```bash
./june [source_file.june]
```

## Acknowledgements
- Frontend(Lexer and Parser) are based on the [jlox](https://craftinginterpreters.com/introduction.html) language by [Robert Nystrom](https://craftinginterpreters.com/)