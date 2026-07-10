#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

// =============================================================================
// Camera.cpp  –  ECE 4122/6122 Final Project  (STUDENT SKELETON)
// =============================================================================
// Implements an arc-ball (orbital) camera centred on a target point.
//
// The camera is described by three scalars:
//   m_distance  – how far the eye is from m_target
//   m_yaw       – horizontal angle (degrees)
//   m_pitch     – vertical angle (degrees, clamped to ±89°)
//
// The eye position in world space is:
//   eye = target + distance * (cos(pitch)*cos(yaw),
//                               sin(pitch),
//                               cos(pitch)*sin(yaw))
//
// Implement each method below.  The header (Camera.h) is complete – do NOT
// modify it.
// =============================================================================


// ---------------------------------------------------------------------------
// Camera::init
//   Store the four arguments as both the current and the default values so
//   the 'R' key can restore them.  Reset m_firstMouse to true.
// ---------------------------------------------------------------------------
void Camera::init(float dist, float yawDeg, float pitchDeg, glm::vec3 target)
{
    // TODO: set m_distance = m_defDistance = dist
    m_distance = m_defDistance = dist;

    // TODO: set m_yaw      = m_defYaw      = yawDeg
    m_yaw = m_defYaw = yawDeg;

    // TODO: set m_pitch    = m_defPitch    = pitchDeg
    m_pitch = m_defPitch = pitchDeg;

    // TODO: set m_target   = m_defTarget   = target
    m_target = m_defTarget = target;

    // TODO: m_firstMouse = true
    m_firstMouse = true;

    clampPitch();
}


// ---------------------------------------------------------------------------
// Camera::position
//   Return the eye position in world space.
//   yR = radians(m_yaw),  pR = radians(m_pitch)
//   eye = m_target + m_distance * {cos(pR)*cos(yR), sin(pR), cos(pR)*sin(yR)}
// ---------------------------------------------------------------------------
glm::vec3 Camera::position() const noexcept
{
    // TODO: convert m_yaw and m_pitch to radians (glm::radians)
    float yR = glm::radians(m_yaw);
    float pR = glm::radians(m_pitch);

    // TODO: return m_target + offset as described above
    glm::vec3 offset{
        std::cos(pR) * std::cos(yR),
        std::sin(pR),
        std::cos(pR) * std::sin(yR)
    };

    return m_target + m_distance * offset;
}


// ---------------------------------------------------------------------------
// Camera::getViewMatrix
//   Return glm::lookAt(position(), m_target, {0,1,0})
// ---------------------------------------------------------------------------
glm::mat4 Camera::getViewMatrix() const noexcept
{
    // TODO: use glm::lookAt
    return glm::lookAt(position(), m_target, glm::vec3{0.f, 1.f, 0.f});
}


// ---------------------------------------------------------------------------
// Camera::getProjectionMatrix
//   Return glm::perspective(radians(m_fovDeg), aspectRatio, m_nearClip, m_farClip)
// ---------------------------------------------------------------------------
glm::mat4 Camera::getProjectionMatrix(float aspect) const noexcept
{
    // TODO: use glm::perspective
    return glm::perspective(glm::radians(m_fovDeg), aspect, m_nearClip, m_farClip);
}


// ---------------------------------------------------------------------------
// Camera::clampPitch  (private)
//   Clamp m_pitch to [-89, +89] degrees to prevent gimbal flip.
// ---------------------------------------------------------------------------
void Camera::clampPitch() noexcept
{
    // TODO: m_pitch = std::clamp(m_pitch, -89.f, 89.f)
    m_pitch = std::clamp(m_pitch, -89.f, 89.f);
}


// ---------------------------------------------------------------------------
// Camera::onMouseButton
//   Track whether the left / right mouse button is held.
//   On any press, reset m_firstMouse to true (fresh drag origin).
// ---------------------------------------------------------------------------
void Camera::onMouseButton(int button, int action)
{
    // TODO: if GLFW_MOUSE_BUTTON_LEFT  set m_leftDown  = (action == GLFW_PRESS)
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        m_leftDown = (action == GLFW_PRESS);

    // TODO: if GLFW_MOUSE_BUTTON_RIGHT set m_rightDown = (action == GLFW_PRESS)
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
        m_rightDown = (action == GLFW_PRESS);

    // TODO: on GLFW_PRESS set m_firstMouse = true
    if (action == GLFW_PRESS)
        m_firstMouse = true;
}


// ---------------------------------------------------------------------------
// Camera::onMouseMove
//   Called every time the cursor moves.
//
//   If neither button is held, reset m_firstMouse and return.
//
//   On the first call after a button press (m_firstMouse == true):
//     record m_lastX, m_lastY and set m_firstMouse = false, then return.
//
//   Otherwise compute dx = xpos - m_lastX, dy = ypos - m_lastY,
//   update m_lastX/Y, then:
//
//   Left button (orbit):
//     m_yaw   += dx * kOrbitSens
//     m_pitch -= dy * kOrbitSens   (screen Y is flipped)
//     clampPitch()
//
//   Right button (pan target in camera-local XY plane):
//     forward = normalize(m_target - position())
//     right   = normalize(cross(forward, {0,1,0}))
//     up      = cross(right, forward)
//     m_target -= right * (dx * kPanSens * m_distance * 0.1f)
//     m_target += up    * (dy * kPanSens * m_distance * 0.1f)
// ---------------------------------------------------------------------------
void Camera::onMouseMove(double xpos, double ypos)
{
    if (!m_leftDown && !m_rightDown) {
        m_firstMouse = true;
        return;
    }
    if (m_firstMouse) {
        m_lastX = xpos; m_lastY = ypos;
        m_firstMouse = false;
        return;
    }

    float dx = static_cast<float>(xpos - m_lastX);
    float dy = static_cast<float>(ypos - m_lastY);
    m_lastX  = xpos;
    m_lastY  = ypos;

    if (m_leftDown) {
        // TODO: orbit – update m_yaw, m_pitch, clampPitch
        m_yaw   += dx * kOrbitSens;
        m_pitch -= dy * kOrbitSens;
        clampPitch();
    } else if (m_rightDown) {
        // TODO: pan – compute forward/right/up and shift m_target
        glm::vec3 forward = glm::normalize(m_target - position());
        glm::vec3 right   = glm::normalize(glm::cross(forward, glm::vec3{0.f, 1.f, 0.f}));
        glm::vec3 up      = glm::cross(right, forward);

        float scale = kPanSens * m_distance * 0.1f;
        m_target -= right * (dx * scale);
        m_target += up    * (dy * scale);
    }
}


// ---------------------------------------------------------------------------
// Camera::onScroll
//   Zoom by scaling m_distance:
//     m_distance *= pow(kZoomSens, -yoffset)
//   Clamp m_distance to [2, 200].
// ---------------------------------------------------------------------------
void Camera::onScroll(double yoffset)
{
    // TODO: scale m_distance and clamp to [2, 200]
    m_distance *= std::pow(kZoomSens, static_cast<float>(-yoffset));
    m_distance = std::clamp(m_distance, 2.f, 200.f);
}


// ---------------------------------------------------------------------------
// Camera::onKey
//   When GLFW_KEY_R is pressed, restore all defaults and reset m_firstMouse.
// ---------------------------------------------------------------------------
void Camera::onKey(int key, int action)
{
    // TODO: on R press, restore m_distance, m_yaw, m_pitch, m_target from defaults
    //       and set m_firstMouse = true
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        m_distance = m_defDistance;
        m_yaw      = m_defYaw;
        m_pitch    = m_defPitch;
        m_target   = m_defTarget;
        m_firstMouse = true;
        clampPitch();
    }
}
