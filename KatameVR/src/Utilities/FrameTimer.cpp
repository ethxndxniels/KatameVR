#include "FrameTimer.h"

namespace Katame
{
	using namespace std::chrono;

	FrameTimer::FrameTimer()
	{
		m_Last = steady_clock::now();
	}

	float FrameTimer::Mark()
	{
		const auto old = m_Last;
		m_Last = steady_clock::now();
		const duration<float> frameTime = m_Last - old;
		return frameTime.count();
	}

	float FrameTimer::Peek() const
	{
		return duration<float>( steady_clock::now() - m_Last ).count();
	}
}