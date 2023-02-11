#pragma once
#include "Bindable.hpp"

class Topology : public Bindable
{
public:
	Topology(Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type);
	void Bind(Graphics& gfx) noexcept;
protected:
	D3D11_PRIMITIVE_TOPOLOGY type;
};

