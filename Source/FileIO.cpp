#pragma once
#include <Windows.h>
#include <assert.h>

static HANDLE CurrentHandle = NULL;

static void
BeginFileWrite(const char* FilePath)
{
    CurrentHandle = CreateFileA(FilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    assert(CurrentHandle != INVALID_HANDLE_VALUE);
}

static void
AddToFile(const void* Ptr, int Size)
{
    assert(CurrentHandle != NULL);
    if(Size <= 0 || Ptr == NULL) return;
    unsigned long BytesWritten  = 0;
    WriteFile(CurrentHandle, Ptr, Size, &BytesWritten, NULL);
}

static void
EndFileWrite()
{
    assert(CurrentHandle != NULL);
    CloseHandle(CurrentHandle);
    CurrentHandle = NULL;
}