#pragma  once

#include <Windows.h>

// CreateProcess

extern BOOL (WINAPI *CreateProcessA_original)(LPCSTR lpApplicationName, 
										LPSTR lpCommandLine, 
										LPSECURITY_ATTRIBUTES lpProcessAttributes, 
										LPSECURITY_ATTRIBUTES lpThreadAttributes, 
										BOOL bInheritHandles, 
										DWORD dwCreationFlags,
										LPVOID lpEnvironment, 
										LPCSTR lpCurrentDirectory,
										LPSTARTUPINFOA lpStartupInfo,
										LPPROCESS_INFORMATION lpProcessInformation);

BOOL WINAPI CreateProcessA_hook(LPCSTR lpApplicationName, 
								LPSTR lpCommandLine, 
								LPSECURITY_ATTRIBUTES lpProcessAttributes, 
								LPSECURITY_ATTRIBUTES lpThreadAttributes, 
								BOOL bInheritHandles, 
								DWORD dwCreationFlags,
								LPVOID lpEnvironment, 
								LPCSTR lpCurrentDirectory,
								LPSTARTUPINFOA lpStartupInfo,
								LPPROCESS_INFORMATION lpProcessInformation);

extern BOOL (WINAPI *CreateProcessW_original)(LPCWSTR lpApplicationName, 
									   LPWSTR lpCommandLine, 
									   LPSECURITY_ATTRIBUTES lpProcessAttributes, 
									   LPSECURITY_ATTRIBUTES lpThreadAttributes, 
									   BOOL bInheritHandles, 
									   DWORD dwCreationFlags,
									   LPVOID lpEnvironment, 
									   LPCWSTR lpCurrentDirectory,
									   LPSTARTUPINFOW lpStartupInfo,
									   LPPROCESS_INFORMATION lpProcessInformation);

BOOL WINAPI CreateProcessW_hook(LPCWSTR lpApplicationName, 
								LPWSTR lpCommandLine, 
								LPSECURITY_ATTRIBUTES lpProcessAttributes, 
								LPSECURITY_ATTRIBUTES lpThreadAttributes, 
								BOOL bInheritHandles, 
								DWORD dwCreationFlags,
								LPVOID lpEnvironment, 
								LPCWSTR lpCurrentDirectory,
								LPSTARTUPINFOW lpStartupInfo,
								LPPROCESS_INFORMATION lpProcessInformation);

