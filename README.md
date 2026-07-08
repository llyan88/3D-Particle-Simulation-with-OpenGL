# 3D-Particle-Simulation
A real-time 3D particle system built with **OpenGL 3.3 Core Profile**, **C++17**,
**GLFW**, **GLEW**, **GLM**, and **OpenMP**.

## Build
mkdir build && cd build 
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

## Run
Open terminal in /build/bin
./FinalProject

## Controls
  Left-drag   : Orbit camera
  Right-drag  : Pan camera
  Scroll      : Zoom
  R           : Reset camera
  1/2/3       : Fountain / Explosion / Vortex
  TAB         : Cycle emitter type
  SPACE       : Pause/resume
  ESC         : Quit

## Implemented features
Object-oriented architecture with separate classes for:
  - `Particle`
  - `ParticleEmitter`
  - `Renderer`
  - `Camera`
  - `SimulationConfig`
Three runtime-selectable emitter modes:
  - Fountain
  - Cyclic Explosion
  - Vortex
Configurable gravity applied to all active particles
Axis-aligned bounding-box collision handling on all six faces
Additional floor friction/damping applied after floor bounces
Configurable OpenMP thread count from the simulation configuration file

