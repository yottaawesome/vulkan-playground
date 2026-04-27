module;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

export module volkus:win32;

export namespace Volkus::Win32
{
	using 
		::HINSTANCE,
		::HWND,
		::LRESULT,
		::WPARAM,
		::LPARAM,
		::LPWSTR,
		::UINT
		;

	

	namespace Crt
	{
		using
			::_set_abort_behavior;
		auto SetAbortBehavior(unsigned int flags, unsigned int mask) -> void
		{
			_set_abort_behavior(flags, mask);
		}

		constexpr auto WriteAbortMsg = _WRITE_ABORT_MSG;
		constexpr auto CallReportFault = _CALL_REPORTFAULT;
	}
	
}
