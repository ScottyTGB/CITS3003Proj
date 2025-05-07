#include "PanningCamera.h"
#include <cmath>
#include <glm/gtx/transform.hpp>
#include "utility/Math.h"
#include "rendering/imgui/ImGuiManager.h"
#include "clippingPlane.h"

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

            // Handle zooming with scroll wheel and close up from the beginning
            float scrollDelta = window.get_scroll_delta();
            if (scrollDelta != 0.0f) {
                distance += ZOOM_SCROLL_MULTIPLIER * ZOOM_SPEED * scrollDelta;
            // Handle mouse scroll for zoom
            float scroll = window.get_scroll_delta();
            if (scroll != 0.0f) {
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

    // Apply constraints to our camera parameters
    // Apply constraints
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
    // Calculate the camera position based on focus point, distance, pitch, and yaw
    glm::mat4 rotation_matrix = glm::rotate(yaw, glm::vec3{0.0f, 1.0f, 0.0f}) *
                               glm::rotate(pitch, glm::vec3{-1.0f, 0.0f, 0.0f});
    glm::vec3 forward = glm::vec3{rotation_matrix[2]};
    glm::vec3 position = focus_point - forward * distance;

    // Create the view matrix
    view_matrix = glm::lookAt(camera_position, focus_point, glm::vec3(0.0f, 1.0f, 0.0f));
    // Create view matrix
    view_matrix = glm::lookAt(position, focus_point, UP);
    inverse_view_matrix = glm::inverse(view_matrix);

    // Create projection matrix

    // Update projection matrix
    projection_matrix = glm::infinitePerspective(fov, window.get_framebuffer_aspect_ratio(), near);
    inverse_projection_matrix = glm::inverse(projection_matrix);

    // Release the cursor if no mouse buttons are pressed
    if (!window.is_mouse_pressed(GLFW_MOUSE_BUTTON_RIGHT) &&
        !window.is_mouse_pressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
        window.set_cursor_disabled(false);
    }
}

void PanningCamera::reset() {
    // Reset to the specified default value
    distance = 3.0f; // closer distance to the object
    focus_point = glm::vec3(0.0f, 0.0f, 0.0f);
    pitch = -45.0f;
    yaw = 315.0f;
    near = 0.01f;
    fov = glm::radians(90.0f);
    gamma = 2.2f;
    distance = init_distance;
    focus_point = init_focus_point;
    pitch = init_pitch;
    yaw = init_yaw;
    fov = init_fov;
    near = init_near;
    gamma = init_gamma;
}

// runtime default distance

void PanningCamera::add_imgui_options_section(const SceneContext& scene_context) {
@@ -112,14 +104,14 @@ void PanningCamera::add_imgui_options_section(const SceneContext& scene_context)

    // initializer for distance
    float MIN_DISTANCE = 0.01f;
    float MAX_DISTANCE = 100.0f;

    ImGui::DragFloat("Distance", &distance, 0.01f, MIN_DISTANCE, MAX_DISTANCE);
    ImGui::DragDisableCursor(scene_context.window);

    float pitch_degrees = pitch;  // Already in degrees
    float pitch_degrees = glm::degrees(pitch);
    ImGui::SliderFloat("Pitch", &pitch_degrees, -89.99f, 89.99f);
    pitch = pitch_degrees;
    pitch = glm::radians(pitch_degrees);

    float yaw_degrees = yaw;  // Already in degrees
    float yaw_degrees = glm::degrees(yaw);
    ImGui::DragFloat("Yaw", &yaw_degrees);
    ImGui::DragDisableCursor(scene_context.window);
    yaw = glm::mod(yaw_degrees, 360.0f);
    yaw = glm::radians(glm::mod(yaw_degrees, 360.0f));

    ImGui::SliderFloat("Near Plane", &near, 0.001f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);

@@ -138,27 +130,36 @@ void PanningCamera::add_imgui_options_section(const SceneContext& scene_context)
CameraProperties PanningCamera::save_properties() const {
    return CameraProperties{
        get_position(),
        glm::radians(yaw),  // Convert to radians for storage
        glm::radians(pitch), // Convert to radians for storage
        yaw,
        pitch,
        fov,
        gamma
    };
}

void PanningCamera::load_properties(const CameraProperties& camera_properties) {
    yaw = glm::degrees(camera_properties.yaw);  // Convert to degrees for our system
    pitch = glm::degrees(camera_properties.pitch);  // Convert to degrees for our system
    yaw = camera_properties.yaw;
    pitch = camera_properties.pitch;
    fov = camera_properties.fov;
    gamma = camera_properties.gamma;

    // Calculate focus point from position, yaw, and pitch
    glm::mat4 rotationMatrix = glm::rotate(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
                              glm::rotate(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 forward = -glm::vec3(rotationMatrix[2]);
    glm::vec3 camera_position = camera_properties.position;

    distance = 1.0f;  // Initial distance
    focus_point = camera_position + forward * distance;
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
@@ -179,12 +180,4 @@ glm::mat4 PanningCamera::get_inverse_projection_matrix() const {

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
