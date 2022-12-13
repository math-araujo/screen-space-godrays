#include <exception>
#include <iostream>

#include "main_application.hpp"

int main()
{
    try
    {
        MainApplication application{1024, 768, "Screen Space Godrays"};
        application.run();
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
    }

    return 0;
}