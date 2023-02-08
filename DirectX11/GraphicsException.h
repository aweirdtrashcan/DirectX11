#pragma once
#include <stdexcept>
#include "ChiliWin.h"
#include "DxgiInfoManager.h"
#include <sstream>

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

