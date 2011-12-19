#pragma once

#include <Windows.h>
#include <map>
#include <algorithm>

using namespace std;

class HookItem 
{
public:

  // DLL file name
  LPSTR szImportModule;
  // Function to be hooked
  LPSTR szFunc;
  // Hook function
  PROC pHookFunc;
  // Original function
  PROC pOrigFunc;

  HookItem()
  {
    szImportModule = NULL;
    szFunc = NULL;
    pHookFunc = NULL;
    pOrigFunc = NULL;

  }

  HookItem(LPCSTR szModule, LPCSTR szFunc, PROC pHookFunc, PROC pOrigFunc = NULL)
  {
    this->szImportModule = newAndCopyString(szModule);
    this->szFunc = newAndCopyString(szFunc);
    this->pHookFunc = pHookFunc;
    this->pOrigFunc = pOrigFunc;
  }

  ~HookItem()
  {
    SafeDeleteString(szImportModule);
    SafeDeleteString(szFunc);
  }

private:
  static void SafeDeleteString(char* str)
  {
    if (str)
    {
      delete[] str;
    }
  }

  static char* newAndCopyString(LPCTSTR str) 
  {
    char* pResult = NULL;
    if (str != NULL)
    {
      int len = strlen(str);
      pResult = new char[len + 1];
      strcpy_s(pResult, len+1, str);
    }
    return pResult;
  }
};

typedef map<PROC, HookItem*> HookMap;

class HookMgr
{
public:
  HookMgr(void);
  ~HookMgr(void);

  BOOL InstallHook(LPCSTR szImportModule, LPCSTR szFunc, PROC paHookFunc, PROC* paOrigFunc);

  BOOL UpdateAllHooksForNewModule(HMODULE hModule, DWORD dwFlags);

  BOOL ClearAllHooks();

  int GetHookCount() const { m_items.size(); }

  HookItem* FindHookByOrignalFunc(PROC paOrigFunc);

private:   
  HookMap m_items;
};
