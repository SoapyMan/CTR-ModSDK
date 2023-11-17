-- premake5.lua

require "premake_modules/usage"

------------------------------------------

-- you can redefine dependencies
SDL2_DIR = os.getenv("SDL2_DIR") or "dependencies/SDL2"
OPENAL_DIR = os.getenv("OPENAL_DIR") or "dependencies/openal-soft"

GAME_REGION = os.getenv("GAME_REGION") or "NTSC_VERSION" -- or PAL_VERSION
GAME_VERSION = os.getenv("APPVEYOR_BUILD_VERSION") or nil

if not (GAME_REGION == "NTSC_VERSION" or GAME_REGION == "PAL_VERSION") then
    error("'GAME_REGION' should be 'NTSC_VERSION' or 'PAL_VERSION'")
end

------------------------------------------
	
workspace "CrashTeamRacingPC"
    location "project_%{_ACTION}_%{os.target()}"
    configurations { "Debug", "Release", "Release_dev" }
	
    defines { 
		VERSION,
		"USE_EXTENDED_PRIM_POINTERS=0"
	} 
	platforms { "x86" } --, "x86_64" }
	
	startproject "CTRPC"
	
	configuration "raspberry-pi"
		defines { "__RPI__" }

	filter "system:Linux"
		buildoptions {
            "-Wno-narrowing",
			"-Wno-endif-labels",
			"-Wno-write-strings",
			"-Wno-format-security",
			"-Wno-unused-result",
            "-fpermissive"
        }
		
		cppdialect "C++11"
		
	filter {"system:Linux", "platforms:x86"}
		buildoptions {
			"-m32"
		}
		
		linkoptions {
			"-m32"
		}

	filter "system:Windows"
		disablewarnings { "4996", "4554", "4244", "4101", "4838", "4309" }

    filter "configurations:Debug"
        defines { 
            "_DEBUG", 
	        "DEBUG"
        }
        symbols "On"

    filter "configurations:Release"
        defines {
            "NDEBUG",
        }
		
	filter "configurations:Release_dev"
        defines {
            "NDEBUG",
        }

-- Psy-Cross layer
include "premake5_psycross.lua"

-- game iteslf
project "CTRPC"
    kind "WindowedApp"

    language "c++"
    targetdir "bin/%{cfg.buildcfg}"
	
	uses { 
		"PsyCross",
	}

    defines { GAME_REGION }
	defines { "BUILD_CONFIGURATION_STRING=\"%{cfg.buildcfg}\"" }
	
	if GAME_VERSION ~= nil then
		local resVersion = string.gsub(GAME_VERSION, "%.", ",")
		defines{ "GAME_VERSION_N=\""..GAME_VERSION.."\"" }
		defines{ "GAME_VERSION_RES="..resVersion.."" }
	end

    files {
        --"decompile/General/**.h",
        --"decompile/General/**.c",
		"../include/**.h", 
    }
	includedirs { 
        "../include", 
    }
	
    filter {"system:Windows or linux or platforms:emscripten"}
        --dependson { "PsyX" }
        --links { "jpeg" }
		defines {
			"REBUILD_PC",
			"BUILD=926"
		}
		files {
			"CrashTeamRacingPC.c",
		}
		
	filter "platforms:emscripten"
	    includedirs { 
			OPENAL_DIR.."/include",
        }
		files { 
            "platform/Emscripten/*.cpp",
        }

    filter "system:Windows"
		entrypoint "mainCRTStartup"
		
		linkoptions { 
			"/FIXED",
			"/DEBUG",
			"/BASE:\"0x10000\""
		}
		
        --files { 
        --    "platform/Windows/resource.h", 
        --    "platform/Windows/Resource.rc", 
        --    "platform/Windows/main.ico" 
        --}

        includedirs { 
            SDL2_DIR.."/include",
            OPENAL_DIR.."/include",
        }
    
        linkoptions {
			"/SAFESEH:NO", -- Image Has Safe Exception Handers: No. Because of openal-soft
        }
        
    filter "system:linux"
        includedirs {
            "/usr/include/SDL2"
        }

        links {
            "GL",
            "openal",
            "SDL2",
            "dl",
        }

    filter "configurations:Debug"
		targetsuffix "_dbg"
        defines { 
            -- add your developer defines here
         }
		 symbols "On"

    filter "configurations:Release"
        optimize "Speed"
		
	filter "configurations:Release_dev"
		targetsuffix "_dev"
        defines { 
			-- add your developer defines here
        }
        optimize "Speed"

    --filter { "files:**.c", "files:**.C" }
    --    compileas "C++"