#ifndef PANNING_CAMERA_H
#define PANNING_CAMERA_H
#include "CameraInterface.h"

/// A concrete implementation of CameraInterface, that is a camera with basic panning like controls.
class PanningCamera : public CameraInterface {
    // Default initial values
    const float init_distance = 8.0f;
    const glm::vec3 init_focus_point = {0.0f, 0.0f, 0.0f};
    const float init_pitch = -45.0f;
    const float init_yaw = 315.0f;
    const float init_near = 0.01f;
    const float init_fov = glm::radians(90.0f);
    const float init_gamma = 2.2f;
    
    float distance;
    glm::vec3 focus_point{};
    float pitch;  // Stored in degrees for easier manipulation
    float yaw;    // Stored in degrees for easier manipulation
    float near;
    float fov;
    
    const glm::vec3 UP = {0.0f, 1.0f, 0.0f};
    const glm::vec3 RIGHT = {1.0f, 0.0f, 0.0f};
    const glm::vec3 FORWARD = {0.0f, 0.0f, -1.0f};
    
    const float YAW_SPEED = 0.3f;
    const float PITCH_SPEED = 0.3f;
    const float ZOOM_SPEED = 0.3f;
    const float ZOOM_SCROLL_MULTIPLIER = 2.0f;
    const float PAN_SPEED = 500.0f;
    const float MIN_DISTANCE = 0.001f;
    const float MAX_DISTANCE = 10000.0f;
    const float YAW_PERIOD = 360.0f;  // Changed to degrees
    const float PITCH_MIN = -89.99f;  // Changed to degrees
    const float PITCH_MAX = 89.99f;   // Changed to degrees
    
    glm::mat4 view_matrix{1.0f};
    glm::mat4 inverse_view_matrix{1.0f};
    glm::mat4 projection_matrix{1.0f};
    glm::mat4 inverse_projection_matrix{1.0f};
    float gamma = 2.2f;
    
public:
    /// Construct the Camera with default initial state
    PanningCamera();
    
    /// Construct the Camera with a specific initial state, which will also be the reset state.
    PanningCamera(float distance, glm::vec3 focus_point, float pitch, float yaw, float near, float fov);
    
    void update(const Window& window, float dt, bool controls_enabled) override;
    void reset() override;
    void add_imgui_options_section(const SceneContext& scene_context) override;
    
    [[nodiscard]] CameraProperties save_properties() const override;
    void load_properties(const CameraProperties& camera_properties) override;
    
    [[nodiscard]] glm::mat4 get_view_matrix() const override;
    [[nodiscard]] glm::mat4 get_inverse_view_matrix() const override;
    [[nodiscard]] glm::mat4 get_projection_matrix() const override;
    [[nodiscard]] glm::mat4 get_inverse_projection_matrix() const override;
    [[nodiscard]] float get_gamma() const override;
    
    // Helper function to get camera position
    [[nodiscard]] glm::vec3 get_position() const;
};
#endif //PANNING_CAMERA_H