#include "vulkan_app.hpp"

int main(int argv, char** args) 
{
    VulkanApp app;

    try 
    {
        app.run();
    } 
    catch (const std::exception& e) 
    {
        SDL_Log("%s", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}