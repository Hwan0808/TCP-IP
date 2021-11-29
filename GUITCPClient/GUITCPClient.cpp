#define _CRT_SECURE_NO_WARNINGS         // �ֽ� VC++ ������ �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>
#include <iostream>
#include <string.h>
#include <process.h>
#include "resource.h"

#define SERVERIP "192.168.0.7"
#define SERVERPORT 9000

SOCKET client_sock = NULL;
HWND hwndName;
HWND hwndSend;
HWND hwndEdit1;
HWND hwndEdit2;
HWND hWnd;
HWND hWndFocus;

char Name[25];
char NameStr[256];
char str[128];
BOOL SEND = FALSE;

HICON hIconS, hIconB; // CHAT ������

HANDLE hThread1, hThread2;
DWORD dwThreadID1, dwThreadID2;

unsigned int __stdcall SendMsg(void* arg); // �޽��� ���� �Լ�
unsigned int __stdcall RecvMsg(void* arg); // �޽��� ���� �Լ�
BOOL CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM IParam); // �޽��� ó�� �Լ�
LPTSTR lpszClass = _T("BasicApi"); // ���� ��ȯ �Լ�

void AddStringToEdit(char* fmt, ...);
void OnClose(HWND hWnd); // ��ȭ���� ���� �Լ�
void OnCommand(HWND hWnd, WPARAM wParam);
void OnDisConnect(HWND hwnd);
void OnSend(HWND hwnd);
void OnConnect(HWND hwnd);
BOOL OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM IParam);

// ���� ��� �Լ�
void err_quit(char *msg);

int Time_Hour() { // �ð� ��� �Լ� (Hour)

    time_t curr_time;
    struct tm* curr_tm;
    curr_time = time(NULL);
    curr_tm = localtime(&curr_time);

    return curr_tm->tm_hour;
}

int Time_Min() { // �ð� ��� �Լ� (Min)

    time_t curr_time;
    struct tm* curr_tm;
    curr_time = time(NULL);
    curr_tm = localtime(&curr_time);

    return curr_tm->tm_min;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLind, int nCmdShow)
{
    hIconS = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE);
    hIconB = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_BIG), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE);

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // ��ȭ���� ����
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc); // ä�� ȭ��

    // ���� ����
    WSACleanup();
    return 0;
}

// Ŭ���̾�Ʈ ä�� ȭ��
BOOL CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    case WM_INITDIALOG:

        OnInitDialog(hWnd, hWnd, lParam);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconS);
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIconB);

    case WM_COMMAND:

        OnCommand(hWnd, wParam); return TRUE;

    case WM_CLOSE:
        
        OnClose(hWnd);

        }
        return FALSE;
}

void OnCommand(HWND hwnd, WPARAM wParam)
{
    switch (LOWORD(wParam))
    {
    case IDC_CONNECT:

        OnConnect(hwnd);
        break;

    case IDC_EXIT:

        OnDisConnect(hwnd);
        break;

    case IDC_SEND:

        OnSend(hwnd);
        break;

    }

}

void OnClose(HWND hWnd)
{
    int retval;

    retval = MessageBox(hWnd, _T("�����Ͻðڽ��ϱ�?"), _T("���� ����"), MB_ICONQUESTION | MB_YESNO);
    if (retval == IDYES) {
        EndDialog(hWnd, 0);
    }
}

void AddStringToEdit(char* fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);

    char cbuf[256];
    vsprintf(cbuf, fmt, arg);

    int nLength = GetWindowTextLength(hwndEdit2);
    SendMessage(hwndEdit2, EM_SETSEL, nLength, nLength);
    SendMessage(hwndEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

    va_end(arg);
}

BOOL OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM IParam)
{
    hwndName = GetDlgItem(hWnd, IDC_ID);
    hwndEdit1 = GetDlgItem(hWnd, IDC_CHATEDIT);
    hwndEdit2 = GetDlgItem(hWnd, IDC_CHATVIEW);

    AddStringToEdit("[TCP Ŭ���̾�Ʈ] ���� ���� ��ư�� ��������.\r\n");
    return TRUE;
}

void OnConnect(HWND hwnd)
{
    int retval;

    // socket()
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == INVALID_SOCKET) err_quit("socket()");

    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
    serveraddr.sin_port = htons(SERVERPORT);

    // connect()
    retval = connect(client_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) {
        err_quit("connect()");
    }
    else {
        SetDlgItemText(hwnd, IDC_CHATVIEW, "");
        MessageBox(hWnd, _T("���� ���� �Ϸ�"), _T("���� ����"), MB_ICONINFORMATION | MB_OK);
        AddStringToEdit("[TCP Ŭ���̾�Ʈ] ä�� ������ ���� �Ǿ����ϴ�.\r\n");
    }

    hThread1 = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)client_sock, 0, (unsigned*)&dwThreadID1);
    hThread2 = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)client_sock, 0, (unsigned*)&dwThreadID2);

}

void OnDisConnect(HWND hwnd)
{
    SetDlgItemText(hwnd, IDC_CHATVIEW, "");
    closesocket(client_sock);
    MessageBox(hWnd, _T("���� ���� ����"), _T("���� ����"), MB_ICONINFORMATION | MB_OK);
    AddStringToEdit("[TCP Ŭ���̾�Ʈ] ä�� ������ ������ ���������ϴ�.\r\n");
}

void OnSend(HWND hwnd)
{
    GetDlgItemText(hwnd, IDC_ID, Name, 25);
    GetDlgItemText(hwnd, IDC_CHATEDIT, str, sizeof(str));

    SetDlgItemText(hwnd, IDC_CHATEDIT, "");
    SetFocus(GetDlgItem(hwnd, IDC_CHATEDIT));

    SEND = TRUE;
}

unsigned int __stdcall SendMsg(void* arg)
{
    while (true) 
    {
        if (SEND) {
            sprintf(NameStr, "[%d:%d][%s]:%s \r\n",Time_Hour(), Time_Min(), Name, str);
            send(client_sock, NameStr, (int)strlen(NameStr), 0);

            SEND = FALSE;
        }
    }
    return 0;
}

unsigned int __stdcall RecvMsg(void* arg)
{
    while (true)
    {
        int retval;
        retval = recv(client_sock, NameStr, sizeof(NameStr) - 1, 0);
        if (retval == -1) return 1;
        NameStr[retval] = 0;

        if (retval > 0) {
            AddStringToEdit(NameStr);

        }
        else {
            MessageBox(hWnd, _T("������ ���� �Ǿ� ���� �ʽ��ϴ�."), _T("���� ����"), MB_ICONERROR | MB_OK);
            AddStringToEdit("[TCP Ŭ���̾�Ʈ] ������ ���� �Ǿ� ���� �ʽ��ϴ�.\r\n");
        }
     
    }
    return 0;
}

// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
{
    MessageBox(NULL, "ä�� ���� ���ῡ �����Ͽ����ϴ�. SERVER DISCONNET" , "Ŭ���̾�Ʈ ����", MB_ICONERROR);
    exit(1);
}