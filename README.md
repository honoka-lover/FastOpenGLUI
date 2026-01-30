# FastOpenGLUI

**FastOpenGLUI** is a lightweight C++ UI library built on **GLFW** and **GLAD**, designed to provide a simple, extensible, and reusable foundation for building UI components in OpenGL-based applications.

It is ideal for creating static UI components, tool windows, debugging interfaces, and more, with a modern CMake build system.

---

## 🚀 Features

- 🎨 Built on GLFW and GLAD for cross-platform window and OpenGL management
- 🧱 Provides static UI component support
- 🛠 Modern CMake + FetchContent for dependency management
- 📦 Modular and extensible C++ UI architecture
- 🐲 Suitable for rapid prototyping of tools and debug UIs

---

## 🧩 Dependencies

FastOpenGLUI uses the following third-party libraries:

- **GLFW** — windowing and input library
- **GLAD** — OpenGL function loader
- **GLM** — mathematics library
- **FreeType** — font rendering
- **bit7z** — a lightweight C++ wrapper around 7-Zip libraries for reading and extracting archive files
> All dependencies can be automatically downloaded using CMake's `FetchContent` module, so no manual installation is required.

---

## 🧠 Build Instructions

Requires **CMake >= 3.16**, **MSVC (Visual Studio)**, and **Ninja**. Example build on a typical Windows system:

```bash
git clone https://github.com/honoka-lover/FastOpenGLUI.git
cd FastOpenGLUI
mkdir build && cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
ninja
```


---
## 📄 License

FastOpenGLUI is licensed under the **Apache License 2.0**.  
See the [LICENSE](./LICENSE) file for full details.


---

## Third-Party Licenses

This project uses the following third-party libraries:

1. **GLM** (version 0.9.9.8)
    - License: MIT License
    - Repository: https://github.com/g-truc/glm
    - MIT License requires preservation of copyright notice

2. **GLFW** (version 3.3.9)
    - License: zlib/libpng License
    - Repository: https://www.glfw.org
    - zlib/libpng License requires acknowledgement in documentation

3. **GLAD**
    - License: MIT License
    - Repository: https://github.com/Dav1dde/glad

4. **FreeType**
    - License: FreeType License (FTL) or GPLv2 (we use FTL)
    - Repository: https://github.com/freetype/freetype

5. **bit7z**
    - License: MPL 2.0
    - Repository: https://github.com/rikyoz/bit7z



