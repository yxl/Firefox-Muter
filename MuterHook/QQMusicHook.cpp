#include <Windows.h>

// GetModuleFileNameEx
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")

#include <tchar.h>

#include "QQMusicHook.h"
#include "ApiHooks.h"

void InjectIntoQQMusic()
{
	// Try to find the QQ music window.
	HWND hWnd = FindWindow(TEXT("QzonePlayerMang"), TEXT("QzonePlayerMang"));
	if (hWnd == NULL)
	{
		return;
	}

	// Open the QQ music process
	DWORD dwProcId = 0;
	GetWindowThreadProcessId(hWnd, &dwProcId);
	if (dwProcId == 0)
	{
		return;
	}
	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | 
		PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, dwProcId);
	if (hProcess == NULL)
	{
		return;
	}

	// Check the process executable file name
	TCHAR szPathBuffer[MAX_PATH];
	GetModuleFileNameEx(hProcess, NULL, szPathBuffer, MAX_PATH);
	if (_tcsstr(szPathBuffer, TEXT("QzoneMusic.exe")) == NULL)
	{
		CloseHandle(hProcess);
		return;
	}

	// Inject into QQ music process
	InjectIntoProcess(hProcess);
	
	CloseHandle(hProcess);
}