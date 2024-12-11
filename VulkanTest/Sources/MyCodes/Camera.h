#ifndef CAMERA_H
#define CAMERA_H

//#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <sstream>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	glm::vec3 EyeCenter = glm::vec3(0.f, 0.f, 0.f);
	// euler Angles
	float Yaw;
	float Pitch;
	// camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	bool bOffsetInitialized = false;
	float preXOffset = 0.f;
	float preYOffset = 0.f;

	// constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, 1.0f)), Right(1.f, 0.f, 0.f), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		Up = WorldUp;
		Front = glm::normalize(EyeCenter - position);
		Right = glm::normalize(glm::cross(Front, Up));
		updateCameraVectors(0.f, 0.f);
	}
	// constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, 1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		Up = WorldUp;
		updateCameraVectors(0.f, 0.f);
	}

	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, EyeCenter, Up);
	}

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
	}

	void ResetPreoffsets()
	{
		bOffsetInitialized = false;
	}

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		if (!bOffsetInitialized)
		{
			preXOffset = xoffset;
			preYOffset = yoffset;

			bOffsetInitialized = true;
		}

		float deltaXOffset = xoffset - preXOffset;
		float deltaYOffset = yoffset - preYOffset;

		preXOffset = xoffset;
		preYOffset = yoffset;

		Yaw += xoffset;
		Pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		/*if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}*/

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors(deltaXOffset, deltaYOffset);
	}

	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		Zoom -= (float)yoffset;
		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
	}

	std::string ToString()
	{
		std::ostringstream oss;
		//oss << "Pitch " << Pitch << ", Yaw " << Yaw << "\n";
		oss << "Front (" << Front.x << ", " << Front.y << ", " << Front.z << ")\n";
		oss << "Up (" << Up.x << ", " << Up.y << ", " << Up.z << ")\n";
		oss << "Right (" << Right.x << ", " << Right.y << ", " << Right.z << ")\n";

		return oss.str();
	}

private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors(float xOffset, float yOffset)
	{
		auto view_transform = glm::mat4(1.f); 
		view_transform = glm::rotate(view_transform, glm::radians(xOffset), Up);
		view_transform = glm::rotate(view_transform, glm::radians(yOffset), Right);

		Position = view_transform * glm::vec4(Position, 1.f);
		Front = glm::normalize(EyeCenter - Position);	
		Up = view_transform * glm::vec4(Up, 0.f);

		// Consider a right-handed coordinate system of vulkan.
		Right = glm::normalize(glm::cross(Up, Front));
	}
};
#endif