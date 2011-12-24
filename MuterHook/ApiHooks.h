#pragma  once

void InstallMuterHooks();
void UnInstallMuterHooks();

BOOL InjectIntoProcess(HANDLE hProcess);

void InjectIntoSubProcesses();

BOOL IsInThisModuleProcess();


