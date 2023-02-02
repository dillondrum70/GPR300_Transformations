#ifndef MATH3D_H
#define MATH3D_H

#include <glm/glm.hpp>

namespace Math3D
{
	glm::mat4 ScaleMatrix(const glm::vec3& s)
	{
		glm::mat4 model = glm::mat4(1);
		model[0][0] = s.x;
		model[1][1] = s.y;
		model[2][2] = s.z;
		return model;
	}

	glm::mat4 RotateMatrix(const glm::vec3& e)
	{
		glm::mat4 pitch = glm::mat4(1);
		pitch[1][1] = cos(e.x);
		pitch[2][1] = -sin(e.x);
		pitch[1][2] = sin(e.x);
		pitch[2][2] = cos(e.x);

		glm::mat4 yaw = glm::mat4(1);
		yaw[0][0] = cos(e.y);
		yaw[2][0] = sin(e.y);
		yaw[0][2] = -sin(e.y);
		yaw[2][2] = cos(e.y);

		glm::mat4 roll = glm::mat4(1);
		roll[0][0] = cos(e.z);
		roll[1][0] = -sin(e.z);
		roll[0][1] = sin(e.z);
		roll[1][1] = cos(e.z);

		return pitch * yaw * roll;
	}

	glm::mat4 TranslateMatrix(const glm::vec3& p)
	{
		glm::mat4 model = glm::mat4(1);
		model[3][0] = p.x;
		model[3][1] = p.y;
		model[3][2] = p.z;
		return model;
	}
}

#endif
