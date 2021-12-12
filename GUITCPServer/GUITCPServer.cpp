#define _CRT_SECURE_NO_WARNINGS         // �ֽ� VC++ ������ �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>
#include <windows.h> 
#include <tchar.h>
#include <time.h>
#include <string.h>
#include <iostream>
#include <io.h>
#include <fcntl.h> 
#include "resource.h"

#define BUFSIZE 1024
#define SERVERPORT 9000

struct SOCKETINFO
{
    SOCKET sock = NULL;
    HANDLE Thread;
    DWORD ThreadID;
    char Name[25] = { "�͸��� �����" };
    int addrlen = 0;
};

SOCKETINFO SocketInfoArray[64];
SOCKET listen_sock = NULL;
SOCKET client_sock = NULL;

HWND hwndEdit; // ����Ʈ �ڽ� ���� ��Ʈ�� (ä�� ȭ��)
HWND hwndList; // ����Ʈ �ڽ� ���� ��Ʈ�� (Ŭ���̾�Ʈ ����Ʈ �׸�)
HICON hIconS, hIconB; // ������

SOCKADDR_IN serveraddr;
SOCKADDR_IN clientaddr;
HANDLE Mutex;
HANDLE Thread2;
DWORD ThreadID2;

int addrlen;
int clntNum = 0;
char buf[BUFSIZE];
char Name[25];
BOOL ON_OFF = FALSE;

// �޽��� ó�� �Լ� (���̾�α�)
BOOL CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// �޽��� ���� �ؽ�Ʈ �Լ�
LPTSTR lpszClass = _T("BasicApi");

// �޽��� ��� �Լ�
void DisplayText(char* fmt, ...);

// ���̾�α� �޽��� ó�� �Լ� 
void OnClose(HWND hWnd);
void OnCommand(HWND hwnd, WPARAM wParam);
void OnDisConnect(HWND hwnd);
void OnDisConnectUser(HWND hwnd);
void OnAbort(HWND hwnd);

// ���̾�α� �ʱ�ȭ
BOOL OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM IParam);

// ������(main)
unsigned int __stdcall ThreadMain(void* arg);

// �޽��� ���� (send)
void SendMsg(char* str, int len);

// Ŭ���̾�Ʈ ������ ���
DWORD WINAPI ProcessClient(void* arg);

// ���� ��� �Լ�
void err_quit(char* msg);

// �ð� ��� �Լ�
int Time_Year();
int Time_Month();
int Time_Day();
int Time_Hour();
int Time_Min();

void voidBuffer(SOCKET s);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    hIconS = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE);
    hIconB = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_BIG), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE);

    // ��ȭ����(���̾�α�) ���� 
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

    WSACleanup();
    return 0;
}

unsigned int __stdcall ThreadMain(void* arg)
{
    int strlen = 0;

    int retval;
    char msg[256];

    Mutex = CreateMutex(NULL, FALSE, NULL);
    if (Mutex == NULL) {
        err_quit("createmutex()");
    }

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);

    // bind()
    bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (listen_sock == SOCKET_ERROR) err_quit("bind()");

    // listen()
    listen(listen_sock, SOMAXCONN);
    if (listen_sock == SOCKET_ERROR) err_quit("listen()");

    DisplayText("[CHATBOT] ����� ���� ����� �Դϴ�.\r\n");

    while (true)
    {
        // accept()
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
    
        WaitForSingleObject(Mutex, INFINITE);
        SocketInfoArray[clntNum].sock = client_sock;
        SocketInfoArray[clntNum].addrlen = addrlen;
        retval = recv(SocketInfoArray[clntNum].sock, Name, sizeof(Name) - 1, 0);
        Name[retval] = 0;
        strcpy_s(SocketInfoArray[clntNum].Name, Name);
        ReleaseMutex(Mutex);

        // ������ Ŭ���̾�Ʈ ���
        for (int i = 0; i < clntNum + 1; i++) {

            if (client_sock == SocketInfoArray[i].sock) {
                
                strlen = sprintf(msg, "[CHATBOT][%d-%02d-%02d][%02d:%02d] %s���� �����߽��ϴ�. \r\n", Time_Year(), Time_Month(), Time_Day(), Time_Hour(), Time_Min(), SocketInfoArray[i].Name);
                DisplayText(msg);

                SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)inet_ntoa(clientaddr.sin_addr));
                SendMsg(msg, strlen);
            
            }

        }
            // ������ ����
            SocketInfoArray[clntNum].Thread = (HANDLE)_beginthreadex(NULL, 0, (unsigned int(__stdcall*)(void*))ProcessClient, (void*)SocketInfoArray[clntNum].sock, 0, (unsigned*)&SocketInfoArray[clntNum].ThreadID);
            clntNum++;

    }
    closesocket(listen_sock);
    return 0;

}
//  ���� ä�� ȭ�� ��ȭ����
BOOL CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HBRUSH hBrush = CreateSolidBrush(RGB(230, 240, 250));

    switch (uMsg) {

    case WM_INITDIALOG:

        OnInitDialog(hWnd, hWnd, lParam);
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIconB);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconS);
        break;

    case WM_CTLCOLORDLG:

        return (LRESULT)hBrush;

    case WM_CTLCOLORSTATIC:

        SetBkColor((HDC)wParam, RGB(230, 240, 250));
        return (LRESULT)hBrush;

    case WM_CLOSE:

        OnClose(hWnd);
        break;

    case WM_COMMAND:

        OnCommand(hWnd, wParam);
        return TRUE;

    }
    return FALSE;
}

void OnCommand(HWND hwnd, WPARAM wParam)
{
    switch (LOWORD(wParam))
    {
    case IDABORT:

        OnAbort(hwnd);
        break;

    case IDIGNORE:

        OnDisConnectUser(hwnd);
        break;

    case IDCANCEL:

        OnDisConnect(hwnd);
        break;
    }
}

void OnAbort(HWND hwnd)
{
    int strlen = 0;
    char msg[256];

    if (ON_OFF == FALSE) {
        for (int i = 0; i < clntNum; i++) {
            SuspendThread(SocketInfoArray[i].Thread);
        }
        strlen = sprintf(msg,"[CHATBOT] �����ڰ� ä���� �������׽��ϴ�.\r\n");
        SendMsg(msg, strlen);
        ON_OFF = TRUE;
    }
    else if(ON_OFF == TRUE){

        for (int i = 0; i < clntNum; i++) {
            ResumeThread(SocketInfoArray[i].Thread);
        }
        for (int i = 0; i < clntNum; i++) {
            voidBuffer(SocketInfoArray[i].sock);
        }
        strlen = sprintf(msg, "[CHATBOT] �����ڰ� ä���� Ȱ��ȭ���׽��ϴ�.\r\n");
        SendMsg(msg, strlen);
        ON_OFF = FALSE;
    }

}

void OnDisConnect(HWND hwnd)
{
    int retval;

    retval = MessageBox(hwnd, _T("�����Ͻðڽ��ϱ�?"), _T("���� ����"), MB_ICONQUESTION | MB_YESNO);
    if (retval == IDYES) {
        OnClose(hwnd);
    }
}

void OnDisConnectUser(HWND hwnd)
{
    for (int i = 0; i < clntNum; i++)
    {
        closesocket(SocketInfoArray[i].sock);
        SendMessage(hwndList, LB_DELETESTRING, 0, (LPARAM)SocketInfoArray[i].Name);
    }
}

void OnClose(HWND hWnd)
{
    closesocket(listen_sock);
    EndDialog(hWnd, 0);
}

void DisplayText(char* fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);

    char cbuf[256];
    vsprintf(cbuf, fmt, arg);

    int nLength = GetWindowTextLength(hwndEdit);
    SendMessage(hwndEdit, EM_SETSEL, nLength, nLength);
    SendMessage(hwndEdit, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

    va_end(arg);
}

BOOL OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM IParam)
{
    hwndEdit = GetDlgItem(hWnd, IDC_EDIT1);
    hwndList = GetDlgItem(hWnd, IDC_LIST1);
    Thread2 = (HANDLE)_beginthreadex(NULL, 0, (unsigned int(__stdcall*)(void*))ThreadMain,
        (void*)listen_sock, 0, (unsigned*)&ThreadID2);

    return TRUE;
}

// ���� �Լ� ���� ��� �� ����
void err_quit(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ProcessClient(void* arg)
{
    SOCKET sock = (SOCKET)arg;

    int strlen = 0;
    int retval = 0;

    char msg[256];

    while ((strlen = recv(sock, buf, BUFSIZE, 0)) > 0 
        && (retval = recv(sock, Name, sizeof(Name) - 1, 0)) > 0)
    {
        Name[retval] = 0;
        buf[strlen] = 0;
        DisplayText(buf);
        SendMsg(buf, strlen);
    }
    WaitForSingleObject(Mutex, INFINITE);

    for (int i = 0; i < clntNum; i++) {

        if (sock == SocketInfoArray[i].sock) {

            strcpy_s(SocketInfoArray[i].Name, Name);
            strlen = sprintf(msg, "[CHATBOT][%d-%02d-%02d][%02d:%02d] %s���� ������ �����߽��ϴ�. \r\n", Time_Year(), Time_Month(), Time_Day(), Time_Hour(), Time_Min(), SocketInfoArray[i].Name);

            DisplayText(msg);
            SendMsg(msg, strlen);

            for (; i < clntNum - 1; i++)
                SocketInfoArray[i] = SocketInfoArray[i + 1];
            SendMessage(hwndList, LB_DELETESTRING, 0, (LPARAM)inet_ntoa(clientaddr.sin_addr));
            break;
        }
    }
    clntNum--;
    ReleaseMutex(Mutex);
    closesocket(sock);
    return 0;
}

void SendMsg(char* str, int len)
{
    WaitForSingleObject(Mutex, INFINITE);
    for (int i = 0; i < clntNum; i++)
        send(SocketInfoArray[i].sock, str, len, 0);
    ReleaseMutex(Mutex);
}

int Time_Year() {

    time_t timer;
    struct tm* now_time;
    timer = time(NULL);
    now_time = localtime(&timer);

    return now_time->tm_year + 1900;
}

int Time_Month() {

    time_t timer;
    struct tm* now_time;
    timer = time(NULL);
    now_time = localtime(&timer);

    return now_time->tm_mon + 1;
}

int Time_Day() {

    time_t timer;
    struct tm* now_time;
    timer = time(NULL);
    now_time = localtime(&timer);

    return now_time->tm_mday;
}

int Time_Hour() {

    time_t timer;
    struct tm* now_time;
    timer = time(NULL);
    now_time = localtime(&timer);

    return now_time->tm_hour;
}

int Time_Min() {

    time_t timer;
    struct tm* now_time;
    timer = time(NULL);
    now_time = localtime(&timer);

    return now_time->tm_min;
}

void voidBuffer(SOCKET s)
{
    u_long tmpl, i;
    char tmpc;
    ioctlsocket(s, FIONREAD, &tmpl);
    for (i = 0; i < tmpl; i++) {
    recv(s, &tmpc, sizeof(char), 0);
    }
}