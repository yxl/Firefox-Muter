#pragma once

#ifndef TRACE

#include <stdio.h>
#include <tchar.h>
#include <stdarg.h>
#include <windows.h>

void TRACE(const CHAR* format,...);

void TRACE_WIDE(const WCHAR* format,...);

#endif        //#ifndef TRACE