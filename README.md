# GD-ModMenu
Free, open-source Geometry Dash modification menu. Powered by ImGui.

Press `INSERT` key to call mod menu.

# How to use
Firstly, download DLLs from release page.
Then, put `xinput9_1_0.dll` and `ModMenu.dll` to the GD executable directory. If you want to add other DLLs, put them in `addons` directory.

# How to build

You will need Visual Studio (2019 or 2022), CMake and Git.

1. Clone the repository
```bash
git clone https://github.com/OneParsec/GD-ModMenu --recursive
```

2. Change directory
```bash
cd GD-ModMenu
```

3. Configure CMake
```bash
cmake -G "Visual Studio 17 2022" -B build -DCMAKE_BUILD_TYPE=Release -T host=x86 -A win32
```
*Note: If you are using VS 2019, enter "Visual Studio 16 2019" instead of "Visual Studio 17 2022".*

4. Build
```bash
cmake --build build --config Release --target ModMenu
```
# Special thanks to:

- [Absolute](https://github.com/absoIute)
- [Adaf](https://github.com/adafcaefc)
- [fig](https://github.com/FigmentBoy)
- [HJFod](https://github.com/HJfod)
- [matcool](https://github.com/matcool)
- [TobyAdd](https://github.com/TobyAdd)
- [Pixelsuft](https://github.com/Pixelsuft/)

# License
```
MIT License
Copyright (c) 2022 Alexander Simonov
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
