//
// Created by matf-rg on 15.11.20..
//

#ifndef PROJECT_BASE_CAMERA_H
#define PROJECT_BASE_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Direction{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera{

    void updateCameraVectors(){
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front,WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }

    glm::mat4 calculateLookAt(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp){

        //na osnovu teorije sa casa
        glm::vec3 direction = glm::normalize(position - target);

        glm::vec3 right = glm::normalize(glm::cross(glm::normalize(worldUp), direction));

        glm::vec3 up = glm::cross(direction, right);



        glm::mat4 translation = glm::mat4(1.0f);
        translation[3][0] = -position.x;
        translation[3][1] = -position.y;
        translation[3][2] = -position.z;

        glm::mat4 rotation = glm::mat4 (1.0f);
        rotation[0][0] = right.x;
        rotation[1][0] = right.y;
        rotation[2][0] = right.z;
        rotation[0][1] = up.x;
        rotation[1][1] = up.y;
        rotation[2][1] = up.z;
        rotation[0][2] = direction.x;
        rotation[1][2] = direction.y;
        rotation[2][2] = direction.z;

        return rotation * translation;

    }

public:
    float  Zoom = 45.0f;
    float MovementSpeed = 2.5f;

    float Yaw = -90.0f;
    float Pitch = 0.0f;
    float MouseSensitivity = 0.2f;
    bool ConstrainPitch = true;

    glm::vec3 Position = glm::vec3(0.0f, 2.0f,10.0f);
    glm::vec3 WorldUp  = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 Front = glm::vec3(0, 0, -1);

    Camera(){
        updateCameraVectors();
    }
    glm::mat4 GetViewMatrix() const{

        //return calculateLookAt(Position, Front + Position, Up);
        return glm::lookAt(Position, Front + Position, Up);
    }
    void ProcessKeyboard(Direction direction,float deltaTime){

        float cameraSpeed = MovementSpeed* deltaTime;
        switch (direction){
            case FORWARD:
                Position += Front * cameraSpeed;
                break;
            case BACKWARD:
                Position -= Front * cameraSpeed;
                break;
            case  LEFT:
                Position -= Right * cameraSpeed;
                break;
            case RIGHT:
                Position += Right * cameraSpeed;
                break;
        }
        Position.y = 0.0f;
    }
    void ProcessMouseMovement(float xoffset, float yoffset) {

        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;   //levo-desno
        Pitch += yoffset;   //gore-dole

        //Ogranicenje da kamera ne bi mogla da se prevrne
        if (ConstrainPitch) {
            if (Pitch > 89.0f) {
            Pitch = 89.0f;
            }
            if (Pitch < -89.0f) {
            Pitch = -89.0f;
            }
        }
        updateCameraVectors();
    }
    void ProcessMouseScroll(float yofset){

        Zoom -= (float)yofset;
        if(Zoom  < 1.0f){
            Zoom  = 1.0f;
        }
        if(Zoom  > 45.0f){
            Zoom  = 45.0f;
        }
    }



};
#endif //PROJECT_BASE_CAMERA_H
