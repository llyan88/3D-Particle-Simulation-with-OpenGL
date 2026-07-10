#include "Particle.h"
#include <glm/glm.hpp>

// =============================================================================
// Particle.cpp  –  ECE 4122/6122 Final Project  (STUDENT SKELETON)
// =============================================================================
// Implement the three methods below.  The header (Particle.h) is already
// complete – do NOT modify it.
//
// Helpful GLM references:
//   glm::clamp(value, lo, hi)
//   glm::abs(v)
// =============================================================================

// -----------------------------------------------------------------------------
// Particle::init
//   Re-initialise a pooled (possibly previously used) particle for reuse.
//   Copy every argument into the corresponding member variable and mark the
//   particle as alive.  Reset age to 0.
// -----------------------------------------------------------------------------
void Particle::init(const glm::vec3& pos,
                    const glm::vec3& vel,
                    const glm::vec4& youngColor,
                    const glm::vec4& oldColor,
                    float sz, float m, float lt) noexcept
{
    // TODO: set position, velocity, colorYoung, colorOld, size, mass, lifetime
    position   = pos;
    velocity   = vel;
    colorYoung = youngColor;
    colorOld   = oldColor;
    size       = sz;
    mass       = m;
    lifetime   = lt;

    // TODO: reset acceleration to {0,0,0} and age to 0.0f
    acceleration = glm::vec3{0.f};
    age = 0.f;

    // TODO: set alive = true
    alive = true;
}

// -----------------------------------------------------------------------------
// Particle::update
//   Advance the particle by dt seconds using semi-implicit Euler integration:
//
//     velocity += (gravity + acceleration) * dt   // update velocity FIRST
//     position += velocity * dt                    // then use NEW velocity
//
//   Also increment age by dt.  If age >= lifetime, set alive = false and return
//   early (before updating position – the particle is dead).
// -----------------------------------------------------------------------------
void Particle::update(float dt, const glm::vec3& gravity) noexcept
{
    if (!alive) return;

    // TODO: increment age by dt
    age += dt;

    // TODO: if age >= lifetime, set alive = false and return
    if (age >= lifetime) {
        alive = false;
        return;
    }

    // TODO: semi-implicit Euler integration
    //         velocity += (gravity + acceleration) * dt
    //         position += velocity * dt
    velocity += (gravity + acceleration) * dt;
    position += velocity * dt;
}

// -----------------------------------------------------------------------------
// Particle::collideBounds
//   Reflect the particle off the six faces of the axis-aligned bounding box
//   [bmin, bmax].  Apply the coefficient of restitution (value in [0,1]) to
//   the normal component of velocity on impact.
//
//   For each axis (X, Y, Z):
//     • If position < bmin  →  clamp position to bmin,
//                               set velocity component to +|velocity| * restitution
//     • If position > bmax  →  clamp position to bmax,
//                               set velocity component to -|velocity| * restitution
//
//   Special case – FLOOR bounce (position.y < bmin.y):
//     After reflecting the Y velocity, also apply friction:
//       velocity.x *= 0.85f;
//       velocity.z *= 0.85f;
// -----------------------------------------------------------------------------
void Particle::collideBounds(const glm::vec3& bmin,
                              const glm::vec3& bmax,
                              float restitution) noexcept
{
    // TODO: X axis  (bmin.x, bmax.x)
    if (position.x < bmin.x) {
        position.x = bmin.x;
        velocity.x = glm::abs(velocity.x) * restitution;
    } else if (position.x > bmax.x) {
        position.x = bmax.x;
        velocity.x = -glm::abs(velocity.x) * restitution;
    }

    // TODO: Y axis  (bmin.y = floor, bmax.y = ceiling)
    //       Don't forget the horizontal damping on floor bounce
    if (position.y < bmin.y) {
        position.y = bmin.y;
        velocity.y = glm::abs(velocity.y) * restitution;
        velocity.x *= 0.85f;
        velocity.z *= 0.85f;
    } else if (position.y > bmax.y) {
        position.y = bmax.y;
        velocity.y = -glm::abs(velocity.y) * restitution;
    }

    // TODO: Z axis  (bmin.z, bmax.z)
    if (position.z < bmin.z) {
        position.z = bmin.z;
        velocity.z = glm::abs(velocity.z) * restitution;
    } else if (position.z > bmax.z) {
        position.z = bmax.z;
        velocity.z = -glm::abs(velocity.z) * restitution;
    }
}