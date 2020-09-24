#include <Windows.h>
#include <tlhelp32.h>


void suspend(DWORD processId)
{
    HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    THREADENTRY32 threadEntry; 
    threadEntry.dwSize = sizeof(THREADENTRY32);

    Thread32First(hThreadSnapshot, &threadEntry);

    do
    {
        if (threadEntry.th32OwnerProcessID == processId)
        {
            HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE,
                threadEntry.th32ThreadID);

            SuspendThread(hThread);
            CloseHandle(hThread);
        }
    } while (Thread32Next(hThreadSnapshot, &threadEntry));

    CloseHandle(hThreadSnapshot);
}


void resume(DWORD processId)
{
    HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    THREADENTRY32 threadEntry; 
    threadEntry.dwSize = sizeof(THREADENTRY32);

    Thread32First(hThreadSnapshot, &threadEntry);

    do
    {
        if (threadEntry.th32OwnerProcessID == processId)
        {
            HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE,
                threadEntry.th32ThreadID);

            ResumeThread(hThread);
            CloseHandle(hThread);
        }
    } while (Thread32Next(hThreadSnapshot, &threadEntry));

    CloseHandle(hThreadSnapshot);
}

void EnableDebugPriv()
{
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tkp;

    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL);

    CloseHandle(hToken); 
}

void balls()
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (stricmp(entry.szExeFile, "GTA5.exe") == 0)
            {  
                HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

                DWORD uFuckU = GetProcessId(hProcess);

				suspend(uFuckU);
				Sleep(12000);
				resume(uFuckU);


                CloseHandle(hProcess);
				break;
            }
        }
    }

    CloseHandle(snapshot);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	EnableDebugPriv();



	balls();

	return 0;
}