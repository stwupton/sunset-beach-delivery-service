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

## Required Libraries
- user32
- gdi32
- d3d11
- d3dcompiler
- dxgi

## Examples

### MSVC
```
cl src/main.cpp /DWIN32 /DUNICODE /I src/ /link user32.lib gdi32.lib d3d11.lib d3dcompiler.lib dxgi.lib
```

### Clang
```
clang++ -g src/main.cpp -Isrc/ -luser32 -lgdi32 -ld3d11 -ld3dcompiler -ldxgi -DUNICODE -DWIN32
```