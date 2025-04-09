# EngineGL

A simple OpenGL-based game engine framework using a custom entity-component system in C++.

## Features

- EnTT inspired entity-component system (ECS)
- OpenGL rendering pipeline
- Assimp model loading
- stb_image texture loading
- Dear ImGui UI integration
- Resource management system
- Configurable rendering system
- And more to come..
## Dependencies

### Installed via [vcpkg](https://github.com/microsoft/vcpkg):

- [GLFW](https://www.glfw.org/)
- [GLEW](http://glew.sourceforge.net/)
- [Assimp](https://github.com/assimp/assimp)

### Included in the repository:

- [glm](https://github.com/g-truc/glm) (in `vendor/glm`)
- [Dear ImGui](https://github.com/ocornut/imgui) (in `vendor/imgui`)
- [stb_image](https://github.com/nothings/stb) (in `vendor/stb_image`)

## Build Instructions

1. Clone the repository:

```bash
git clone https://github.com/Guarded0/EngineGL.git
cd EngineGL
