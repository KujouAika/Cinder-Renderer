#include "Application.h"
#include "Core/Macro.h"

int main(int argc, char* argv[])
{
    try {
        FApplication App;
        try {
            App.Init();
        }
        catch (const std::exception& e) {
            std::cerr << "Initialization Error: " << e.what() << std::endl;
            return -1;
        }
        App.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Critical Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}