#include "Graphics.h"

#define DX_CLEAR_MESSAGES dxgiIM.Set // clear past messages in the DXGI Message Queue

namespace WRL = Microsoft::WRL;

Graphics::Graphics(HWND hWnd, UINT width, UINT height)
	: width(width), height(height)
{
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.RefreshRate.Numerator = 1;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferCount = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.Flags = 0;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	DX_CHECK_ERROR(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwap,
		&pDevice,
		nullptr,
		&pContext
	));

	DX_CLEAR_MESSAGES();

	WRL::ComPtr<ID3D11Resource> pResource;
	DX_CHECK_ERROR(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pResource));
	DX_CHECK_ERROR(pDevice->CreateRenderTargetView(
		pResource.Get(),
		nullptr,
		&pTarget
	));

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = FALSE;
	dsDesc.StencilReadMask = NULL;
	dsDesc.StencilWriteMask = NULL;
	WRL::ComPtr<ID3D11DepthStencilState> pDSState;
	DX_CHECK_ERROR(pDevice->CreateDepthStencilState(&dsDesc, &pDSState));

	pContext->OMSetDepthStencilState(pDSState.Get(), 1u);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
	D3D11_TEXTURE2D_DESC textDesc = {};
	textDesc.Format = DXGI_FORMAT_D32_FLOAT;
	textDesc.ArraySize = 1;
	textDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textDesc.Usage = D3D11_USAGE_DEFAULT;
	textDesc.CPUAccessFlags = NULL;
	textDesc.Height = height;
	textDesc.Width = width;
	textDesc.MipLevels = 1;
	textDesc.SampleDesc.Count = 1;
	textDesc.SampleDesc.Quality = 0;

	DX_CHECK_ERROR(pDevice->CreateTexture2D(&textDesc, nullptr, &pTexture));

	D3D11_DEPTH_STENCIL_VIEW_DESC pDSVDesc = {};
	pDSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
	pDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	pDSVDesc.Texture2D.MipSlice = 0;

	DX_CHECK_ERROR(pDevice->CreateDepthStencilView(pTexture.Get(), &pDSVDesc, &pDSV));

	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), pDSV.Get());
}

void Graphics::EndFrame()
{
#ifndef NDEBUG
	DX_CHECK_ERROR(pSwap->Present(0u, 0u));
#else
	pSwap->Present(0u, 0u);
#endif
}

void Graphics::ClearBuffer(float red, float green, float blue, float alpha)
{
	const float color[] = { red, green, blue, alpha };
	pContext->ClearRenderTargetView(pTarget.Get(), color);
	pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, NULL);
}

void Graphics::DrawTestTriangle(float angle, float x, float z)
{
	struct Vertex
	{
		struct
		{
			float x;
			float y;
			float z;
		} pos;
	};

	Vertex vertices[] =
	{
		{-1.0f, -1.0f, -1.0f }, // 0
		{ 1.0f, -1.0f, -1.0f }, // 1
		{-1.0f,  1.0f, -1.0f }, // 2
		{ 1.0f,  1.0f, -1.0f }, // 3

		{-1.0f, -1.0f, 1.0f }, // 4
		{ 1.0f, -1.0f, 1.0f }, // 5
		{-1.0f,  1.0f, 1.0f }, // 6
		{ 1.0f,  1.0f, 1.0f }, // 7
	};

	D3D11_BUFFER_DESC bd{};
	D3D11_SUBRESOURCE_DATA sd{};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.StructureByteStride = sizeof(Vertex);
	bd.MiscFlags = 0;
	bd.ByteWidth = sizeof(vertices);

	sd.pSysMem = vertices;

	DX_CHECK_ERROR(pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer));

	UINT stride = sizeof(Vertex);
	UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);

	const unsigned short indices[] =
	{
		0,2,1,	2,3,1,
		1,3,5,	3,7,5,
		2,6,3,	3,6,7,
		4,5,7,	4,7,6,
		0,4,2,	2,4,6,
		0,1,4,	1,5,4
	};

	WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
	D3D11_BUFFER_DESC ibd = {};
	D3D11_SUBRESOURCE_DATA isd = {};
	ibd.ByteWidth = sizeof(indices);
	ibd.StructureByteStride = sizeof(unsigned short);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = NULL;
	ibd.MiscFlags = NULL;
	ibd.Usage = D3D11_USAGE_DEFAULT;

	isd.pSysMem = indices;

	DX_CHECK_ERROR(pDevice->CreateBuffer(&ibd, &isd, &pIndexBuffer));

	pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	struct ConstantBuffer
	{
		DirectX::XMMATRIX transform;
	};

	const ConstantBuffer cb =
	{
		{
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixRotationZ(angle) *
				DirectX::XMMatrixRotationX(angle) *
				DirectX::XMMatrixTranslation(x, 0.0f, z + 4.0f) *
				DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 10.f)
			)
		}
	};

	WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
	D3D11_BUFFER_DESC cbd = {};
	D3D11_SUBRESOURCE_DATA csd = {};
	cbd.ByteWidth = sizeof(cb);
	cbd.StructureByteStride = NULL;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.MiscFlags = NULL;

	csd.pSysMem = &cb;

	DX_CHECK_ERROR(pDevice->CreateBuffer(&cbd, &csd, &pConstantBuffer));

	pContext->VSSetConstantBuffers(0u, 1, pConstantBuffer.GetAddressOf());

	struct ConstantBuffer2
	{
		struct
		{
			float r;
			float g;
			float b;
			float a;
		} face_colors[6];
	};

	const ConstantBuffer2 cb2 =
	{
		{
			{1.0f, 0.0f, 1.0f, 1.0f},
			{1.0f, 0.0f, 0.0f, 1.0f},
			{0.0f, 1.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f, 1.0f},
			{1.0f, 1.0f, 0.0f, 1.0f},
			{0.0f, 1.0f, 1.0f, 1.0f},
		}
	};

	WRL::ComPtr<ID3D11Buffer> pConstantBuffer2;
	D3D11_BUFFER_DESC cbd2 = {};
	D3D11_SUBRESOURCE_DATA csd2 = {};
	cbd2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd2.Usage = D3D11_USAGE_DEFAULT;
	cbd2.CPUAccessFlags = NULL;
	cbd2.MiscFlags = NULL;
	cbd2.ByteWidth = sizeof(cb2);
	cbd2.StructureByteStride = NULL;
	csd2.pSysMem = &cb2;

	DX_CHECK_ERROR(pDevice->CreateBuffer(&cbd2, &csd2, &pConstantBuffer2));

	pContext->PSSetConstantBuffers(0u, 1u, pConstantBuffer2.GetAddressOf());

	WRL::ComPtr<ID3D11VertexShader> pVertexShader;
	WRL::ComPtr<ID3DBlob> pBlob;
	D3DReadFileToBlob(L"VertexShader.cso", &pBlob);
	DX_CHECK_ERROR(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));

	pContext->VSSetShader(pVertexShader.Get(), NULL, NULL);

	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	WRL::ComPtr<ID3D11InputLayout> pInputLayout;

	DX_CHECK_ERROR(pDevice->CreateInputLayout(ied, (UINT)std::size(ied), pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout));

	pContext->IASetInputLayout(pInputLayout.Get());

	WRL::ComPtr<ID3D11PixelShader> pPixelShader;

	DX_CHECK_ERROR(D3DReadFileToBlob(L"PixelShader.cso", &pBlob));
	DX_CHECK_ERROR(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));

	pContext->PSSetShader(pPixelShader.Get(), nullptr, NULL);

	//pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);

	D3D11_VIEWPORT vp = {};
	vp.Width = 800;
	vp.Height = 600;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	pContext->RSSetViewports(1u, &vp);

	pContext->DrawIndexed((UINT)std::size(indices), 0, 0);
	CheckException();
}

void Graphics::CheckException() const
{
	const std::vector<std::string> messages = dxgiIM.GetMessages();
	if (!messages.empty())
		DX_THROW_ERROR(messages);
}
