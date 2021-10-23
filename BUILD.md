# Overview

Only `src/main.cpp` is compiled and `#include`'s all other relating C++ source files (known as a unity build). 

The entry point is the platform layer which:
- creates the window 
- handles rendering 
- handles sound 
- handles input
- manages the game loop
- makes calls to the game code

# Building for Windows

## Required Definitions
- WIN32
- UNICODE
- _USE_MATH_DEFINES

## Required Libraries
- user32
- gdi32
- d3d11
- d3dcompiler
- d2d1
- dwrite
- dxgi
- ole32

## Examples

### MSVC
```
cl src/main.cpp /DWIN32 /DUNICODE /D_USE_MATH_DEFINES /DDEBUG /I src/ /link user32.lib gdi32.lib d3d11.lib ole32.lib d3dcompiler.lib d2d1.lib dwrite.lib dxgi.lib
```

### Clang
```
clang++ -g src/main.cpp -Isrc/ -luser32 -lgdi32 -ld3d11 -ld3dcompiler -ld2d1 -ldwrite -ldxgi -lole32 -DUNICODE -DWIN32
```