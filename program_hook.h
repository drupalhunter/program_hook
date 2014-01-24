#ifndef _PROGRAM_HOOK_H_
#define _PROGRAM_HOOK_H_

#include <windows.h>

class program_hook
{
public:
	program_hook();
	~program_hook();

	// 初始化并指定要监控的程序
	bool init(const TCHAR * szName);
	// 判断要监视的程序是否启动
	bool is_valid();

	// 对被监控的程序进行内存操作，a必须是DWORD数组
	// 基地址起始值0x008d0000
	template <typename VALUE,size_t N>
	bool get_value(VALUE & v, DWORD (&a)[N]);
	template <typename VALUE,size_t N>
	bool set_value(VALUE & v, DWORD (&a)[N]);

private:

	bool monitor();				// 对程序进行监控
	bool create_monitor();		// 创建监控线程
	bool adjust_privileges();	// 提高访问权限

	friend DWORD WINAPI monitor_thread(LPVOID lParam);

private:
	bool	m_quit;				// 退出标记位

	TCHAR	m_szName[MAX_PATH];	// 程序名
	HANDLE	m_monitor_thread;	// 监控线程句柄
	HWND	m_program_hwnd;		// 程序窗口句柄
	DWORD   m_program_id;		// 程序进程ID
	HANDLE  m_program_handle;	// 程序句柄
};

template <typename VALUE, size_t N>
bool program_hook::get_value(VALUE & v, DWORD (&a)[N])
{
	if (!is_valid()) return false;

	DWORD ReadValue = 0;
	DWORD ReadNumber = 0;
	BOOL  bReturn = true;

	int count = 0;
	LPVOID lpBaseAddress = NULL;
	for (; count < N - 1 && bReturn; count++)
	{
		lpBaseAddress = LPVOID(ReadValue + a[count]);
		bReturn = ::ReadProcessMemory(m_program_handle, lpBaseAddress,
			LPVOID(&ReadValue), sizeof(DWORD), &ReadNumber);
	}

	if (bReturn)
	{
		lpBaseAddress = LPVOID(ReadValue + a[count]);
		::ReadProcessMemory(m_program_handle, lpBaseAddress, LPVOID(&v), sizeof(v), &ReadNumber);
	}

	return !!bReturn;

}

template <typename VALUE, size_t N>
bool program_hook::set_value(VALUE & v, DWORD (&a)[N])
{
	if (!is_valid()) return false;

	DWORD ReadValue = 0;
	DWORD ReadNumber = 0;
	BOOL  bReturn = true;

	int count = 0;
	LPVOID lpBaseAddress = NULL;
	for (; count < N - 1 && bReturn; count++)
	{
		lpBaseAddress = LPVOID(ReadValue + a[count]);
		bReturn = ::ReadProcessMemory(m_program_handle, lpBaseAddress,
			LPVOID(&ReadValue), sizeof(DWORD), &ReadNumber);
	}

	if (bReturn)
	{
		lpBaseAddress = LPVOID(ReadValue + a[count]);
		::WriteProcessMemory(m_program_handle, lpBaseAddress, LPVOID(&v), sizeof(v), &ReadNumber);
	}

	return !!bReturn;
}

#endif