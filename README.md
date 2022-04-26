# GD-ModMenu
Free, open-source Geometry Dash modification menu. Powered by ImGui.

# How to use
Put xinput9_1_0.dll and ModMenu.dll to the GD executable directory. If you want to add another dlls, put them to `addons` directory

# How to build
1. Clone the repository
```bash
git clone https://github.com/OneParsec/GD-ModMenu --recursive
```
2. Configure CMake
```bash
cmake -G "Visual Studio 17 2022" -B build -DCMAKE_BUILD_TYPE=Release -T host=x86 -A win32
```
3. Build
```bash
cmake --build build --config Release --target ModMenu
```
# Special thanks to:

- Absolute

- Adaf

- fig

- HJFod

- matcool

- TobyAdd
