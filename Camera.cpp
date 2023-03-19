#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        
        //TODO - Update the rest of camera parameters
        this->cameraFrontDirection = glm::vec3(0.f, 0.f, -1.f);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));

        this->pitch = 0.f;
        this->yaw = -90.f;

    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, glm::vec3(0.f, 1.f, 0.f));
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        
        switch (direction)
        {
            case MOVE_FORWARD:
                cameraPosition += speed * cameraFrontDirection;
                break;
            case MOVE_BACKWARD:
                cameraPosition -= speed * cameraFrontDirection;
                break;
            case MOVE_RIGHT:
                cameraPosition += speed * cameraRightDirection;
                break;
            case MOVE_LEFT: 
                cameraPosition -= speed * cameraRightDirection;
                break;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        
        this->pitch = pitch;
        this->yaw = yaw;

        glm::vec3 aux;
        aux.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        aux.y = -sin(glm::radians(pitch));
        aux.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        aux = glm::normalize(aux);

        cameraFrontDirection = glm::normalize(aux);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.f, 1.f, 0.f)));

    }
}