#pragma once

#define DX_CLEAR_MESSAGES dxgiIM.Set // clear past messages in the DXGI Message Queue

#define DX_THROW_ERROR(reason) throw GraphicsException(reason, __FILE__, __LINE__) // will check for DX errors and if an error is found, this macro will throw an exception

#define DX_CHECK_ERROR(hr) if (FAILED(hr)) DX_THROW_ERROR(dxgiIM.GetMessages())
