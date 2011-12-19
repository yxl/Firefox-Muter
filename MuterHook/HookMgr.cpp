#include "HookMgr.h"

HookMgr::HookMgr(void)
{
}

HookMgr::~HookMgr(void)
{
}

BOOL HookMgr::InstallHook( LPCSTR szImportModule, LPCSTR szFunc, PROC paHookFunc, PROC* paOrigFunc )
{
  return TRUE;
}

BOOL HookMgr::ClearAllHooks()
{
  return TRUE;
}

const HookItem* HookMgr::FindHookByOrignalFunc( PROC paOrigFunc )
{
  return NULL;
}