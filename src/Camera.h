#pragma once
#include <glm/glm.hpp>

// ---------------------------------------------------------------------------
// Camera
//   Arc-ball (orbital) camera centred on the simulation bounding box.
//
//   Controls:
//     Left-drag   – orbit (yaw / pitch)
//     Right-drag  – pan target
//     Scroll      – zoom (distance)
//     'R'         – reset to defaults
// ---------------------------------------------------------------------------
class Camera {
public:
    Camera() = default;

    // Rule-of-five: trivially copyable, so all special members are defaulted
    Camera(const Camera&)            = default;
    Camera& operator=(const Camera&) = default;
    Camera(Camera&&)                 = default;
    Camera& operator=(Camera&&)      = default;
    ~Camera()                        = default;

    // Call once to set up initial orientation
    void init(float distance, float yawDeg, float pitchDeg, glm::vec3 target);

    // GLFW callback adapters (call from your static callbacks)
    void onMouseButton(int button, int action);
    void onMouseMove  (double xpos, double ypos);
    void onScroll     (double yoffset);
    void onKey        (int key, int action);

    // Matrices
    [[nodiscard]] glm::mat4 getViewMatrix()                        const noexcept;
    [[nodiscard]] glm::mat4 getProjectionMatrix(float aspectRatio) const noexcept;

    // Const accessors
    [[nodiscard]] float     distance() const noexcept { return m_distance; }
    [[nodiscard]] glm::vec3 position() const noexcept;

private:
    void clampPitch() noexcept;

    float     m_distance  {30.f};
    float     m_yaw       {-90.f};   // degrees
    float     m_pitch     { 25.f};   // degrees
    glm::vec3 m_target    { 0.f, 2.f, 0.f};

    float m_fovDeg   {60.f};
    float m_nearClip { 0.1f};
    float m_farClip  {500.f};

    // Orbit state
    bool   m_leftDown{false};
    bool   m_rightDown{false};
    double m_lastX{0.}, m_lastY{0.};
    bool   m_firstMouse{true};

    // Sensitivity
    static constexpr float kOrbitSens = 0.25f;
    static constexpr float kPanSens   = 0.015f;
    static constexpr float kZoomSens  = 1.2f;

    // Defaults for reset
    float     m_defDistance {30.f};
    float     m_defYaw      {-90.f};
    float     m_defPitch    { 25.f};
    glm::vec3 m_defTarget   { 0.f, 2.f, 0.f};
};
