# Eldengrove

## What is Eldengrove?
- Eldengrove was once a game engine that I was developing to make my own horror game (it features a 3D Voxel Engine with player physics, entities, slope blocks, water, infinite terrain generation, world saving, lighting engine, etc.)
- I initially made the base game engine from scratch in C over the last year, but this year I am developing the actual gameplay.
- During the day, you collect materials and scout nearby caves and rivers to escape to at night. At night, the world dissolves, revealing toxic grounds riddled with enemies, and the only way to escape is by cave/river and building a shelter.

## Programming Languages
- C, using OpenGL as the graphics library as opposed to a game engine (game engine made by me)
- Dependencies: GCC (MinGW-w64), OpenGL, GLEW, FreeGLUT, GLU, Assimp, MSYS2

## Problems I Had to Tackle
- Saving chunks to memory efficiently without taking up too much latency (using a dirty chunk labeling system)
- Infinite chunk generation (uses a hashmap with keys generated from terrain coordinates and using 3D Perlin noise)
- Slopes (I had to figure out how to make the player physics snap to a slope block, which proved to be a huge challenge with gravity and movement as well)
- Chunk Meshing via Greedy Meshing (Creates quads that span sections of repeating blocks to reduce stored memory)
- Using C (This was one of my first large-scale projects with C, and I had to learn a lot about OpenGL, pointers, shaders, the heap vs. stack, etc.)

## How to Run This
1. Clone the source code to your device.
```sh
git clone https://github.com/devPenguin55/Eldengrove.git
```
3. Run main.exe directly if on Windows from the Eldengrove directory. (This project is intended solely for Windows users right now).
```sh
./main.exe
```
### AI Policy
- I used AI to search for certain math functionalities and efficient implementations (3d Perlin noise and other frustum calculations).

<img width="1919" height="1076" alt="image" src="https://github.com/user-attachments/assets/9ee070f2-51fb-4ff8-aeb8-cbf3a22064b6" />
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/77c4981c-0f9f-46df-af5b-449a27841708" />
