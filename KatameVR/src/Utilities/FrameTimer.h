#pragma once
#include <chrono>

namespace Katame
{
	class FrameTimer
	{
	public:
		FrameTimer();
		float Mark();
		float Peek() const;
	private:
		std::chrono::steady_clock::time_point m_Last;
	};
}