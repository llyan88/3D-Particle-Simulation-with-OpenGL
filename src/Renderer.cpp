#include "Renderer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <utility>
#include <array>
#include <cstddef>
#include <glm/gtc/type_ptr.hpp>

// =============================================================================
// Renderer.cpp  –  ECE 4122/6122 Final Project  (STUDENT SKELETON)
// =============================================================================
// The Renderer encapsulates ALL OpenGL state for the simulation.
// You must implement the functions marked TODO below.
//
// The class owns two shader programs (particle and bounding-box) and two
// VAO/VBO pairs.  All GPU resources are freed in the destructor via release().
//
// OpenGL concepts used here:
//   VAO  – Vertex Array Object: remembers attribute layout
//   VBO  – Vertex Buffer Object: stores geometry on the GPU
//   Shader program: compiled GLSL vertex + fragment shader pair
//   glBufferData / glBufferSubData: upload data to a VBO
//   glVertexAttribPointer: describe one attribute within a VBO
//   glDrawArrays(GL_POINTS, …): draw one point per vertex
//   glDrawArrays(GL_LINES,  …): draw line segments (two vertices per edge)
//
// The header (Renderer.h) is already complete – do NOT modify it.
// =============================================================================


// ---------------------------------------------------------------------------
// Move constructor / move-assignment / destructor
//   These are provided for you – they handle RAII ownership transfer.
//   You do NOT need to modify them, but read them to understand the pattern.
// ---------------------------------------------------------------------------
Renderer::Renderer(Renderer&& o) noexcept
    : m_particleProgram(o.m_particleProgram),
      m_bboxProgram    (o.m_bboxProgram),
      m_particleVAO   (o.m_particleVAO),
      m_particleVBO   (o.m_particleVBO),
      m_bboxVAO       (o.m_bboxVAO),
      m_bboxVBO       (o.m_bboxVBO),
      m_maxParticles  (o.m_maxParticles),
      m_staging       (std::move(o.m_staging)),
      m_initialised   (o.m_initialised)
{
    o.m_particleProgram = o.m_bboxProgram = 0;
    o.m_particleVAO = o.m_particleVBO = 0;
    o.m_bboxVAO     = o.m_bboxVBO    = 0;
    o.m_initialised = false;
}

Renderer& Renderer::operator=(Renderer&& o) noexcept {
    if (this != &o) {
        release();
        m_particleProgram = o.m_particleProgram;
        m_bboxProgram     = o.m_bboxProgram;
        m_particleVAO     = o.m_particleVAO;
        m_particleVBO     = o.m_particleVBO;
        m_bboxVAO         = o.m_bboxVAO;
        m_bboxVBO         = o.m_bboxVBO;
        m_maxParticles    = o.m_maxParticles;
        m_staging         = std::move(o.m_staging);
        m_initialised     = o.m_initialised;
        o.m_particleProgram = o.m_bboxProgram = 0;
        o.m_particleVAO = o.m_particleVBO = 0;
        o.m_bboxVAO     = o.m_bboxVBO    = 0;
        o.m_initialised = false;
    }
    return *this;
}

Renderer::~Renderer() { release(); }


// ---------------------------------------------------------------------------
// release()  –  PROVIDED: frees all GPU resources
// ---------------------------------------------------------------------------
void Renderer::release() noexcept {
    if (m_particleVBO) { glDeleteBuffers(1, &m_particleVBO); m_particleVBO = 0; }
    if (m_particleVAO) { glDeleteVertexArrays(1, &m_particleVAO); m_particleVAO = 0; }
    if (m_bboxVBO)     { glDeleteBuffers(1, &m_bboxVBO); m_bboxVBO = 0; }
    if (m_bboxVAO)     { glDeleteVertexArrays(1, &m_bboxVAO); m_bboxVAO = 0; }
    if (m_particleProgram) { glDeleteProgram(m_particleProgram); m_particleProgram = 0; }
    if (m_bboxProgram)     { glDeleteProgram(m_bboxProgram);     m_bboxProgram     = 0; }
    m_initialised = false;
}


// ---------------------------------------------------------------------------
// Renderer::init
//   Call once after an OpenGL context is current.
//   Returns true on success, false on any shader/buffer error.
//
//   Steps:
//     1. Store maxParticles; reserve m_staging.
//     2. Load the particle shader pair  → m_particleProgram
//     3. Load the bbox shader pair      → m_bboxProgram
//     4. Create the PARTICLE VAO/VBO:
//          glGenVertexArrays / glGenBuffers
//          glBindVertexArray, glBindBuffer(GL_ARRAY_BUFFER, …)
//          glBufferData with size = maxParticles * sizeof(ParticleVertex),
//                         data = nullptr, usage = GL_STREAM_DRAW
//          Set up three vertex attributes using glVertexAttribPointer:
//            location 0 – vec3  aPosition  (offset of ParticleVertex::position)
//            location 1 – vec4  aColor     (offset of ParticleVertex::color)
//            location 2 – float aSize      (offset of ParticleVertex::size)
//          Unbind VAO and VBO.
//     5. Create the BBOX VAO/VBO:
//          24 vec3 vertices (12 edges × 2 endpoints), usage = GL_DYNAMIC_DRAW
//          One attribute: location 0 – vec3 (full stride = sizeof(glm::vec3))
//          Unbind.
//     6. Set m_initialised = true, print "[Renderer] Initialised OK."
// ---------------------------------------------------------------------------
bool Renderer::init(int maxParticles,
                    const std::string& pVert, const std::string& pFrag,
                    const std::string& bVert, const std::string& bFrag)
{
    m_maxParticles = maxParticles;
    m_staging.reserve(maxParticles);

    // TODO: load particle shaders  → m_particleProgram
    if (!loadShaders(pVert, pFrag, m_particleProgram)) {
        release();
        return false;
    }

    // TODO: load bbox shaders      → m_bboxProgram
    if (!loadShaders(bVert, bFrag, m_bboxProgram)) {
        release();
        return false;
    }

    // TODO: create particle VAO/VBO and define three vertex attributes
    glGenVertexArrays(1, &m_particleVAO);
    glGenBuffers(1, &m_particleVBO);

    glBindVertexArray(m_particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_particleVBO);

    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_maxParticles * sizeof(ParticleVertex)),
                 nullptr,
                 GL_STREAM_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(ParticleVertex),
                          reinterpret_cast<void*>(offsetof(ParticleVertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          4,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(ParticleVertex),
                          reinterpret_cast<void*>(offsetof(ParticleVertex, color)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,
                          1,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(ParticleVertex),
                          reinterpret_cast<void*>(offsetof(ParticleVertex, size)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // TODO: create bbox VAO/VBO (24 vec3 vertices, GL_DYNAMIC_DRAW)
    glGenVertexArrays(1, &m_bboxVAO);
    glGenBuffers(1, &m_bboxVBO);

    glBindVertexArray(m_bboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_bboxVBO);

    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(24 * sizeof(glm::vec3)),
                 nullptr,
                 GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(glm::vec3),
                          reinterpret_cast<void*>(0));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // TODO: set m_initialised = true and print status
    m_initialised = true;
    std::cout << "[Renderer] Initialised OK.\n";
    return true;
}


// ---------------------------------------------------------------------------
// Renderer::setBoundingBox
//   Build the 24 edge-endpoint vertices (12 edges of an AABB) and upload
//   them to the bbox VBO with glBufferSubData.
//
//   The 12 edges (bottom face, top face, 4 vertical pillars) as pairs:
//     Bottom: (mn.x,mn.y,mn.z)↔(mx.x,mn.y,mn.z), (mx.x,mn.y,mn.z)↔(mx.x,mn.y,mx.z),
//             (mx.x,mn.y,mx.z)↔(mn.x,mn.y,mx.z), (mn.x,mn.y,mx.z)↔(mn.x,mn.y,mn.z)
//     Top:    same pattern at my = mx.y
//     Pillars: each bottom corner connected to the corresponding top corner
// ---------------------------------------------------------------------------
void Renderer::setBoundingBox(const glm::vec3& mn, const glm::vec3& mx)
{
    // TODO: build std::array<glm::vec3, 24> verts with all 12 edge pairs
    std::array<glm::vec3, 24> verts = {
        // Bottom face
        glm::vec3{mn.x, mn.y, mn.z}, glm::vec3{mx.x, mn.y, mn.z},
        glm::vec3{mx.x, mn.y, mn.z}, glm::vec3{mx.x, mn.y, mx.z},
        glm::vec3{mx.x, mn.y, mx.z}, glm::vec3{mn.x, mn.y, mx.z},
        glm::vec3{mn.x, mn.y, mx.z}, glm::vec3{mn.x, mn.y, mn.z},

        // Top face
        glm::vec3{mn.x, mx.y, mn.z}, glm::vec3{mx.x, mx.y, mn.z},
        glm::vec3{mx.x, mx.y, mn.z}, glm::vec3{mx.x, mx.y, mx.z},
        glm::vec3{mx.x, mx.y, mx.z}, glm::vec3{mn.x, mx.y, mx.z},
        glm::vec3{mn.x, mx.y, mx.z}, glm::vec3{mn.x, mx.y, mn.z},

        // Pillars
        glm::vec3{mn.x, mn.y, mn.z}, glm::vec3{mn.x, mx.y, mn.z},
        glm::vec3{mx.x, mn.y, mn.z}, glm::vec3{mx.x, mx.y, mn.z},
        glm::vec3{mx.x, mn.y, mx.z}, glm::vec3{mx.x, mx.y, mx.z},
        glm::vec3{mn.x, mn.y, mx.z}, glm::vec3{mn.x, mx.y, mx.z}
    };

    // TODO: glBindBuffer / glBufferSubData / glBindBuffer(0)
    glBindBuffer(GL_ARRAY_BUFFER, m_bboxVBO);
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    static_cast<GLsizeiptr>(verts.size() * sizeof(glm::vec3)),
                    verts.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// ---------------------------------------------------------------------------
// Renderer::draw
//   Main per-frame render call.
//
//   Steps:
//     1. Guard: return if !m_initialised.
//     2. Clear: glClearColor(0.02, 0.02, 0.06, 1); glClear(COLOR|DEPTH).
//     3. Draw the bounding box (opaque, depth writes on) – call drawBBox.
//     4. Draw particles (additive blending, depth writes off) – call drawParticles.
// ---------------------------------------------------------------------------
void Renderer::draw(const std::vector<Particle>& pool,
                    const glm::mat4& view,
                    const glm::mat4& proj,
                    float screenHeight)
{
    if (!m_initialised) return;

    // TODO: set clear colour and clear colour + depth buffers
    glClearColor(0.02f, 0.02f, 0.06f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: call drawBBox(view, proj)
    drawBBox(view, proj);

    // TODO: call drawParticles(pool, view, proj, screenHeight)
    drawParticles(pool, view, proj, screenHeight);
}


// ---------------------------------------------------------------------------
// Renderer::drawParticles  (private)
//   1. Build m_staging: iterate pool, skip dead particles, push ParticleVertex
//      {p.position, p.currentColor(), p.size} for each alive particle.
//   2. Return early if m_staging is empty.
//   3. Upload to GPU:
//        glBindBuffer(GL_ARRAY_BUFFER, m_particleVBO)
//        glBufferData(… maxParticles * sizeof(ParticleVertex), nullptr, GL_STREAM_DRAW)
//        glBufferSubData(… m_staging.size() * sizeof(ParticleVertex), m_staging.data())
//   4. Enable GL_PROGRAM_POINT_SIZE.
//   5. Use m_particleProgram; set uniforms uView, uProjection, uScreenHeight.
//   6. Bind m_particleVAO; glDrawArrays(GL_POINTS, 0, staging.size()); unbind.
//   7. Restore state: disable GL_PROGRAM_POINT_SIZE.
// ---------------------------------------------------------------------------
void Renderer::drawParticles(const std::vector<Particle>& pool,
                              const glm::mat4& view,
                              const glm::mat4& proj,
                              float screenHeight)
{
    // TODO: build m_staging from alive particles
    m_staging.clear();

    for (const Particle& p : pool) {
        if (!p.alive) continue;
        m_staging.push_back(ParticleVertex{
            p.position,
            p.currentColor(),
            p.size
        });
    }

    // TODO: return early if m_staging is empty
    if (m_staging.empty()) return;

    // TODO: orphan the VBO and upload m_staging
    glBindBuffer(GL_ARRAY_BUFFER, m_particleVBO);

    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_maxParticles * sizeof(ParticleVertex)),
                 nullptr,
                 GL_STREAM_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    static_cast<GLsizeiptr>(m_staging.size() * sizeof(ParticleVertex)),
                    m_staging.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // TODO: set blend / depth state, bind program and uniforms, draw, restore state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glUseProgram(m_particleProgram);

    GLint viewLoc = glGetUniformLocation(m_particleProgram, "uView");
    GLint projLoc = glGetUniformLocation(m_particleProgram, "uProjection");
    GLint screenLoc = glGetUniformLocation(m_particleProgram, "uScreenHeight");

    if (viewLoc >= 0)
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    if (projLoc >= 0)
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    if (screenLoc >= 0)
        glUniform1f(screenLoc, screenHeight);

    glBindVertexArray(m_particleVAO);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_staging.size()));
    glBindVertexArray(0);

    glUseProgram(0);

    glDisable(GL_PROGRAM_POINT_SIZE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}


// ---------------------------------------------------------------------------
// Renderer::drawBBox  (private)
//   Use m_bboxProgram.
//   Set uniforms: uView, uProjection, uColor (suggest rgba 0.25, 0.35, 0.55, 1).
//   Bind m_bboxVAO; glDrawArrays(GL_LINES, 0, 24); unbind.
// ---------------------------------------------------------------------------
void Renderer::drawBBox(const glm::mat4& view, const glm::mat4& proj)
{
    // TODO: use m_bboxProgram, set uniforms, draw 24 line vertices
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    glUseProgram(m_bboxProgram);

    GLint viewLoc  = glGetUniformLocation(m_bboxProgram, "uView");
    GLint projLoc  = glGetUniformLocation(m_bboxProgram, "uProjection");
    GLint colorLoc = glGetUniformLocation(m_bboxProgram, "uColor");

    if (viewLoc >= 0)
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    if (projLoc >= 0)
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    if (colorLoc >= 0)
        glUniform4f(colorLoc, 0.25f, 0.35f, 0.55f, 1.f);

    glBindVertexArray(m_bboxVAO);
    glDrawArrays(GL_LINES, 0, 24);
    glBindVertexArray(0);

    glUseProgram(0);
}


// ===========================================================================
// Shader helper methods  –  implement these utilities used by init()
// ===========================================================================

// ---------------------------------------------------------------------------
// readFile  –  read a text file from disk into a std::string.
//              Print an error and return {} if the file cannot be opened.
// ---------------------------------------------------------------------------
std::string Renderer::readFile(const std::string& path)
{
    // TODO: open path with std::ifstream; stream contents into std::ostringstream
    //       return the string, or {} on failure
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[Renderer] Failed to open file: " << path << "\n";
        return {};
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// ---------------------------------------------------------------------------
// compileShader  –  create a GL shader of the given type, attach src, compile.
//                   On failure print the info log and return 0.
// ---------------------------------------------------------------------------
GLuint Renderer::compileShader(GLenum type, const std::string& src)
{
    // TODO: glCreateShader, glShaderSource, glCompileShader
    //       check GL_COMPILE_STATUS; on error print log and return 0
    GLuint shader = glCreateShader(type);
    const char* csrc = src.c_str();

    glShaderSource(shader, 1, &csrc, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

    if (!ok) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

        std::string log(static_cast<std::size_t>(logLen), '\0');
        if (logLen > 0)
            glGetShaderInfoLog(shader, logLen, nullptr, log.data());

        std::cerr << "[Renderer] Shader compile failed:\n" << log << "\n";
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

// ---------------------------------------------------------------------------
// linkProgram  –  attach vert + frag to a new program object and link.
//                 On failure print the info log and return 0.
//                 Detach shaders after a successful link.
// ---------------------------------------------------------------------------
GLuint Renderer::linkProgram(GLuint vert, GLuint frag)
{
    // TODO: glCreateProgram, attach, link, check GL_LINK_STATUS
    //       detach shaders; on error return 0
    GLuint program = glCreateProgram();

    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);

    if (!ok) {
        GLint logLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);

        std::string log(static_cast<std::size_t>(logLen), '\0');
        if (logLen > 0)
            glGetProgramInfoLog(program, logLen, nullptr, log.data());

        std::cerr << "[Renderer] Program link failed:\n" << log << "\n";
        glDeleteProgram(program);
        return 0;
    }

    glDetachShader(program, vert);
    glDetachShader(program, frag);

    return program;
}

// ---------------------------------------------------------------------------
// loadShaders  –  convenience wrapper: read both files, compile, link.
//                 Stores the resulting program in outProgram.
//                 Returns false on any error.
// ---------------------------------------------------------------------------
bool Renderer::loadShaders(const std::string& vertPath, const std::string& fragPath,
                            GLuint& outProgram)
{
    // TODO: call readFile for both paths (return false if either is empty)
    std::string vertSrc = readFile(vertPath);
    std::string fragSrc = readFile(fragPath);

    if (vertSrc.empty() || fragSrc.empty())
        return false;

    // TODO: compile both shaders (return false and clean up if either fails)
    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc);
    if (!vert)
        return false;

    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    if (!frag) {
        glDeleteShader(vert);
        return false;
    }

    // TODO: link; return true if successful
    GLuint program = linkProgram(vert, frag);

    glDeleteShader(vert);
    glDeleteShader(frag);

    if (!program)
        return false;

    outProgram = program;
    return true;
}