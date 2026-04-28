// Single non-module translation unit that emits the VMA implementation.
//
// VMA is header-only and uses the "define VMA_IMPLEMENTATION in exactly one
// TU" idiom. Keeping that definition isolated here means:
//
//   1. The module export surface (vma.exports.ixx) can be re-included in
//      multiple TUs without ODR violations.
//   2. Editing the export list does not force a rebuild of VMA itself.
//
// This file is NOT a module unit — it has no `module;` / `export module`
// preamble. The symbols it defines have external linkage on the global
// module, which is exactly what we want for a C API.

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
