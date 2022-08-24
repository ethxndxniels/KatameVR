#pragma once
#include "Bindable.h"

namespace Katame
{

	namespace Bind
	{
		class Sampler : public Bindable
		{
		public:
			Sampler( Graphics* gfx );
			void Bind( Graphics* gfx ) noexcept override;
			static std::shared_ptr<Sampler> Resolve( Graphics* gfx );
			static std::string GenerateUID();
			std::string GetUID() const noexcept override;
		protected:
			ID3D11SamplerState* pSampler;
		};
	}

}