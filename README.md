# ECSGL

A simple OpenGL-based game engine using a custom entity-component system in C++. This project is purely made for my own enjoyment.

## Features

- EnTT inspired entity-component system (ECS)
- OpenGL rendering pipeline
- OBJ loading
- basic shader that can use textures or vector as color
- Dear ImGui UI integration
- Resource management system
- Configurable rendering system
- Scene graph for parent-child transformation (AND ONLY FOR THAT)
- And more to come..
## Dependencies

### Installed via [vcpkg](https://github.com/microsoft/vcpkg):

- [GLFW](https://www.glfw.org/)
- [GLEW](http://glew.sourceforge.net/)

### Included in the repository:

- [glm](https://github.com/g-truc/glm) (in `vendor/glm`)
- [Dear ImGui](https://github.com/ocornut/imgui) (in `vendor/imgui`)
- [stb_image](https://github.com/nothings/stb) (in `vendor/stb_image`)

## Build Instructions

1. Clone the repository:

```bash
git clone https://github.com/Guarded0/ECSGL.git
cd ECSGL
```
2. Build the project (THROUGH VS STUDIO 2022 RECOMMENDED)
   - use cmake + vcpkg that comes with visual studio or your own
   - create your own build dir if needed
```bash
cmake --preset x64-debug
cmake --build buildDir
```
