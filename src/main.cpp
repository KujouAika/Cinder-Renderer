#include "Application.h"
#include "Core/Macro.h"

int main(int argc, char* argv[])
{
    try {
        FApplication App;
        App.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Critical Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}