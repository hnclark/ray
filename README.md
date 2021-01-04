# Top-down orthographic renderer
A simple top-down orthographic renderer. Currently supported:
* Meshes with multiple raytracing acceleration structures
* Multiple instances of one mesh with independent movement
* Multiple light sources

## Building
Run ```make``` in the root directory. Requires LibSDL2 and OpenMP to build.

This may not work well on a Windows machine, I have only built it on Linux.

## Running
```./main```

## Controls
WASD + QE to move the camera in X, Y, and Z dimensions(changing Z dimension only adjusts the clipping plane on orthographic mode)
Esc to exit
