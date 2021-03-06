#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
namespace Themp
{
	using namespace DirectX;
	class Mesh;
	class D3D;

	class Object3D
	{
	public:
		struct Object3DConstantBufferData
		{
			XMFLOAT4X4 worldMatrix;
		};
		Object3D();
		~Object3D();
		void Update(float dt);

		//pass is a enum Themp::Mesh::DrawPass type;
		void Draw(D3D & d3d, int pass);
		void ForceBufferUpdate();
		void CreateCube(std::string shader,  bool geometryShader);
		void CreateTriangle(std::string shader, bool geometryShader);
		void CreateQuad(std::string shader, bool geometryShader);

		//Only need to call this when you're manually building a mesh, this function builds all vertex and index buffers for you
		void Construct();

		std::vector<Themp::Mesh*> m_Meshes;
		XMFLOAT3 m_Position, m_Rotation, m_Scale;
		
		//axis aligned
		XMFLOAT3 m_BoundsMin, m_BoundsMax;
		ID3D11Buffer* m_ConstantBuffer = nullptr;
		Object3DConstantBufferData m_ConstantBufferData;
		bool isDirty = true;
	};
}