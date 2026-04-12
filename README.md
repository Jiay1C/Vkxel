# <p align="center">Vkxel</p>

<p align="center">
  <strong>An experimental C++20 / Vulkan 1.3 engine prototype for signed-distance fields, dual contouring, and editor-first procedural rendering workflows.</strong>
</p>

<p align="center">
  <a href="https://github.com/Jiay1C/Vkxel/actions/workflows/build.yml"><img src="https://github.com/Jiay1C/Vkxel/actions/workflows/build.yml/badge.svg" alt="Build status"></a>
  <img src="https://img.shields.io/badge/C%2B%2B-20-00599C?logo=c%2B%2B&logoColor=white" alt="C++20">
  <img src="https://img.shields.io/badge/Vulkan-1.3-A41E22?logo=vulkan&logoColor=white" alt="Vulkan 1.3">
  <img src="https://img.shields.io/badge/Platform-Windows-0078D6?logo=windows&logoColor=white" alt="Windows">
  <img src="https://img.shields.io/badge/Platform-macOS-111111?logo=apple&logoColor=white" alt="macOS">
  <a href="./LICENSE"><img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="MIT License"></a>
</p>

<p align="center">
  <a href="#downloads">Downloads</a> &middot;
  <a href="#overview">Overview</a> &middot;
  <a href="#build-and-run">Build</a> &middot;
  <a href="#controls">Controls</a> &middot;
  <a href="#repository-layout">Layout</a> &middot;
  <a href="#license">License</a>
</p>

## Downloads

| Windows                                                                                                          | macOS                                                                                                          |
|------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------|
| [Release](https://nightly.link/Jiay1C/Vkxel/workflows/build/main/Vkxel-Release-windows-latest.zip)               | [Release](https://nightly.link/Jiay1C/Vkxel/workflows/build/main/Vkxel-Release-macos-latest.zip)               |
| [RelWithDebInfo](https://nightly.link/Jiay1C/Vkxel/workflows/build/main/Vkxel-RelWithDebInfo-windows-latest.zip) | [RelWithDebInfo](https://nightly.link/Jiay1C/Vkxel/workflows/build/main/Vkxel-RelWithDebInfo-macos-latest.zip) |
| [Debug](https://nightly.link/Jiay1C/Vkxel/workflows/build/main/Vkxel-Debug-windows-latest.zip)                   | [Debug](https://nightly.link/Jiay1C/Vkxel/workflows/build/main/Vkxel-Debug-macos-latest.zip)                   |

## Overview

Vkxel combines a small component-based runtime with an editor-style UI so you can inspect scenes, author procedural
content, and iterate on rendering experiments from inside the app.

Today, the project includes:

- A scene graph and component-driven world model
- ImGui-powered editor panels for scene browsing and inspection
- Reflection-backed inspector editing for supported component data
- A Slang shader pipeline that compiles to SPIR-V at runtime
- CPU dual contouring over `SDFSurface` primitives and CSG combinations
- An experimental GPU dual contouring path driven by Vulkan compute shaders

CI currently builds the project on Windows and macOS across `Debug`, `RelWithDebInfo`, and `Release`.

## Build And Run

### Prerequisites

- Git with submodule support
- CMake `3.25+`
- Ninja
- A C++20 compiler
- A working Vulkan environment and compatible GPU driver/runtime

Platform notes:

- **Windows:** MSVC is required by the CMake configuration.
- **macOS:** Clang is required, and the project currently targets `arm64`.

Clone with recursive submodules:

```bash
git clone --recurse-submodules https://github.com/Jiay1C/Vkxel.git
cd Vkxel
```

If you already cloned the repository without submodules:

```bash
git submodule update --init --recursive
```

Build with CMake:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target Vkxel
```

After the build finishes, launch the executable from the generated output directory:

- Windows: `build/Vkxel/bin/Vkxel.exe`
- macOS: `build/Vkxel/bin/Vkxel`

During the build, the `shader/` directory is copied next to the executable. On first use, Slang shaders are compiled to
SPIR-V and cached as `.spv` files beside their source in the runtime shader directory.

## Controls

The current camera controller uses the following inputs:

| Input                 | Action                             |
|-----------------------|------------------------------------|
| `W` / `A` / `S` / `D` | Move forward / left / back / right |
| `Q` / `E`             | Move down / up                     |
| `Shift`               | Accelerate movement                |
| Right mouse drag      | Rotate camera                      |

## Repository Layout

Vkxel keeps most of its project-specific code under `source/`, with rendering and shader code split into focused
modules:

| Path            | Purpose                                                                                          |
|-----------------|--------------------------------------------------------------------------------------------------|
| `source/engine` | Windowing, input, renderer setup, GUI integration, timing, resources, and compute infrastructure |
| `source/editor` | Editor-facing panels such as the scene tree and inspector UI                                     |
| `source/world`  | Scene, game object, transform, camera, drawer, mesh, and gameplay-style components               |
| `source/custom` | Procedural surface logic, CPU dual contouring, and experimental GPU dual contouring              |
| `source/entry`  | App entrypoint plus the demo scene and model library setup                                       |
| `shader`        | Slang source files for graphics, compute, SDF helpers, and utility math                          |

The repository also vendors most third-party dependencies through `external/`.

## License

This project is licensed under the [MIT License](./LICENSE).
