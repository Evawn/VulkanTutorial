-- premake5.lua
workspace "VulkanTutorial"
   configurations { "Debug", "Release" }
   platforms {"Win64"}
   location "Solution"

project "VulkanTutorial"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20" 
   location "Solution"

   files { "src/*", "dep/VWrap/src/*.cpp", 
                    "dep/VWrap/include/*.h" }

   includedirs {"dep", 
    "dep/VWrap/include",
    "dep/glm",
    "dep/glfw-3.3.8.bin.WIN64/include",
    "dep/Vulkan/Include"
    }

    -- TODO: change to not just support vc2022

    local glfw = os.getenv("GLFW")
    local vulkanSDK = os.getenv("VULKAN_SDK")
    libdirs { vulkanSDK .. "/Lib", glfw .. "/Lib" }

    links { "vulkan-1", "glfw3"}

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
    
   filter { "platforms:Win64" }
      system "Windows"
      architecture "x86_64"