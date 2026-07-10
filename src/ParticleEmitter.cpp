#include "ParticleEmitter.h"
#include <omp.h>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <iostream>

// =============================================================================
// ParticleEmitter.cpp  –  ECE 4122/6122 Final Project  (STUDENT SKELETON)
// =============================================================================
// This file is the heart of the simulation.  You must implement:
//
//   1. Constructor         – allocate the particle pool and free-list
//   2. update(dt)          – emission, parallel physics, dead-particle harvest
//   3. spawnOne()          – pull a free slot and initialise a Particle
//   4. fountainVelocity()  – velocity for a fountain / cone spray
//   5. explosionVelocity() – velocity for a uniform-sphere burst
//   6. vortexVelocity()    – velocity for an upward spiral
//   7. cycleEmitterType()  – advance to the next EmitterType (already declared)
//   8. emitterTypeName()   – return a printable string for the current type
//
// The header (ParticleEmitter.h) is already complete – do NOT modify it.
//
// Implementation hints are provided in each stub below.
// =============================================================================


// -----------------------------------------------------------------------------
// Constructor
//   Initialise all member variables from the config struct.
//   Pre-allocate m_pool to m_maxParticles entries (all default-constructed
//   Particles start with alive = false).
//   Push indices [maxParticles-1 … 0] onto m_freeList so every slot is free.
//   Call omp_set_num_threads(m_numThreads) and print a status line.
// -----------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter(const SimulationConfig& cfg)
{
    // TODO: initialise m_maxParticles, m_spawnRate, m_numThreads from cfg
    m_maxParticles = cfg.maxParticles;
    m_spawnRate    = cfg.spawnRate;
    m_numThreads   = cfg.numThreads;

    // TODO: initialise m_gravity, m_restitution, m_bboxMin, m_bboxMax from cfg
    m_gravity     = cfg.gravity;
    m_restitution = cfg.restitution;
    m_bboxMin     = cfg.bboxMin;
    m_bboxMax     = cfg.bboxMax;

    // TODO: initialise m_type (cast cfg.emitterType to EmitterType) and m_pos
    m_type = static_cast<EmitterType>(cfg.emitterType);
    m_pos  = cfg.emitterPos;

    // TODO: seed m_rng with std::random_device{}()
    m_rng.seed(std::random_device{}());

    // TODO: resize m_pool to m_maxParticles
    m_pool.resize(m_maxParticles);

    // TODO: push every index onto m_freeList
    //       for (int i = m_maxParticles - 1; i >= 0; --i) m_freeList.push(i);
    for (int i = m_maxParticles - 1; i >= 0; --i)
        m_freeList.push(i);

    // TODO: call omp_set_num_threads(m_numThreads)
    omp_set_num_threads(m_numThreads);

    // TODO: print "[Emitter] Pool: X  Threads: Y"
    std::cout << "[Emitter] Pool: " << m_maxParticles
              << "  Threads: " << m_numThreads << "\n";
}


// -----------------------------------------------------------------------------
// update(dt)
//   Three-phase loop called once per frame:
//
//   PHASE 1 – Emission (single-threaded, modifies m_freeList):
//     • Fountain/Vortex: accumulate m_spawnRate*dt into m_spawnAccum;
//       floor(m_spawnAccum) gives the integer count to spawn this frame;
//       keep the fractional remainder in m_spawnAccum for the next frame.
//     • Explosion mode: increment m_explosionTimer by dt; when it exceeds
//       m_explosionInterval, set m_explosionActive=true and compute a burst
//       count of 8% of the pool.  Spawn the burst in a single frame and
//       reset the timer.
//     Call spawnOne() up to count times while m_freeList is non-empty.
//
//   PHASE 2 – Physics update (parallelised with OpenMP):
//     Iterate over every slot in m_pool.  For alive particles call:
//       p.update(dt, m_gravity);
//       p.collideBounds(m_bboxMin, m_bboxMax, m_restitution);
//     Use:  #pragma omp parallel for schedule(static)
//
//   PHASE 3 – Harvest dead particles (single-threaded):
//     Walk m_pool; count alive particles into m_activeCount.
//     For slots that just died (alive==false but age > 0), return the index
//     to m_freeList and reset p.age to 0 (sentinel to avoid double-push).
// -----------------------------------------------------------------------------
void ParticleEmitter::update(float dt)
{
    // -------------------------------------------------------------------------
    // Phase 1 – Emission
    // -------------------------------------------------------------------------
    float toSpawn = 0.f;

    if (m_type == EmitterType::Explosion) {
        // TODO: advance m_explosionTimer
        m_explosionTimer += dt;

        // TODO: if timer >= m_explosionInterval, trigger a burst:
        //         m_explosionTimer = 0, m_explosionActive = true,
        //         m_explosionBurst = m_maxParticles * 0.08f
        if (m_explosionTimer >= m_explosionInterval) {
            m_explosionTimer = 0.f;
            m_explosionActive = true;
            m_explosionBurst = static_cast<float>(m_maxParticles) * 0.08f;
        }

        // TODO: if m_explosionActive, set toSpawn = m_explosionBurst,
        //         then set m_explosionActive = false
        if (m_explosionActive) {
            toSpawn = m_explosionBurst;
            m_explosionActive = false;
        }
    } else {
        // TODO: m_spawnAccum += m_spawnRate * dt
        m_spawnAccum += m_spawnRate * dt;

        // TODO: toSpawn = m_spawnAccum; keep fractional remainder
        toSpawn = std::floor(m_spawnAccum);
        m_spawnAccum -= toSpawn;
    }

    int count = static_cast<int>(toSpawn);
    for (int i = 0; i < count && !m_freeList.empty(); ++i)
        spawnOne();

    // -------------------------------------------------------------------------
    // Phase 2 – Physics (OpenMP)
    // -------------------------------------------------------------------------
    int n = m_maxParticles;
    // TODO: add OpenMP parallel for pragma with schedule(static)
#pragma omp parallel for schedule(static)
    for (int i = 0; i < n; ++i) {
        Particle& p = m_pool[i];
        if (!p.alive) continue;
        // TODO: call p.update(dt, m_gravity)
        p.update(dt, m_gravity);

        // TODO: call p.collideBounds(m_bboxMin, m_bboxMax, m_restitution)
        if (p.alive)
            p.collideBounds(m_bboxMin, m_bboxMax, m_restitution);
    }

    // -------------------------------------------------------------------------
    // Phase 3 – Harvest dead particles
    // -------------------------------------------------------------------------
    m_activeCount = 0;
    for (int i = 0; i < n; ++i) {
        // TODO: if alive, increment m_activeCount
        if (m_pool[i].alive) {
            ++m_activeCount;
        }
        // TODO: else if age > 0.f (just died), push index back onto m_freeList
        //       and set age = 0.f to prevent double-push
        else if (m_pool[i].age > 0.f) {
            m_freeList.push(i);
            m_pool[i].age = 0.f;
        }
    }
}


// -----------------------------------------------------------------------------
// spawnOne()
//   Pop the top index from m_freeList.
//   Build a velocity using the current emitter type's helper.
//   Choose random young/old colours, lifetime, size, and mass.
//   Add a small random jitter to m_pos.
//   Call m_pool[idx].init(...) with all parameters.
//
//   Suggested ranges (use rnd(lo, hi)):
//     youngColor:  r in [0.8,1.0],  g in [0.7,1.0],  b in [0.3,0.8],  a = 1
//     oldColor:    {0.1, 0.05, 0.5, 0.0}  (deep blue, transparent)
//     lifetime:    rnd(2.5f, 10.0f)
//     size:        rnd(0.05f, 0.50f)
//     mass:        rnd(0.01f, 0.10f)
//     jitter:      x,z each in [-0.1, 0.1],  y = 0
// -----------------------------------------------------------------------------
void ParticleEmitter::spawnOne()
{
    if (m_freeList.empty()) return;

    // TODO: pop idx from m_freeList
    int idx = m_freeList.top();
    m_freeList.pop();

    // TODO: choose velocity based on m_type
    glm::vec3 vel{0.f};
    switch (m_type) {
        case EmitterType::Fountain:
            vel = fountainVelocity();
            break;
        case EmitterType::Explosion:
            vel = explosionVelocity();
            break;
        case EmitterType::Vortex:
            vel = vortexVelocity();
            break;
    }

    // TODO: build youngColor and oldColor
    glm::vec4 youngCol{
        rnd(0.8f, 1.0f),
        rnd(0.7f, 1.0f),
        rnd(0.3f, 0.8f),
        1.0f
    };

    glm::vec4 oldCol{0.1f, 0.05f, 0.5f, 0.0f};

    // TODO: compute lt, sz, m, jitter
    float lt = rnd(2.5f, 10.0f);
    float sz = rnd(0.05f, 0.50f);
    float m  = rnd(0.01f, 0.10f);

    glm::vec3 jitter{
        rnd(-0.1f, 0.1f),
        0.f,
        rnd(-0.1f, 0.1f)
    };

    // TODO: call m_pool[idx].init(m_pos + jitter, vel, youngCol, oldCol, sz, m, lt)
    m_pool[idx].init(m_pos + jitter, vel, youngCol, oldCol, sz, m, lt);
}


// -----------------------------------------------------------------------------
// fountainVelocity()
//   Return a velocity that lies inside a cone of half-angle ~25° around +Y.
//
//   Algorithm:
//     theta = rnd(0, 2π)          – azimuthal angle
//     phi   = rnd(0, coneAngle)   – polar angle (25° = glm::radians(25.f))
//     speed = rnd(5, 20)
//     vx = speed * sin(phi) * cos(theta)
//     vy = speed * cos(phi)
//     vz = speed * sin(phi) * sin(theta)
// -----------------------------------------------------------------------------
glm::vec3 ParticleEmitter::fountainVelocity()
{
    // TODO: implement as described above
    float theta = rnd(0.f, glm::two_pi<float>());
    float phi   = rnd(0.f, glm::radians(25.f));
    float speed = rnd(5.f, 20.f);

    float vx = speed * std::sin(phi) * std::cos(theta);
    float vy = speed * std::cos(phi);
    float vz = speed * std::sin(phi) * std::sin(theta);

    return {vx, vy, vz};
}


// -----------------------------------------------------------------------------
// explosionVelocity()
//   Return a velocity uniformly distributed over a sphere.
//
//   Algorithm (uniform sphere sampling):
//     theta = rnd(0, 2π)
//     phi   = acos(rnd(-1, 1))
//     speed = rnd(3, 16)
//     vx = speed * sin(phi) * cos(theta)
//     vy = speed * sin(phi) * sin(theta)
//     vz = speed * cos(phi)
// -----------------------------------------------------------------------------
glm::vec3 ParticleEmitter::explosionVelocity()
{
    // TODO: implement as described above
    float theta = rnd(0.f, glm::two_pi<float>());
    float phi   = std::acos(rnd(-1.f, 1.f));
    float speed = rnd(3.f, 16.f);

    float vx = speed * std::sin(phi) * std::cos(theta);
    float vy = speed * std::sin(phi) * std::sin(theta);
    float vz = speed * std::cos(phi);

    return {vx, vy, vz};
}


// -----------------------------------------------------------------------------
// vortexVelocity()
//   Return a velocity that gives an upward-spiralling motion.
//
//   Algorithm:
//     theta     = rnd(0, 2π)
//     tangSpeed = rnd(4, 8)        – tangential (swirl) speed
//     upSpeed   = rnd(10, 30)      – vertical speed
//     outSpeed  = rnd(0.5, 20)     – outward radial speed
//
//     vx = tangSpeed * -sin(theta)  +  outSpeed * cos(theta)
//     vy = upSpeed
//     vz = tangSpeed *  cos(theta)  +  outSpeed * sin(theta)
// -----------------------------------------------------------------------------
glm::vec3 ParticleEmitter::vortexVelocity()
{
    // TODO: implement as described above
    float theta     = rnd(0.f, glm::two_pi<float>());
    float tangSpeed = rnd(4.f, 8.f);
    float upSpeed   = rnd(10.f, 30.f);
    float outSpeed  = rnd(0.5f, 20.f);

    float vx = tangSpeed * -std::sin(theta) + outSpeed * std::cos(theta);
    float vy = upSpeed;
    float vz = tangSpeed *  std::cos(theta) + outSpeed * std::sin(theta);

    return {vx, vy, vz};
}


// -----------------------------------------------------------------------------
// cycleEmitterType()
//   Advance m_type to the next EmitterType (wrapping 0→1→2→0).
//   Reset explosion state (m_explosionTimer, m_explosionActive).
//   Print "[Emitter] Switched to: <name>"
// -----------------------------------------------------------------------------
void ParticleEmitter::cycleEmitterType() noexcept
{
    // TODO: int next = (static_cast<int>(m_type) + 1) % 3;
    //       m_type = static_cast<EmitterType>(next);
    int next = (static_cast<int>(m_type) + 1) % 3;
    m_type = static_cast<EmitterType>(next);

    // TODO: reset explosion timer and active flag
    m_explosionTimer = 0.f;
    m_explosionActive = false;

    // TODO: print status
    std::cout << "[Emitter] Switched to: " << emitterTypeName() << "\n";
}


// -----------------------------------------------------------------------------
// emitterTypeName()
//   Return "Fountain", "Explosion", or "Vortex" based on m_type.
// -----------------------------------------------------------------------------
std::string ParticleEmitter::emitterTypeName() const noexcept
{
    // TODO: switch on m_type and return the matching string
    switch (m_type) {
        case EmitterType::Fountain:
            return "Fountain";
        case EmitterType::Explosion:
            return "Explosion";
        case EmitterType::Vortex:
            return "Vortex";
    }

    return "Unknown";
}