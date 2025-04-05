#include "Engine.hpp"

// std
#include <cstdlib>
#include <iostream>

int main()
{
    Minimal::Engine engine{};

    try
    {
        engine.runECS();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
