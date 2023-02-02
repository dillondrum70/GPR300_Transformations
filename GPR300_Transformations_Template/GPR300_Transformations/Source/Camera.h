#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

struct Camera
{
	glm::vec3 eye = glm::vec3(0, 0, 1);
	glm::vec3 target = glm::vec3(0, 0, 0);
	glm::vec3 worldUp = glm::vec3(0, 1, 0);

	float fov;
	float orthographicSize;
	bool orthographic;

	void UpdateEye(float orbitSpeed, float orbitRadius, float deltaTime)
	{
		glm::vec3 forward = glm::normalize(target - eye);
		glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));

		eye = glm::normalize(eye) + (right * orbitSpeed * deltaTime);
		glm::vec3 diff = glm::normalize(glm::vec3(eye - target)) * orbitRadius;
		eye = target + diff;
	}

	glm::mat4 GetViewMatrix()
	{
		glm::mat4 rotation = glm::mat4(1);

		glm::vec3 forward = glm::normalize(target - eye);
		glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
		glm::vec3 up = glm::normalize(glm::cross(right, forward));

		rotation[0] = glm::vec4(right, 0);
		rotation[1] = glm::vec4(up, 0);
		rotation[2] = glm::vec4(-forward, 0);

		glm::mat4 translation = glm::mat4(1);

		translation[3] = glm::vec4(eye, 1);

		return glm::inverse(rotation) * glm::inverse(translation);
	}

	glm::mat4 GetProjectionMatrixPerspective(float fov, float aspectRatio, float nearPlane, float farPlane)
	{
		float c = glm::tan(fov / 2);

		glm::mat4 proj = glm::mat4(1);

		proj[0][0] = 1 / (aspectRatio * c);
		proj[1][1] = 1 / c;
		proj[2][2] = -((farPlane + nearPlane) / (farPlane - nearPlane));
		proj[3][2] = -((2 * farPlane * nearPlane) / (farPlane - nearPlane));
		proj[2][3] = -1;

		return proj;
	}

	glm::mat4 GetProjectionMatrixOrtho(float height, float aspectRatio, float nearPlane, float farPlane)
	{
		float t = height / 2;
		float b = -t;
		float l = b * aspectRatio;
		float r = t * aspectRatio;

		glm::mat4 proj = glm::mat4(1);

		proj[0][0] = 2 / (r - l);
		proj[1][1] = 2 / (t - b);
		proj[2][2] = -2 / (farPlane - nearPlane);
		proj[3][0] = -(r + l) / (r - l);
		proj[3][1] = -(t + b) / (t - b);
		proj[3][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);

		return proj;
	}
};

#endif
