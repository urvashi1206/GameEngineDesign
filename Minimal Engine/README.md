# Minimal Engine

Minimal Engine is a lightweight engine project that leverages powerful libraries
like [GLFW](https://github.com/glfw/glfw/releases), [GLM](https://github.com/g-truc/glm/releases/latest), and
the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/). This guide provides step-by-step instructions to set up and build the engine.

## Requirements

- **CMake:** Ensure you have CMake installed on your system.
- **GLFW:** Used for handling windowing and input.
- **GLM:** Provides mathematical operations for graphics programming.
- **Vulkan SDK:** Required for Vulkan API support.

## Installation Instructions

### A] Install GLFW

1. **Download GLFW:**  
   Download the GLFW archive from the [GLFW 3.4 release](https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN64.zip).
2. **Unzip:**  
   Unzip the downloaded file and place the extracted folder in a convenient location.

### B] Install GLM

1. **Download GLM:**  
   Visit the [GLM Latest Release](https://github.com/g-truc/glm/releases/latest) page and download the "source code (zip)" file.
2. **Unzip:**  
   Unzip the file and place the GLM folder in a convenient location.

### C] Install Vulkan SDK

1. **Download Vulkan SDK:**  
   Download the Vulkan SDK installer from the [LunarG Vulkan SDK](https://www.lunarg.com/vulkan-sdk/).
2. **Install:**  
   Run the installer and follow the provided instructions to install the Vulkan SDK.

### D] Create the `.env.cmake` File

1. **Copy Example File:**  
   Make a copy of either `envWindowsExample.cmake` (or the corresponding example file for your platform).
2. **Configure Paths:**  
   Replace the paths for GLFW, GLM, and the Vulkan SDK in the copied file.
3. **Rename:**  
   Rename the configured file to `.env.cmake`.

### E] Build the Engine

1. Build the Minimal Engine by executing:
   `[Path to CMake]` --build `[Path to build output]` --target MinimalEngine -j 30
2. Copy the models and shaders folders from the `MinimalEngine` folder to the `Minimal Engine` folder.
3. After a successful build, run the executable `Minimal Engine.exe` to launch the engine.

## Usage

After following the installation and build steps, simply run the engine executable to start using Minimal Engine.

## Contributing

Contributions are welcome! Please follow our contribution guidelines to submit issues or pull requests.

## License

MIT License

## Acknowledgements

- [GLFW](https://www.glfw.org/)
- [GLM](https://github.com/g-truc/glm)
- [Vulkan SDK](https://www.vulkan.org/)
