#include "MyBrowser.h"
#include <Windows.h>

int main()
{
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    MyBrowser myBrowser;
    myBrowser.start();
    return 0;
}