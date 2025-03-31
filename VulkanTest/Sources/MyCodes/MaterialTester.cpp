#include "MaterialTester.h"
#include "Vertex.h"
#include "Sphere.h"
#include "Cube.h"
#include "VulkanTutorialExtension.h"

void MaterialTester::init(VulkanTutorialExtension* engine)
{
	sphere = std::make_shared<Sphere<Vertex>>();
	sphere->createMesh(engine);

	cube = std::make_shared<Cube<Vertex>>();
	cube->createMesh(engine);
}

void MaterialTester::createMaterial(VulkanTutorialExtension* engine,
const std::string& name,
const char* albedoPath,
const char* normalPath,
const char* metallicPath,
const char* roughnessPath,
const char* aoPath
)
{
	glm::vec4 textureFlags(0.f); // x=useNormalMap, y=useMetallicMap, z=useRoughnessMap, w=useAOMap

	assert(albedoPath);
	std::shared_ptr<AllocatedImage> colorImage = engine->createTexture2D(albedoPath, VK_FORMAT_R8G8B8A8_UNORM);
	images.push_back(colorImage);

	std::shared_ptr<AllocatedImage> normal, metallic, roughness, AO;
	if (normalPath)
	{
		normal = engine->createTexture2D(normalPath, VK_FORMAT_R8G8B8A8_UNORM);
		textureFlags.x = 1.f;
		images.push_back(normal);
	}

	if (metallicPath)
	{
		metallic = engine->createTexture2D(metallicPath, VK_FORMAT_R8G8B8A8_UNORM);
		textureFlags.y = 1.f;
		images.push_back(metallic);
	}

	if (roughnessPath)
	{
		roughness = engine->createTexture2D(roughnessPath, VK_FORMAT_R8G8B8A8_UNORM);
		textureFlags.z = 1.f;
		images.push_back(roughness);
	}

	if (aoPath)
	{
		AO = engine->createTexture2D(aoPath, VK_FORMAT_R8G8B8A8_UNORM);
		textureFlags.a = 1.f;
		images.push_back(AO);
	}

	GLTFMetallic_Roughness::Material material = engine->metalRoughMaterial.create_material_resources(engine, colorImage, normal, metallic, roughness, AO, textureFlags);

	materialMap.emplace(name, std::move(material));
}

void MaterialTester::draw(DrawContext& mainDrawContext, const std::string& name)
{
	if (materialMap.find(name) == materialMap.end())
	{
		std::cout << "No material " << name << std::endl;
		return;
	}

	RenderObject renderObject;
	renderObject.material = materialMap[name].materialInstances;
	renderObject.vertexBuffer = sphere->mesh.vertexBuffer.Buffer;
	renderObject.indexBuffer = sphere->mesh.indexBuffer.Buffer;
	renderObject.firstIndex = 0;
	renderObject.indexCount = sphere->mesh.indexBuffer.indices.size();
	renderObject.transform = glm::identity<glm::mat4>();

	mainDrawContext.OpaqueSurfaces.push_back(std::move(renderObject));
}

void MaterialTester::cleanUp(VkDevice device)
{
	//for(auto& [name, material] : materialMap)
	//{
	//	material.constants.destroy(0);
	//}

	sphere->cleanUp(device);
	cube->cleanUp(device);
}