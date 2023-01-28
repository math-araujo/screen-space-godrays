# Screen Space God Rays

| |
| :---: 
| ![Scene](docs/gifs/main.gif?raw=True) <br/> GIF showing the Sibenik cathedral with screen space god rays and shadow mapping |

This project is an implementation of the paper ["Volumetric Light Scattering as a Post-Process"](https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-13-volumetric-light-scattering-post-process) using C++20 and OpenGL 4.5. In essence, the technique consists of three rendering passes: 

1) an occlusion pre-pass renders to a framebuffer the light sources with their default colors and the occluding geometry as black;

2) the default scene is rendered as usual (this pass can actually be divided into more than one sub-pass e.g. a sub-pass to render a shadow map and a sub-pass to render the scene itself);

3) a radial blur is applied to the occlusion framebuffer, and the result is blended with the image generated at the second render pass.


| | | | |
| :---: | :---: | :---: | :---: |
| ![First Pass](docs/images/first_pass.png?raw=True) <br/> First Pass: <br/> Occlusion Map | ![Second Pass](docs/images/second_pass.png?raw=True) <br/> Second Pass: <br/> Default scene | ![Third Pass](docs/images/third_pass.png?raw=True) <br/> Third Pass: <br/> Radial Blur | ![Result](docs/images/result.png?raw=True) <br/> Third Pass: <br/> Result |

The project is configured such that the user can switch between different render modes and visualize the first, second and third render passes independently, as well as the final result, as can be seen above.

Check the [Gallery section](https://github.com/math-araujo/screen-space-godrays/#gallery) for more images/GIFs and the [Build section](https://github.com/math-araujo/screen-space-godrays/#build) for instructions on how to build the project.

## Features

* God rays as a post-processing effect to approximate volumetric light scattering, including support for multiple light sources.

* User interface provided by Dear ImGui, exposing post-processing coefficients, shadow map parameters and a switch between different render modes.

* Loading of triangle meshes from Wavefront files using `tinyobjloader`, including multiple meshes per file and meshes with multiple material files.

* Support for blending of ordered semi-transparent objects (e.g. Sibenik cathedral's windows).

* Basic directional shadow mapping, including percentage-closer filtering (PCF).

## Gallery

In the image below you can see the result of the post-processing effect on one of the main circular windows of the Sibenik cathedral:

![Circular Window with Godrays](docs/images/circular_godray.png?raw=True)

As a plus, by positioning the light source in the circular window, I liked the effect of the window's projected shadows over the cathedral floor:

![Circular Window Shadows](docs/images/circular_shadow.png?raw=True)

The image below shows the post-processing parameters that can be changed via the user interface:

![User Interface to control post-processing parameters](docs/images/gui_godray.png?raw=True)

Here's one image with the light source positioned at the top window of the cathedral:

![Tower Window with Godrays](docs/images/tower_godray.png?raw=True)


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

Feel free to open an issue should any problem arise. 

## Controls

* A/S/D/W Key - Move left/backward/right/forward

* Q/E Key - Move down/up

* F Key - Switch between free FPS movement and locked FPS movement

* P Key - Switch between polygon mode and wireframe mode

* ESC Key - Exit application

* Left Mouse Button - Hold and move mouse to look-around when in locked FPS mode


## LICENSE

All the code is released under MIT License. Check the LICENSE file on this repository for details.