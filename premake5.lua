workspace 'SBDS'
  configurations { 'Debug', 'Release' }
  platforms { 'Win64' }
  location 'build'
  includedirs { 'src' }
  
project 'SBDS'
  kind 'WindowedApp'
  language 'C++'
  cppdialect 'C++17'
  files { 'src/main.cpp' }
  defines { 'ASSET_PATH="./assets/"' }

  filter 'configurations:Release'
    defines { 'NDEBUG' }
    optimize 'On'

  filter 'configurations:Debug'
    symbols 'On'

  filter 'action:vs*'
    defines { 'ASSET_PATH="./../assets/"' }

  filter 'platforms:Win64'
    system 'Windows'
    architecture 'x86_64'
    
  filter 'system:Windows'
    defines { 'WIN32', 'UNICODE' }
    links { 'user32', 'gdi32', 'd3d11', 'd3dcompiler', 'd2d1', 'dwrite', 'dxgi', 'Xaudio2', 'ole32' }