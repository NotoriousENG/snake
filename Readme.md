# Snake
The game snake, but you can change the game settings while playing.

![](./preview.png)

## Controls
* Moving - WASD / Arrow Keys

## Settings
* Hover over the field you want to change (Pixel Size, Game World Size, or Latency (milliseconds)) and type.
* Press Enter to apply all changes, C to clear changes or R to reset to game defaults.

## Building
The only dependency for this project is [raylib 5.x](https://github.com/raysan5/raylib?tab=readme-ov-file) 
* This guide assumes you are using a package manager to install

### Linux / MacOS

- install dependencies using package manager i.e. apt, pacman, etc. for MacOS this would be [brew](https://brew.sh/)
- note: game.so hot reloading is supported with Clang

```zsh
mkdir build
cd build
cmake ..
make
./GlGame
```

### Windows

- You can install dependencies using [vcpkg](https://github.com/microsoft/vcpkg)
- Open Project in Visual Studio (cmake support installed)
- Build
- Run snake.exe