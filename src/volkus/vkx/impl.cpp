// Single, non-module translation unit that emits the definitions for volk
// (the Vulkan function-pointer loader) and VMA (the Vulkan Memory Allocator).
//
// Both libraries are header-only and use the "define the _IMPLEMENTATION
// macro in exactly one TU" idiom. Keeping those definitions isolated here
// has two benefits:
//
//   1. Decouples the module export surface (vulkan.exports.ixx,
//      vma/vma.exports.ixx) from the (large) implementation code, so
//      editing the re-export lists doesn't force a rebuild of volk + VMA.
//   2. Lets separate module partitions `#include` <volk.h> and
//      <vma/vk_mem_alloc.h> for declarations without risking duplicate
//      definitions — the *_IMPLEMENTATION macros live only here.
//
// This file intentionally is NOT a module unit (no `module;` / `export
// module` preamble). The symbols it defines have external linkage and are
// attached to the global module, which is exactly what we want for C APIs.

#define VOLK_IMPLEMENTATION
#include <volk.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
