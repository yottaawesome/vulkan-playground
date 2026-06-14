module;

#include <gsl/pointers>
#include <gsl/zstring>

export module vulkangfx:gsl;

// https://github.com/microsoft/GSL
export namespace gsl
{
	using 
		gsl::not_null,
		gsl::czstring
		;
}
