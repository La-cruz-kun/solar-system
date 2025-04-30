# Solar System Simulator with Galactic Background

![Solar System Demo](screenshots/demo.gif) <!-- Replace with actual screenshot path -->

A 3D solar system simulation built with OpenGL (C), featuring realistic planetary orbits, scaled celestial bodies, and an immersive 360° galaxy/stars background.

## Features

- 🪐 **3D Planetary System**
  - Sun with procedural surface texture
  - Planets with relative sizes and orbital periods
  - Configurable orbital tilt and rotation speeds
- 🌌 **360° Galaxy Background**
  - Equirectangular texture mapping (HDR/JPG supported)

## Prerequisites

- OpenGL 3.3+ compatible GPU
- C compiler (GCC/MSVC)
- CMake 3.12+
- GLFW
- GLAD
- STB Libraries (included)

## Installation

On Linux
```bash
# Clone repository
git clone https://github.com/yourusername/opengl-solar-system.git
cd opengl-solar-system

# compile
clang main.c include/stb.c include/shader_s.c-o main -lglad -lglfw -lGL -lm -lcglm

# then run
./main
