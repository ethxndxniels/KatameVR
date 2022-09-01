#pragma once

namespace Katame
{
	struct XRCallback
	{
		XrStructureType type;
		void(*callback)(XrEventDataBuffer);
	};
}