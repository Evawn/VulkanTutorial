#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

class Camera
{
	glm::vec3 m_position;
	glm::vec3 m_up;
	glm::vec3 m_forward;
	float m_fovy, m_aspect, m_znear, m_zfar;

	static std::shared_ptr<Camera> Create() {
		auto ret = std::make_shared<Camera>();
		ret->m_position = glm::vec3(0.0f, 0.0f, 0.0f);
		ret->m_up = glm::vec3(0.0f, 1.0f, 0.0f);
		ret->m_forward = glm::vec3(0.0f, 0.0f, -1.0f);
		ret->m_fovy = 45.0f;
		ret->m_aspect = 1.0f;
		ret->m_znear = 0.1f;
		ret->m_zfar = 100.0f;
		return ret;
	}

	glm::mat4 GetViewMatrix() {
		return glm::lookAt(m_position, m_position + m_forward, m_up);
	}

	glm::mat4 GetProjectionMatrix() {
		auto mat = glm::perspective(glm::radians(m_fovy), m_aspect, m_znear, m_zfar);
		mat[1][1] *= -1;
		return mat;
	}
};

