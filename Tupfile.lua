tup.include("libs/tupblocks/Tuprules.lua")

-- SDL
-- ---

SDL = sourcepath("libs/SDL/")
SDL_COMPILE = {
	base = {
		cflags = "/DDLL_EXPORT",
		lflags = (
			"advapi32.lib imm32.lib gdi32.lib kernel32.lib ole32.lib " ..
			"oleaut32.lib setupapi.lib user32.lib version.lib winmm.lib"
		),
		objdir = "SDL/",
	},
	buildtypes = {
		debug = { lflags = "ucrtd.lib" },
		release = { lflags = "ucrt.lib" },
	}
}
SDL_LINK = {
	base = {
		cflags = ("-I" .. SDL.join("include/")),
		lflags = "shell32.lib", -- for SDL_main()'s CommandLineToArgvW()
	},
}

sdl_src += SDL.glob("src/*.c")
sdl_src += SDL.glob("src/atomic/*.c")
sdl_src += (SDL.glob("src/audio/*.c") - { "SDL_audiodev.c$" })
sdl_src += SDL.glob("src/audio/directsound/*.c")
sdl_src += SDL.glob("src/audio/disk/*.c")
sdl_src += SDL.glob("src/audio/dummy/*.c")
sdl_src += SDL.glob("src/audio/winmm/*.c")
sdl_src += (SDL.glob("src/audio/wasapi/*.c") - { "winrt.cpp$" })
sdl_src += SDL.glob("src/core/windows/*.c")
sdl_src += SDL.glob("src/cpuinfo/*.c")
sdl_src += SDL.glob("src/dynapi/*.c")
sdl_src += (
	SDL.glob("src/events/*.c") -
	{ "imKStoUCS.c$", "SDL_keysym_to_scancode.c$", "SDL_scancode_tables.c$" }
)
sdl_src += SDL.glob("src/file/*.c")
sdl_src += SDL.glob("src/filesystem/windows/*.c")
sdl_src += SDL.glob("src/haptic/*.c")
sdl_src += SDL.glob("src/haptic/windows/*.c")
sdl_src += SDL.glob("src/hidapi/*.c")
sdl_src += SDL.glob("src/joystick/*.c")
sdl_src += SDL.glob("src/joystick/hidapi/*.c")
sdl_src += SDL.glob("src/joystick/virtual/*.c")
sdl_src += SDL.glob("src/joystick/windows/*.c")
sdl_src += SDL.glob("src/libm/*.c")
sdl_src += SDL.glob("src/loadso/windows/*.c")
sdl_src += SDL.glob("src/locale/*.c")
sdl_src += SDL.glob("src/locale/windows/*.c")
sdl_src += SDL.glob("src/misc/*.c")
sdl_src += SDL.glob("src/misc/windows/*.c")
sdl_src += SDL.glob("src/power/*.c")
sdl_src += SDL.glob("src/power/windows/*.c")
sdl_src += SDL.glob("src/render/*.c")
sdl_src += SDL.glob("src/render/direct3d/*.c")
sdl_src += (SDL.glob("src/render/direct3d11/*.c") - { "winrt.cpp$" })
sdl_src += (SDL.glob("src/render/direct3d12/*.c") - { "winrt.cpp$" })
sdl_src += SDL.glob("src/render/opengl/*.c")
sdl_src += SDL.glob("src/render/opengles2/*.c")
sdl_src += SDL.glob("src/render/software/*.c")
sdl_src += SDL.glob("src/sensor/*.c")
sdl_src += SDL.glob("src/sensor/windows/*.c")
sdl_src += (SDL.glob("src/stdlib/*.c") - { "SDL_mslibc.c$" })
sdl_src += SDL.glob("src/thread/*.c")
sdl_src += SDL.glob("src/thread/windows/*.c")
sdl_src += SDL.glob("src/timer/*.c")
sdl_src += SDL.glob("src/timer/windows/*.c")
sdl_src += SDL.glob("src/video/*.c")
sdl_src += SDL.glob("src/video/dummy/*.c")
sdl_src += SDL.glob("src/video/windows/*.c")
sdl_src += SDL.glob("src/video/yuv2rgb/*.c")
sdl_src += SDL.join("src/thread/generic/SDL_syscond.c")
sdl_winmain_src += SDL.glob("src/main/windows/*.c")

sdl_cfg = CONFIG:branch("", SDL_COMPILE, SDL_LINK)
sdl_obj = (
	cxx(sdl_cfg, sdl_src) +
	rc(sdl_cfg, SDL.join("src/main/windows/version.rc"))
)
sdl_dll = (
	dll(sdl_cfg, sdl_obj, "SDL2") +
	cxx(sdl_cfg, sdl_winmain_src)
)
-- ---

-- Static analysis using the C++ Core Guideline checker plugin.
ANALYSIS_CFLAGS = (
	"/analyze:autolog- /analyze:plugin EspXEngine.dll " ..
	"/external:W0 /external:anglebrackets /analyze:external- " ..

	-- Critical warnings
	"/we26819 " .. -- Unannotated fallthrough between switch labels

	-- Disabled warnings
	"/wd4834 " .. -- Discarding `[[nodiscard]]` (C6031 covers this and more)
	"/wd26408 " .. -- Avoid _malloca()
	"/wd26432 " .. -- Rule of Five boilerplate
	"/wd26440 " .. -- `noexcept` all the things
	"/wd26481 " .. -- Don't use pointer arithmetic
	"/wd26482 " .. -- Only index into arrays using constant expressions
	"/wd26490 " .. -- Don't use `reinterpret_cast`
	"/wd26429 /wd26446 /wd26472 /wd26821" -- Guideline Support Library
)

ANALYSIS = {
	buildtypes = {
		release = { cflags = ANALYSIS_CFLAGS },
	},
}

main_cfg = CONFIG:branch(tup.getconfig("BUILDTYPE"), SDL_LINK, {
	base = {
		cflags = (
			"/std:c++latest " ..
			"/DWIN32 " ..
			"/I. " ..
			"/EHsc " ..
			"/source-charset:utf-8 " ..
			"/execution-charset:utf-8"
		),
		lflags = "obj/dinput.lib",
		objdir = "ssg/",
	},
	buildtypes = {
		debug = { cflags = "/DPBG_DEBUG" },
		release = { cflags = "/DNDEBUG" },
	}
})

modern_cfg = main_cfg:branch(tup.getconfig("BUILDTYPE"), ANALYSIS)
modern_src += tup.glob("game/*.cpp")
modern_src += tup.glob("platform/windows/*.cpp")
main_src += tup.glob("DirectXUTYs/*.CPP")
main_src += tup.glob("DirectXUTYs/*.cpp")
main_src += tup.glob("GIAN07/*.cpp")
main_src += tup.glob("GIAN07/*.CPP")

main_obj = (cxx(modern_cfg, modern_src) + cxx(main_cfg, main_src))

main_win32_src += "MAIN/MAIN.CPP"
main_win32_src += tup.glob("platform/windows_vintage/*.CPP")
main_win32_obj = cxx(main_cfg, main_win32_src)
exe(main_cfg, (main_win32_obj + main_obj), "GIAN07_WIN32")

main_sdl_cfg = main_cfg:branch("", ANALYSIS, {
	base = { lflags = "/SUBSYSTEM:windows" }
})

main_sdl_src += "MAIN/main_sdl.cpp"
main_sdl_src += tup.glob("platform/miniaudio/*.cpp")
main_sdl_obj = cxx(main_sdl_cfg, main_sdl_src)
exe(main_sdl_cfg, (main_sdl_obj + main_obj + sdl_dll), "GIAN07")
