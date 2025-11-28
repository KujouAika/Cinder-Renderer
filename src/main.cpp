#include "Application.h"
#include <iostream>

int main(int argc, char* argv[])
{
    try {
        Application app;
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Critical Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}