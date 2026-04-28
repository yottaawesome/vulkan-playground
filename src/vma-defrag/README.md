# vma-defrag

A focused, headless sample showing how to drive the **Vulkan Memory
Allocator (VMA)** defragmentation API on top of **Vulkan 1.4**, while keeping
buffer device addresses in sync.

This complements the sibling `vulkan-memory` project, which implements
defragmentation manually against a hand-rolled sub-allocator. Here we let VMA
own the heap and only handle the per-move work it asks of us.

## What it demonstrates

1. **Vulkan 1.4 instance + device** with the modern feature opt-ins:
   - `bufferDeviceAddress` (core 1.2)
   - `synchronization2`     (core 1.3)
2. **VMA allocator** created with
   `VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT` so every block it
   allocates is eligible for `vkGetBufferDeviceAddress`.
3. **Buffer device addresses** queried per buffer
   (`VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT`).
4. **Heap fragmentation** induced by allocating a mixed-size set of buffers
   and freeing every other one.
5. **VMA defragmentation loop** using the canonical four-call dance:
   `vmaBeginDefragmentation` →
   `vmaBeginDefragmentationPass` →
   `vmaEndDefragmentationPass` →
   `vmaEndDefragmentation`.
6. **Address invalidation**: pre/post-defrag device addresses are diffed so
   you can see which buffers physically moved.
7. **Data integrity check**: each buffer carries a recognisable byte pattern
   that is verified post-defrag.

## How VMA's defragmentation works

VMA cannot rewrite your `VkBuffer` handles for you, because a `VkBuffer` is
permanently bound to a specific `(VkDeviceMemory, offset)` pair and has no
"rebind" entry point. Instead the API hands you a list of *moves* and asks
that, for each one, you:

1. **Recreate the buffer** with the same `VkBufferCreateInfo` as the original.
2. **Bind it** to the freshly-allocated `dstTmpAllocation` using
   `vmaBindBufferMemory`.
3. **Copy the data** from the source memory to the destination memory.
   - For HOST-visible memory (this sample): plain `memcpy` between mapped
     pointers obtained from `vmaGetAllocationInfo`.
   - For DEVICE-local memory: record `vkCmdCopyBuffer` between the old and
     new buffers in a one-time-submit command buffer and wait on a fence
     before calling `vmaEndDefragmentationPass`.
4. **Destroy the old `VkBuffer`** (VMA frees the old memory itself when
   `vmaEndDefragmentationPass` runs).
5. Set `move.operation` to `VMA_DEFRAGMENTATION_MOVE_OPERATION_COPY` (the
   default; we set it explicitly to document intent).

After `vmaEndDefragmentationPass`, VMA atomically swaps the underlying
storage so the **`VmaAllocation` handle you stored stays valid** and now
refers to the compacted memory. Your **`VkBuffer` handle, however, has been
replaced** by the one you created — and any **device address you cached
must be re-queried** with `vkGetBufferDeviceAddress` because the underlying
`(memory, offset)` has changed.

`vmaBeginDefragmentationPass` returns:

- `VK_SUCCESS`    — defragmentation is complete; exit the loop.
- `VK_INCOMPLETE` — there is more work; service the moves and continue.
- anything else   — error.

`vmaEndDefragmentationPass` returns the same tri-state to indicate whether
another pass is worthwhile.

## Files

| File              | Purpose                                                 |
| ----------------- | ------------------------------------------------------- |
| `vk.exports.ixx`  | Module partition re-exporting the Vulkan C API.         |
| `vma.exports.ixx` | Module partition re-exporting VMA into `vma::`.         |
| `vma.impl.cpp`    | The single TU that defines `VMA_IMPLEMENTATION`.        |
| `vmadefrag.ixx`   | Top-level module aggregating the partitions.            |
| `main.cpp`        | The demo: setup, allocate, fragment, defrag, verify.    |
| `vcpkg.json`      | Manifest pulling in `vulkan` and `vulkan-memory-allocator`. |

## Build

This is an MSVC / Visual Studio project (`vma-defrag.vcxproj`) targeting
C++23 modules (`stdcpplatest`, `BuildStlModules=true`) on x64. vcpkg manifest
mode handles the dependencies.

Open `vulkan-playground.slnx` and build the `vma-defrag` project, or:

```pwsh
msbuild src\vma-defrag\vma-defrag.vcxproj /p:Configuration=Debug /p:Platform=x64
```

Run the resulting console executable; it prints the heap state before and
after defragmentation along with the pass-by-pass move log.

## Caveats

- The sample uses HOST-visible memory so the defragmentation copy is a
  `memcpy`. For DEVICE-local memory the same flow applies but step 3 above
  becomes `vkCmdCopyBuffer` + queue submit + fence wait between
  `BeginDefragmentationPass` and `EndDefragmentationPass`.
- `bufferDeviceAddress` requires the corresponding feature to be enabled at
  device creation time **and** the allocator to be created with
  `VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT`. Forgetting either will
  surface as a validation error or `VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS`.
- The defrag flag here is `VMA_DEFRAGMENTATION_FLAG_ALGORITHM_FAST_BIT`. Try
  `_BALANCED_` or `_FULL_` for more aggressive compaction at the cost of
  more move work.
