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
  PROC paHookFunc;
  // Original function
  PROC paOrigFunc;

  HookItem()
  {
    szImportModule = NULL;
    szFunc = NULL;
    paHookFunc = NULL;
    paOrigFunc = NULL;
  }

  HookItem(LPCSTR szModule, LPCSTR szFunc, PROC paHookFunc, PROC paOrigFunc = NULL)
  {
    this->szImportModule = newAndCopyString(szModule);
    this->szFunc = newAndCopyString(szFunc);
    this->paHookFunc = paHookFunc;
    this->paOrigFunc = paOrigFunc;
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

  BOOL ClearAllHooks();

  int GetHookCount() const { m_items.size(); }

  HookItem* FindHookByOrignalFunc(PROC paOrigFunc);

private:   
  HookMap m_items;
};
