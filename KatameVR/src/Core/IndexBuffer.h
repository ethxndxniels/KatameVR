#pragma once

#include <d3d11.h>

#include <vector>
#include <string>

namespace Katame
{
	class IndexBuffer
	{
	public:
		IndexBuffer( ID3D11Device* device, const std::vector<unsigned short>& indices );
		IndexBuffer( ID3D11Device* device, std::string tag, const std::vector<unsigned short>& indices );
		void Bind( ID3D11DeviceContext* device ) noexcept;
		UINT GetCount() const noexcept;
	protected:
		std::string tag;
		UINT count;
		ID3D11Buffer* pIndexBuffer;
	};
}