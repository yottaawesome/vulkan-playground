module;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module vulkangfx:glfw.exports;

export namespace glfw
{
	using
		::GLFWwindow,
		::glfwInit,
		::glfwInitHint,
		::glfwWindowShouldClose,
		::glfwWindowHint,
		::glfwCreateWindow,
		::glfwDestroyWindow,
		::glfwPollEvents,
		::glfwGetRequiredInstanceExtensions,
		::glfwGetPhysicalDevicePresentationSupport,
		::glfwCreateWindowSurface,
		::glfwTerminate
		;

	namespace WindowHints
	{
		// Window related hints
		constexpr auto
			Focused = GLFW_FOCUSED,
			Iconified = GLFW_ICONIFIED,
			Resizable = GLFW_RESIZABLE,
			Visible = GLFW_VISIBLE,
			Decorated = GLFW_DECORATED,
			AutoIconify = GLFW_AUTO_ICONIFY,
			Floating = GLFW_FLOATING,
			Maximized = GLFW_MAXIMIZED,
			CenterCursor = GLFW_CENTER_CURSOR,
			TransparentFramebuffer = GLFW_TRANSPARENT_FRAMEBUFFER,
			Hovered = GLFW_HOVERED,
			FocusOnShow = GLFW_FOCUS_ON_SHOW
			;

		// Framebuffer related hints
		constexpr auto
			RedBits = GLFW_RED_BITS,
			GreenBits = GLFW_GREEN_BITS,
			BlueBits = GLFW_BLUE_BITS,
			AlphaBits = GLFW_ALPHA_BITS,
			DepthBits = GLFW_DEPTH_BITS,
			StencilBits = GLFW_STENCIL_BITS,
			AccumRedBits = GLFW_ACCUM_RED_BITS,
			AccumGreenBits = GLFW_ACCUM_GREEN_BITS,
			AccumBlueBits = GLFW_ACCUM_BLUE_BITS,
			AccumAlphaBits = GLFW_ACCUM_ALPHA_BITS,
			AuxBuffers = GLFW_AUX_BUFFERS,
			Stereo = GLFW_STEREO,
			Samples = GLFW_SAMPLES,
			SrgbCapable = GLFW_SRGB_CAPABLE,
			Doublebuffer = GLFW_DOUBLEBUFFER
			;

		// Monitor related hints
		constexpr auto
			RefreshRate = GLFW_REFRESH_RATE
			;

		// Context related hints
		constexpr auto
			ClientApi = GLFW_CLIENT_API,
			ContextVersionMajor = GLFW_CONTEXT_VERSION_MAJOR,
			ContextVersionMinor = GLFW_CONTEXT_VERSION_MINOR,
			ContextRevision = GLFW_CONTEXT_REVISION,
			ContextRobustness = GLFW_CONTEXT_ROBUSTNESS,
			OpenglForwardCompat = GLFW_OPENGL_FORWARD_COMPAT,
			OpenglDebugContext = GLFW_OPENGL_DEBUG_CONTEXT,
			OpenglProfile = GLFW_OPENGL_PROFILE,
			ContextReleaseBehavior = GLFW_CONTEXT_RELEASE_BEHAVIOR,
			ContextNoError = GLFW_CONTEXT_NO_ERROR,
			ContextCreationApi = GLFW_CONTEXT_CREATION_API,
			ScaleToMonitor = GLFW_SCALE_TO_MONITOR
			;

		// Client API values
		constexpr auto
			NoApi = GLFW_NO_API,
			OpenglApi = GLFW_OPENGL_API,
			OpenglEsApi = GLFW_OPENGL_ES_API
			;

		// Context robustness values
		constexpr auto
			NoRobustness = GLFW_NO_ROBUSTNESS,
			NoResetNotification = GLFW_NO_RESET_NOTIFICATION,
			LoseContextOnReset = GLFW_LOSE_CONTEXT_ON_RESET
			;

		// OpenGL profile values
		constexpr auto
			OpenglAnyProfile = GLFW_OPENGL_ANY_PROFILE,
			OpenglCoreProfile = GLFW_OPENGL_CORE_PROFILE,
			OpenglCompatProfile = GLFW_OPENGL_COMPAT_PROFILE
			;

		// Context release behavior values
		constexpr auto
			AnyReleaseBehavior = GLFW_ANY_RELEASE_BEHAVIOR,
			ReleaseBehaviorFlush = GLFW_RELEASE_BEHAVIOR_FLUSH,
			ReleaseBehaviorNone = GLFW_RELEASE_BEHAVIOR_NONE
			;

		// Context creation API values
		constexpr auto
			NativeContextApi = GLFW_NATIVE_CONTEXT_API,
			EglContextApi = GLFW_EGL_CONTEXT_API,
			OsMesaContextApi = GLFW_OSMESA_CONTEXT_API
			;

		// macOS specific window hints
		constexpr auto
			CocoaRetinaFramebuffer = GLFW_COCOA_RETINA_FRAMEBUFFER,
			CocoaFrameName = GLFW_COCOA_FRAME_NAME,
			CocoaGraphicsSwitching = GLFW_COCOA_GRAPHICS_SWITCHING
			;

		// X11 specific window hints
		constexpr auto
			X11ClassName = GLFW_X11_CLASS_NAME,
			X11InstanceName = GLFW_X11_INSTANCE_NAME
			;
	}

	namespace InitHints
	{
		// Shared init hints
		constexpr auto
			Platform = GLFW_PLATFORM,
			JoystickHatButtons = GLFW_JOYSTICK_HAT_BUTTONS,
			AnglePlatformType = GLFW_ANGLE_PLATFORM_TYPE
			;

		// Platform values
		constexpr auto
			AnyPlatform = GLFW_ANY_PLATFORM,
			PlatformWin32 = GLFW_PLATFORM_WIN32,
			PlatformCocoa = GLFW_PLATFORM_COCOA,
			PlatformWayland = GLFW_PLATFORM_WAYLAND,
			PlatformX11 = GLFW_PLATFORM_X11,
			PlatformNull = GLFW_PLATFORM_NULL
			;

		// ANGLE platform type values
		constexpr auto
			AnglePlatformTypeNone = GLFW_ANGLE_PLATFORM_TYPE_NONE,
			AnglePlatformTypeOpengl = GLFW_ANGLE_PLATFORM_TYPE_OPENGL,
			AnglePlatformTypeOpengles = GLFW_ANGLE_PLATFORM_TYPE_OPENGLES,
			AnglePlatformTypeD3d9 = GLFW_ANGLE_PLATFORM_TYPE_D3D9,
			AnglePlatformTypeD3d11 = GLFW_ANGLE_PLATFORM_TYPE_D3D11,
			AnglePlatformTypeVulkan = GLFW_ANGLE_PLATFORM_TYPE_VULKAN,
			AnglePlatformTypeMetal = GLFW_ANGLE_PLATFORM_TYPE_METAL
			;

		// macOS specific init hints
		constexpr auto
			CocoaChdirResources = GLFW_COCOA_CHDIR_RESOURCES,
			CocoaMenubar = GLFW_COCOA_MENUBAR
			;

		// Wayland specific init hints
		constexpr auto
			WaylandLibdecor = GLFW_WAYLAND_LIBDECOR
			;

		// Wayland libdecor values
		constexpr auto
			WaylandPreferLibdecor = GLFW_WAYLAND_PREFER_LIBDECOR,
			WaylandDisableLibdecor = GLFW_WAYLAND_DISABLE_LIBDECOR
			;

		// X11 specific init hints
		constexpr auto
			X11XcbVulkanSurface = GLFW_X11_XCB_VULKAN_SURFACE
			;
	}
}
