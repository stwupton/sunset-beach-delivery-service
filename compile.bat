@echo off
clang-cl src/main.cpp /o build/main /link user32.lib gdi32.lib d2d1.lib