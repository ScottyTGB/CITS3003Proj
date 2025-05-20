#include "PanningCamera.h"

#include <cmath>

#include <glm/gtx/transform.hpp>

#include "utility/Math.h"
#include "rendering/imgui/ImGuiManager.h"

PanningCamera::PanningCamera() : distance(init_distance), focus_point(init_focus_point), pitch(init_pitch), yaw(init_yaw), near(init_near), fov(init_fov) {}

PanningCamera::PanningCamera(float distance, glm::vec3 focus_point, float pitch, float yaw, float near, float fov)
    : init_distance(distance), init_focus_point(focus_point), init_pitch(pitch), init_yaw(yaw), init_near(near), init_fov(fov), distance(distance), focus_point(focus_point), pitch(pitch), yaw(yaw), near(near), fov(fov) {}

void PanningCamera::update(const Window& window, float dt, bool controls_enabled) {
    if (controls_enabled) {
        bool ctrl_is_pressed = window.is_key_pressed(GLFW_KEY_LEFT_CONTROL) || window.is_key_pressed(GLFW_KEY_RIGHT_CONTROL);

        bool resetSeq = false;
        if (window.was_key_pressed(GLFW_KEY_R) && !ctrl_is_pressed) {
            reset();
            resetSeq = true;
        }

        if (!resetSeq) {
            // Get the world space directions based on the current view
            glm::mat4 rotation_matrix = glm::rotate(yaw, glm::vec3{0.0f, 1.0f, 0.0f}) *
                                        glm::rotate(pitch, glm::vec3{1.0f, 0.0f, 0.0f});
            auto right_vector = glm::vec3{rotation_matrix[0]};
            auto up_vector = glm::vec3{rotation_matrix[1]};

            // Handle middle mouse button panning
            auto pan = window.get_mouse_delta(GLFW_MOUSE_BUTTON_MIDDLE);
            if (pan.x != 0.0 || pan.y != 0.0) {
                float pan_scale = PAN_SPEED * dt * distance / (float) window.get_window_height();
                focus_point += right_vector * (float)pan.x * pan_scale;
                focus_point += up_vector * (float)pan.y * pan_scale;
            }

            // Handle right mouse button for rotation
            auto rotate = window.get_mouse_delta(GLFW_MOUSE_BUTTON_RIGHT);
            if (rotate.x != 0.0) {
                // Horizontal drag - rotate around the UP axis (yaw)
                yaw -= YAW_SPEED * (float)rotate.x;
            }
            if (rotate.y != 0.0) {
                // Vertical drag - change elevation angle (pitch)
                pitch -= PITCH_SPEED * (float)rotate.y;
            }

            // Handle mouse scroll for zoom
            float scroll = window.get_scroll_delta();
            if (scroll != 0.0) {
                distance -= ZOOM_SCROLL_MULTIPLIER * ZOOM_SPEED * scroll;
            }

            // Hide cursor during drag operations
            auto is_dragging = window.is_mouse_pressed(GLFW_MOUSE_BUTTON_RIGHT) ||
                              window.is_mouse_pressed(GLFW_MOUSE_BUTTON_MIDDLE);
            if (is_dragging) {
                window.set_cursor_disabled(true);
            }
        }
    }

    // Apply constraints
    yaw = std::fmod(yaw + YAW_PERIOD, YAW_PERIOD);
    pitch = clamp(pitch, PITCH_MIN, PITCH_MAX);
    distance = clamp(distance, MIN_DISTANCE, MAX_DISTANCE);
    near = clamp(near, 0.00001f, 10.0f);  // clamp near plane so it’s not too large

    // Calculate the camera position based on focus point, distance, pitch, and yaw
    glm::mat4 rotation_matrix = glm::rotate(yaw, glm::vec3{0.0f, 1.0f, 0.0f}) *
                               glm::rotate(pitch, glm::vec3{-1.0f, 0.0f, 0.0f});
    glm::vec3 forward = glm::vec3{rotation_matrix[2]};
    glm::vec3 position = focus_point - forward * distance;

    // Create view matrix
    view_matrix = glm::lookAt(position, focus_point, UP);
    inverse_view_matrix = glm::inverse(view_matrix);

    // Update projection matrix
    float aspect_ratio = window.get_framebuffer_aspect_ratio();
    float far = 1000.0f; // or expose this as a UI-controlled variable if needed
    projection_matrix = glm::perspective(fov, window.get_framebuffer_aspect_ratio(), near, far);

    inverse_projection_matrix = glm::inverse(projection_matrix);
}

void PanningCamera::reset() {
    distance = init_distance;
    focus_point = init_focus_point;
    pitch = init_pitch;
    yaw = init_yaw;
    fov = init_fov;
    near = init_near;
    gamma = init_gamma;
}

void PanningCamera::add_imgui_options_section(const SceneContext& scene_context) {
    if (!ImGui::CollapsingHeader("Camera Options")) {
        return;
    }

    ImGui::DragFloat3("Focus Point (x,y,z)", &focus_point[0], 0.01f);
    ImGui::DragDisableCursor(scene_context.window);

    ImGui::DragFloat("Distance", &distance, 0.01f, MIN_DISTANCE, MAX_DISTANCE);
    ImGui::DragDisableCursor(scene_context.window);

    float pitch_degrees = glm::degrees(pitch);
    ImGui::SliderFloat("Pitch", &pitch_degrees, -89.99f, 89.99f);
    pitch = glm::radians(pitch_degrees);

    float yaw_degrees = glm::degrees(yaw);
    ImGui::DragFloat("Yaw", &yaw_degrees);
    ImGui::DragDisableCursor(scene_context.window);
    yaw = glm::radians(glm::mod(yaw_degrees, 360.0f));

    ImGui::SliderFloat("Near Plane", &near, 0.001f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Far Plane", &far, 1.0f, 2000.0f, "%.1f");

    float fov_degrees = glm::degrees(fov);
    ImGui::SliderFloat("FOV", &fov_degrees, 40.0f, 170.0f);
    fov = glm::radians(fov_degrees);

    ImGui::Spacing();
    ImGui::SliderFloat("Gamma", &gamma, 1.0f, 5.0f, "%.2f");

    if (ImGui::Button("Reset (R)")) {
        reset();
    }
}

CameraProperties PanningCamera::save_properties() const {
    return CameraProperties{
        get_position(),
        yaw,
        pitch,
        fov,
        gamma,
        near
    };
}

void PanningCamera::load_properties(const CameraProperties& camera_properties) {
    yaw = camera_properties.yaw;
    pitch = camera_properties.pitch;
    fov = camera_properties.fov;
    gamma = camera_properties.gamma;
    near = camera_properties.near;

    // Set position as the new focus point and reset distance
    focus_point = camera_properties.position;
    distance = 1.0f;

    // Adjust focus point based on forward direction and distance
    auto rotation_matrix = glm::rotate(yaw, glm::vec3{0.0f, 1.0f, 0.0f}) *
                           glm::rotate(pitch, glm::vec3{1.0f, 0.0f, 0.0f});
    auto forward = glm::vec3{rotation_matrix[2]};
    focus_point -= distance * forward;
}

glm::vec3 PanningCamera::get_position() const {
    // Calculate the camera position based on focus point, distance, pitch, and yaw
    glm::mat4 rotation_matrix = glm::rotate(yaw, glm::vec3{0.0f, 1.0f, 0.0f}) *
                               glm::rotate(pitch, glm::vec3{1.0f, 0.0f, 0.0f});
    glm::vec3 forward = glm::vec3{rotation_matrix[2]};
    return focus_point - forward * distance;
}

glm::mat4 PanningCamera::get_view_matrix() const {
    return view_matrix;
}

glm::mat4 PanningCamera::get_inverse_view_matrix() const {
    return inverse_view_matrix;
}

glm::mat4 PanningCamera::get_projection_matrix() const {
    return projection_matrix;
}

glm::mat4 PanningCamera::get_inverse_projection_matrix() const {
    return inverse_projection_matrix;
}

float PanningCamera::get_gamma() const {
    return gamma;
}