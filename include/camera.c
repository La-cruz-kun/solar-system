#ifndef CAMERA_H
#include "camera.h"
#define CAMERA_H
#endif /* ifndef CAMERA_H */

void CameraInitVector(Camera *camera, float *position, float *up, float yaw,
                      float pitch) {
  glm_vec3_copy((vec3){0.0, 0.0, -1.0}, camera->Front);
  camera->MovementSpeed = SPEED;
  camera->MouseSensitivity = SENSITIVITY;
  camera->Zoom = ZOOM;
  glm_vec3_copy(position, camera->Position);
  glm_vec3_copy(up, camera->WorldUp);
  camera->Yaw = yaw;
  camera->Pitch = pitch;
  updateCameraVectors(camera);
};

void CameraInitScaler(Camera *camera, float posX, float posY, float posZ,
                      float upX, float upY, float upZ, float yaw, float pitch) {
  glm_vec3_copy((vec3){0.0, 0.0, -1.0}, camera->Front);
  camera->MovementSpeed = SPEED;
  camera->MouseSensitivity = SENSITIVITY;
  camera->Zoom = ZOOM;
  glm_vec3_copy((vec3){posX, posY, posZ}, camera->Position);
  glm_vec3_copy((vec3){upX, upY, upZ}, camera->WorldUp);
  camera->Yaw = yaw;
  camera->Pitch = pitch;
  updateCameraVectors(camera);
}

void CameraGetViewMatrix(Camera camera, mat4 destination) {
  vec3 temp;
  glm_vec3_add(camera.Position, camera.Front, temp);
  glm_lookat(camera.Position, temp, camera.Up, destination);
}

void CameraProcessKeyboard(Camera *camera, enum Camera_Movement direction,
                           float deltaTime) {
  float velocity = camera->MovementSpeed * deltaTime;
  if (direction == FORWARD) {
    vec3 temp;
    vec3 temp2;
    glm_vec3_scale(camera->Front, velocity, temp);
    glm_vec3_add(camera->Position, temp, temp2);
    glm_vec3_copy(temp2, camera->Position);
  }
  if (direction == BACKWARD) {
    vec3 temp;
    vec3 temp2;
    glm_vec3_scale(camera->Front, velocity, temp);
    glm_vec3_sub(camera->Position, temp, temp2);
    glm_vec3_copy(temp2, camera->Position);
  }
  if (direction == LEFT) {
    vec3 temp;
    vec3 temp2;
    glm_vec3_scale(camera->Right, velocity, temp);
    glm_vec3_add(camera->Position, temp, temp2);
    glm_vec3_copy(temp2, camera->Position);
  }
  if (direction == RIGHT) {
    vec3 temp;
    vec3 temp2;
    glm_vec3_scale(camera->Right, velocity, temp);
    glm_vec3_sub(camera->Position, temp, temp2);
    glm_vec3_copy(temp2, camera->Position);
  }
}

void CameraProcessMouseMovement(Camera *camera, float xoffset, float yoffset,
                                GLboolean constrainPitch) {
  xoffset *= camera->MouseSensitivity;
  yoffset *= camera->MouseSensitivity;

  camera->Yaw += xoffset;
  camera->Pitch += yoffset;

  if (constrainPitch) {
    if (camera->Pitch > 89.0)
      camera->Pitch = 89.0;
    if (camera->Pitch < -89.0)
      camera->Pitch = -89.0;
  }
  updateCameraVectors(camera);
}

void CameraProcessMouseScroll(Camera *camera, float yoffset) {
  camera->Zoom -= yoffset;
  if (camera->Zoom < 1.0)
    camera->Zoom = 1.0;
  if (camera->Zoom > 45.0)
    camera->Zoom = 45.0;
}

void updateCameraVectors(Camera *camera) {
  vec3 front;
  vec3 temp;
  front[0] = cos(glm_rad(camera->Yaw)) * sin(glm_rad(camera->Pitch));
  front[1] = sin(glm_rad(camera->Pitch));
  front[2] = sin(glm_rad(camera->Yaw)) * cos(glm_rad(camera->Pitch));
  glm_vec3_normalize(front);
  glm_vec3_copy(front, camera->Front);
  glm_cross(camera->Front, camera->WorldUp, temp);
  glm_vec3_normalize(temp);
  glm_vec3_copy(temp, camera->Right);
  glm_cross(camera->Right, camera->Front, temp);
  glm_normalize(temp);
  glm_vec3_copy(temp, camera->Up);
}
