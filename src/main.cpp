//=============================================================================
// ECE 4122/6122  –  Final Project
// 3D Particle Simulation with OpenGL
//=============================================================================
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <filesystem>

#include "SimulationConfig.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include "Camera.h"
#include "Renderer.h"

// ---------------------------------------------------------------------------
// Globals needed by static GLFW callbacks
// ---------------------------------------------------------------------------
static Camera*          g_camera   = nullptr;
static ParticleEmitter* g_emitter  = nullptr;
static bool             g_paused   = false;

// ---------------------------------------------------------------------------
// GLFW callbacks
// ---------------------------------------------------------------------------
static void onError(int, const char* msg) {
    std::cerr << "[GLFW] " << msg << "\n";
}

static void onKey(GLFWwindow* win, int key, int /*scancode*/, int action, int /*mods*/) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(win, GL_TRUE);

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        g_paused = !g_paused;

    // 1/2/3 — select emitter type directly
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_1) g_emitter->setEmitterType(EmitterType::Fountain);
        if (key == GLFW_KEY_2) g_emitter->setEmitterType(EmitterType::Explosion);
        if (key == GLFW_KEY_3) g_emitter->setEmitterType(EmitterType::Vortex);
        if (key == GLFW_KEY_TAB) g_emitter->cycleEmitterType();
    }

    if (g_camera) g_camera->onKey(key, action);
}

static void onMouseButton(GLFWwindow* /*win*/, int btn, int action, int /*mods*/) {
    if (g_camera) g_camera->onMouseButton(btn, action);
}

static void onMouseMove(GLFWwindow* /*win*/, double x, double y) {
    if (g_camera) g_camera->onMouseMove(x, y);
}

static void onScroll(GLFWwindow* /*win*/, double /*x*/, double y) {
    if (g_camera) g_camera->onScroll(y);
}

static void onFramebuffer(GLFWwindow* /*win*/, int w, int h) {
    glViewport(0, 0, w, h);
}

// ---------------------------------------------------------------------------
// Shader / config path helpers (relative to executable location)
// ---------------------------------------------------------------------------
static std::string resolvePath(const std::string& rel) {
    // Try CWD first, then one directory up (handles build/ subdirectory)
    if (std::filesystem::exists(rel)) return rel;
    auto up = std::filesystem::path("..") / rel;
    if (std::filesystem::exists(up)) return up.string();
    return rel; // return as-is; error will be caught at load time
}

// ---------------------------------------------------------------------------
// Frame-rate display helper
// ---------------------------------------------------------------------------
static void updateTitle(GLFWwindow* win, const SimulationConfig& cfg,
                        int active, const std::string& emitterName,
                        double& lastTime, int& frames)
{
    double now = glfwGetTime();
    ++frames;
    if (now - lastTime >= 0.5) {
        double fps = frames / (now - lastTime);
        lastTime = now;
        frames   = 0;

        std::ostringstream ss;
        ss << cfg.windowTitle
           << "  |  " << std::fixed << std::setprecision(1) << fps << " FPS"
           << "  |  " << active << " particles"
           << "  |  Mode: " << emitterName
           << "  |  [TAB] cycle  [SPACE] pause  [R] reset cam";
        glfwSetWindowTitle(win, ss.str().c_str());
    }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
    // --- Load config --------------------------------------------------------
    SimulationConfig cfg;
    cfg.load(resolvePath("config/simulation.ini"));

    // --- GLFW ---------------------------------------------------------------
    glfwSetErrorCallback(onError);
    if (!glfwInit()) { std::cerr << "glfwInit failed\n"; return 1; }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(
        cfg.windowWidth, cfg.windowHeight, cfg.windowTitle.c_str(), nullptr, nullptr);
    if (!window) { std::cerr << "glfwCreateWindow failed\n"; glfwTerminate(); return 1; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);  // Disable VSync for uncapped FPS measurement

    // --- GLEW ---------------------------------------------------------------
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { std::cerr << "glewInit failed\n"; return 1; }

    std::cout << "[OpenGL] Version: " << glGetString(GL_VERSION)
              << "  Renderer: " << glGetString(GL_RENDERER) << "\n";

    // --- OpenGL global state ------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    // --- Create simulation objects ------------------------------------------
    Camera          camera;
    ParticleEmitter emitter(cfg);
    Renderer        renderer;

    // Register globals for callbacks
    g_camera  = &camera;
    g_emitter = &emitter;

    // Camera: orbit around the midpoint of the bounding box
    glm::vec3 centre = (cfg.bboxMin + cfg.bboxMax) * 0.5f;
    float     extent = glm::length(cfg.bboxMax - cfg.bboxMin);
    camera.init(extent * 1.1f, -90.f, 20.f, centre);

    // Renderer
    if (!renderer.init(cfg.maxParticles,
                       resolvePath("shaders/particle.vert"),
                       resolvePath("shaders/particle.frag"),
                       resolvePath("shaders/bbox.vert"),
                       resolvePath("shaders/bbox.frag")))
    {
        std::cerr << "Renderer initialization failed.\n";
        glfwTerminate();
        return 1;
    }
    renderer.setBoundingBox(cfg.bboxMin, cfg.bboxMax);

    // --- Register callbacks -------------------------------------------------
    glfwSetKeyCallback          (window, onKey);
    glfwSetMouseButtonCallback  (window, onMouseButton);
    glfwSetCursorPosCallback    (window, onMouseMove);
    glfwSetScrollCallback       (window, onScroll);
    glfwSetFramebufferSizeCallback(window, onFramebuffer);

    // Print controls
    std::cout << "\n=== Controls ===\n"
              << "  Left-drag   : Orbit camera\n"
              << "  Right-drag  : Pan camera\n"
              << "  Scroll      : Zoom\n"
              << "  R           : Reset camera\n"
              << "  1/2/3       : Fountain / Explosion / Vortex\n"
              << "  TAB         : Cycle emitter type\n"
              << "  SPACE       : Pause/resume\n"
              << "  ESC         : Quit\n\n";

    // --- Main loop ----------------------------------------------------------
    double prevTime  = glfwGetTime();
    double titleTime = prevTime;
    int    titleFrames = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        double now = glfwGetTime();
        float  dt  = static_cast<float>(now - prevTime);
        prevTime   = now;
        dt = glm::clamp(dt, 0.f, 0.05f); // cap at 50 ms to prevent explosion on stalls

        // Physics
        if (!g_paused)
            emitter.update(dt);

        // Render
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        float aspect = (fbH > 0) ? static_cast<float>(fbW) / fbH : 1.f;

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 proj = camera.getProjectionMatrix(aspect);

        renderer.draw(emitter.particles(), view, proj, static_cast<float>(fbH));

        glfwSwapBuffers(window);

        updateTitle(window, cfg, emitter.activeCount(),
                    emitter.emitterTypeName(), titleTime, titleFrames);
    }

    // Cleanup
    g_camera  = nullptr;
    g_emitter = nullptr;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
