#include "Application.h"

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