// customprinter.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "customprinter.h"


// This is an example of an exported variable
customprinter_API int ncustomprinter=0;

// This is an example of an exported function.
customprinter_API int fncustomprinter(void)
{
    return 0;
}

// This is the constructor of a class that has been exported.
Ccustomprinter::Ccustomprinter()
{
    return;
}
