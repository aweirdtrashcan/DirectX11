#include "Graphics.hpp"
#include "dxerr.hpp"
#include <sstream>

namespace wrl = Microsoft::WRL;

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// graphics exception checking/throwing macros (some with dxgi infos)
#define GFX_EXCEPT_NOINFO(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw Graphics::HrException( __LINE__,__FILE__,hr )

#ifndef NDEBUG
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#else
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#endif


Graphics::Graphics(HWND hWnd)
	:
	matrix(DirectX::XMMatrixIdentity())
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 3;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// for checking results of d3d functions
	HRESULT hr;

	// create device and front/back buffers, and swap chain and rendering context
	GFX_THROW_INFO(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		swapCreateFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		pSwap.ReleaseAndGetAddressOf(),
		&pDevice,
		nullptr,
		&pContext
	));
	// gain access to texture subresource in swap chain (back buffer)
	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	GFX_THROW_INFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
	GFX_THROW_INFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTarget));

	D3D11_VIEWPORT vp{};
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = 800.f;
	vp.Height = 600.f;
	pContext->RSSetViewports(1, &vp);

	D3D11_RECT rect{};
	rect.top = 0;
	rect.left = 0;
	rect.bottom = LONG_MAX;
	rect.right = LONG_MAX;
	pContext->RSSetScissorRects(1u, &rect);

	D3D11_TEXTURE2D_DESC depthBuffer{};
	depthBuffer.Width = 800u;
	depthBuffer.Height = 600u;
	depthBuffer.MipLevels = 1u;
	depthBuffer.ArraySize = 1u;
	depthBuffer.Format = DXGI_FORMAT_D32_FLOAT;
	depthBuffer.SampleDesc.Count = 1u;
	depthBuffer.SampleDesc.Quality = 0u;
	depthBuffer.Usage = D3D11_USAGE_DEFAULT;
	depthBuffer.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBuffer.CPUAccessFlags = 0u;
	depthBuffer.MiscFlags = 0u;

	GFX_THROW_INFO(pDevice->CreateTexture2D(&depthBuffer, nullptr, &depthTexture));

	D3D11_DEPTH_STENCIL_DESC dsDesc{};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = FALSE;

	GFX_THROW_INFO(pDevice->CreateDepthStencilState(&dsDesc, &pDSState));

	pContext->OMSetDepthStencilState(pDSState.Get(), 0u);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = depthBuffer.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0u;

	GFX_THROW_INFO(pDevice->CreateDepthStencilView(depthTexture.Get(), &dsvDesc, &pDSV));

}

void Graphics::EndFrame()
{
	HRESULT hr;
#ifndef NDEBUG
	infoManager.Set();
#endif
	if (FAILED(hr = pSwap->Present(0u, DXGI_PRESENT_ALLOW_TEARING)))
	{
		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
		}
		else
		{
			throw GFX_EXCEPT(hr);
		}
	}
}

void Graphics::ClearBuffer(float red, float green, float blue) noexcept
{
	const float color[] = { red,green,blue,1.0f };
	pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
	pContext->ClearRenderTargetView(pTarget.Get(), color);
}

void Graphics::DrawTestTriangle(float x, float y)
{
	static bool init = false;
	HRESULT hr;
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		struct
		{
			BYTE col[4];
		} color;
	};

	static float theta = 0.0f;
	theta += 1.0f / 3000.f;
	static float theta2 = 0.0f;
	theta2 += 1.3f / 3000.f;

	if (!init)
	{
		const Vertex vertices[] =
		{
			// pos						// color
			{ {-0.5f, 0.5f,0.0f },		{ 255,   0,   0, 255 } },
			{ { 0.5f,-0.5f,0.0f },		{	0, 255,   0, 255 } },
			{ {-0.5f,-0.5f,0.0f },		{	0,	 0, 255, 255 } },
			{ { 0.5f, 0.5f,0.0f },		{ 255,   0, 255, 255 } },

			{ {-0.5f, 0.5f, 1.0f },		{ 255,   0,   0, 255 } },
			{ { 0.5f,-0.5f, 1.0f },		{	0, 255,   0, 255 } },
			{ {-0.5f,-0.5f, 1.0f },		{	0,	 0, 255, 255 } },
			{ { 0.5f, 0.5f, 1.0f },		{ 255,   0, 255, 255 } },

		};
		const UINT16 indices[] =
		{
			0, 1, 2,
			0, 3, 1,

			1, 3, 5,
			3, 7, 5,

			5, 7, 6,
			7, 4, 6,

			6, 4, 2,
			4, 0, 2,

			0, 4, 3,
			4, 7, 3,

			1, 5, 2,
			5, 6, 2
		};

		indicesCount = _countof(indices);
		D3D11_BUFFER_DESC vertexBufferDesc{};
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = sizeof(vertices);
		vertexBufferDesc.StructureByteStride = sizeof(Vertex);
		vertexBufferDesc.CPUAccessFlags = 0u;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA sd{};
		sd.pSysMem = vertices;

		GFX_THROW_INFO(pDevice->CreateBuffer(&vertexBufferDesc, &sd, &vertexBuffer));

		D3D11_BUFFER_DESC indexBufferDesc{};
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.StructureByteStride = sizeof(UINT16);
		indexBufferDesc.ByteWidth = sizeof(indices);
		indexBufferDesc.CPUAccessFlags = 0u;
		sd = {};
		sd.pSysMem = indices;

		GFX_THROW_INFO(pDevice->CreateBuffer(&indexBufferDesc, &sd, &indexBuffer));

		wrl::ComPtr<ID3DBlob> vertexShaderBlob;
		{
			wrl::ComPtr<ID3DBlob> pixelShaderBlob;
			GFX_THROW_INFO(D3DReadFileToBlob(L"PixelShader.cso", &pixelShaderBlob));
			pDevice->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &pixelShader);
		}

		GFX_THROW_INFO(D3DReadFileToBlob(L"VertexShader.cso", &vertexShaderBlob));

		D3D11_PRIMITIVE_TOPOLOGY topology = { D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
		pContext->IASetPrimitiveTopology(topology);

		pDevice->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &vertexShader);

		pContext->PSSetShader(pixelShader.Get(), nullptr, 0u);
		pContext->VSSetShader(vertexShader.Get(), nullptr, 0u);

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		D3D11_BUFFER_DESC cBuffer{};
		cBuffer.ByteWidth = sizeof(matrix);
		cBuffer.Usage = D3D11_USAGE_DYNAMIC;
		cBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cBuffer.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA cbsrd{};
		cbsrd.pSysMem = &matrix;

		GFX_THROW_INFO(pDevice->CreateBuffer(&cBuffer, &cbsrd, &constantBuffer));

		pContext->VSSetConstantBuffers(0u, 1u, constantBuffer.GetAddressOf());
		pContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
		pContext->IASetVertexBuffers(0u, 1u, vertexBuffer.GetAddressOf(), &stride, &offset);

		D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			{ "Position", 0u, DXGI_FORMAT_R32G32B32_FLOAT, 0u, 0u, D3D11_INPUT_PER_VERTEX_DATA, 0u },
			{ "Color", 0u, DXGI_FORMAT_B8G8R8A8_UNORM, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u }
		};

		GFX_THROW_INFO(pDevice->CreateInputLayout(ied, _countof(ied), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &inputLayout));

		pContext->IASetInputLayout(inputLayout.Get());
		init = true;
	}

	DirectX::XMVECTOR v = DirectX::XMVectorSet(3.0f, 3.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR scalar = DirectX::XMVector4Dot(v, v);
	float res = DirectX::XMVectorGetX(scalar);
	DirectX::XMVerifyCPUSupport();

	// 1
	DirectX::XMVECTOR eyePosition = DirectX::XMVectorSet(xPos, yPos, zPos, 0.0f);
	DirectX::XMVECTOR focusPoint = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR upDirection = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	matrix = DirectX::XMMatrixTranspose(
		DirectX::XMMatrixRotationZ(theta) *
		DirectX::XMMatrixTranslation(x, y, 0.0f) *
		DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection) *
		DirectX::XMMatrixPerspectiveLH(1.f, 3.f / 4.f, 0.5f, 10.f)
	);

	D3D11_MAPPED_SUBRESOURCE msr{};
	pContext->Map(constantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr);
	memcpy(msr.pData, &matrix, sizeof(matrix));
	pContext->Unmap(constantBuffer.Get(), 0u);

	pContext->VSSetConstantBuffers(0u, 1u, constantBuffer.GetAddressOf());

	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), pDSV.Get());

	pContext->DrawIndexed(indicesCount, 0u, 0u);

	// 2
	matrix = DirectX::XMMatrixTranspose(
		DirectX::XMMatrixRotationZ(theta2) *
		DirectX::XMMatrixRotationX(theta2) *
		DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		DirectX::XMMatrixTranslation(0.0f, 0.0f, 6.0f) *
		DirectX::XMMatrixPerspectiveFovLH(80.f, 800.f / 600.f, 0.1, 100.f)
	);

	pContext->Map(constantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr);
	memcpy(msr.pData, &matrix, sizeof(matrix));
	pContext->Unmap(constantBuffer.Get(), 0u);

	pContext->VSSetConstantBuffers(0u, 1u, constantBuffer.GetAddressOf());

	pContext->DrawIndexed(indicesCount, 0u, 0u);
}


// Graphics exception stuff
Graphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file),
	hr(hr)
{
	// join all info messages with newlines into single string
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	// remove final newline if exists
	if (!info.empty())
	{
		info.pop_back();
	}
}

const char* Graphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;
	if (!info.empty())
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept
{
	return "Chili Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorString(hr);
}

std::string Graphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescription(hr, buf, sizeof(buf));
	return buf;
}

std::string Graphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}


const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "Chili Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}
