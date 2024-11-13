## Building
### Requirements:
- CMake
- VCPKG
- Visual Studio (with desktop C++ development)
---
### Instructions:
```
git clone https://github.com/SeaSparrowOG/SKSE-Plugin-Template
cd SKSE-Plugin-Template
git submodule init
git submodule update --recursive
cmake --preset vs2022-windows-vcpkg 
cmake --build Release --config Release
```
---
### Automatic deployment to MO2:
You can automatically deploy to MO2's mods folder by defining an [Environment Variable](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_environment_variables?view=powershell-7.4) named SKYRIM_MODS_FOLDER and pointing it to your MO2 mods folder. It will create a new mod with the appropriate name. After that, simply refresh MO2 and enable the mod.
