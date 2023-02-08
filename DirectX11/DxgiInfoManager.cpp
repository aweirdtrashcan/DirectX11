#include "DxgiInfoManager.h"

#include "Window.h"
#include "Graphics.h"
#include "GraphicsException.h"

#pragma comment(lib, "dxguid.lib")

DxgiInfoManager::DxgiInfoManager()
{
	typedef HRESULT(WINAPI* DXGIGetDebugInterface)(REFIID, void**);

	const auto hModDxgiDebug = LoadLibraryEx("dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (hModDxgiDebug == nullptr)
	{
		//throw GraphicsException("Failed to Load dxgidebug.dll", __FILE__, __LINE__);
	}

	const auto DxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(
		reinterpret_cast<void*>(GetProcAddress(hModDxgiDebug, "DXGIGetDebugInterface"))
		);

	if (DxgiGetDebugInterface == nullptr)
	{
		//throw GraphicsException("Failed to load DXGIGetDebugInterface", __FILE__, __LINE__);
	}

	DxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &pDxgiInfoQueue);
}

void DxgiInfoManager::Set() noexcept
{
	next = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
}

std::vector<std::string> DxgiInfoManager::GetMessages() const
{
	SIZE_T end = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
	std::vector<std::string> messages;
	for (int i = next; i < end; i++)
	{
		SIZE_T messageLen = 0;
		pDxgiInfoQueue->GetMessageA(DXGI_DEBUG_ALL, i, nullptr, &messageLen);
		auto bytes = std::make_unique<byte[]>(messageLen);
		auto pMessage = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.get());
		pDxgiInfoQueue->GetMessageA(DXGI_DEBUG_ALL, i, pMessage, &messageLen);
		messages.emplace_back(pMessage->pDescription);
	}
	return messages;
}
