#pragma once
#include <stdexcept>
#include "ChiliWin.h"
#include "DxgiInfoManager.h"
#include <sstream>

#define DX_THROW_ERROR(reason) throw GraphicsException(reason, __FILE__, __LINE__) // will check for DX errors and if an error is found, this macro will throw an exception

#define DX_CHECK_ERROR(hr) if (FAILED(hr)) DX_THROW_ERROR(dxgiIM.GetMessages())

class GraphicsException : public std::exception
{
public:
	GraphicsException(std::vector<std::string> reason, const char* file, int line) noexcept;
	GraphicsException(const char* reason, const char* file, int line) noexcept;
	const char* what() const noexcept override;
	const char* GetType() const noexcept;
private:
	const char* file;
	int line;
	std::vector<std::string> messages;

	mutable std::string whatBuffer;
};

