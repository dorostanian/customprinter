// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the customprinter_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// customprinter_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef customprinter_EXPORTS
#define customprinter_API __declspec(dllexport)
#else
#define customprinter_API __declspec(dllimport)
#endif

// This class is exported from the dll
class customprinter_API Ccustomprinter {
public:
	Ccustomprinter(void);
	// TODO: add your methods here.
};

extern customprinter_API int ncustomprinter;

customprinter_API int fncustomprinter(void);
