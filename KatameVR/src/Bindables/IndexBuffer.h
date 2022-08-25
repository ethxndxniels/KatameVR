#pragma once

#include "Bindable.h"
#include "BindableCodex.h"

namespace Katame
{
	namespace Bind
	{
		class IndexBuffer
		{
		public:
			IndexBuffer( unsigned int size );
			virtual ~IndexBuffer();

			void SetData( void* buffer, unsigned int size, unsigned int offset = 0 );
			void Bind() const;

			uint32_t GetCount() const { return m_Size / sizeof( uint32_t ); }

			unsigned int GetSize() const { return m_Size; }
		private:
			unsigned int m_Size;
		};
	}
}