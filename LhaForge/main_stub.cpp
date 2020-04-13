#include <windows.h>

int LF_main(HINSTANCE hInstance, HINSTANCE hPrev, LPWSTR lpCmdLine, int nCmdShow);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPWSTR lpCmdLine, int nCmdShow)
{
	return LF_main(hInstance, hPrev, lpCmdLine, nCmdShow);
}

