#include "ThempSystem.h"
#include "ThempObject3D.h"
#include "ThempMesh.h"
#include "ThempMaterial.h"
#include "ThempD3D.h"
#include "ThempResources.h"
#include <DirectXMath.h>
#include <algorithm>

#include <iostream>
using namespace DirectX;
namespace Themp
{
	Object3D::Object3D()
	{
		m_Position = XMFLOAT3(0, 0, 0);
		m_Scale = XMFLOAT3(1,1,1);
		m_Rotation = XMFLOAT3(0, 0, 0);
	}
	Object3D::~Object3D()
	{
		if (m_ConstantBuffer)
		{
			m_ConstantBuffer->Release();
			m_ConstantBuffer = nullptr;
		}
	}
	void Object3D::Update(float dt)
	{
		m_Rotation.y += dt*0.2;
		isDirty = true;
	}
	void Object3D::Draw(D3D& d3d, int pass)
	{
		if (isDirty)
		{
			XMVECTOR trans =XMLoadFloat3(&m_Position), rot=XMLoadFloat3(&m_Rotation), scale=XMLoadFloat3(&m_Scale);

			XMMATRIX RotScale =  XMMatrixRotationRollPitchYawFromVector(rot) *  XMMatrixScalingFromVector(scale);
			XMMATRIX WorldMatrix =  RotScale * XMMatrixTranslationFromVector(trans);

			XMStoreFloat4x4(&m_ConstantBufferData.worldMatrix, (WorldMatrix));

			if (!m_ConstantBuffer)
			{
				// Fill in a buffer description.
				D3D11_BUFFER_DESC cbDesc;
				cbDesc.ByteWidth = sizeof(Object3DConstantBufferData);
				cbDesc.Usage = D3D11_USAGE_DYNAMIC;
				cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				cbDesc.MiscFlags = 0;
				cbDesc.StructureByteStride = 0;

				// Fill in the subresource data.
				D3D11_SUBRESOURCE_DATA InitData;
				InitData.pSysMem = &m_ConstantBufferData;
				InitData.SysMemPitch = 0;
				InitData.SysMemSlicePitch = 0;
				// Create the buffer.
				Themp::System::tSys->m_D3D->m_Device->CreateBuffer(&cbDesc, &InitData, &m_ConstantBuffer);
			}
			D3D11_MAPPED_SUBRESOURCE ms;
			d3d.m_DevCon->Map(m_ConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
			memcpy(ms.pData, &m_ConstantBufferData, sizeof(Object3DConstantBufferData));
			d3d.m_DevCon->Unmap(m_ConstantBuffer, NULL);

			isDirty = false;
		}
		d3d.SetObject3DConstantBuffer(m_ConstantBuffer);

		d3d.VSUploadConstantBuffersToGPU();

		for (int i = 0; i < m_Meshes.size(); ++i)
		{
			m_Meshes[i]->Draw(d3d, (Mesh::DrawPass)pass);
		}
	}
	void Object3D::ForceBufferUpdate()
	{
		XMVECTOR trans = XMLoadFloat3(&m_Position), rot = XMLoadFloat3(&m_Rotation), scale = XMLoadFloat3(&m_Scale);

		XMMATRIX RotScale = XMMatrixRotationRollPitchYawFromVector(rot) *  XMMatrixScalingFromVector(scale);
		XMMATRIX WorldMatrix = RotScale * XMMatrixTranslationFromVector(trans);

		XMStoreFloat4x4(&m_ConstantBufferData.worldMatrix, (WorldMatrix));

		if (!m_ConstantBuffer)
		{
			// Fill in a buffer description.
			D3D11_BUFFER_DESC cbDesc;
			cbDesc.ByteWidth = sizeof(Object3DConstantBufferData);
			cbDesc.Usage = D3D11_USAGE_DYNAMIC;
			cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbDesc.MiscFlags = 0;
			cbDesc.StructureByteStride = 0;

			// Fill in the subresource data.
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = &m_ConstantBufferData;
			InitData.SysMemPitch = 0;
			InitData.SysMemSlicePitch = 0;
			// Create the buffer.
			Themp::System::tSys->m_D3D->m_Device->CreateBuffer(&cbDesc, &InitData, &m_ConstantBuffer);
		}
		D3D11_MAPPED_SUBRESOURCE ms;
		D3D::s_D3D->m_DevCon->Map(m_ConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &m_ConstantBufferData, sizeof(Object3DConstantBufferData));
		D3D::s_D3D->m_DevCon->Unmap(m_ConstantBuffer, NULL);
	}
	void Object3D::CreateCube(std::string shader, bool geometryShader)
	{
		Mesh* mesh = new Mesh();
		Themp::System::tSys->m_Resources->m_Meshes.push_back(mesh);
		mesh->m_Vertices = new Vertex[8];
		mesh->m_Vertices[0] = { -0.5f, +0.5f, -0.5f  , 1.0f, 0.0f, 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 1.0f,0.0f };
		mesh->m_Vertices[1] = { +0.5f, +0.5f, -0.5f  , 0.0f, 1.0f, 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 0.0f,1.0f };
		mesh->m_Vertices[2] = { +0.5f, +0.5f,  0.5f  , 0.0f, 0.0f, 1.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 0.0f,0.0f };
		mesh->m_Vertices[3] = { -0.5f, +0.5f,  0.5f  , 1.0f, 1.0f, 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 1.0f,1.0f };
		mesh->m_Vertices[4] = { -0.5f, -0.5f,  0.5f  , 0.0f, 1.0f, 1.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 1.0f,0.0f };
		mesh->m_Vertices[5] = { +0.5f, -0.5f,  0.5f  , 1.0f, 1.0f, 1.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 0.0f,1.0f };
		mesh->m_Vertices[6] = { +0.5f, -0.5f, -0.5f  , 1.0f, 0.0f, 1.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 0.0f,0.0f };
		mesh->m_Vertices[7] = { -0.5f, -0.5f, -0.5f  , 1.0f, 0.0f, 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 1.0f,1.0f };

		mesh->m_Indices = new uint32_t[36]{
			0, 1, 2,
			0, 2, 3,
			4, 5, 6,
			4, 6, 7,
			3, 2, 5,
			3, 5, 4,
			2, 1, 6,
			2, 6, 5,
			1, 7, 6,
			1, 0, 7,
			0, 3, 4,
			0, 4, 7
		};


		mesh->m_NumIndices = 36;
		mesh->m_NumVertices = 8;
		mesh->ConstructVertexBuffer();
		mesh->m_Material = Themp::System::tSys->m_Resources->GetMaterial("","DefaultDiffuse.dds", shader, geometryShader);
		m_Meshes.push_back(mesh);
	}
	void Object3D::CreateTriangle(std::string shader, bool geometryShader)
	{
		Mesh* mesh = new Mesh();
		Themp::System::tSys->m_Resources->m_Meshes.push_back(mesh);
		mesh->m_Vertices = new Vertex[3];
		mesh->m_Vertices[0] = { -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 0.0f,0.0f };
		mesh->m_Vertices[1] = { -0.5f, +0.5f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 0.0f,1.0f };
		mesh->m_Vertices[2] = { +0.5f, +0.5f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 1.0f,1.0f };

		mesh->m_Indices = new uint32_t[3]{
			// front face
			0, 1, 2
		};


		mesh->m_NumIndices = 3;
		mesh->m_NumVertices = 3;
		mesh->ConstructVertexBuffer();
		mesh->m_Material = Themp::System::tSys->m_Resources->GetMaterial("", "DefaultDiffuse.dds", shader,  geometryShader);
		m_Meshes.push_back(mesh);
	}
	void Object3D::CreateQuad(std::string shader, bool geometryShader)
	{
		Mesh* mesh = new Mesh();
		Themp::System::tSys->m_Resources->m_Meshes.push_back(mesh);
		mesh->m_Vertices = new Vertex[4];
		mesh->m_Vertices[0] = { -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 0.0f,0.0f };
		mesh->m_Vertices[1] = { -1.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 0.0f,1.0f };
		mesh->m_Vertices[2] = { +1.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 1.0f,1.0f };
		mesh->m_Vertices[3] = { +1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 1.0f,0.0f };

		mesh->m_Indices = new uint32_t[6]{
			// front face
			0, 1, 2,
			0, 2, 3
		};


		mesh->m_NumIndices = 6;
		mesh->m_NumVertices = 4;
		mesh->ConstructVertexBuffer();
		mesh->m_Material = Themp::System::tSys->m_Resources->GetMaterial("", "DefaultDiffuse.dds", shader, geometryShader);
		m_Meshes.push_back(mesh);
	}
	void Object3D::Construct()
	{
		//sort meshes from big to small
		//(Helps to Early-Z when drawing)
		std::sort(m_Meshes.begin(), m_Meshes.end(),
			[](const void* a, const void* b)
		{
			return (static_cast<const Themp::Mesh*>(a)->m_NumVertices > static_cast<const Themp::Mesh*>(b)->m_NumVertices);
		}
		);

		//construct all vertex/index buffers (sets up the mesh ready for drawing)
		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Meshes[i]->ConstructVertexBuffer();
		}
		
		//calculate the bounds of this object
		//every mesh gets calculated on ConstructVertexBuffer();

		m_BoundsMax.x = -FLT_MAX;
		m_BoundsMax.y = -FLT_MAX;
		m_BoundsMax.z = -FLT_MAX;
		m_BoundsMin.x = FLT_MAX;
		m_BoundsMin.y = FLT_MAX;
		m_BoundsMin.z = FLT_MAX;
		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			Mesh* m = m_Meshes[i];
			XMFLOAT3& max = m->m_BoundsMax;
			XMFLOAT3& min = m->m_BoundsMin;
			
			m_BoundsMax.x = max.x > m_BoundsMax.x ? max.x : m_BoundsMax.x;
			m_BoundsMax.y = max.y > m_BoundsMax.y ? max.y : m_BoundsMax.y;
			m_BoundsMax.z = max.z > m_BoundsMax.z ? max.z : m_BoundsMax.z;
			m_BoundsMin.x = min.x < m_BoundsMin.x ? min.x : m_BoundsMin.x;
			m_BoundsMin.y = min.y < m_BoundsMin.y ? min.y : m_BoundsMin.y;
			m_BoundsMin.z = min.z < m_BoundsMin.z ? min.z : m_BoundsMin.z;
		}
	}
}