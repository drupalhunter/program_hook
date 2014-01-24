// 如果编译错误提示你此文件没有包含预编译文件的话，你需要取消下面这行的注释
//#include "stdafx.h"
#include "program_hook.h"
#include <tchar.h>

DWORD WINAPI monitor_thread(LPVOID lParam)
{
	program_hook * phook = (program_hook *)lParam;
	phook->monitor();

	// close
	::CloseHandle(phook->m_monitor_thread);

	return 0;
}

program_hook::program_hook()
{
	m_quit = false;
	m_monitor_thread = NULL;
	m_program_hwnd = NULL;
	m_program_id = 0;
	m_program_handle = NULL;
}

program_hook::~program_hook()
{
	m_quit = true;
}

bool program_hook::init(const TCHAR * szName)
{
	if (szName == NULL) return false;

	_tcscpy_s(m_szName,MAX_PATH, szName); // 程序名

	return create_monitor();
}

bool program_hook::is_valid()
{
	if (m_program_hwnd != NULL && m_program_handle != NULL)
		return true;
	return false;
}

// 对指定的程序进行监控
bool program_hook::monitor()
{
	while ( !m_quit )
	{
		HWND hWndTemp = ::FindWindow(NULL, m_szName);
		if ( hWndTemp != m_program_hwnd ) // open
		{
			m_program_hwnd = hWndTemp;
			if ( m_program_hwnd != NULL && adjust_privileges() )
			{
				::GetWindowThreadProcessId(m_program_hwnd, &m_program_id);
				m_program_handle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE,
					m_program_id);
			}
		}

		Sleep(100);
	}

	return true;
}

// 创建线程对程序进行监控
bool program_hook::create_monitor()
{
	DWORD dwThreadId = 0;
	m_monitor_thread = ::CreateThread(NULL, 0, monitor_thread, 
		this, 0, &dwThreadId);

	return m_monitor_thread != NULL ? true : false;
}

bool program_hook::adjust_privileges()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	TOKEN_PRIVILEGES oldtp;
	DWORD dwSize = sizeof(TOKEN_PRIVILEGES);
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED) return true;
		else return false;
	}
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
		CloseHandle(hToken);
		return false;
	}
	ZeroMemory(&tp, sizeof(tp));
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	// Adjust Token Privileges
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &oldtp, &dwSize)) {
		CloseHandle(hToken);
		return false;
	}
	// close handles
	CloseHandle(hToken);

	return true;
}