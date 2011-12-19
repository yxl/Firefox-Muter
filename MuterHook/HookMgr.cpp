#include "ThTypes.h"
#include "HookMgr.h"
#include "HookImportFunction.h"
#include <tlhelp32.h>

HookMgr::HookMgr(void)
{
}

HookMgr::~HookMgr(void)
{
}

void HookMgr::InstallHookForAllModules(LPCSTR szImportModule, LPCSTR szFunc, PROC pHookFunc)
{
	HANDLE hSnapshot;
	MODULEENTRY32 me = {sizeof(MODULEENTRY32)};

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,0);

	BOOL bOk = Module32First(hSnapshot,&me);
	while (bOk) 
	{
		// We don't hook functions in our own module
		if (strcmp(me.szModule, DLL_NAME) != 0)
		{
			InstallHookForOneModule(me.hModule, szImportModule, szFunc, pHookFunc);
		}
		bOk = Module32Next(hSnapshot,&me);
	}
}

void HookMgr::InstallHookForOneModule( HMODULE hModule, LPCSTR szImportModule, LPCSTR szFunc, PROC pHookFunc )
{
	HookItem* pItem = new HookItem(hModule, szImportModule, szFunc, pHookFunc);
	if (!HookImportFunction(hModule, szImportModule, szFunc, pHookFunc, &pItem->pOrigFunc))
	{
		delete pItem;
		return;
	}

	HookMap* pHookMap = NULL;
	ModuleMap::iterator moduleIter = m_modules.find(hModule);
	if (moduleIter == m_modules.end()) 
	{
		pHookMap = new HookMap();
		m_modules.insert(pair<HMODULE, HookMap*>(hModule, pHookMap));
	}
	else
	{
		pHookMap = moduleIter->second;
	}
	pHookMap->insert(pair<PROC, HookItem*>(pItem->pOrigFunc, pItem));
	m_originalFunctions.insert(pair<PROC, PROC>(pItem->pHookFunc, pItem->pOrigFunc));
}

void HookMgr::ClearAllHooks()
{
	//
	// Uninstall the existing modules' hooks
	//

	HANDLE hSnapshot;
	MODULEENTRY32 me = {sizeof(MODULEENTRY32)};

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,0);

	BOOL bOk = Module32First(hSnapshot,&me);
	while (bOk) 
	{
		// We don't hook functions in our own module
		if (strcmp(me.szModule, DLL_NAME) != 0)
		{
			UnInstallAllHooksForOneModule(me.hModule);
		}
		bOk = Module32Next(hSnapshot,&me);
	}

	//
	// Clear invalid modules' hooks
	//

	while (m_modules.size() > 0)
	{
		HookMap* pHookMap = m_modules.begin()->second;
		while (pHookMap->size() > 0)
		{
			HookItem* pItem = pHookMap->begin()->second;
			pHookMap->erase(pHookMap->begin());
			delete pItem;
		}
		m_modules.erase(m_modules.begin());
		delete pHookMap;
	}
}

void HookMgr::UnInstallAllHooksForOneModule( HMODULE hModule )
{
	ModuleMap::iterator moduleIter = m_modules.find(hModule);
	if (moduleIter == m_modules.end())
		return;
	HookMap* pHookMap = moduleIter->second;
	while (pHookMap->size() > 0)
	{
		HookItem* pItem = pHookMap->begin()->second;
		HookImportFunction(pItem->hModule, pItem->szImportModule, pItem->szFunc, pItem->pOrigFunc, NULL);
		pHookMap->erase(pHookMap->begin());
		delete pItem;
	}
	m_modules.erase(moduleIter);
	delete pHookMap;
}

HookItem* HookMgr::FindHook(HMODULE hModule, PROC pOrigFunc)
{
	ModuleMap::iterator moduleIter = m_modules.find(hModule);
	if (moduleIter != m_modules.end())
	{
		HookMap* pHookMap = moduleIter->second;
		HookMap::iterator iter = pHookMap->find(pOrigFunc);
		if (iter != pHookMap->end()) 
		{
			return iter->second;
		}
	}
	return NULL;
}

PROC HookMgr::GetOriginalFunc( PROC pHook )
{
	map<PROC, PROC>::iterator iter = m_originalFunctions.find(pHook);
	if (iter != m_originalFunctions.end())
	{
		return iter->second;
	}
	return NULL;
}
