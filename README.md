# Screen Space God Rays

(WIP) Implementation of the paper "Volumetric Light Scattering as a Post-Process" using C++20 and OpenGL 4.5.

First results with the technique on a toy scene aka "minimum viable scene":

![Toy Scene showing Screen Space God Rays](docs/images/toy-scene-1.png?raw=True)

## Features

TODO

## Gallery

TODO

## Build

### Build Instructions

This project uses CMake as build system generator. It's necessary to have it installed on your system. The external libraries will be downloaded and installed in the `build` folder during build configuration. Execute the following commands on the command-line to clone, configure and build the project:

```
# If you hadn't clone the repository yet
git clone https://github.com/math-araujo/screen-space-godrays

cd screen-space-godrays

# On Windows + MSVC
cmake --preset=default

cmake --build build --config Release

# On Unix
cmake --preset=default-unix

cmake --build build

```

## Controls

* A/S/D/W Key - Move left/backward/right/forward

* Q/E Key - Move down/up

* F Key - Switch between free FPS movement and locked FPS movement

* P Key - Switch between polygon mode and wireframe mode

* ESC Key - Exit application

* Left Mouse Button - Hold and move mouse to look-around when in locked FPS mode


## LICENSE

All the code is released under MIT License. Check the LICENSE file on this repository for details.