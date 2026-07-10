#pragma once
#include <glm/glm.hpp>

// ---------------------------------------------------------------------------
// Particle
//   Represents a single particle in the simulation.
//   Satisfies the rule-of-five: all special members are defaulted/defined.
//   All read-only accessors are marked const.
// ---------------------------------------------------------------------------
class Particle {
public:
    glm::vec3 position    {0.f};
    glm::vec3 velocity    {0.f};
    glm::vec3 acceleration{0.f};

    // Color gradient: particle interpolates from youngColor → oldColor over lifetime
    glm::vec4 colorYoung  {1.f, 0.9f, 0.4f, 1.f};  // bright yellow-white
    glm::vec4 colorOld    {0.8f, 0.1f, 0.0f, 0.f};  // deep red, faded out

    float size     {8.f};   // point size in pixels (screen-space base)
    float mass     {1.f};
    float age      {0.f};
    float lifetime {3.f};
    bool  alive    {false};

    // Rule-of-five: trivially copyable data, so all special members are default
    Particle()                           = default;
    ~Particle()                          = default;
    Particle(const Particle&)            = default;
    Particle& operator=(const Particle&) = default;
    Particle(Particle&&)                 = default;
    Particle& operator=(Particle&&)      = default;

    // Re-initialise a pooled particle for reuse
    void init(const glm::vec3& pos,
              const glm::vec3& vel,
              const glm::vec4& youngColor,
              const glm::vec4& oldColor,
              float sz, float mass, float lt) noexcept;

    // Advance state by dt seconds using semi-implicit Euler integration
    void update(float dt, const glm::vec3& gravity) noexcept;

    // Reflect velocity off a bounding-box face; apply coefficient of restitution
    void collideBounds(const glm::vec3& bmin, const glm::vec3& bmax,
                       float restitution) noexcept;

    // --- Const accessors ---
    [[nodiscard]] bool  isAlive()       const noexcept { return alive; }
    [[nodiscard]] float normalizedAge() const noexcept {
        return (lifetime > 0.f) ? glm::clamp(age / lifetime, 0.f, 1.f) : 1.f;
    }

    // Linearly interpolate between youngColor and oldColor by age
    [[nodiscard]] glm::vec4 currentColor() const noexcept {
        return glm::mix(colorYoung, colorOld, normalizedAge());
    }
};
