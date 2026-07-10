#pragma once
#include "Particle.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

// ---------------------------------------------------------------------------
// Renderer
//   Encapsulates ALL OpenGL state.  Owns (RAII) VAOs, VBOs, and shader
//   programs; destroys them in the destructor.
//
//   Two shader programs:
//     1. Particle program  – point sprites, additive blending, colour gradient
//     2. BBox program      – wireframe bounding-box lines
// ---------------------------------------------------------------------------

// Per-particle data uploaded to the GPU each frame
struct ParticleVertex {
    glm::vec3 position;
    glm::vec4 color;
    float     size;
};

class Renderer {
public:
    Renderer() = default;

    // Non-copyable (owns unique OpenGL resources)
    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    // Movable
    Renderer(Renderer&&)                 noexcept;
    Renderer& operator=(Renderer&&)      noexcept;

    ~Renderer();

    // Call once after an OpenGL context is current
    bool init(int maxParticles,
              const std::string& particleVert, const std::string& particleFrag,
              const std::string& bboxVert,     const std::string& bboxFrag);

    // Set bounding box geometry (call whenever it changes)
    void setBoundingBox(const glm::vec3& bmin, const glm::vec3& bmax);

    // Draw particles and bounding-box
    void draw(const std::vector<Particle>& pool,
              const glm::mat4& view,
              const glm::mat4& proj,
              float screenHeight);

private:
    // Shader helpers
    GLuint compileShader (GLenum type, const std::string& src);
    GLuint linkProgram   (GLuint vert, GLuint frag);
    bool   loadShaders   (const std::string& vertPath, const std::string& fragPath,
                          GLuint& outProgram);
    static std::string readFile(const std::string& path);

    void drawParticles(const std::vector<Particle>& pool,
                       const glm::mat4& view,
                       const glm::mat4& proj,
                       float screenHeight);

    void drawBBox(const glm::mat4& view, const glm::mat4& proj);

    // Release all GPU resources
    void release() noexcept;

    // --- OpenGL handles ---
    GLuint m_particleProgram {0};
    GLuint m_bboxProgram     {0};

    GLuint m_particleVAO     {0};
    GLuint m_particleVBO     {0};

    GLuint m_bboxVAO         {0};
    GLuint m_bboxVBO         {0};

    int    m_maxParticles    {0};

    // Staging buffer – reused every frame to avoid per-frame allocation
    std::vector<ParticleVertex> m_staging;

    bool   m_initialised     {false};
};
