#pragma once

#include <Windows.h>
#include <map>
#include <algorithm>

using namespace std;

struct HookItem 
{
  // DLL file name
  LPSTR szImportModule;
  // Function to be hooked
  LPSTR szFunc;
  // Hook function
  PROC paHookFunc;
  // Original function
  PROC paOrigFunc;

public:
  HookItem()
  {
    szImportModule = NULL;
    szFunc = NULL;
    paHookFunc = NULL;
    paOrigFunc = NULL;
  }
};

class HookMgr
{
public:
  HookMgr(void);
  ~HookMgr(void);

  BOOL InstallHook(LPCSTR szImportModule, LPCSTR szFunc, PROC paHookFunc, PROC* paOrigFunc);

  BOOL ClearAllHooks(); 

  const HookItem* FindHookByOrignalFunc(PROC paOrigFunc);

private: 
  map<PROC, const HookItem*> m_items;
};
