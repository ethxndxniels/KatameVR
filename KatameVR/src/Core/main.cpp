#include "Application.h"
#include "Log.h"

int main( int argc, char** argv )
{
	try
	{
		Katame::Log::Init();
		KM_CORE_INFO( "Initialized Log!" );
		Katame::Application{}.Launch();
	}
	catch (std::exception& e)
	{
		MessageBox( nullptr, (LPCTSTR)e.what(), L"Standard Exception", MB_OK | MB_ICONEXCLAMATION );
	}
	catch (...)
	{
		MessageBox( nullptr, L"No details available", L"Unknown Exception", MB_OK | MB_ICONEXCLAMATION );
	}
	return -1;
}
