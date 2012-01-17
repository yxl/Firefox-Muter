#pragma  once

void InstallMuterHooks();
void UnInstallMuterHooks();

BOOL InjectIntoProcess(HANDLE hProcess);

BOOL InjectIntoProcessByForce(HANDLE hProcess);

void InjectIntoSubProcesses();

BOOL IsInThisModuleProcess();


