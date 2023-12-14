//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
//
////#define STB_IMAGE_IMPLEMENTATION
////#include <stb_image.h>
//
//#include <iostream>
//#include <stdexcept>
//#include <cstdlib>
//#include <vector>
//#include <unordered_set>
//#include <optional>
//#include <set>
//#include <algorithm>
//#include <fstream>
//#include <array>
//
//#define GLM_FORCE_RADIANS
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//
//#include <chrono>

#include "Application.h"



// Q: why am I getting a bunch of "blank" already defined in Application.obj errors?
// A: the reason for this is that the header file is included in both main.cpp and Application.cpp
// fix this by using the preprocessor directive #pragma once in the header file
// Q: but I am already using #pragma once in the header file, why am I still getting the error?
// A: the reason for this is that the header file is included in both main.cpp and Application.cpp



/// <summary>
/// Entry point of our application. Creates the app, and runs it while catching any exceptions.
/// </summary>
/// <returns> EXIT_FAILURE if an exception is thrown, otherwise EXIT_SUCCESS. </returns>
int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}