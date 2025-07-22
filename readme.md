A sandbox game using C++20 and SFML 2.5.1

Built with gcc (12.2.0) and CMake (3.25.1) on Debian GNU/Linux 12

Use:
- Left Mouse Button to place particles and choose particle type
- Right Mouse Button to remove particle
- Z and X to scale down/up (also possible by grabbing window edges)
- Down and Up to regulate brush size
- Esc to close

Prequisitions:

Make sure that GCC is installed:

Debian/Ubuntu:
```bash
sudo apt-get install build-essential
```

Install SFML:

Debian/Ubuntu:
```bash
sudo apt-get install sudo apt-get install libsfml-dev
```

How to run the app:

1: download and extract anywhere (avoid non-latin letters in the path)

2:

3:
a:
VSCode:
Make sure that CMake and C++ extensions are installed
Open the project directory from within the app (ctrl+k ctrl+o) and build using cmake 
Run with CTRL+SHIFT+F5

b:
Linux:

Make your way into the project directory
Run this command while inside the directory:

```sh
cmake CMakeLists.txt
```

then run the app via 
```sh
./build/sandbox
```

Example:
![alt text](example.png)
