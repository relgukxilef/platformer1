#include "vr.h"

#include <stdexcept>

#include <glm/gtc/type_ptr.hpp>

using namespace rendering;

glm::mat4x3 pose_to_mat(vr::HmdMatrix34_t pose) {
    auto matrix = transpose(glm::make_mat3x4(reinterpret_cast<float*>(pose.m)));
    return matrix;
}

glm::mat4x4 pose_to_mat(vr::HmdMatrix44_t pose) {
    auto matrix = transpose(glm::make_mat4(reinterpret_cast<float*>(pose.m)));
    return matrix;
}

vr_window::vr_window() {
    vr::EVRInitError error = vr::VRInitError_None;
    vr_system = vr::VR_Init(&error, vr::VRApplication_Scene);

    if (error != vr::VRInitError_None) {
        auto message = VR_GetVRInitErrorAsEnglishDescription(error);
        throw new std::runtime_error(message);
    }

    vr_system->GetRecommendedRenderTargetSize(&width, &height);

    for (auto i = 0u; i < 2; i++) {
        // TODO: matrices might update
        projection_matrices[i] =
            pose_to_mat(vr_system->GetProjectionMatrix(eyes[i], 0.01f, 10.f)) *
            inverse(glm::mat4(pose_to_mat(
                vr_system->GetEyeToHeadTransform(eyes[i])
            )));
    }
}

vr_window::~vr_window() {
    vr::VR_Shutdown();
}

void vr_window::set_textures(GLuint left, GLuint right) {
    vr_frame_textures[0] = {
        reinterpret_cast<void*>(left),
        vr::TextureType_OpenGL, vr::ColorSpace_Gamma
    };
    vr_frame_textures[1] = {
        reinterpret_cast<void*>(right),
        vr::TextureType_OpenGL, vr::ColorSpace_Gamma
    };
}

void vr_window::wait_and_poll() {
    vr::VRCompositor()->WaitGetPoses(
        tracked_device_poses, vr::k_unMaxTrackedDeviceCount,
        nullptr, 0
    );

    glm::mat4x3 headset_pose;
    for (auto i = 0u; i < vr::k_unMaxTrackedDeviceCount; i++) {
        if (
            vr_system->GetTrackedDeviceClass(i) ==
            vr::TrackedDeviceClass::TrackedDeviceClass_HMD
        ) {
            auto pose = tracked_device_poses[i].mDeviceToAbsoluteTracking;
            headset_pose = pose_to_mat(pose);
            break;
        }
    }


    /*glViewport(
        0, 0,
        static_cast<GLsizei>(vr_width), static_cast<GLsizei>(vr_height)
    );*/

    glm::mat4 view_matrix = {
        1, 0, 0, 0,
        0, 0, -1, 0,
        0, 1, 0, 0,
        0, 0, 0, 1
    };

    for (auto i = 0u; i < 2; i++) {
        view_projection_matrices[i] =
            projection_matrices[i] * inverse(glm::mat4(headset_pose)) *
            view_matrix;
    }

    vr::VREvent_t event;
    while (vr_system->PollNextEvent(&event, sizeof(event))) {
        // TODO
    }
}

void vr_window::submit() {
    for (auto i = 0u; i < 2; i++) {
        vr::EVRCompositorError error =
            vr::VRCompositor()->Submit(eyes[i], &vr_frame_textures[i]);
        if (error != vr::VRCompositorError_None) {
            throw std::runtime_error("Compositor error");
        }
    }
}
