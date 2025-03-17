#pragma once

//  최대 포인트 라이트 개수. ObjectShader.frag 안의 NR_POINT_LIGHTS와 반드시 일치시킨다. 
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

struct GPUSceneData {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	//glm::mat4 viewproj;
	glm::vec4 ambientColor;
	glm::vec4 sunlightDirection; // w for sun power
	glm::vec4 sunlightColor;
};