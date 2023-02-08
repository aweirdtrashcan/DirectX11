#include "GraphicsException.h"

GraphicsException::GraphicsException(const char* reason, const char* file, int line) noexcept
{
	messages.emplace_back(reason);
	this->file = file;
	this->line = line;
}

GraphicsException::GraphicsException(std::vector<std::string> reason, const char* file, int line) noexcept
	: messages(reason), file(file), line(line)
{
}

const char* GraphicsException::what() const noexcept
{
	std::stringstream oss;
	oss << "A Graphics error has occurred" << std::endl
		<< "[File] " << file << std::endl
		<< "[Line] " << line << std::endl
		<< "=== ERROR MESSAGE ===" << std::endl;
	for (const auto& message : messages)
	{
		oss << std::endl << message << std::endl;
	}
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* GraphicsException::GetType() const noexcept
{
	return "Graphics Exception";
}