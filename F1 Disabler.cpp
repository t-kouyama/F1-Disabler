
#define WINVER       0x0600
#define _WIN32_WINNT 0x0600

#include <windows.h>


WCHAR ProcessNames[32768];


LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wp, LPARAM lp);


int WinMainCRTStartup(void)
{
	CreateMutex(NULL, FALSE, TEXT("F1 Disabler"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return 0;
	}


	GetPrivateProfileSection(TEXT("ProcessNames"), ProcessNames, 32768, TEXT(".\\F1 Disabler.ini"));


	SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, NULL, 0);


	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wp, LPARAM lp)
{
	if (nCode == HC_ACTION)
	{
		KBDLLHOOKSTRUCT* kbs = (KBDLLHOOKSTRUCT*)lp;
		if (!(kbs->flags & LLKHF_INJECTED))
		{
			if (kbs->vkCode == VK_F1 && wp == WM_KEYDOWN)
			{
				DWORD dwProcessId;
				GetWindowThreadProcessId(GetForegroundWindow(), &dwProcessId);
				
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
				if (hProcess != NULL)
				{
					WCHAR ProcessFullPath[MAX_PATH + 1];
					DWORD dwSize = MAX_PATH;
					QueryFullProcessImageName(hProcess, 0, ProcessFullPath, &dwSize);
					CloseHandle(hProcess);

					WCHAR *ProcessName = ProcessFullPath;
					for (; dwSize; dwSize--)
					{
						if (ProcessFullPath[dwSize] == '\\')
						{
							ProcessName = ProcessFullPath + dwSize + 1;
							break;
						}
						
					}
					
					for (WCHAR *CompareProcessName = ProcessNames; *CompareProcessName != '\0'; CompareProcessName += lstrlen(CompareProcessName) + 1) {
						if (lstrcmpi(ProcessName, CompareProcessName) == 0) return -1;
					}
				}
			}
		}
	}

	return CallNextHookEx(NULL, nCode, wp, lp);
}
