#include "Graphics.h"

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

	D3D11_VIEWPORT vp;
	vp.Width = 800.0f;
	vp.Height = 600.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	pContext->RSSetViewports(1u, &vp);
}

void Graphics::EndFrame()
{
#ifndef NDEBUG
	DX_CHECK_ERROR(pSwap->Present(1u, 0u));
#else
	pSwap->Present(1u, 0u);
#endif
}

void Graphics::ClearBuffer(float red, float green, float blue, float alpha)
{
	const float color[] = { red, green, blue, alpha };
	pContext->ClearRenderTargetView(pTarget.Get(), color);
	pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, NULL);
}

void Graphics::DrawIndexed(UINT count) noexcept
{
	pContext->DrawIndexed(count, 0u, 0u);
	CheckException();
}

void Graphics::CheckException() const
{
	const std::vector<std::string> messages = dxgiIM.GetMessages();
	if (!messages.empty())
		DX_THROW_ERROR(messages);
}

void Graphics::SetProjection(DirectX::FXMMATRIX proj) noexcept
{
	projection = proj;
}

DirectX::XMMATRIX Graphics::GetProjection() const noexcept
{
	return projection;
}
