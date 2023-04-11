
# 3D Raycasting Engine

CHIP-8 emulator written in C using SDL2 library. Frequency of fetch-execute cycles and delay/sound timers is 500hz and 60hz respectively. No sound implemented.

Includes a simple debugger, which shows register values, memory contents, input states and recent instructions.

Raycasting engine written in C using SDL2 library. Uses DDA algorithm to cast rays and detect collisions. Reads raw data from uncompressed BMPs as textures.


## Screenshots

![App Screenshot](https://i.imgur.com/a/Iz4rI5b)


## Requirements

- sdl2
- sdl2_image



## Building
Clone the project
```bash
  git clone https://github.com/blr-ophon/simple3DRC
```
Compile using:

```bash
  cd simple3DRC 
  make
```
## Running

Run the executable directly or use make:

```bash
  make run
```



## Usage

Use WASD to move

Use `esc` to quit 

