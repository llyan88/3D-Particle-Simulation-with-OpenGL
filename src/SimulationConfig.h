#pragma once
#include <string>
#include <glm/glm.hpp>

// ---------------------------------------------------------------------------
// SimulationConfig
//   Plain-data container loaded from config/simulation.ini at startup.
//   Provides sensible defaults so the program runs without a config file.
// ---------------------------------------------------------------------------
struct SimulationConfig {
    // Window
    int    windowWidth   = 1280;
    int    windowHeight  = 720;
    std::string windowTitle = "ECE 4122_6122 – 3D Particle Simulation";

    // Simulation
    int    maxParticles  = 30000;
    float  spawnRate     = 2000.f;   // particles per second
    int    numThreads    = 4;

    // Physics
    glm::vec3 gravity    = {0.f, -9.81f, 0.f};
    float  restitution   = 0.65f;    // coefficient of restitution [0,1]

    // Emitter
    // type: 0 = fountain, 1 = explosion (cyclic), 2 = vortex
    int       emitterType = 0;
    glm::vec3 emitterPos  = {0.f, -4.5f, 0.f};

    // Bounding box
    glm::vec3 bboxMin = {-8.f, -5.f, -8.f};
    glm::vec3 bboxMax = { 8.f, 10.f,  8.f};

    // Load from file; silently uses defaults if file is missing
    void load(const std::string& path);
};
