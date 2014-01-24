#ifndef _PROGRAM_HOOK_H_
#define _PROGRAM_HOOK_H_

#include <windows.h>

class program_hook
{
public:
	program_hook();
	~program_hook();

	// ��ʼ����ָ��Ҫ��صĳ���
	bool init(const TCHAR * szName);
	// �ж�Ҫ���ӵĳ����Ƿ�����
	bool is_valid();

	// �Ա���صĳ�������ڴ������a������DWORD����
	// ����ַ��ʼֵ0x008d0000
	template <typename VALUE,size_t N>
	bool get_value(VALUE & v, DWORD (&a)[N]);
	template <typename VALUE,size_t N>
	bool set_value(VALUE & v, DWORD (&a)[N]);

private:

	bool monitor();				// �Գ�����м��
	bool create_monitor();		// ��������߳�
	bool adjust_privileges();	// ��߷���Ȩ��

	friend DWORD WINAPI monitor_thread(LPVOID lParam);

private:
	bool	m_quit;				// �˳����λ

	TCHAR	m_szName[MAX_PATH];	// ������
	HANDLE	m_monitor_thread;	// ����߳̾��
	HWND	m_program_hwnd;		// ���򴰿ھ��
	DWORD   m_program_id;		// �������ID
	HANDLE  m_program_handle;	// ������
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