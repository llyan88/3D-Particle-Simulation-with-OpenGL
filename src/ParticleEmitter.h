#pragma once
#include "Particle.h"
#include "SimulationConfig.h"
#include <vector>
#include <stack>
#include <random>
#include <string>

// ---------------------------------------------------------------------------
// EmitterType – selects the velocity distribution used when spawning particles
// ---------------------------------------------------------------------------
enum class EmitterType { Fountain = 0, Explosion = 1, Vortex = 2 };

// ---------------------------------------------------------------------------
// ParticleEmitter
//   Owns a fixed-capacity pool of Particle objects allocated once at startup.
//   Dead particles are pushed onto a free-stack rather than deallocated,
//   avoiding per-frame heap traffic entirely.
//
//   Physics updates run inside a parallel OpenMP region for performance.
// ---------------------------------------------------------------------------
class ParticleEmitter {
public:
    explicit ParticleEmitter(const SimulationConfig& cfg);

    // Non-copyable (owns unique pool resources)
    ParticleEmitter(const ParticleEmitter&)            = delete;
    ParticleEmitter& operator=(const ParticleEmitter&) = delete;

    // Movable
    ParticleEmitter(ParticleEmitter&&)                 = default;
    ParticleEmitter& operator=(ParticleEmitter&&)      = default;

    ~ParticleEmitter() = default;

    // Advance simulation by dt seconds
    void update(float dt);

    // Read-only access for renderer
    [[nodiscard]] const std::vector<Particle>& particles() const noexcept { return m_pool; }
    [[nodiscard]] int activeCount() const noexcept { return m_activeCount; }

    // Runtime configuration
    void setEmitterType(EmitterType t) noexcept { m_type = t; }
    void setEmitterPos(const glm::vec3& p) noexcept { m_pos = p; }
    void cycleEmitterType() noexcept;

    [[nodiscard]] std::string emitterTypeName() const noexcept;

private:
    // Spawn one new particle using the current emitter mode
    void spawnOne();

    // Build initial velocity for each emitter type
    glm::vec3 fountainVelocity();
    glm::vec3 explosionVelocity();
    glm::vec3 vortexVelocity();

    // --- Pool ---
    std::vector<Particle> m_pool;      // fixed-capacity flat array
    std::stack<int>       m_freeList;  // indices of dead/unborn particles
    int                   m_activeCount{0};

    // --- Config ---
    int       m_maxParticles;
    float     m_spawnRate;             // particles / second
    int       m_numThreads;
    glm::vec3 m_gravity;
    float     m_restitution;
    glm::vec3 m_bboxMin, m_bboxMax;

    EmitterType m_type;
    glm::vec3   m_pos;

    // Spawn accumulator: fractional particles carry over between frames
    float m_spawnAccum{0.f};

    // Explosion cycling
    float m_explosionTimer{0.f};
    float m_explosionInterval{3.5f};
    bool  m_explosionActive{false};
    float m_explosionBurst{0.f};

    // Random number generation (one engine, shared)
    std::mt19937                          m_rng;
    std::uniform_real_distribution<float> m_dist01{0.f, 1.f};

    float rnd()          { return m_dist01(m_rng); }
    float rnd(float lo, float hi) {
        return lo + (hi - lo) * rnd();
    }
};
