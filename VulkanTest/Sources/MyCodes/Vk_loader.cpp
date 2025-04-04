#include "stb_image.h"
#include <iostream>
#include <vk_loader.h>

#include <glm/gtx/quaternion.hpp>

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

#include "Vertex.h"
#include "VulkanTutorialExtension.h"
#include "vk_initializers.h"
#include "VulkanTools.h"
#include "vk_resource_utils.h"
#include "vk_pathes.h"

VkFilter extract_filter(fastgltf::Filter filter)
{
    switch (filter) {
        // nearest samplers
    case fastgltf::Filter::Nearest:
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::NearestMipMapLinear:
        return VK_FILTER_NEAREST;

        // linear samplers
    case fastgltf::Filter::Linear:
    case fastgltf::Filter::LinearMipMapNearest:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
        return VK_FILTER_LINEAR;
    }
}

VkSamplerMipmapMode extract_mipmap_mode(fastgltf::Filter filter)
{
    switch (filter) {
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::LinearMipMapNearest:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;

    case fastgltf::Filter::NearestMipMapLinear:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }
}

std::optional<std::shared_ptr<AllocatedImage>> load_image(VulkanTutorialExtension* engine, fastgltf::Asset& asset, fastgltf::Image& image)
{
    std::shared_ptr<AllocatedImage> newImage = std::make_shared<AllocatedImage>(engine->getDevicePtr());

    int width, height, nrChannels;

    std::visit(
        fastgltf::visitor
        {
            [](auto& arg) {},
            [&](fastgltf::sources::URI& filePath)
            {
                assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
                assert(filePath.uri.isLocalPath()); // We're only capable of loading
                // local files.

                const std::string path(filePath.uri.path().begin(), filePath.uri.path().end()); // Thanks C++.
                stbi_uc* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
                if (data) {
                    VkExtent3D imagesize;
                    imagesize.width = width;
                    imagesize.height = height;
                    imagesize.depth = 1;

                    newImage = engine->createTexture2D(data, imagesize, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT);

                    //stbi_image_free(data);
                }
            },

            [&](fastgltf::sources::Vector& vector)
            {
                stbi_uc* data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(vector.bytes.data()), static_cast<int>(vector.bytes.size()),
                    &width, &height, &nrChannels, 4);
                if (data) {
                    VkExtent3D imagesize;
                    imagesize.width = width;
                    imagesize.height = height;
                    imagesize.depth = 1;

                    newImage = engine->createTexture2D(data, imagesize, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT);

                    //stbi_image_free(data);
                }
            },

            [&](fastgltf::sources::BufferView& view)
            {
                auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                auto& buffer = asset.buffers[bufferView.bufferIndex];

                std::visit(fastgltf::visitor
                {
                        // We only care about VectorWithMime here, because we
                        // specify LoadExternalBuffers, meaning all buffers
                        // are already loaded into a vector.
                        [](auto& arg) 
                        {
                            // for debugging type
                            //std::cout << "Type: " << typeid(arg).name() << std::endl;
                        },

                        [&](fastgltf::sources::Vector& vector)
                        {
                            stbi_uc* data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(vector.bytes.data() + bufferView.byteOffset),
                                static_cast<int>(bufferView.byteLength),
                                &width, &height, &nrChannels, 4);
                            if (data)
                            {
                                VkExtent3D imagesize;
                                imagesize.width = width;
                                imagesize.height = height;
                                imagesize.depth = 1;

                                newImage = engine->createTexture2D(data, imagesize, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT);

                                //stbi_image_free(data);
                            }
                        },
                        [&](fastgltf::sources::Array& array)
                        {
                            stbi_uc* data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(array.bytes.data() + bufferView.byteOffset),
                                static_cast<int>(bufferView.byteLength),
                                &width, &height, &nrChannels, 4);
                            if (data)
                            {
                                VkExtent3D imagesize;
                                imagesize.width = width;
                                imagesize.height = height;
                                imagesize.depth = 1;
                                newImage = engine->createTexture2D(data, imagesize, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT);
                                //stbi_image_free(data);
                            }
                        }
                    }, buffer.data);
                },
        }, image.data);

    // if any of the attempts to load the data failed, we havent written the image
    // so handle is null
    if (newImage->image == VK_NULL_HANDLE) {
        return {};
    }
    else {
        return newImage;
    }
}

// 메시의 winding order 분석을 위한 함수 - 함수 시작 전에 추가
bool isCounterClockwise(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& normal) {
    // 삼각형의 두 엣지 계산
    glm::vec3 edge1 = p1 - p0;
    glm::vec3 edge2 = p2 - p0;

    // 외적을 통해 삼각형의 법선 계산
    glm::vec3 computedNormal = glm::cross(edge1, edge2);

    // 계산된 법선과 주어진 법선의 방향이 일치하는지 확인
    return glm::dot(computedNormal, normal) > 0.0f;
}

std::optional<std::shared_ptr<LoadedGLTF>> loadGltf(VulkanTutorialExtension* engine, std::string_view filePath)
{
	std::cout << "Loading GLTF : " << filePath << std::endl;

    std::shared_ptr<LoadedGLTF> scene = std::make_shared<LoadedGLTF>();
    scene->creator = engine;
    LoadedGLTF& file = *scene.get();

	constexpr auto gltfOptions = fastgltf::Options::LoadExternalBuffers;

	fastgltf::Parser parser{};
    fastgltf::Expected<fastgltf::GltfDataBuffer> data = Utils::loadModel(filePath);
    fastgltf::Asset gltf;
    std::filesystem::path path = Utils::GetProjectRoot() / filePath;

	if (data.error() == fastgltf::Error::InvalidPath)
	{
		return std::nullopt;
	}

    auto type = fastgltf::determineGltfFileType(data.get());
    if (type == fastgltf::GltfType::glTF) {
        auto load = parser.loadGltf(data.get(), path.parent_path(), gltfOptions);
        if (load) {
            gltf = std::move(load.get());
        }
        else {
            std::cerr << "Failed to load glTF: " << fastgltf::to_underlying(load.error()) << std::endl;
            return {};
        }
    }
    else if (type == fastgltf::GltfType::GLB) {
        auto load = parser.loadGltfBinary(data.get(), path.parent_path(), gltfOptions);
        if (load) {
            gltf = std::move(load.get());
        }
        else {
            std::cerr << "Failed to load glb: " << fastgltf::to_underlying(load.error()) << std::endl;
            return {};
        }
    }
    else {
        std::cerr << "Failed to determine glTF container" << std::endl;
        return {};
    }

    // we can estimate the descriptors we will need accurately
    std::vector<VkDescriptorPoolSize> sizes =
    {
        vkb::initializers::descriptor_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 7 * gltf.materials.size() * engine->getSwapchainImageNum()),
        vkb::initializers::descriptor_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, gltf.materials.size() * engine->getSwapchainImageNum()),
        vkb::initializers::descriptor_pool_size(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, gltf.materials.size())
    };

    VkDescriptorPoolCreateInfo poolInfo = vkb::initializers::descriptor_pool_create_info(sizes, gltf.materials.size() * engine->getSwapchainImageNum());
    VK_CHECK_RESULT(vkCreateDescriptorPool(engine->getDevice(), &poolInfo, nullptr, &file.descriptorPool));

    // load samplers
    for (fastgltf::Sampler& sampler : gltf.samplers) {

        VkSamplerCreateInfo sampl = vkb::initializers::sampler_create_info();
        sampl.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampl.pNext = nullptr;
        sampl.maxLod = VK_LOD_CLAMP_NONE;
        sampl.minLod = 0;

        sampl.magFilter = extract_filter(sampler.magFilter.value_or(fastgltf::Filter::Nearest));
        sampl.minFilter = extract_filter(sampler.minFilter.value_or(fastgltf::Filter::Nearest));

        sampl.mipmapMode = extract_mipmap_mode(sampler.minFilter.value_or(fastgltf::Filter::Nearest));

        VkSampler newSampler;
        vkCreateSampler(engine->getDevice(), &sampl, nullptr, &newSampler);

        file.samplers.push_back(newSampler);
    }

    // temporal arrays for all the objects to use while creating the GLTF data
    std::vector<std::shared_ptr<MeshAsset<Vertex>>> meshes;
    std::vector<std::shared_ptr<Node>> nodes;
    std::vector<std::shared_ptr<AllocatedImage>> images;
    std::vector<std::shared_ptr<GLTFMaterial>> materials;

    // load all textures
    for (fastgltf::Image& image : gltf.images) {
        std::optional<std::shared_ptr<AllocatedImage>> img = load_image(engine, gltf, image);

        if (img.has_value()) {
            images.push_back(*img);
            file.images.push_back(*img);
        }
        else {
            // we failed to load, so lets give the slot a default white texture to not
            // completely break loading
            images.push_back(engine->getDefaultTexture2D());
            std::cout << "gltf failed to load texture " << image.name << std::endl;
        }
    }

    // create buffer to hold the material data
    file.materialDataBuffer->createUniformBuffer(1, engine->getDevicePtr(), engine->physicalDevice, gltf.materials.size());

    int data_index = 0;
    std::vector<GLTFMetallic_Roughness::MaterialConstants>& sceneMaterialConstants = file.materialDataBuffer->getData();

    for (fastgltf::Material& mat : gltf.materials) {
        std::shared_ptr<GLTFMaterial> newMat = std::make_shared<GLTFMaterial>();
        materials.push_back(newMat);
        file.materials[mat.name.c_str()] = newMat;

        GLTFMetallic_Roughness::MaterialConstants constants;
        constants.colorFactors.x = mat.pbrData.baseColorFactor[0];
        constants.colorFactors.y = mat.pbrData.baseColorFactor[1];
        constants.colorFactors.z = mat.pbrData.baseColorFactor[2];
        constants.colorFactors.w = mat.pbrData.baseColorFactor[3];

        constants.metal_rough_factors.x = mat.pbrData.metallicFactor;
        constants.metal_rough_factors.y = mat.pbrData.roughnessFactor;

        constants.textureFlags = glm::vec4(0.f);

        MaterialPass passType = MaterialPass::MainColor;
        if (mat.alphaMode == fastgltf::AlphaMode::Blend) {
            passType = MaterialPass::Transparent;
		}

		GLTFMetallic_Roughness::MaterialResources materialResources {};
		// default the material textures
		std::shared_ptr<AllocatedImage> defaultTexture = engine->getDefaultTexture2D();
		materialResources.colorImage = defaultTexture;
		materialResources.colorSampler = engine->getDefaultTextureSampler();
		materialResources.metalRoughImage = defaultTexture;
		materialResources.metalRoughSampler = engine->getDefaultTextureSampler();

        // set the uniform buffer for the material data
        materialResources.dataBuffer = file.materialDataBuffer->getUniformBuffer();
        materialResources.dataBufferOffset = data_index * sizeof(GLTFMetallic_Roughness::MaterialConstants);

        auto getSampler = [&](std::size_t textureIndex) {
            VkSampler sampler = {};
            auto samplerIndex = gltf.textures[textureIndex].samplerIndex;
            if (samplerIndex.has_value())
            {
                size_t samplerIndex = gltf.textures[textureIndex].samplerIndex.value();
                sampler = file.samplers[samplerIndex];
            }
            else
            {
                sampler = engine->getDefaultTextureSampler();
            }

            return sampler;
        };

		// grab textures from gltf file
		if (mat.pbrData.baseColorTexture.has_value())
		{
			size_t img = gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].imageIndex.value();
			materialResources.colorImage = images[img];
			materialResources.colorSampler = getSampler(mat.pbrData.baseColorTexture.value().textureIndex);
		}

		if (mat.pbrData.metallicRoughnessTexture.has_value())
		{
			size_t img = gltf.textures[mat.pbrData.metallicRoughnessTexture.value().textureIndex].imageIndex.value();
			materialResources.metalRoughImage = images[img];
			materialResources.metalRoughSampler = getSampler(mat.pbrData.metallicRoughnessTexture.value().textureIndex);
		}

		if (mat.normalTexture.has_value())
		{
			size_t img = gltf.textures[mat.normalTexture.value().textureIndex].imageIndex.value();
			materialResources.normalImage = images[img];
            constants.metal_rough_factors.b = mat.normalTexture.value().scale;
		}

		if (mat.occlusionTexture.has_value())
		{
			size_t img = gltf.textures[mat.occlusionTexture.value().textureIndex].imageIndex.value();
			materialResources.AOImage = images[img];
            constants.metal_rough_factors.a = mat.occlusionTexture.value().strength;
		}

        if (mat.emissiveTexture.has_value())
        {
            size_t img = gltf.textures[mat.emissiveTexture.value().textureIndex].imageIndex.value();
            materialResources.emissiveImage = images[img];
            constants.emissiveFactors.x = mat.emissiveFactor[0];
            constants.emissiveFactors.y = mat.emissiveFactor[1];
            constants.emissiveFactors.z = mat.emissiveFactor[2];
        }

        // write material parameters to buffer
        sceneMaterialConstants[data_index] = constants;

        // build material
        newMat->data = engine->metalRoughMaterial.write_material(engine, passType, materialResources, file.descriptorPool);

        data_index++;
    }

    file.materialDataBuffer->CopyData();

    // use the same vectors for all meshes so that the memory doesnt reallocate as
    // often
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;

    for (fastgltf::Mesh& mesh : gltf.meshes) {
        std::shared_ptr<MeshAsset<Vertex>> newmesh = std::make_shared<MeshAsset<Vertex>>();
        meshes.push_back(newmesh);
        file.meshes.push_back(newmesh);
        newmesh->name = mesh.name;

        // clear the mesh arrays each mesh, we dont want to merge them by error
        indices.clear();
        vertices.clear();

        for (auto&& p : mesh.primitives) {
            GeoSurface newSurface;
            newSurface.startIndex = (uint32_t)indices.size();
            newSurface.count = (uint32_t)gltf.accessors[p.indicesAccessor.value()].count;

            size_t initial_vtx = vertices.size();

            // load indexes
            {
                fastgltf::Accessor& indexaccessor = gltf.accessors[p.indicesAccessor.value()];
                indices.reserve(indices.size() + indexaccessor.count);

                fastgltf::iterateAccessor<std::uint32_t>(gltf, indexaccessor,
                    [&](std::uint32_t idx) {
                        indices.push_back(idx + initial_vtx);
                    });
            }

            // load vertex positions
            {
                fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->accessorIndex];
                vertices.resize(vertices.size() + posAccessor.count);

                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                    [&](glm::vec3 v, size_t index) {
                        Vertex newvtx;
                        newvtx.pos = v;
                        newvtx.normal = { 1, 0, 0 };
                        newvtx.color = glm::vec4{ 1.f };
                        newvtx.texCoord.x = 0;
                        newvtx.texCoord.y = 0;
                        vertices[initial_vtx + index] = newvtx;
                    });
            }

            // load vertex normals
            auto normals = p.findAttribute("NORMAL");
            if (normals != p.attributes.end()) {

                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).accessorIndex],
                    [&](glm::vec3 v, size_t index) {
                        vertices[initial_vtx + index].normal = v;
                    });
            }

            // load UVs
            auto uv = p.findAttribute("TEXCOORD_0");
            if (uv != p.attributes.end()) {

                fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).accessorIndex],
                    [&](glm::vec2 v, size_t index) {
                        vertices[initial_vtx + index].texCoord.x = v.x;
                        vertices[initial_vtx + index].texCoord.y = v.y;
                    });
            }

            // load vertex colors
            auto colors = p.findAttribute("COLOR_0");
            if (colors != p.attributes.end()) {

                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*colors).accessorIndex],
                    [&](glm::vec4 v, size_t index) {
                        vertices[initial_vtx + index].color = v;
                    });
            }
    
            // 탄젠트/바이탄젠트 계산
            for (size_t i = 0; i < newSurface.count; i += 3) {
                // 삼각형의 세 정점 인덱스
                uint32_t idx0 = indices[newSurface.startIndex + i] - initial_vtx;
                uint32_t idx1 = indices[newSurface.startIndex + i + 1] - initial_vtx;
                uint32_t idx2 = indices[newSurface.startIndex + i + 2] - initial_vtx;

                // 삼각형의 세 정점
                Vertex& v0 = vertices[initial_vtx + idx0];
                Vertex& v1 = vertices[initial_vtx + idx1];
                Vertex& v2 = vertices[initial_vtx + idx2];

                // 모서리 벡터
                glm::vec3 edge1 = v1.pos - v0.pos;
                glm::vec3 edge2 = v2.pos - v0.pos;

                // 텍스처 좌표 차이
                glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
                glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

                // 분모 값 계산
                float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

                // 탄젠트 계산
                glm::vec3 tangent;
                tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

                // 바이탄젠트 계산
                glm::vec3 bitangent;
                bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

                // 삼각형의 세 정점에 탄젠트와 바이탄젠트 할당
                /*v0.tangent += tangent;
                v0.bitangent += bitangent;

                v1.tangent += tangent;
                v1.bitangent += bitangent;

                v2.tangent += tangent;
                v2.bitangent += bitangent;*/
            }

            // 탄젠트와 바이탄젠트 정규화
            //for (size_t i = 0; i < gltf.accessors[p.findAttribute("POSITION")->accessorIndex].count; i++) {
            //    Vertex& v = vertices[initial_vtx + i];

            //    // 정규화
            //    v.tangent = glm::normalize(v.tangent);
            //    v.bitangent = glm::normalize(v.bitangent);

            //    // 문서의 마지막 부분에서 언급된 Gram-Schmidt 과정 적용
            //    // 탄젠트를 노말에 대해 직교화
            //    v.tangent = glm::normalize(v.tangent - glm::dot(v.tangent, v.normal) * v.normal);

            //    // 바이탄젠트 재계산
            //    v.bitangent = glm::cross(v.normal, v.tangent);
            //}


#if DEBUG_MODEL
            {
                std::cout << "Analyzing winding order for mesh primitive...\n";
                int ccwCount = 0;
                int cwCount = 0;

                for (size_t i = 0; i < newSurface.count; i += 3) {
                    // 삼각형의 세 정점 인덱스
                    uint32_t idx0 = indices[newSurface.startIndex + i] - initial_vtx;
                    uint32_t idx1 = indices[newSurface.startIndex + i + 1] - initial_vtx;
                    uint32_t idx2 = indices[newSurface.startIndex + i + 2] - initial_vtx;

                    // 삼각형의 세 정점
                    Vertex& v0 = vertices[initial_vtx + idx0];
                    Vertex& v1 = vertices[initial_vtx + idx1];
                    Vertex& v2 = vertices[initial_vtx + idx2];

                    // 삼각형이 CCW인지 확인
                    bool isCCW = isCounterClockwise(v0.pos, v1.pos, v2.pos, v0.normal);

                    if (isCCW) {
                        ccwCount++;
                    }
                    else {
                        cwCount++;
                    }

                    // 첫 10개 삼각형의 분석 결과 출력
                    if (i / 3 < 10) {
                        std::cout << "Triangle " << (i / 3) << ": "
                            << (isCCW ? "CCW" : "CW") << "\n";
                        std::cout << "  Positions: ("
                            << v0.pos.x << ", " << v0.pos.y << ", " << v0.pos.z << "), ("
                            << v1.pos.x << ", " << v1.pos.y << ", " << v1.pos.z << "), ("
                            << v2.pos.x << ", " << v2.pos.y << ", " << v2.pos.z << ")\n";
                        std::cout << "  Normal: ("
                            << v0.normal.x << ", " << v0.normal.y << ", " << v0.normal.z << ")\n";
                    }
                }

                std::cout << "Winding order analysis complete: "
                    << ccwCount << " CCW triangles, "
                    << cwCount << " CW triangles\n";
                std::cout << "Model is primarily "
                    << (ccwCount > cwCount ? "COUNTER-CLOCKWISE" : "CLOCKWISE")
                    << " (" << (float)max(ccwCount, cwCount) / (ccwCount + cwCount) * 100.0f
                    << "% consistent)\n";
            }
#endif
            if (p.materialIndex.has_value()) {
                newSurface.material = materials[p.materialIndex.value()];
            }
            else {
                newSurface.material = materials[0];
            }

            newmesh->surfaces.push_back(newSurface);
        }

        engine->createVertexBuffer(vertices, newmesh->meshBuffers.vertexBuffer.Buffer, newmesh->meshBuffers.vertexBuffer.BufferMemory);
        engine->createIndexBuffer(indices, newmesh->meshBuffers.indexBuffer.Buffer, newmesh->meshBuffers.indexBuffer.BufferMemory);
    }


    // load all nodes and their meshes
    for (fastgltf::Node& node : gltf.nodes) {
        std::shared_ptr<Node> newNode;

        // find if the node has a mesh, and if it does hook it to the mesh pointer and allocate it with the meshnode class
        if (node.meshIndex.has_value()) {
            newNode = std::make_shared<MeshNode>();
            static_cast<MeshNode*>(newNode.get())->mesh = meshes[*node.meshIndex];
        }
        else {
            newNode = std::make_shared<Node>();
        }

        nodes.push_back(newNode);
        file.nodes[node.name.c_str()];

        std::visit(fastgltf::visitor{ [&](const fastgltf::math::fmat4x4& matrix) {
                                          memcpy(&newNode->localTransform, matrix.data(), sizeof(matrix));
                                      },
                       [&](fastgltf::TRS transform) {
                           glm::vec3 tl(transform.translation[0], transform.translation[1],
                               transform.translation[2]);
                           glm::quat rot(transform.rotation[3], transform.rotation[0], transform.rotation[1],
                               transform.rotation[2]);
                           glm::vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);

                           glm::mat4 tm = glm::translate(glm::mat4(1.f), tl);
                           glm::mat4 rm = glm::toMat4(rot);
                           glm::mat4 sm = glm::scale(glm::mat4(1.f), sc);

                           newNode->localTransform = tm * rm * sm;
                       } },
            node.transform);
    }

    // run loop again to setup transform hierarchy
    for (int i = 0; i < gltf.nodes.size(); i++) {
        fastgltf::Node& node = gltf.nodes[i];
        std::shared_ptr<Node>& sceneNode = nodes[i];

        for (auto& c : node.children) {
            sceneNode->children.push_back(nodes[c]);
            nodes[c]->parent = sceneNode;
        }
    }

    // find the top nodes, with no parents
    for (auto& node : nodes) {
        if (node->parent.lock() == nullptr) {
            file.topNodes.push_back(node);
            node->refreshTransform(glm::mat4{ 1.f });
        }
    }

    std::cout << "Loading GLTF Complete : " << filePath << std::endl;

    return scene;
}

void LoadedGLTF::Draw(const glm::mat4& topMatrix, DrawContext& ctx)
{
    // create renderables from the scenenodes
    for (auto& n : topNodes) {
        n->Draw(topMatrix, ctx);
    }
}

void LoadedGLTF::clearAll()
{
    vkDestroyDescriptorPool(creator->getDevice(), descriptorPool, nullptr);

    //materialDataBuffer.destroy(0);

    for (std::shared_ptr<MeshAsset<Vertex>> mesh : meshes)
    {
        if (mesh.get())
        {
            vkDestroyBuffer(creator->getDevice(), mesh->meshBuffers.vertexBuffer.Buffer, nullptr);
            vkFreeMemory(creator->getDevice(), mesh->meshBuffers.vertexBuffer.BufferMemory, nullptr);

            vkDestroyBuffer(creator->getDevice(), mesh->meshBuffers.indexBuffer.Buffer, nullptr);
            vkFreeMemory(creator->getDevice(), mesh->meshBuffers.indexBuffer.BufferMemory, nullptr);
        }
    }

    for (VkSampler sampler : samplers)
    {
        vkDestroySampler(creator->getDevice(), sampler, nullptr);
    }
}