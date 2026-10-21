# ECSGL

A work in progress OpenGL/Vulkan game engine using a custom entity-component system in C++. This project is purely made for my own enjoyment and for learning purposes so do not expect anything crazy documented or anything. I'm using this as a learning sandbox.

## Features

- EnTT inspired entity-component system (ECS)
- common abstraction layer for both both vulkan and OpenGL
- OBJ loading
- basic shader that can use textures or vector as color
- Dear ImGui UI integration
- Resource management system
- Configurable rendering system
- Scene graph for parent-child transformation (AND ONLY FOR THAT)
- And more to come..
## Dependencies

- [GLFW](https://www.glfw.org/)
- [GLEW](http://glew.sourceforge.net/)
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
- [Shaderc](https://github.com/google/shaderc) (included in vulkan SDK)

### Included in the repository as submodules/headers:

- [glm](https://github.com/g-truc/glm) (in `vendor/glm`)
- [Dear ImGui](https://github.com/ocornut/imgui) (in `vendor/imgui`)
- [stb_image](https://github.com/nothings/stb) (in `vendor/stb_image`)

## Build Instructions

1. make a directory where you wish your build to be outputted in
, from that directory simply run :
```bash
cmake ..
```

or with VS2026 on cmake version 4.2+
```bash
cmake .. -G "Visual Studio 18 2026"
```
