#pragma once

//  �ִ� ����Ʈ ����Ʈ ����. ObjectShader.frag ���� NR_POINT_LIGHTS�� �ݵ�� ��ġ��Ų��. 
#define NR_POINT_LIGHTS 4

struct Transform {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct ColorUBO {

	alignas(16) glm::vec3 objectColor;
	alignas(16) glm::vec3 lightColor;
	//alignas(16) glm::vec3 lightPos;
	alignas(16) glm::vec3 viewPos;
	alignas(16) glm::u32vec4 Debug;
};

struct Material {
	alignas(16) glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);;
	alignas(16) glm::vec3 diffuse = glm::vec3(0.0f, 0.0f, 0.0f);;
	alignas(16) glm::vec3 specular = glm::vec3(0.0f, 0.0f, 0.0f);;
	alignas(16) glm::vec3 shininess = glm::vec3(0.0f, 0.0f, 0.0f);;
};

struct DirLight {
	alignas(16) glm::vec3 direction;
	alignas(16) glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
	alignas(16) glm::vec3 diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
	alignas(16) glm::vec3 specular = glm::vec3(0.0f, 0.0f, 0.0f);
};

struct PointLight {
	alignas(16) glm::vec3 position;
	alignas(16) glm::vec3 clq; // constant, linear, quadratic
	alignas(16) glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
	alignas(16) glm::vec3 diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
	alignas(16) glm::vec3 specular = glm::vec3(0.0f, 0.0f, 0.0f);
};

struct PointLightsUniform {
	std::array<PointLight, NR_POINT_LIGHTS> pointLights;
	int activeLightMask;
};