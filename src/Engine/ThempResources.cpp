#include "ThempSystem.h"
#include "ThempResources.h"
#include "ThempD3D.h"
#include "ThempObject3D.h"
#include "ThempMesh.h"
#include "ThempMaterial.h"

#include <DDSTextureLoader.h>

#include <iostream>
#include <fstream>

namespace Themp
{
	std::string SanitizeSlashes(std::string input);
	
	Resources* Resources::TRes = nullptr;
	Resources::Resources()
	{
		Resources::TRes = this;
		m_VertexBuffers.push_back(0);
		m_IndexBuffers.push_back(0);
	}
	Resources::~Resources()
	{
		for (auto i : m_VertexShaders)
		{
			i.second->Release();
			i.second = nullptr;
		}
		m_VertexShaders.clear();

		for (auto i : m_PixelShaders)
		{
			i.second->Release();
			i.second = nullptr;
		}
		m_PixelShaders.clear();

		for (auto i : m_GeometryShaders)
		{
			i.second->Release();
			i.second = nullptr;
		}
		m_GeometryShaders.clear();
		
		for (auto i : m_Textures)
		{
			delete i.second;
			i.second = nullptr;
		}
		m_Textures.clear();

		for (auto i : m_Materials)
		{
			delete i.second;
			i.second = nullptr;
		}
		m_Materials.clear();

		for (auto i : m_VertexBuffers)
		{
			if (i == 0)continue;
			i->Release();
			i = nullptr;
		}
		m_VertexBuffers.clear();

		for (auto i : m_IndexBuffers)
		{
			if (i == 0)continue;
			i->Release();
			i = nullptr;
		}
		m_IndexBuffers.clear();

		for (auto i : m_Meshes)
		{
			delete i;
			i = nullptr;
		}
		m_Meshes.clear();

		for (auto i : m_3DObjects)
		{
			delete i;
			i = nullptr;
		}
		m_3DObjects.clear();
		Resources::TRes = nullptr;
	}
	ID3D10Blob* Resources::ReadToBlob(std::string path)
	{
		ID3D10Blob* nBlob;
		std::ifstream ifs(path, std::ios::binary | std::ios::ate);
		if (!ifs.good()) return nullptr;
		std::ifstream::pos_type pos = ifs.tellg();
		size_t length = static_cast<size_t>(pos);
		D3D10CreateBlob(length, &nBlob);
		ifs.seekg(0, std::ios::beg);
		ifs.read((char*)nBlob->GetBufferPointer(), length);
		ifs.close();
		return nBlob;
	}
	Texture* Resources::GetTexture(std::string path)
	{
		if (path == "")
		{
			return nullptr;
		}
		std::string basePath = BASE_TEXTURE_PATH;
		basePath = basePath.append(path);
		std::unordered_map<std::string, Texture*>::iterator s = m_Textures.find(basePath);
		if (s != m_Textures.end()) return s->second;

		Texture* tex = new Texture();
		memset(tex, 0, sizeof(Texture));
		std::wstring wPath = std::wstring(basePath.begin(), basePath.end());
		HRESULT res = DirectX::CreateDDSTextureFromFile(Themp::System::tSys->m_D3D->m_Device, wPath.c_str(), &tex->m_Resource, &tex->m_View);
		
		if(res != S_OK) 
		{
			System::Print("DirectTK couldn't load the texture: %s , Returning Default Texture instead!!  \n",basePath.c_str());
			delete tex; 
			return GetTexture("DefaultDiffuse.dds", true); 
		}

		m_Textures[basePath] = tex;
		return tex;
	}

	Texture* Resources::GetTexture(std::string path, bool isDefaulted = true)
	{
		std::string basePath = BASE_TEXTURE_PATH;
		basePath.append(path);
		std::unordered_map<std::string, Texture*>::iterator s = m_Textures.find(basePath);
		if (s != m_Textures.end()) return s->second;

		Texture* tex = new Texture();
		memset(tex, 0, sizeof(Texture));
		std::wstring wPath = std::wstring(basePath.begin(), basePath.end());
		HRESULT res = DirectX::CreateDDSTextureFromFile(Themp::System::tSys->m_D3D->m_Device, wPath.c_str(), &tex->m_Resource, &tex->m_View);

		if (res != S_OK) 
		{ 
			System::Print("DirectTK couldn't load the default texture: %s!!!\n", basePath.c_str());
			delete tex; 
			return nullptr; 
		}

		m_Textures[basePath] = tex;
		return tex;
	}
	Themp::Material* Resources::GetMaterial(std::string materialName, std::string texture, std::string shaderPath, bool geometryShader, D3D11_INPUT_ELEMENT_DESC* nonDefaultIED, int numElements, bool multisample)
	{
		std::unordered_map<std::string, Themp::Material*>::iterator s = m_Materials.find(materialName);
		if (s != m_Materials.end()) return s->second;
		if (shaderPath == "")
		{
			shaderPath = "default";
		}
		return LoadMaterial(materialName, texture, shaderPath, geometryShader, nonDefaultIED, numElements, multisample);
	}
	Themp::Material* Resources::GetMaterial(std::string materialName, std::vector<std::string>& textures, std::vector<uint8_t>& textureTypes, std::string shaderPath, bool geometryShader, bool multisample)
	{
		std::unordered_map<std::string, Themp::Material*>::iterator s = m_Materials.find(materialName);
		if (s != m_Materials.end()) return s->second;
		if (shaderPath == "")
		{
			shaderPath = "default";
		}
		return LoadMaterial(materialName, textures, textureTypes, shaderPath, geometryShader, multisample);
	}
	Themp::Object3D* Resources::GetModel(std::string path, bool uniqueMesh)
	{
		std::unordered_map<std::string, Themp::Object3D*>::iterator s = m_Models.find(path);
		if (s != m_Models.end())
		{
			//clone the model
			Object3D* source = s->second;
			Object3D* clone = new Object3D(*source);
			clone->m_Position = XMFLOAT3(0, 0, 0);
			clone->m_Scale = XMFLOAT3(1,1,1);
			clone->m_Rotation = XMFLOAT3(0, 0, 0);
			clone->m_ConstantBuffer = nullptr;

			if (uniqueMesh)
			{
				clone->m_Meshes.clear();
				for (size_t i = 0; i < source->m_Meshes.size(); i++)
				{
					Mesh* srcMesh = source->m_Meshes[i];
					Mesh* newMesh = new Mesh(*source->m_Meshes[i]);
					m_Meshes.push_back(newMesh);
					newMesh->m_Indices = new uint32_t[srcMesh->m_NumIndices];
					memcpy(newMesh->m_Indices, srcMesh->m_Indices, sizeof(uint32_t)*srcMesh->m_NumIndices);
					newMesh->m_Vertices = new Vertex[srcMesh->m_NumVertices];
					memcpy(newMesh->m_Vertices, srcMesh->m_Vertices, sizeof(Vertex)*srcMesh->m_NumVertices);
					newMesh->m_NumIndices = srcMesh->m_NumIndices;
					newMesh->m_NumVertices = srcMesh->m_NumVertices;
					newMesh->ConstructVertexBuffer();
					clone->m_Meshes.push_back(newMesh);
				}
			}
			else
			{
				clone->m_Meshes = source->m_Meshes;
			}
			m_3DObjects.push_back(clone);
			return clone;
		}
		Object3D* newObject = LoadModel(path);
		m_3DObjects.push_back(newObject);
		return newObject;
	}
	size_t Resources::CreateVertexBuffer(Vertex* vertices, size_t numVertices)
	{
		Themp::D3D* d = Themp::System::tSys->m_D3D;
		D3D11_BUFFER_DESC bd;
		D3D11_MAPPED_SUBRESOURCE ms;
		ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&ms, sizeof(D3D11_MAPPED_SUBRESOURCE));

		//set up for vertices
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(Vertex) * numVertices;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		ID3D11Buffer* vertexBuffer;
		HRESULT res = d->m_Device->CreateBuffer(&bd, NULL, &vertexBuffer);
		if (res == S_OK)
		{
			d->m_DevCon->Map(vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
			memcpy(ms.pData, vertices, sizeof(Vertex)*numVertices);
			d->m_DevCon->Unmap(vertexBuffer, NULL);
			m_VertexBuffers.push_back(vertexBuffer);
			return m_VertexBuffers.size() - 1;
		}
		return 0;
	}
	size_t Resources::CreateIndexBuffer(uint32_t* indices, size_t numIndices)
	{
		Themp::D3D* d = Themp::System::tSys->m_D3D;
		HRESULT res;

		D3D11_BUFFER_DESC bd;
		D3D11_MAPPED_SUBRESOURCE ms;
		ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&ms, sizeof(D3D11_MAPPED_SUBRESOURCE));
		//set up for indices
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(uint32_t) * numIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ID3D11Buffer* indexBuffer;
		res = d->m_Device->CreateBuffer(&bd, NULL, &indexBuffer);
		if (res == S_OK)
		{
			d->m_DevCon->Map(indexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
			memcpy(ms.pData, indices, sizeof(uint32_t)*numIndices);
			d->m_DevCon->Unmap(indexBuffer, NULL);
			m_IndexBuffers.push_back(indexBuffer);
			return m_IndexBuffers.size() - 1;
		}
		return 0;
	}
	ID3D11VertexShader * Resources::GetVertexShader(std::string name)
	{
		std::string basePath = BASE_SHADER_PATH;
		basePath.append(name);

		std::unordered_map<std::string, ID3D11VertexShader*>::iterator s = m_VertexShaders.find(basePath);
		if (s != m_VertexShaders.end()) return s->second;

		HRESULT result = 0;
		ID3D11VertexShader* vShader = nullptr;
		ID3D10Blob* VSRaw = ReadToBlob(basePath);
		result = System::tSys->m_D3D->m_Device->CreateVertexShader(VSRaw->GetBufferPointer(), VSRaw->GetBufferSize(), nullptr, &vShader);
		if (result != S_OK) { System::Print("Could not create Vertex shader from: %s", name.c_str()); return nullptr; }
		m_VertexShaders[basePath] = vShader;

		return vShader;
	}

	ID3D11PixelShader * Resources::GetPixelShader(std::string name)
	{
		std::string basePath = BASE_SHADER_PATH;
		basePath.append(name);
		std::unordered_map<std::string, ID3D11PixelShader*>::iterator s = m_PixelShaders.find(basePath);
		if (s != m_PixelShaders.end()) return s->second;
		
		HRESULT result = 0;
		ID3D11PixelShader* pShader = nullptr;
		ID3D10Blob* PSRaw = ReadToBlob(basePath);
		result = System::tSys->m_D3D->m_Device->CreatePixelShader(PSRaw->GetBufferPointer(), PSRaw->GetBufferSize(), nullptr, &pShader);
		if (result != S_OK) { System::Print("Could not create Pixel shader from: %s", name.c_str() ); return nullptr; }
		m_PixelShaders[basePath] = pShader;

		return pShader;
	}
	ID3D11GeometryShader * Resources::GetGeometryShader(std::string name)
	{
		std::string basePath = BASE_SHADER_PATH;
		basePath.append(name);
		std::unordered_map<std::string, ID3D11GeometryShader*>::iterator s = m_GeometryShaders.find(basePath);
		if (s != m_GeometryShaders.end()) return s->second;
		
		HRESULT result = 0;
		ID3D11GeometryShader* gShader = nullptr;
		ID3D10Blob* GSRaw = ReadToBlob(basePath);
		result = System::tSys->m_D3D->m_Device->CreateGeometryShader(GSRaw->GetBufferPointer(), GSRaw->GetBufferSize(), nullptr, &gShader);
		if (result != S_OK) { System::Print("Could not create Geometry shader from: %", name.c_str()); return nullptr; }
		m_GeometryShaders[basePath] = gShader;

		return gShader;
	}
	Themp::Material* Resources::LoadMaterial(std::string materialName, std::string texture, std::string shaderPath, bool geometryShader, D3D11_INPUT_ELEMENT_DESC* nonDefaultIED, int numElements, bool multisample)
	{
		std::string tempPath = shaderPath;
		if (shaderPath == "default" && materialName != "")
		{
			Material::GetGBufferShaderName(materialName, shaderPath, geometryShader);
		}
		std::unordered_map<std::string, Material*>::iterator s = m_Materials.find(materialName);
		if (s != m_Materials.end()) return s->second;

		Material* material = new Themp::Material();
		//VERTEX SHADER
		{
			HRESULT res;
#ifdef _DEBUG
			tempPath.append("_VS_d.cso");
#else
			tempPath.append("_VS.cso");
#endif
			//material = new Themp::Material();
			material->m_VertexShader = Resources::TRes->GetVertexShader(tempPath);
			if (!material->m_VertexShader)
			{
				System::Print("Couldn't find Vertex shader: %s" ,tempPath.c_str());
				delete material;
				return nullptr;
			}
			std::string blobPath = BASE_SHADER_PATH;
			blobPath.append(tempPath);
			ID3D10Blob* vsShaderBlob = material->ReadToBlob(blobPath);
			if (nonDefaultIED != nullptr)
			{
				res = Themp::System::tSys->m_D3D->m_Device->CreateInputLayout(nonDefaultIED, numElements, vsShaderBlob->GetBufferPointer(), vsShaderBlob->GetBufferSize(), &material->m_InputLayout);
			}
			else
			{
				res = Themp::System::tSys->m_D3D->m_Device->CreateInputLayout(Themp::D3D::DefaultInputLayoutDesc, Themp::D3D::DefaultInputLayoutNumElements, vsShaderBlob->GetBufferPointer(), vsShaderBlob->GetBufferSize(), &material->m_InputLayout);
			}
			if (res == S_OK)
				vsShaderBlob->Release();
			if (res != S_OK) 
			{
				System::Print("Could not create shader input layout"); 
				delete material;
				return nullptr; 
			}
		}
		//PIXEL SHADER
		{
			tempPath = shaderPath;
#ifdef _DEBUG
			tempPath.append("_PS");
			if (multisample)
			{
				tempPath.append("MS");
			}
			tempPath.append("_d");
			tempPath.append(".cso");
#else
			tempPath.append("_PS");
			if (multisample)
			{
				tempPath.append("MS");
			}
			tempPath.append(".cso");
#endif
			material->m_PixelShader = Resources::TRes->GetPixelShader(tempPath);
			if (!material->m_PixelShader)
			{
				System::Print("Couldn't find Pixel shader: %s", tempPath.c_str());
				delete material;
				return nullptr;
			}
		}
		if (geometryShader)
		{
			tempPath = shaderPath;
#ifdef _DEBUG
			tempPath.append("_GS_d.cso");
#else
			tempPath.append("_GS.cso");
#endif
			material->m_GeometryShader = Resources::TRes->GetGeometryShader(tempPath);
			if (!material->m_GeometryShader)System::Print("Couldn't find Geometry shader: %s", tempPath.c_str());
		}
		std::vector<std::string> textures = { texture, "" };
		std::vector<std::uint8_t> textureTypes = { Material::DIFFUSE, Material::PBR};
		material->GetMaterialProperties(materialName,&textures[1]);
		if (textures[1] == "")textureTypes[1] = Material::UNUSED;
		material->ReadTextures(textures, textureTypes);
		m_Materials[materialName] = material;
		return material;
	}

	Themp::Material* Resources::LoadMaterial(std::string materialName, std::vector<std::string>& textures, std::vector<uint8_t>& textureTypes, std::string shaderPath, bool geometryShader, bool multisample)
	{
		if (shaderPath == "default")
		{
			Material::GetGBufferShaderName(materialName,shaderPath, geometryShader);
		}
		std::string tempPath = shaderPath;

		std::unordered_map<std::string, Material*>::iterator s = m_Materials.find(materialName);
		if (s != m_Materials.end()) return s->second;

		Themp::Material* material = new Themp::Material();
		//VERTEX SHADER
		{
			HRESULT res;
#ifdef _DEBUG
			tempPath.append("_VS_d.cso");
#else
			tempPath.append("_VS.cso");
#endif
			
			material->m_VertexShader = Resources::TRes->GetVertexShader(tempPath);
			if (!material->m_VertexShader)
			{
				System::Print("Couldn't find Vertex shader: %s" ,tempPath.c_str());
				delete material;
				return nullptr;
			}

			std::string blobPath = BASE_SHADER_PATH;
			blobPath.append(tempPath);
			ID3D10Blob* vsShaderBlob = material->ReadToBlob(blobPath);
			res = Themp::System::tSys->m_D3D->m_Device->CreateInputLayout(Themp::D3D::DefaultInputLayoutDesc, Themp::D3D::DefaultInputLayoutNumElements, vsShaderBlob->GetBufferPointer(), vsShaderBlob->GetBufferSize(), &material->m_InputLayout);
			if(res == S_OK)
				vsShaderBlob->Release();
			if (res != S_OK) {System::Print( "Could not create shader input layout"); delete material;  return nullptr; }
		}
		//PIXEL SHADER
		{
			tempPath = shaderPath;
#ifdef _DEBUG
			tempPath.append("_PS");
			if (multisample)
			{
				tempPath.append("MS");
			}
			tempPath.append("_d");
			tempPath.append(".cso");
#else
			tempPath.append("_PS");
			if (multisample)
			{
				tempPath.append("MS");
			}
			tempPath.append(".cso");
#endif
			material->m_PixelShader = Resources::TRes->GetPixelShader(tempPath);
			if (!material->m_PixelShader)System::Print( "Couldn't find Pixel shader: %s" ,tempPath.c_str());
		}

		if (geometryShader)
		{
			tempPath = shaderPath;
#ifdef _DEBUG
			tempPath.append("_GS_d.cso");
#else
			tempPath.append("_GS.cso");
#endif
			material->m_GeometryShader = Resources::TRes->GetGeometryShader(tempPath);
			if (!material->m_GeometryShader)System::Print( "Couldn't find Geometry shader: %s" , tempPath.c_str());
		}

		std::string PBRTex = "";
		material->GetMaterialProperties(materialName,&PBRTex);
		if (PBRTex != "")
		{
			bool replacedSpec = false;
			for (size_t i = 0; i < textureTypes.size(); i++)
			{
				//detect "specular textures", we don't use them so replace them with the PBR texture
				if (textureTypes[i] == Material::PBR || textureTypes[i] == 7)
				{
					replacedSpec = true;
					textureTypes[i] = Material::PBR;
					while(textures.size()-1 < i)
					{
						textures.push_back("");
					}
					textures[i] = PBRTex;
					break;
				}
			}
			if (!replacedSpec)
			{
				if (textures.size() < MAX_TEXTURES)
				{
					textures.push_back(PBRTex);
					textureTypes.push_back(Material::PBR);
				}
				else
				{
					bool emptySlot = false;
					for (size_t i = 0; i < textureTypes.size(); i++)
					{
						if (textureTypes[i] == Material::UNUSED)
						{
							emptySlot = true;
							textures[i] = PBRTex;
							textureTypes[i] = Material::PBR;
							break;
						}
					}
					if (!emptySlot)
					{
						//Well shit son, this shouldn't ever happen..
						assert(false);
						System::Print("Tried to load PBR Texture but textures.size was >= MAX_TEXTURES");
					}
				}
			}
		}
		material->ReadTextures(textures,textureTypes);
		m_Materials[materialName] = material;
		return material;
	}

	Themp::Object3D* Resources::LoadModel(std::string name)
	{
		struct MeshHeader
		{
			uint32_t numVertices;
			uint32_t numIndices;
			uint32_t materialID;
		};
		struct ModelHeader
		{
			char modelVersionHigh;
			char modelVersionLow;
			uint32_t numMeshes;
			uint32_t numMaterials;
			uint32_t numTextures;
			uint32_t numChars;
		};
		std::string modelPath = BASE_MODEL_PATH + name;
		FILE* modelFile = fopen(modelPath.c_str(), "rb");

		if (modelFile == nullptr)
		{
			System::Print("Could not find model: %s", modelPath.c_str());
			return nullptr;
		}
		Object3D* newObject = new Object3D();
		m_Models[name] = newObject;

		std::vector<int> materialOrder;

		ModelHeader header;
		fread(&header, sizeof(ModelHeader), 1, modelFile);
		if (header.modelVersionHigh != MODEL_VERSION_HIGH || header.modelVersionLow != MODEL_VERSION_LOW)
		{
			System::Print("Model version mismatch, cannot load this model!: %s", name.c_str());
			return nullptr;
		}
		for (size_t i = 0; i < header.numMeshes; i++)
		{
			MeshHeader meshHeader;
			//read in mesh header info, contains number of vertices,indices and the belonging material ID
			fread_s(&meshHeader, sizeof(meshHeader), sizeof(MeshHeader), 1, modelFile);
			 
			//allocate room for the vertices and indices, read them in afterwards, these pointers are also used for the object themselves so we don't need to delete them.
			Themp::Vertex* vertices = new Vertex[meshHeader.numVertices];
			std::uint32_t* indices = new uint32_t[meshHeader.numIndices];

			fread_s(vertices, sizeof(Vertex)*meshHeader.numVertices, sizeof(Vertex), meshHeader.numVertices, modelFile);
			fread_s(indices, sizeof(uint32_t)*meshHeader.numIndices, sizeof(uint32_t), meshHeader.numIndices, modelFile);

			//create the mesh object, assign all data.
			Mesh* mesh = new Mesh();
			mesh->m_Vertices = vertices;
			mesh->m_Indices = indices;
			mesh->m_NumVertices = meshHeader.numVertices;
			mesh->m_NumIndices = meshHeader.numIndices;
			//the mesh itself doesn't store a material ID but a pointer to material itself, we assign it later in the program so for now keep track of it..
			materialOrder.push_back(meshHeader.materialID);
			m_Meshes.push_back(mesh);
			
			newObject->m_Meshes.push_back(mesh);
		}

		std::vector<Themp::Material*> loadedMaterials;
		for (size_t i = 0; i < header.numMaterials; i++)
		{
			std::vector<std::string> textureNames;
			std::vector<uint8_t> textureTypes;
			uint32_t numTextures = 0;

			//read in number of textures this material contains.
			fread_s(&numTextures,sizeof(numTextures), sizeof(uint32_t), 1, modelFile);

			//read in the number of characters the material name has.
			uint32_t materialNameSize = 0;
			fread_s(&materialNameSize, sizeof(materialNameSize), sizeof(uint32_t), 1, modelFile);

			//allocate room for the string and read in all characters of the material name
			std::string materialName(materialNameSize, 0);
			fread_s(&materialName[0],materialName.capacity(), sizeof(char)*materialNameSize, 1, modelFile);

			std::string fileNameWithoutExtension = name.substr(0, name.size() - 4);

			for (size_t j = 0; j < numTextures; j++)
			{
				uint8_t textureType = 0;
				uint32_t textureNameSize = 0;
				
				//read in the texture type and size of the texture name (path)
				fread_s(&textureType,sizeof(textureType), sizeof(uint8_t), 1, modelFile);
				fread_s(&textureNameSize, sizeof(textureNameSize), sizeof(uint32_t), 1, modelFile);

				//read in the texture name (path)
				std::string textureName(textureNameSize, 0);
				fread_s(&textureName[0],textureName.capacity(), sizeof(char)*textureNameSize, 1, modelFile);

				textureTypes.push_back(textureType);
				textureNames.push_back(SanitizeSlashes(fileNameWithoutExtension +"/"+ textureName));
			}
			if (numTextures != 0)
			{
				Themp::Material* newMaterial = GetMaterial(SanitizeSlashes(fileNameWithoutExtension + "/"+ materialName),textureNames,textureTypes, "default", false);
				textureTypes.clear();
				loadedMaterials.push_back(newMaterial);
			}
			else
			{
				Themp::Material* newMaterial = D3D::DefaultMaterial;
				loadedMaterials.push_back(newMaterial);
			}
		}
		for (size_t j = 0; j < newObject->m_Meshes.size(); j++)
		{
			if (loadedMaterials.size() == 0)
			{
				newObject->m_Meshes[j]->m_Material = D3D::DefaultMaterial;
				continue;
			}
			newObject->m_Meshes[j]->m_Material = loadedMaterials[materialOrder[j]];
			//newObject->m_Meshes[j]->m_Material = loadedMaterials[0];
		}
		newObject->Construct();
		return newObject;
	}
	std::string SanitizeSlashes(std::string input)
	{
		std::string output = "";
		output.reserve(input.size());

		int64_t i = (int64_t)(input.size()-1);
		while (i >= 0)
		{
			//remove double (or more) slashes "//" or "\\" in filepaths
			if (i - 1 >= 0 && (input[i] == '/' || input[i] == '\\' || input[i] == 255) && (input[i - 1] == '/' || input[i - 1] == '\\'))
			{
				//mark for ignore
				input[i] = (unsigned char)255;
			}
			i--;
		}
		for (i = 0; i < input.size(); i++)
		{
			if (input[i] != (unsigned char)255)
			{
				output += input[i];
			}
		}

		return output;
	}
}