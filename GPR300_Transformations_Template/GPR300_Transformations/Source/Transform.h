#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Math3D.h"
#include <glm/glm.hpp>

struct Transform
{
	glm::vec3 position;
	glm::vec3 rotation;	//Euler Angles
	glm::vec3 scale;

	glm::mat4 GetModelMatrix()
	{
		glm::mat4 modelMatrix = glm::mat4(1);

		modelMatrix = Math3D::TranslateMatrix(position) * Math3D::RotateMatrix(rotation) * Math3D::ScaleMatrix(scale) * modelMatrix;

		return modelMatrix;
	}

	void CreateImGui(int pushId)
	{
		ImGui::PushID(pushId);

		float pos[3] = { position.x, position.y, position.z };
		if (ImGui::DragFloat3("Position", pos, .01, -40.0f, 40.0f))
		{
			position.x = pos[0];
			position.y = pos[1];
			position.z = pos[2];
		}

		float rot[3] = { rotation.x, rotation.y, rotation.z };
		if (ImGui::DragFloat3("Rotation", rot, .01, 0.0f, 360.0f))
		{
			rotation.x = rot[0];
			rotation.y = rot[1];
			rotation.z = rot[2];
		}

		float s[3] = { scale.x, scale.y, scale.z };
		if (ImGui::DragFloat3("Scale", s, .01, 0.0f, 360.0f))
		{
			scale.x = s[0];
			scale.y = s[1];
			scale.z = s[2];
		}

		ImGui::PopID();
	}
};

#endif
