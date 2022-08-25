#pragma once

#include "Bindable.h"
#include "../Graphics/Vertex.h"
#include <memory>

namespace Katame
{
	namespace Bind
	{

		class VertexBuffer
		{
		public:
			VertexBuffer( unsigned int size );
			~VertexBuffer() {}

			void SetData( void* buffer, unsigned int size, unsigned int offset = 0 );
			void Bind() const ;

			unsigned int GetSize() const;

			static VertexBuffer* Create( unsigned int size = 0 );
		private:
			unsigned int m_Size;
		};
	
	}
}