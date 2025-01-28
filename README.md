# GameEngineDesign
To get started with the project

Step 1: Install CMake
You can open an Administrator PowerShell or Command Prompt.
Run the following command:
- choco install cmake

Step 2: Install MSVC C++ compiler(Microsoft Visual C++):
- https://www.msys2.org/ - Follow the steps

Step 3: Install ninja
Run the following command:
- choco install ninja

Step 4: Install vcpkg

To install vcpkg package
Steps:
- In any directory of your choice clone the GitHub repo: git clone https://github.com/microsoft/vcpkg.git
- On Command Prompt use commands :
  - cd vcpkg
  - bootstrap-vcpkg.bat
  - vcpkg integrate install
- Lastly install the raylib library: vcpkg install raylib

Step 5: Open the ECSProject in VSCode.

In VSCODE press CTRL+SHIFT+P and select the C/C++:Edit Configuration(UI)

In the include path add "(your path to the vcpkg package)/vcpkg/installed/x64-windows/include"

![image](https://github.com/user-attachments/assets/2df8b996-501d-40f0-8ffc-4a688e49f621)

Step 6: Build the project on **x64 Native Tools Command Prompt for VS 2022**

To build use the following commands: 
- rmdir /S /Q build
- mkdir build
- cd build
- cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Debug ^
  -DCMAKE_TOOLCHAIN_FILE="C:/Users/Owner/Documents/GameEngineDesign/vcpkg/scripts/buildsystems/vcpkg.cmake"
- ninja

To run:
- .\bin\ECSProject.exe


