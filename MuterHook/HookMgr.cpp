#include "HookMgr.h"
#include "HookImportFunction.h"

HookMgr::HookMgr(void)
{
}

HookMgr::~HookMgr(void)
{
}

BOOL HookMgr::InstallHook( LPCSTR szImportModule, LPCSTR szFunc, PROC paHookFunc, PROC* paOrigFunc )
{
  HookItem* pItem = new HookItem(szImportModule, szFunc, paHookFunc);
  if (!HookAPI(pItem->szImportModule, pItem->szFunc, pItem->paHookFunc, &pItem->paOrigFunc))
  {
    delete pItem;
    return FALSE;
  }
  *paOrigFunc = pItem->paOrigFunc;
  m_items.insert(pair<PROC, HookItem*>(pItem->paOrigFunc, pItem));
  return TRUE;
}

BOOL HookMgr::ClearAllHooks()
{
  while(m_items.size() > 0)
  {
    HookMap::iterator iter = m_items.begin();
    HookItem* pItem = iter->second;
    if (!HookAPI(pItem->szImportModule, pItem->szFunc, pItem->paOrigFunc, NULL))
    {
      return FALSE;
    }
    delete pItem;
  }
  return TRUE;
}

HookItem* HookMgr::FindHookByOrignalFunc( PROC paOrigFunc )
{
  HookMap::iterator iter = m_items.find(paOrigFunc);
  if (iter != m_items.end()) 
  {
    return iter->second;
  }
  return NULL;
}