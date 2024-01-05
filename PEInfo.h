#pragma once
#include <Windows.h>

struct PEInfo
{
	PEInfo(HMODULE hModule = 0);

	void Reset(HMODULE hModule);
	ULONG_PTR Offset(ULONG_PTR off);
	BOOL IsCode(LPVOID ptr);
	BOOL IsData(LPVOID ptr);

	HANDLE Handle;
	IMAGE_DOS_HEADER* DOS;
	IMAGE_NT_HEADERS* NT;
	IMAGE_FILE_HEADER* FH;
	IMAGE_OPTIONAL_HEADER* OPT;
};
