module;

#include <tiny_obj_loader.h>

export module vulkan26:tinyobj.exports;

export namespace tinyobj
{
	using 
		::tinyobj::attrib_t,
		::tinyobj::shape_t,
		::tinyobj::material_t,
		::tinyobj::LoadObj
		;
}
