## Enchantment Art Extender

This is a framework that allows for more interesting visuals for enchanted weapons in TESV: Skyrim Special Edition. It achieves this by silently adding specific spells with hit arts. Each of these "swaps" is defined in a configuration file.

- [AE](https://www.nexusmods.com/skyrimspecialedition/mods/105492)
- [SSE](https://www.nexusmods.com/skyrimspecialedition/mods/111444)
- [VR](https://www.nexusmods.com/skyrimspecialedition/mods/116601)

## Requirements

- [CMake](https://cmake.org/)
  - Add this to your `PATH`
- [PowerShell](https://github.com/PowerShell/PowerShell/releases/latest)
- [Vcpkg](https://github.com/microsoft/vcpkg)
  - Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
- [Visual Studio Community 2019](https://visualstudio.microsoft.com/)
  - Desktop development with C++
- [CommonLibSSE](https://github.com/powerof3/CommonLibSSE/tree/dev)
  - You need to build from the powerof3/dev branch
  - Add this as as an environment variable `CommonLibSSEPath`
- [CommonLibVR](https://github.com/alandtse/CommonLibVR/tree/vr)
  - Add this as as an environment variable `CommonLibVRPath` instead of /external

## User Requirements

- [Address Library for SKSE](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
  - Needed for SSE/AE
- [VR Address Library for SKSEVR](https://www.nexusmods.com/skyrimspecialedition/mods/58101)
  - Needed for VR

## Register Visual Studio as a Generator

- Open `x64 Native Tools Command Prompt`
- Run `cmake`
- Close the cmd window

## Building

### Requirements:

- CMake
- VCPKG
- Visual Studio (with desktop C++ development)

---

### Instructions (AE):

```
git clone https://github.com/SeaSparrowOG/EnchantmentArtExtender
cd EnchantmentArtExtender
# pull submodules
git submodule update --init --recursive
cmake --preset vs2022-windows-vcpkg
cmake --build Release --config Release
```

---

### SE

```
cmake --preset vs2022-windows-vcpkg-se
cmake --build build-15 --config Release
```

### VR

```
cmake --preset vs2022-windows-vcpkg-vr
cmake --build buildvr --config Release
```

---

### Automatic deployment to MO2:

You can automatically deploy to MO2's mods folder by defining an [Environment Variable](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_environment_variables?view=powershell-7.4) named SKYRIM_MODS_FOLDER and pointing it to your MO2 mods folder. It will create a new mod with the appropriate name. After that, simply refresh MO2 and enable the mod. Image:
![Example of Environment Variable setup](https://cdn.discordapp.com/attachments/625292279468523522/1204193482600615936/Screenshot_61.png?ex=65d3d793&is=65c16293&hm=ed710c138bc02ead7ca11d85963c164feb1ea39e501ca46ffb1bac8609008473&)

## License

[APACHE-2.0](LICENSE)
