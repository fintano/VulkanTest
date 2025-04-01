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
const std::string& albedoPath,
const std::string& normalPath,
const std::string& metallicPath,
const std::string& roughnessPath,
const std::string& aoPath
)
{
	auto contains = [&](const std::string& path) {
			return images.find(path) != images.end();
		};
	glm::vec4 textureFlags(0.f); // x=useNormalMap, y=useMetallicMap, z=useRoughnessMap, w=useAOMap

	assert(!albedoPath.empty());
	std::shared_ptr<AllocatedImage> colorImage = engine->createTexture2D(albedoPath, VK_FORMAT_R8G8B8A8_UNORM);
	images[albedoPath] = colorImage;

	std::shared_ptr<AllocatedImage> normal, metallic, roughness, AO;
	if (!normalPath.empty())
	{
		normal = contains(normalPath) ? images[normalPath] : engine->createTexture2D(normalPath, VK_FORMAT_R8G8B8A8_UNORM);
		textureFlags.x = 1.f;
		images[normalPath] = normal;
	}

	if (!metallicPath.empty())
	{
		metallic = contains(metallicPath) ? images[metallicPath] : engine->createTexture2D(metallicPath, VK_FORMAT_R8G8B8A8_UNORM);
		textureFlags.y = 1.f;
		images[metallicPath] = metallic;
	}

	if (!roughnessPath.empty())
	{
		roughness = contains(roughnessPath) ? images[roughnessPath] : engine->createTexture2D(roughnessPath, VK_FORMAT_R8G8B8A8_UNORM);
		textureFlags.z = 1.f;
		images[roughnessPath] = roughness;
	}

	if (!aoPath.empty())
	{
		AO = contains(aoPath) ? images[aoPath] : engine->createTexture2D(aoPath, VK_FORMAT_R8G8B8A8_UNORM);
		textureFlags.a = 1.f;
		images[aoPath] = AO;
	}

	std::shared_ptr<GLTFMetallic_Roughness::Material> material = engine->metalRoughMaterial.create_material_resources(engine, colorImage, normal, metallic, roughness, AO, textureFlags);

	DeferredDeletionQueue::get().pushResource(materialMap[name]);
	materialMap[name] = std::move(material);

	engine->markCommandBufferRecreation();
}

void MaterialTester::selectModel(MaterialTester::Model inModel)
{
	model = inModel;
}

void MaterialTester::draw(DrawContext& mainDrawContext, const std::string& name)
{
	if (materialMap.find(name) == materialMap.end())
	{
		return;
	}

	RenderObject renderObject;
	renderObject.material = materialMap[name]->materialInstances;
	renderObject.vertexBuffer = model == Model::Sphere ? sphere->mesh.vertexBuffer.Buffer : cube->mesh.vertexBuffer.Buffer;
	renderObject.indexBuffer = model == Model::Sphere ? sphere->mesh.indexBuffer.Buffer : cube->mesh.indexBuffer.Buffer;
	renderObject.firstIndex = 0;
	renderObject.indexCount = model == Model::Sphere ? sphere->mesh.indexBuffer.indices.size() : cube->mesh.indexBuffer.indices.size();
	renderObject.transform = glm::identity<glm::mat4>();

	mainDrawContext.OpaqueSurfaces.push_back(std::move(renderObject));
}

void MaterialTester::cleanUp(VkDevice device)
{
	sphere->cleanUp(device);
	cube->cleanUp(device);
}