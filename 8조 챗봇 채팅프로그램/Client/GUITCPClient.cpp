#define _CRT_SECURE_NO_WARNINGS         // �ֽ� VC++ ������ �� ��� ����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // �ֽ� VC++ ������ �� ��� ����
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>
#include <process.h>
#include<iostream>
#include<fstream>
#include<string>
#include <string.h>
#include <cstring>
#include "resource.h"

#define MAX_FILENAME_SIZE 100 // ���� ��ο� ���� �̸��� �ִ� ũ��
#define BUFSIZE 1024 // ���� ������
using namespace std;

HINSTANCE hInst;
SOCKET client_sock;
HWND hwndIP; // IP �ּ� 
HWND hwndPort; // ��Ʈ
HWND hwndName; // �г���
HWND hwndServConnect; // ���� ��ư
HWND hwndServDisConnect; // ���� ���� ��ư
HWND hwndSend; // ������ ��ư
HWND hwndFont; // ��Ʈ ���� ��ư
HWND hwndEdit1; // �޽��� �Է� â
HWND hwndEdit2; // ä�� ȭ��
HWND hwndUserID; // ���� ���̵�
HWND hwndNickName; // �г��� ����
HWND hWnd;
HWND hWndFocus;

char IP[25]; // ������
char Port[25]; // ��Ʈ
char Name[25]; // �̸�
char NameStr[256]; // �̸� + �޽���
char str[128]; // �޽���
BOOL SEND = FALSE;

HICON hIconS, hIconB; // ������
HANDLE Thread1, Thread2; // ������
DWORD ThreadID1, ThreadID2; // ������ ID

BOOL CALLBACK DlgProc1(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM IParam); // ���� ��ȭ���� (���̾�α�)
BOOL CALLBACK DlgProc2(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM IParam); // IP �Է� ��ȭ���� (���̾�α�)
BOOL CALLBACK DlgProc3(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // Ŭ���̾�Ʈ ���� ��ȭ���� (���̾�α�)
BOOL CALLBACK DlgProc4(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // ID �Է� ��ȭ���� (���̾�α�)
BOOL OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM IParam); // ��ȭ���� (�ʱ�ȭ)
LPTSTR lpszClass = _T("BasicApi"); // ���� ��ȯ �Լ�

OPENFILENAME OpenFileName;
TCHAR FilePathName[MAX_FILENAME_SIZE];
static TCHAR Filter[] = "��� ����\0*.*\0�ؽ�Ʈ ����\0*.txt\0��Ʈ�� ����\0*.bmp";

HDC hdc;
PAINTSTRUCT ps;
HFONT hFont, OldFont;

CHOOSEFONT FONT;
static COLORREF fColor;
static LOGFONT LogFont;

void DisplayText(char* fmt, ...); // �޽��� ��� �Լ�
void err_quit(char* msg); // ���� ��� �Լ�
void err_server(char* msg); // ���� ��� �Լ�

void OnCommand1(HWND hWnd, WPARAM wParam);
void OnCommand2(HWND hwnd, WPARAM wParam);
void OnCommand3(HWND hwnd, WPARAM wParam);
void OnConnect1(HWND hwnd);
void OnConnect2(HWND hwnd);
void OnDisConnect(HWND hwnd);
void OnSend(HWND hwnd);
void OnClose(HWND hWnd);
void OnInfo(HWND hwnd);
void OnClear(HWND hwnd);
void OnChangeName(HWND hwnd);

unsigned int __stdcall SendMsg(void* arg); // �޽��� ���� �Լ�
unsigned int __stdcall RecvMsg(void* arg); // �޽��� ���� �Լ�

int Time_Hour(); // �ð� ��� �Լ� (Hour)
int Time_Min(); // �ð� ��� �Լ� (Min)

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLind, int nCmdShow)
{
    hIconS = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE);
    hIconB = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_BIG), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE);

    hInst = hInstance;

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // ��ȭ���� ����
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG2), NULL, DlgProc2);

    // ���� ����
    WSACleanup();
    return 0;
}

BOOL CALLBACK DlgProc1(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HBRUSH hBrush = CreateSolidBrush(RGB(230, 240, 250));

    HBITMAP hBitmap1, hBitmap2, hBitmap3, hBitmap7, hBitmap8;

    switch (uMsg) {

    case WM_INITDIALOG:

        OnInitDialog(hWnd, hWnd, lParam);
        hBitmap1 = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP2));
        hBitmap2 = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP3));
        hBitmap3 = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP4));
        hBitmap7 = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP9));
        hBitmap8 = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP10));

        SendMessage(hwndServConnect, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap1);
        SendMessage(hwndServDisConnect, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap2);
        SendMessage(hwndSend, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap3);
        SendMessage(hwndFont, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap7);
        SendMessage(hwndNickName, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap8);

        break;

    case WM_CTLCOLORDLG:

        return (LRESULT)hBrush;

    case WM_CTLCOLORBTN:

        return (LRESULT)hBrush;

    case WM_CTLCOLORSTATIC:

        SetBkColor((HDC)wParam, RGB(230, 240, 250));
        return (LRESULT)hBrush;

    case WM_COMMAND:

        OnCommand1(hWnd, wParam);
        return TRUE;

    case WM_PAINT:

        hdc = BeginPaint(hWnd, &ps);
        hFont = CreateFontIndirect(&LogFont);
        OldFont = (HFONT)SelectObject(hdc, hFont);
        SetTextColor(hdc, fColor);

        SelectObject(hdc, OldFont);
        DeleteObject(hFont);
        EndPaint(hWnd, &ps);
        break;

    case WM_CLOSE:

        OnClose(hWnd);
        break;

    }
    return FALSE;
}

BOOL CALLBACK DlgProc2(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HBRUSH hBrush = CreateSolidBrush(RGB(230, 240, 250));;

    HDC hdc;
    HDC memdc;
    PAINTSTRUCT ps;
    static HBITMAP hBitMap4;
    HBITMAP hBitmap5, hBitmap6;

    switch (uMsg) {

    case WM_INITDIALOG:
        
        SetDlgItemText(hWnd, IDC_IPADDRESS, "221.139.96.157");
        SetDlgItemText(hWnd, IDC_PORT, "9000");
        hwndIP = GetDlgItem(hWnd, IDC_IPADDRESS);
        hwndPort = GetDlgItem(hWnd, IDC_PORT);
        hwndUserID = GetDlgItem(hWnd, IDC_USERID);
        SetFocus(hwndUserID);
        hBitMap4 = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP6));
        hBitmap5 = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP7));
        hBitmap6 = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP8));
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconS);
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIconB);
        break;

    case WM_CTLCOLORDLG:

        return (LRESULT)hBrush;

    case WM_CTLCOLORSTATIC:

        SetBkColor((HDC)wParam, RGB(230, 240, 250));
        return (LRESULT)hBrush;

    case WM_PAINT:

        hdc = BeginPaint(hWnd, &ps);
        memdc = CreateCompatibleDC(hdc);
        SelectObject(memdc, hBitMap4);
        BitBlt(hdc, 68, 25, 128, 128, memdc, 0, 0, SRCCOPY);
        DeleteObject(memdc);
        EndPaint(hWnd, &ps);
        break;

    case WM_COMMAND:

        OnCommand2(hWnd, wParam); return TRUE;

    case WM_CLOSE:

        OnClose(hWnd);
        break;
    }
    return FALSE;
}

BOOL CALLBACK DlgProc3(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));;

    HDC hdc;
    HDC memdc;
    PAINTSTRUCT ps;
    static HBITMAP hBitMap;

    switch (uMsg) {

    case WM_INITDIALOG:
        hBitMap = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP5));
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconS);
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIconB);
        break;

    case WM_CTLCOLORDLG:

        return (LRESULT)hBrush;

    case WM_CTLCOLORSTATIC:

        return (LRESULT)hBrush;

    case WM_PAINT:

        hdc = BeginPaint(hWnd, &ps);
        memdc = CreateCompatibleDC(hdc);
        SelectObject(memdc, hBitMap);
        BitBlt(hdc, 60, 30, 100, 100, memdc, 0, 0, SRCCOPY);
        DeleteObject(memdc);
        EndPaint(hWnd, &ps);
        break;

    case WM_CLOSE:

        EndDialog(hWnd, 0);
        break;
    }
    return FALSE;
}

BOOL CALLBACK DlgProc4(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HBRUSH hBrush = CreateSolidBrush(RGB(230, 240, 250));;

    switch (uMsg) {

    case WM_INITDIALOG:
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconS);
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIconB);
        break;

    case WM_CTLCOLORDLG:

        return (LRESULT)hBrush;

    case WM_CTLCOLORSTATIC:

        SetBkColor((HDC)wParam, RGB(230, 240, 250));
        return (LRESULT)hBrush;

    case WM_PAINT:

        break;

    case WM_COMMAND:

        OnCommand3(hWnd, wParam); return TRUE;

    case WM_CLOSE:

        EndDialog(hWnd, 0);
        break;
    }
    return FALSE;
}

BOOL OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM IParam)
{
    hwndName = GetDlgItem(hWnd, IDC_ID);
    hwndEdit1 = GetDlgItem(hWnd, IDC_CHATEDIT);
    hwndEdit2 = GetDlgItem(hWnd, IDC_CHATVIEW);
    hwndServConnect = GetDlgItem(hWnd, IDC_CONNECT);
    hwndServDisConnect = GetDlgItem(hWnd, IDC_EXIT);
    hwndSend = GetDlgItem(hWnd, IDC_SEND);
    hwndFont = GetDlgItem(hWnd, IDC_FONT);
    hwndNickName = GetDlgItem(hWnd, IDC_NICKNAME);

    SetFocus(hwndName);

    SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconS); 
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIconB); 

    DisplayText("[CHATBOT] ä�� ������ ���� ���� ȯ���մϴ�.\r\n");
    DisplayText("[CHATBOT] �޽����� �Է��Ͻð� ä���� �������ּ���.\r\n");

    EnableWindow(hwndServConnect, FALSE);

    return TRUE;
}

void OnCommand1(HWND hwnd, WPARAM wParam)
{

    switch (LOWORD(wParam))
    {

    case IDC_NICKNAME:

        OnChangeName(hwnd);
        break;

    case IDC_FONT:

        ZeroMemory(&FONT, 0, sizeof(CHOOSEFONT));
        FONT.lStructSize = sizeof(CHOOSEFONT);
        FONT.hwndOwner = hwnd;
        FONT.lpLogFont = &LogFont;
        FONT.Flags = CF_EFFECTS | CF_SCREENFONTS;

        if (ChooseFont(&FONT) != 0)
        {
            fColor = FONT.rgbColors;
            InvalidateRgn(hwnd, NULL, TRUE);
        }
        break;
      
    case IDC_CONNECT:

        OnConnect2(hwnd);
        break;

    case IDC_EXIT:

        OnDisConnect(hwnd);
        break;

    case IDC_SEND:

        OnSend(hwnd);
        break;

    case ID_INFO:

        OnInfo(hwnd);
        break;

    case ID_CLEAR:

        OnClear(hwnd);
        break;

    case ID_SAVE_FILE: // ��ȭ���� ���� �ϱ�

        ZeroMemory(&OpenFileName, 0, sizeof(OPENFILENAME));
        OpenFileName.lStructSize = sizeof(OPENFILENAME);
        OpenFileName.hwndOwner = hwnd;
        OpenFileName.lpstrFilter = Filter;
        OpenFileName.lpstrFile = FilePathName;
        OpenFileName.nMaxFile = MAX_FILENAME_SIZE;
        OpenFileName.lpstrInitialDir = "C:\\";

        if (GetSaveFileName(&OpenFileName) != 0)
        {
            char msg[BUFSIZE];
            DWORD dwSize = GetWindowTextLength(hwndEdit2);
            GetDlgItemText(hwnd, IDC_CHATVIEW, msg, dwSize);

            ofstream writeFile;
            writeFile.open(OpenFileName.lpstrFile);
            writeFile.write(msg, dwSize);
            writeFile.close();

            MessageBox(hwnd, _T("������ ����Ǿ����ϴ�."), _T("���� ����"), MB_ICONINFORMATION | MB_OK);
        }
        else
        {
            MessageBox(hwnd, _T("�����ϱ⸦ ����Ͽ����ϴ�."), _T("���� ���� ���"), MB_ICONINFORMATION | MB_OK);
        }
        break;

    case ID_LOAD_FILE: // ���� ���� �ϱ�

        MessageBox(hwnd, _T("�׽�Ʈ ���Դϴ�. (�̱���)"), _T("���� ����"), MB_ICONWARNING | MB_OK);

        /* ZeroMemory(&OpenFileName, 0, sizeof(OPENFILENAME));
        OpenFileName.lStructSize = sizeof(OPENFILENAME);
        OpenFileName.hwndOwner = hwnd;
        OpenFileName.lpstrFilter = SFilter;
        OpenFileName.lpstrFile = SFilePathName;
        OpenFileName.nMaxFile = MAX_FILENAME_SIZE;
        OpenFileName.lpstrInitialDir = "C:\\";

        if (GetOpenFileName(&OpenFileName) != 0)
        {
            MessageBox(hwnd, _T("������ ���� �Ǿ����ϴ�."), _T("���� ����"), MB_ICONINFORMATION | MB_OK);
        }
        else
        {
            MessageBox(hwnd, _T("�����ϱ⸦ ����Ͽ����ϴ�."), _T("���� ���� ���"), MB_ICONINFORMATION | MB_OK);
        }
        */
        break;
    }

}

void OnCommand2(HWND hwnd, WPARAM wParam)
{
    switch (LOWORD(wParam))
    {
    case IDOK:

        OnConnect1(hwnd);
        DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc1);
        break;

    case IDCANCEL:

        EndDialog(hwnd, 0);
        break;

    }
}

void OnCommand3(HWND hwnd, WPARAM wParam)
{
    switch (LOWORD(wParam))
    {
    case IDOK:

        GetDlgItemText(hwnd, IDC_USERNAME, Name, 25);
        MessageBox(hWnd, _T("�г��� ���� �Ϸ�"), _T("�г��� ����"), MB_ICONINFORMATION | MB_OK);
        EndDialog(hwnd, 0);
        break;

    case IDCANCEL:

        EndDialog(hwnd, 0);
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

void OnConnect1(HWND hwnd)
{
    int retval;

    GetDlgItemText(hwnd, IDC_IPADDRESS, IP, 25);
    GetDlgItemText(hwnd, IDC_PORT, Port, 25);
    GetDlgItemText(hwnd, IDC_USERID, Name, 25);

    // socket()
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == INVALID_SOCKET) err_quit("socket()");

    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(IP);
    serveraddr.sin_port = htons(atoi(Port));

    // connect()
    retval = connect(client_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    send(client_sock, Name, (int)strlen(Name), 0);

    if (retval == SOCKET_ERROR) {
        err_quit("connect()");
    }
    else {
        MessageBox(hWnd, _T("���� ���� �Ϸ�"), _T("���� ����"), MB_ICONINFORMATION | MB_OK);
        EndDialog(hwnd, 0);
    }

    Thread1 = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)client_sock, 0, (unsigned*)&ThreadID1);
    Thread2 = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)client_sock, 0, (unsigned*)&ThreadID2);
}

void OnConnect2(HWND hwnd)
{

    EnableWindow(hwndSend, TRUE);

    int retval;

    // socket()
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == INVALID_SOCKET) err_quit("socket()");

    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(IP);
    serveraddr.sin_port = htons(atoi(Port));

    // connect()
    retval = connect(client_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    send(client_sock, Name, (int)strlen(Name), 0);

    if (retval == SOCKET_ERROR) {
        err_quit("connect()");
    }
    else {
        MessageBox(hWnd, _T("���� ���� �Ϸ�"), _T("���� ����"), MB_ICONINFORMATION | MB_OK);
        SetDlgItemText(hwnd, IDC_CHATVIEW, "");
        SetDlgItemText(hwnd, IDC_CHATEDIT, "");
        DisplayText("[CHATBOT] ä�� ������ ���� ���� ȯ���մϴ�.\r\n");
        DisplayText("[CHATBOT] �޽����� �Է��Ͻð� ä���� �������ּ���.\r\n");
        EnableWindow(hwndName, TRUE);
        EnableWindow(hwndEdit1, TRUE);
        EnableWindow(hwndServDisConnect, TRUE);
        EnableWindow(hwndServConnect, FALSE);
        EnableWindow(hwndFont, TRUE);

    }

    Thread1 = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)client_sock, 0, (unsigned*)&ThreadID1);
    Thread2 = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)client_sock, 0, (unsigned*)&ThreadID2);

}

void OnDisConnect(HWND hwnd)
{
    int retval;

    retval = MessageBox(hwnd, _T("������ �����մϴ�"), _T("���� ����"), MB_ICONINFORMATION | MB_YESNO);

    TerminateThread(Thread1, ThreadID1);
    TerminateThread(Thread2, ThreadID2);

    if (retval == IDYES) {

        EnableWindow(hwndSend, FALSE); // ������ ��ư ��Ȱ��ȭ
        SetDlgItemText(hwnd, IDC_CHATVIEW, "");
        SetDlgItemText(hwnd, IDC_CHATEDIT, "");
        closesocket(client_sock);
        DisplayText("[CHATBOT] ä�� ������ ������ ���������ϴ�.\r\n");
        EnableWindow(hwndName, FALSE);
        EnableWindow(hwndEdit1, FALSE);
        EnableWindow(hwndServDisConnect, FALSE);
        EnableWindow(hwndServConnect, TRUE);
        EnableWindow(hwndFont, FALSE);

    }
}

void OnSend(HWND hwnd)
{
    int msg;

    msg = GetDlgItemText(hwnd, IDC_CHATEDIT, str, sizeof(str));

    if (msg == NULL) {
        MessageBox(hwnd, _T("���� �޽����� �Է��ϼ���!"), _T("�޽��� �Է�"), MB_ICONWARNING | MB_OK);
        SEND = FALSE;
    }
    else {
        SetDlgItemText(hwnd, IDC_CHATEDIT, "");
        SetFocus(hwndEdit1);
        SEND = TRUE;
    }
}

void OnInfo(HWND hwnd)
{
    DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), NULL, DlgProc3);
}

void OnClear(HWND hwnd)
{
    SetDlgItemText(hwnd, IDC_CHATVIEW, "");
    DisplayText("[CHATBOT] ��ȭ ������ �ʱ�ȭ �Ǿ����ϴ�.\r\n");
}

void OnChangeName(HWND hwnd)
{
    DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG4), NULL, DlgProc4);

}

unsigned int __stdcall SendMsg(void* arg)
{
    while (true)
    {
        if (SEND) {

            sprintf(NameStr, "[%02d:%02d][%s]:%s \r\n", Time_Hour(), Time_Min(), Name, str);
            send(client_sock, NameStr, (int)strlen(NameStr), 0);
            send(client_sock, Name, (int)strlen(Name), 0);

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
        if (retval == -1) {
            err_server("socket()");
        }
        NameStr[retval] = 0;

        if (retval > 0) {
            DisplayText(NameStr);
        }

    }
    return 0;
}

void DisplayText(char* fmt, ...)
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

void err_quit(char* msg)
{
    MessageBox(NULL, "ä�� ���� ���ῡ �����Ͽ����ϴ�. SERVER DISCONNET", "���� ����", MB_ICONERROR);
    exit(1);
}

void err_server(char* msg)
{
    MessageBox(NULL, "���� �����ڰ� ���� �߹���׽��ϴ�. SERVER DENYED", "���� ����", MB_ICONERROR);
    exit(1);
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
