#pragma once

#include "ChiliWin.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <DirectXMath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

#include "DxgiInfoManager.h"
#include "GraphicsException.h"

namespace WRL = Microsoft::WRL;

struct ConstantBuffer
{
	DirectX::XMMATRIX transform;
};

class Graphics
{
	friend class Renderer;
public:
	Graphics(HWND hWnd, UINT, UINT);
	~Graphics() = default;
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	void EndFrame();
	void ClearBuffer(float red, float green, float blue, float alpha = 1.0f);
	void DrawTestTriangle(float angle, float x, float z, float y = 1.0f);
	void CheckException() const;
private:
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;
	WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
	DxgiInfoManager dxgiIM;
	bool allSetup = false;
	size_t indices_size = 0;
private:
	UINT width;
	UINT height;
};

