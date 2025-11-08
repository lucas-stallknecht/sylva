#include "app.h"

int main()
{
    sylva::App app{};
    while (true)
    {
        if (app.update())
        {
            break;
        }
    }
    return 0;
}
