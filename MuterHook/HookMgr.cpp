#include "HookMgr.h"
#include "HookImportFunction.h"

HookMgr::HookMgr(void)
{
}

HookMgr::~HookMgr(void)
{
}

BOOL HookMgr::InstallHook( LPCSTR szImportModule, LPCSTR szFunc, PROC pHookFunc, PROC* ppOrigFunc )
{
  
  HookItem* pItem = new HookItem(szImportModule, szFunc, pHookFunc);
  BOOL bOK = HookAPI(pItem->szImportModule, pItem->szFunc, pItem->pHookFunc, &(pItem->pOrigFunc));
  if (bOK)
  {
    *ppOrigFunc = pItem->pOrigFunc;
  }
  m_items.insert(pair<PROC, HookItem*>(pItem->pOrigFunc, pItem));
  return TRUE;
}

BOOL HookMgr::UpdateAllHooksForNewModule(HMODULE hModule, DWORD dwFlags)
{
  if ((hModule != NULL) && ((dwFlags & LOAD_LIBRARY_AS_DATAFILE) == 0)) 
  {
    for (HookMap::iterator iter = m_items.begin(); iter != m_items.end(); iter++)
    {
      char szName[MAX_PATH];
      if (GetModuleFileNameA(hModule, szName,MAX_PATH) == 0)
        continue;

      HookItem* pItem = iter->second;
      if (strcmp(szName, pItem->szImportModule) != 0)
        continue;
      PROC pOrigFunc = NULL;
      HookImportFunction(hModule, pItem->szImportModule, pItem->szFunc, pItem->pHookFunc, &pItem->pOrigFunc);
    }
  }
  return TRUE;
}

BOOL HookMgr::ClearAllHooks()
{
  while(m_items.size() > 0)
  {
    HookMap::iterator iter = m_items.begin();
    HookItem* pItem = iter->second;
    if (pItem->pOrigFunc && !HookAPI(pItem->szImportModule, pItem->szFunc, pItem->pOrigFunc, NULL))
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
