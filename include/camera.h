#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>
#include "glad/glad.h"

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

#define YAW         -90.0f
#define PITCH       0.0f
#define SPEED       30.f
#define SENSITIVITY 0.1f
#define ZOOM        45.0f

typedef struct Camera {
    vec3 Position;
    vec3 Front;
    vec3 Up;
    vec3 Right;
    vec3 WorldUp;
    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
} Camera;

static inline void updateCameraVectors(Camera* cam);

static inline void Camera_init(Camera* cam, vec3 position, vec3 up, float yaw, float pitch) {
    glm_vec3_copy(position, cam->Position);
    glm_vec3_copy(up, cam->WorldUp);
    cam->Yaw = yaw;
    cam->Pitch = pitch;
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam->Front);
    cam->MovementSpeed = SPEED;
    cam->MouseSensitivity = SENSITIVITY;
    cam->Zoom = ZOOM;
    updateCameraVectors(cam);
}

static inline void Camera_initPos(Camera* cam, float posX, float posY, float posZ, 
                                 float upX, float upY, float upZ, float yaw, float pitch) {
    Camera_init(cam, (vec3){posX, posY, posZ}, (vec3){upX, upY, upZ}, yaw, pitch);
}

static inline void GetViewMatrix(Camera* cam, mat4 out) {
    vec3 center;
    glm_vec3_add(cam->Position, cam->Front, center);
    glm_lookat(cam->Position, center, cam->Up, out);
}

static inline void ProcessKeyboard(Camera* cam, enum Camera_Movement direction, float deltaTime) {
    float velocity = cam->MovementSpeed * deltaTime;
    switch (direction) {
        case FORWARD:
            glm_vec3_muladds(cam->Front, velocity, cam->Position);
            break;
        case BACKWARD:
            glm_vec3_muladds(cam->Front, -velocity, cam->Position);
            break;
        case LEFT:
            glm_vec3_muladds(cam->Right, -velocity, cam->Position);
            break;
        case RIGHT:
            glm_vec3_muladds(cam->Right, velocity, cam->Position);
            break;
        case UP:
            glm_vec3_muladds(cam->Up, velocity, cam->Position);
            break;
        case DOWN:
            glm_vec3_muladds(cam->Up, -velocity, cam->Position);
            break;
    }
}

static inline void ProcessMouseMovement(Camera* cam, float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= cam->MouseSensitivity;
    yoffset *= cam->MouseSensitivity;

    cam->Yaw += xoffset;
    cam->Pitch += yoffset;

    if (constrainPitch) {
        if (cam->Pitch > 89.0f)
            cam->Pitch = 89.0f;
        else if (cam->Pitch < -89.0f)
            cam->Pitch = -89.0f;
    }

    updateCameraVectors(cam);
}

static inline void ProcessMouseScroll(Camera* cam, float yoffset) {
    cam->Zoom -= yoffset;
    if (cam->Zoom < 1.0f)
        cam->Zoom = 1.0f;
    else if (cam->Zoom > 45.0f)
        cam->Zoom = 45.0f;
}

static inline void updateCameraVectors(Camera* cam) {
    // Calculate new Front vector
    vec3 front;
    front[0] = cosf(glm_rad(cam->Yaw)) * cosf(glm_rad(cam->Pitch));
    front[1] = sinf(glm_rad(cam->Pitch));
    front[2] = sinf(glm_rad(cam->Yaw)) * cosf(glm_rad(cam->Pitch));
    glm_vec3_normalize_to(front, cam->Front);

    // Re-calculate Right and Up vectors
    glm_vec3_crossn(cam->Front, cam->WorldUp, cam->Right);
    glm_vec3_crossn(cam->Right, cam->Front, cam->Up);
}

#endif /* CAMERA_H */
