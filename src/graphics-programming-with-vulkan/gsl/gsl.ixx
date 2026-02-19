module;

#include <gsl/gsl>

export module vulkangfx:gsl;

// https://github.com/microsoft/GSL
export namespace gsl
{
	using 
		gsl::not_null,
		gsl::span,
		gsl::czstring
		;
}