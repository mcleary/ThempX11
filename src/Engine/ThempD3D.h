#pragma once

#include <d3d11.h>
#include <d3d10.h>
#include <DirectXMath.h>


#define NUM_RENDER_TEXTURES 5

//the +6 is for the depth texture, shadow map and one skybox for reflections and one IBL + another skybox set
#define NUM_SHADER_RESOURCE_VIEWS NUM_RENDER_TEXTURES+6

namespace Themp
{ 
	using namespace DirectX;

	struct CONSTANT_BUFFER
	{
		float screenWidth;
		float screenHeight;
		float visualType;
		float SkyboxLerp;

		float shadowType;
		float globalRoughness = 0.05f;
		float globalMetallic;
		float MSAAValue = 1.0;

		float time;
		float F0x;
		float F0y;
		float F0z;

		float num_cascades = 4.0;
		float shadow_atlas_size;
		float d6;
		float d7;
	};
	
	class RenderTexture;
	class Camera;
	class Game;
	class Material;
	class Object3D;

	class D3D
	{
	public:
		enum LightType { Directional,Point, Spot, };
		D3D() {};
		~D3D();
		bool Init();
		void ResizeWindow(int newX, int newY);
		void SetShadowType(int type);
		bool SetMultiSample( int num);
		void SetLightDirty(LightType type, int index);
		void SetDirectionalLight(int index,bool enabled, XMFLOAT4 pos, XMFLOAT4 dir, XMFLOAT4 color);
		void SetNumberCascades(int numCascades);
		void PrepareSystemBuffer();
		void Draw(Game& game);
		void DrawImGUI();
		void DrawGBufferPass(Game& game);
		void DrawShadowMaps(Game& game);
		void DrawLightPass();
		void DrawPostProcess();

		void SetViewPort(float xPos, float yPos, float width, float height);
		void SetViewPorts(int* nViewports, float* xPos, float* yPos, float* width, float* height);
		void SetObject3DConstantBuffer(ID3D11Buffer* buf);
		void SetCameraConstantBuffer(ID3D11Buffer* buf);
		void SetSystemConstantBuffer(ID3D11Buffer* buf);
		void SetMaterialConstantBuffer(ID3D11Buffer* buf);
		void SetLightConstantBuffer(ID3D11Buffer* buf);
		void VSUploadConstantBuffersToGPU();
		void VSUploadConstantBuffersToGPUNull();
		void PSUploadConstantBuffersToGPUNull();
		void GSUploadConstantBuffersToGPUNull();
		void PSUploadConstantBuffersToGPU();
		void GSUploadConstantBuffersToGPU();

		bool CreateBackBuffer();
		bool CreateRenderTextures(int width, int height);
		bool CreateDepthStencil(int width, int height, int multisample);

		ID3D10Blob * ReadToBlob(std::string path);

		CONSTANT_BUFFER m_ConstantBufferData;
		RenderTexture* m_RenderTextures[NUM_RENDER_TEXTURES];
		RenderTexture* m_MainRender = nullptr;
		ID3D11RenderTargetView* m_Rtvs[NUM_RENDER_TEXTURES];

		//enough potential SRV's I won't ever go out of bounds.
		ID3D11ShaderResourceView* m_ShaderResourceViews[32]; 

		ID3D11RenderTargetView* m_BackBuffer = nullptr;
		ID3D11BlendState* m_OMBlendState = nullptr;
		ID3D11Texture2D* m_DepthStencil = nullptr;
		ID3D11DepthStencilState* m_DepthStencilState = nullptr;
		ID3D11DepthStencilState* m_SkyboxDepthStencilState = nullptr;
		ID3D11DepthStencilState* m_ShadowClearDepthStencilState = nullptr;
		ID3D11DepthStencilView* m_DepthStencilView = nullptr;
		ID3D11ShaderResourceView* m_DepthStencilSRV = nullptr;

		ID3D11RasterizerState* m_RasterizerState = nullptr;
		ID3D11RasterizerState* m_WireframeRasterizerState = nullptr;
		ID3D11RasterizerState* m_ShadowRasterizerState = nullptr;
		ID3D11Buffer* m_CBuffer = nullptr;      //System Constant Buffer
		ID3D11Device* m_Device = nullptr;
#ifdef _DEBUG
		ID3D11Debug* m_DebugInterface = nullptr;
		ID3D11InfoQueue* m_D3dInfoQueue = nullptr;
#endif
		IDXGISwapChain* m_Swapchain = nullptr;             // the pointer to the swap chain interface
		ID3D11DeviceContext* m_DevCon = nullptr;           // the pointer to our Direct3D device context
		ID3D11InputLayout* m_InputLayout = nullptr;
		bool SupportsVPArrayIndex = true;
	
		bool dirtySystemBuffer = true;
		bool m_Wireframe = false;
		int m_ShadowType = 0;

		Object3D* m_FullScreenQuad = nullptr;
		Object3D* m_Skybox = nullptr;

		static D3D* s_D3D;
		static D3D11_INPUT_ELEMENT_DESC DefaultInputLayoutDesc[];
		static uint32_t DefaultInputLayoutNumElements;
		static Material* DefaultMaterial;
		static Material* DefaultMaterialSkybox;
		static Material* DefaultPostProcess;
		static ID3D11SamplerState* DefaultTextureSampler;
		static ID3D11Buffer* ConstantBuffers[5];
	};
};