#pragma  once

#include <Windows.h>

typedef FARPROC (WINAPI *GetProcAddress_t)(HMODULE hmod, PCSTR pszProcName);
FARPROC WINAPI GetProcAddress_hook(HMODULE hmod, PCSTR pszProcName);

// LoadLibray and LoadLibrayEx

typedef HMODULE (WINAPI *LoadLibraryA_t)(PCSTR pszModulePath);
HMODULE WINAPI LoadLibraryA_hook(PCSTR pszModulePath);

typedef HMODULE (WINAPI *LoadLibraryW_t)(PCWSTR pszModulePath);
HMODULE WINAPI LoadLibraryW_hook(PCWSTR pszModulePath);

typedef HMODULE (WINAPI *LoadLibraryExA_t)(PCSTR pszModulePath, HANDLE hFile, DWORD dwFlags);
HMODULE WINAPI LoadLibraryExA_hook(PCSTR pszModulePath, HANDLE hFile, DWORD dwFlags) ;

typedef HMODULE (WINAPI *LoadLibraryExW_t)(PCWSTR pszModulePath, HANDLE hFile, DWORD dwFlags);
HMODULE WINAPI LoadLibraryExW_hook(PCWSTR pszModulePath, HANDLE hFile, DWORD dwFlags);

// CreateProcess

typedef BOOL (WINAPI *CreateProcessA_t)(LPCSTR lpApplicationName, 
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

typedef BOOL (WINAPI *CreateProcessW_t)(LPCWSTR lpApplicationName, 
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

