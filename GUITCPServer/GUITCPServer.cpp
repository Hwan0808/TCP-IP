#define _CRT_SECURE_NO_WARNINGS         // �ֽ� VC++ ������ �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <commdlg.h>
#include <time.h>
#include <iostream>
#include <string>
#include "resource.h"

#define SERVERPORT 9000
#define BUFSIZE 4096
#define NAMESIZE 20
#define MAXCLNT 256

// ��ȭ���� ���ν���
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
LPTSTR lpszClass = _T("BasicApi");

// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char *fmt, ...);
// ���� ��� �Լ�
void err_quit(char *msg);
void err_display(char *msg);
// ���� ��� ������ �Լ�
DWORD WINAPI ServerMain(LPVOID arg);
DWORD WINAPI ProcessClient(LPVOID arg);

HINSTANCE hInst; // �ν��Ͻ� �ڵ�
HANDLE hReadEvent, hWriteEvent; // �̺�Ʈ
HWND hEdit; // ����Ʈ �ڽ� ���� ��Ʈ�� (ä��)
HWND iEdit; // ����Ʈ �ڽ� ���� ��Ʈ�� (Ŭ���̾�Ʈ)

CRITICAL_SECTION cs; // �Ӱ� ����
HWND hWnd; // ������ ���ν���
HICON hIconS, hIconB; // ������

char Name[NAMESIZE] = { "�����" };
char msg[BUFSIZE];

int cnt = 0;

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

void PlaceInCenterOfScreen(HWND hDlg) // ������ ��ǥ ���� �Լ�
{ 
    RECT rect; 
    
    GetWindowRect(hDlg, &rect); 
    SetWindowPos(hDlg, NULL,
    
    (GetSystemMetrics(SM_CXSCREEN) - rect.right + rect.left) / 2,
    (GetSystemMetrics(SM_CYSCREEN) - rect.bottom + rect.top) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    hInst = hInstance;
    InitializeCriticalSection(&cs);

    hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    if (hReadEvent == NULL) return 1;
    hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hWriteEvent == NULL) return 1;

    hIconS = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE);
    hIconB = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_BIG), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE);

    // ���� ��� ������ ����
    CreateThread(NULL, 0, ServerMain, NULL, 0, NULL);

    // ��ȭ����(���̾�α�) ���� 
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

    // �޽��� ����
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    DeleteCriticalSection(&cs);
    return msg.wParam;
}

//  ���� ä�� ȭ�� ��ȭ����
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int Return;

    switch (uMsg) {

    case WM_INITDIALOG:
        PlaceInCenterOfScreen(hDlg);
        hEdit = GetDlgItem(hDlg, IDC_EDIT1);
        iEdit = GetDlgItem(hDlg, IDC_LIST1);
        SendMessage(hEdit, EM_SETLIMITTEXT, BUFSIZE, 0);
        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIconB);
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIconS);
        return TRUE;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
 
    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDABORT:

            return TRUE;
            break;
  
        case IDCANCEL:
            
            Return = MessageBox(hWnd, _T("�����Ͻðڽ��ϱ�?"), _T("Ȯ��"), MB_YESNO);
            if (Return == IDYES) {
                DestroyWindow(hDlg);
                break;
            }
            else {
                return 0;
            }

        }
        return FALSE;
    }
    return FALSE;
}

// ����Ʈ ��Ʈ�� ��� �Լ�
void DisplayText(char *fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);

    char cbuf[BUFSIZE + 256];
    vsprintf(cbuf, fmt, arg);

    EnterCriticalSection(&cs);
    int nLength = GetWindowTextLength(hEdit);
    SendMessage(hEdit, EM_SETSEL, nLength, nLength);
    SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
    LeaveCriticalSection(&cs);

    va_end(arg);
}

// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
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

// ���� �Լ� ���� ���
void err_display(char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    DisplayText("[%s] %s", msg, (char *)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// TCP ���� ���� �κ�
DWORD WINAPI ServerMain(LPVOID arg)
{
    int retval;

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) err_quit("listen()");

    // ������ ��ſ� ����� ����
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;
    HANDLE hThread;

    while (1) {
        // accept()

        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            err_display("accept()");
            break;
        }

        // ������ Ŭ���̾�Ʈ ���� ���
        DisplayText("[TCP ����] ���ο� ����ڰ� �����߽��ϴ�. IP �ּ�=%s, ��Ʈ ��ȣ=%d\r\n",
            inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

        SendMessage(iEdit, LB_ADDSTRING, 0, (LPARAM)inet_ntoa(clientaddr.sin_addr));

        // ������ ����
        hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);

        if (hThread == NULL) {
            closesocket(client_sock);
        }
        else {
            CloseHandle(hThread);
        }
    }

    // closesocket()
    closesocket(listen_sock);

    // ���� ����
    WSACleanup();
    return 0;
}

// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ProcessClient(LPVOID arg)
{
    SOCKET client_sock = (SOCKET)arg;
    int nameval;
    int retval;

    SOCKADDR_IN clientaddr;
    int addrlen;

    // Ŭ���̾�Ʈ ���� ���
    addrlen = sizeof(clientaddr);
    getpeername(client_sock, (SOCKADDR *)&clientaddr, &addrlen);

    while (1) {
        
        // ������ �ޱ�
        nameval = recv(client_sock, Name, NAMESIZE, 0);
        retval = recv(client_sock, msg, BUFSIZE , 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0)
            break;
        
        // ���� ������ ���
        Name[nameval] = '\0';
        msg[retval] = '\0';
        DisplayText("[%d:%d][%s]:%s\r\n", Time_Hour(), Time_Min(), Name, msg);


        // ������ ������
        nameval = send(client_sock, Name, nameval, 0);
        retval = send(client_sock, msg, retval, 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }
                
    }
    // closesocket()
    SendMessage(iEdit, LB_DELETESTRING, 0, (LPARAM)inet_ntoa(clientaddr.sin_addr));
    closesocket(client_sock);
    DisplayText("[TCP ����] %s���� �����̽��ϴ�. IP �ּ�=%s, ��Ʈ ��ȣ=%d\r\n", Name,
        inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

    return 0;
}

