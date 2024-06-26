// #include <pch.h>
#include "load-library.h"

bool LoadLibraryDLL(HANDLE hProc, const std::string& dllpath)
{
	HMODULE hKernel = GetModuleHandle("kernel32.dll");
	if (hKernel == NULL)
	{
		printf("[DLL Injection] Failed to get kernel32.dll module address.\n");
		return false;
	}

	LPVOID pLoadLibrary = (LPVOID)GetProcAddress(hKernel, "LoadLibraryA");
	if (pLoadLibrary == NULL) {
		printf("[DLL Injection] Failed to get LoadLibraryA address.\n");
		return false;
	}

	LPVOID pDLLPath = VirtualAllocEx(hProc, NULL, strlen(dllpath.c_str()) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (pDLLPath == NULL) {
		printf("[DLL Injection] Failed to allocate memory for DLLPath in target process.\n");
		return false;
	}

	// Write the string name of our DLL in the memory allocated
	BOOL writeResult = WriteProcessMemory(hProc, pDLLPath, dllpath.c_str(), strlen(dllpath.c_str()), NULL);
	if (writeResult == FALSE) {
		printf("[DLL Injection] Failed to write remote process memory.\n");
		return false;
	}

	// Load our DLL by calling loadlibrary in the other process and passing our dll name
	HANDLE hThread = CreateRemoteThread(hProc, NULL, NULL, (LPTHREAD_START_ROUTINE)pLoadLibrary, (LPVOID)pDLLPath, NULL, NULL);
	if (hThread == NULL) {
		printf("[DLL Injection] Failed to create remote thread.\n");
		VirtualFreeEx(hProc, pDLLPath, 0, MEM_RELEASE);
		return false;
	}

	// Waiting for thread end and release unnecessary data.
	if (WaitForSingleObject(hThread, 2000) == WAIT_OBJECT_0)
	{
		// ILog("[DLL Injection] Remote thread ended successfully.\n");
		VirtualFreeEx(hProc, pDLLPath, 0, MEM_RELEASE);
	}

	CloseHandle(hThread);

	printf("[DLL Injection] Successfully LoadLibraryA injection.\n");
	return true;
}