#include <iostream>
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <process.h>

using namespace std;
typedef enum _THREADINFOCLASS {
    ThreadBasicInformation,
    ThreadTimes,
    ThreadPriority,
    ThreadBasePriority,
    ThreadAffinityMask,
    ThreadImpersonationToken,
    ThreadDescriptorTableEntry,
    ThreadEnableAlignmentFaultFixup,
    ThreadEventPair_Reusable,
    ThreadQuerySetWin32StartAddress,
    ThreadZeroTlsCell,
    ThreadPerformanceCount,
    ThreadAmILastThread,
    ThreadIdealProcessor,
    ThreadPriorityBoost,
    ThreadSetTlsArrayAddress,
    ThreadIsIoPending,
    ThreadHideFromDebugger,
    ThreadBreakOnTermination,
    MaxThreadInfoClass
}THREADINFOCLASS;

extern "C" LONG(__stdcall * ZwQueryInformationThread)(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    OUT PVOID ThreadInformation,
    IN ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    ) = NULL;

unsigned  __stdcall Programs_Inject Program(void* pArguments)
{

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        THREADENTRY32 te;
        te.dwSize = sizeof(THREADENTRY32);
        if (Thread32First(hSnap, &te))
        {
            do
            {
                if (te.th32OwnerProcessID == GetCurrentProcessId())
                {
                    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, false, te.th32ThreadID);
                    if (hThread == NULL) { printf("打开线程句柄失败\n"); }
                    HINSTANCE hNTDLL = ::GetModuleHandle("ntdll");
                    (FARPROC&)ZwQueryInformationThread = ::GetProcAddress(hNTDLL, "ZwQueryInformationThread");
                    PVOID startaddr;
                    ZwQueryInformationThread(
                        hThread,						
                        ThreadQuerySetWin32StartAddress,
                        &startaddr,						
                        sizeof(startaddr),				
                        NULL
                    );
                    if ((long long)startaddr == 0x7FFDBC196000 || (long long)startaddr == 0x1801E2620 || 
                        (long long)startaddr == 0X1801e45C4 || (long long)startaddr == 0X1801FA050
                        || (long long)startaddr == 0X180202A00 || (long long)startaddr == 0X180202A10 )
                    {
                        printf("startaddr %010p \n", startaddr);
                        TerminateThread(hThread,1);
                    }
                  


                    

                   // TerminateThread(hThread, 1);
                    CloseHandle(hThread);
                }
            } while (Thread32Next(hSnap, &te));
        }
    }

    CloseHandle(hSnap);


	return 0;
}

bool __stdcall DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		AllocConsole();
		FILE* Stream;
		freopen_s(&Stream, "CON", "w", stdout);
		_beginthreadex(NULL, 0,Programs_Inject Program,NULL,0,NULL);

	}
	return true;
}