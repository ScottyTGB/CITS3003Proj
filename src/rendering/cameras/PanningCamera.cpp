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
            // Handle yaw and pitch rotation with right mouse button
            if (window.is_mouse_pressed(GLFW_MOUSE_BUTTON_RIGHT)) {
                auto mouseDelta = window.get_mouse_delta(GLFW_MOUSE_BUTTON_RIGHT);
                pitch -= PITCH_SPEED * (float)mouseDelta.y;
                yaw -= YAW_SPEED * (float)mouseDelta.x;
                
                // Ensure cursor is disabled when dragging
                window.set_cursor_disabled(true);
            }
            
            // Handle panning with middle mouse button
            if (window.is_mouse_pressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
                auto panDelta = window.get_mouse_delta(GLFW_MOUSE_BUTTON_MIDDLE);
                
                // First compute the correct forward, right, and up vectors based on our current orientation
                glm::mat4 rotationMatrix = glm::rotate(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f)) * 
                                          glm::rotate(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
                
                glm::vec3 forward = -glm::vec3(rotationMatrix[2]);
                glm::vec3 right = glm::vec3(rotationMatrix[0]);
                glm::vec3 up = glm::vec3(rotationMatrix[1]);
                
                // Pan the camera by moving the focus point
                float panScale = PAN_SPEED * dt * distance / (float)window.get_window_height();
                focus_point += right * (float)(-panDelta.x) * panScale + up * (float)(panDelta.y) * panScale;
                
                // Ensure cursor is disabled when panning
                window.set_cursor_disabled(true);
            }
            
            // Handle zooming with scroll wheel
            float scrollDelta = window.get_scroll_delta();
            if (scrollDelta != 0.0f) {
                distance -= ZOOM_SCROLL_MULTIPLIER * ZOOM_SPEED * scrollDelta;
            }
        }
    }

    // Apply constraints to our camera parameters
    yaw = std::fmod(yaw + YAW_PERIOD, YAW_PERIOD);
    pitch = clamp(pitch, PITCH_MIN, PITCH_MAX);
    distance = clamp(distance, MIN_DISTANCE, MAX_DISTANCE);

    // Calculate view matrix based on current camera parameters
    // First create rotation matrix for pitch and yaw
    glm::mat4 rotationMatrix = glm::rotate(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f)) * 
                              glm::rotate(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    
    // Calculate camera position from focus point, distance, and orientation
    glm::vec3 forward = -glm::vec3(rotationMatrix[2]);
    glm::vec3 camera_position = focus_point - forward * distance;
    
    // Create the view matrix
    view_matrix = glm::lookAt(camera_position, focus_point, glm::vec3(0.0f, 1.0f, 0.0f));
    inverse_view_matrix = glm::inverse(view_matrix);
    
    // Create projection matrix
    projection_matrix = glm::infinitePerspective(fov, window.get_framebuffer_aspect_ratio(), near);
    inverse_projection_matrix = glm::inverse(projection_matrix);
    
    // Release the cursor if no mouse buttons are pressed
    if (!window.is_mouse_pressed(GLFW_MOUSE_BUTTON_RIGHT) && 
        !window.is_mouse_pressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
        window.set_cursor_disabled(false);
    }
}

void PanningCamera::reset() {
    // Reset to the specified default values
    distance = 8.0f;
    focus_point = glm::vec3(0.0f, 0.0f, 0.0f);
    pitch = -45.0f;
    yaw = 315.0f;
    near = 0.01f;
    fov = glm::radians(90.0f);
    gamma = 2.2f;
}

void PanningCamera::add_imgui_options_section(const SceneContext& scene_context) {
    if (!ImGui::CollapsingHeader("Camera Options")) {
        return;
    }

    ImGui::DragFloat3("Focus Point (x,y,z)", &focus_point[0], 0.01f);
    ImGui::DragDisableCursor(scene_context.window);

    ImGui::DragFloat("Distance", &distance, 0.01f, MIN_DISTANCE, MAX_DISTANCE);
    ImGui::DragDisableCursor(scene_context.window);

    float pitch_degrees = pitch;  // Already in degrees
    ImGui::SliderFloat("Pitch", &pitch_degrees, -89.99f, 89.99f);
    pitch = pitch_degrees;

    float yaw_degrees = yaw;  // Already in degrees
    ImGui::DragFloat("Yaw", &yaw_degrees);
    ImGui::DragDisableCursor(scene_context.window);
    yaw = glm::mod(yaw_degrees, 360.0f);

    ImGui::SliderFloat("Near Plane", &near, 0.001f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);

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
        glm::radians(yaw),  // Convert to radians for storage
        glm::radians(pitch), // Convert to radians for storage
        fov,
        gamma
    };
}

void PanningCamera::load_properties(const CameraProperties& camera_properties) {
    yaw = glm::degrees(camera_properties.yaw);  // Convert to degrees for our system
    pitch = glm::degrees(camera_properties.pitch);  // Convert to degrees for our system
    fov = camera_properties.fov;
    gamma = camera_properties.gamma;
    
    // Calculate focus point from position, yaw, and pitch
    glm::mat4 rotationMatrix = glm::rotate(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f)) * 
                              glm::rotate(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 forward = -glm::vec3(rotationMatrix[2]);
    glm::vec3 camera_position = camera_properties.position;
    
    distance = 1.0f;  // Initial distance
    focus_point = camera_position + forward * distance;
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

glm::vec3 PanningCamera::get_position() const {
    // Calculate the actual camera position based on focus point, distance, and orientation
    glm::mat4 rotationMatrix = glm::rotate(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f)) * 
                              glm::rotate(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 forward = -glm::vec3(rotationMatrix[2]);
    return focus_point - forward * distance;
}