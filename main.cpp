#include <Windows.h>
#include <tlhelp32.h>

#include <GdiPlus.h>
using namespace Gdiplus;


void suspend(DWORD processId)
{
    HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    THREADENTRY32 threadEntry; 
    threadEntry.dwSize = sizeof(THREADENTRY32);

    Thread32First(hThreadSnapshot, &threadEntry);

    do
    {
        if (threadEntry.th32OwnerProcessID == processId)
        {
            HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE,
                threadEntry.th32ThreadID);

            SuspendThread(hThread);
            CloseHandle(hThread);
        }
    } while (Thread32Next(hThreadSnapshot, &threadEntry));

    CloseHandle(hThreadSnapshot);
}


void resume(DWORD processId)
{
    HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    THREADENTRY32 threadEntry; 
    threadEntry.dwSize = sizeof(THREADENTRY32);

    Thread32First(hThreadSnapshot, &threadEntry);

    do
    {
        if (threadEntry.th32OwnerProcessID == processId)
        {
            HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE,
                threadEntry.th32ThreadID);

            ResumeThread(hThread);
            CloseHandle(hThread);
        }
    } while (Thread32Next(hThreadSnapshot, &threadEntry));

    CloseHandle(hThreadSnapshot);
}

void EnableDebugPriv()
{
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tkp;

    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL);

    CloseHandle(hToken); 
}

bool FindGTA(HANDLE& hProcess, DWORD& uFuckU)
{
	hProcess = INVALID_HANDLE_VALUE;
	uFuckU = 0;


    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (stricmp(entry.szExeFile, "GTA5.exe") == 0)
            {  
                hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

                uFuckU = GetProcessId(hProcess);

				CloseHandle(snapshot);
				return true;
            }
        }
    }

    CloseHandle(snapshot);
	return false;
}

HANDLE g_hGTA = INVALID_HANDLE_VALUE;
DWORD g_uGTA = 0;

const double g_dSuspendTotalTime = 12.0;
double g_dSuspendTime = g_dSuspendTotalTime;
#define INDICATORTICK 100  /* in milliseconds;  at least 50 */

bool Initialize(HINSTANCE hInstance);
void Shutdown(HINSTANCE hInstance);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	EnableDebugPriv();


	if(!FindGTA(g_hGTA, g_uGTA))
	{
#ifndef _DEBUG
		return 0;
#endif
	}

	Initialize(hInstance);

	suspend(g_uGTA);

	MSG msg;
	
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	Shutdown(hInstance);

	resume(g_uGTA);

	return 0;
}


#define INDICATORCLASSNAME	"flushtheturds"

HWND g_hIndicatorWnd = 0;
unsigned long g_uIndicatorLastHB = 0;
int g_nIndicatorVal = -1;



void AddRoundedRectangle(GraphicsPath& gp, const RectF& rc, REAL cr)
{
	cr*=1.55f;
	gp.StartFigure();
	gp.AddArc(rc.X, rc.Y, cr, cr, 180.0f, 90.0f);
	gp.AddArc(rc.X+rc.Width-cr, rc.Y, cr, cr, 270.0f, 90.0f);
	gp.AddArc(rc.X+rc.Width-cr, rc.Y+rc.Height-cr, cr, cr, 0.0f, 90.0f);
	gp.AddArc(rc.X, rc.Y+rc.Height-cr, cr, cr, 90.0f, 90.0f);
	gp.CloseFigure();
}

void DrawIndicator(Graphics& gfx)
{
	if(g_nIndicatorVal == -1)
		return;

	RECT screenRC;
	GetWindowRect(GetDesktopWindow(), &screenRC);
	int screenWidth = screenRC.right-screenRC.left;
	int screenHeight = screenRC.bottom-screenRC.top;

	RECT rrc;
	GetClientRect(g_hIndicatorWnd, &rrc);
	RectF rc((REAL)rrc.left, (REAL)rrc.top, (REAL)(rrc.right-rrc.left)-1.0f, (REAL)(rrc.bottom-rrc.top)-1.0f);

	GraphicsPath p2;
	AddRoundedRectangle(p2, rc, 7.5f * (float)screenHeight / 900.0f);

	SolidBrush pfill(Color(200, 0, 0, 0));
	Pen ppen(Color(128, 96, 96, 96), 2.0f);
	gfx.FillPath(&pfill, &p2);
	gfx.DrawPath(&ppen, &p2);


	const int numBars = 60;

	double barWidth = (double)rc.Width / (double)(numBars*2+1);
	for(int x=0; x<numBars; x++)
	{
		double cx = (double)(x*2+1) * barWidth + barWidth/2.0;
		double cy = (double)(rc.X+rc.Height/2.0f);

		Color clr;

		if( (x * 100 / numBars) < g_nIndicatorVal )
			clr = Color(237, 0, 255, 0);
		else
			clr = Color(237, 0, 0, 0);

		float rre = 4.0f * (float)screenHeight / 900.0f;

		RectF rc2(0.0f, 0.0f, (float)barWidth*1.1f, (float)rc.Height*0.7f);

		SolidBrush batbrush(clr);

		RectF rc2a = rc2;
		rc2a.Offset((float)cx-rc2.Width/2.0f, (float)cy-rc2.Height/2.0f);
		GraphicsPath gp2a;
		AddRoundedRectangle(gp2a, rc2a, rre);
		gfx.FillPath(&batbrush, &gp2a);

		Pen batpen(Color(100, 70, 70, 70), 2.0f);
		gfx.DrawPath(&batpen, &gp2a);
	}
}

void RedrawIndicator()
{
	RECT rc;
	GetWindowRect(g_hIndicatorWnd, &rc);

	HDC hDCScreen = GetDC(NULL);
	HDC hDC = CreateCompatibleDC(hDCScreen);
	HBITMAP hBBBMP = CreateCompatibleBitmap(hDCScreen, rc.right-rc.left, rc.bottom-rc.top);
	HGDIOBJ hBMPOld = SelectObject(hDC, hBBBMP);

	{
		Graphics gfx(hDC);
		gfx.SetSmoothingMode(SmoothingMode::SmoothingModeHighQuality);
		DrawIndicator(gfx);
	}

	BLENDFUNCTION bf = { 0 };
    bf.AlphaFormat = AC_SRC_ALPHA;
    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = 255;

	POINT ptPos = { rc.left, rc.top };
	SIZE sizeWnd = { rc.right-rc.left, rc.bottom-rc.top };
	POINT ptSrc = { 0, 0 };

	UpdateLayeredWindow(g_hIndicatorWnd, hDCScreen, &ptPos, &sizeWnd, hDC, &ptSrc, 0, &bf, ULW_ALPHA);

	SelectObject(hDC, hBMPOld);
	DeleteObject(hBBBMP);
	DeleteDC(hDC);
	ReleaseDC(NULL, hDCScreen);
}

void ProcessIndicator()
{
	unsigned long c = GetTickCount();
	if( g_uIndicatorLastHB > 0 && c > g_uIndicatorLastHB )
	{
		double dt = ((double)c - (double)g_uIndicatorLastHB) / 1000.0;

		g_dSuspendTime -= dt;
		if(g_dSuspendTime <= 0.0)
		{
			DestroyWindow(g_hIndicatorWnd);
			PostQuitMessage(0);
			return;
		}

		if( GetTopWindow(NULL) != g_hIndicatorWnd )
			SetWindowPos(g_hIndicatorWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);


		// LOLL
		g_nIndicatorVal = (int)(g_dSuspendTime / g_dSuspendTotalTime * 100.0);


		RedrawIndicator();
	}

	g_uIndicatorLastHB = c;
}

LRESULT CALLBACK IndicatorWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_TIMER:
		switch(wParam)
		{
		case 0:
			ProcessIndicator();
			break;
		}
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

ULONG_PTR gdiplustoken;

bool Initialize(HINSTANCE hInstance)
{
	GdiplusStartupInput gdsi;
	gdsi.SuppressExternalCodecs = TRUE;
	GdiplusStartup(&gdiplustoken, &gdsi, nullptr);


	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(WNDCLASS));
	wc.hInstance = hInstance;
	wc.lpfnWndProc = IndicatorWndProc;
	wc.lpszClassName = INDICATORCLASSNAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&wc);

	RECT screenRC;
	GetWindowRect(GetDesktopWindow(), &screenRC);
	int screenWidth = screenRC.right-screenRC.left;
	int screenHeight = screenRC.bottom-screenRC.top;

	int indWidth = 600*screenWidth/1600;
	int indHeight = 45*screenHeight/900;
	int indX = screenWidth/2 - indWidth/2;
	int indY = screenHeight/2 - indHeight/2;

	g_hIndicatorWnd = CreateWindowEx(WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_TOPMOST|WS_EX_TOOLWINDOW, INDICATORCLASSNAME, "", WS_POPUP|WS_VISIBLE,  indX, indY, indWidth, indHeight, NULL, NULL, hInstance, NULL);

	RedrawIndicator();
	SetTimer(g_hIndicatorWnd, 0, INDICATORTICK, NULL);

	return true;
}

void Shutdown(HINSTANCE hInstance)
{
	if(g_hIndicatorWnd != INVALID_HANDLE_VALUE && g_hIndicatorWnd != NULL)
	{
		KillTimer(g_hIndicatorWnd, 0);
		DestroyWindow(g_hIndicatorWnd);
		g_hIndicatorWnd = NULL;

		UnregisterClass(INDICATORCLASSNAME, hInstance);
	}


	GdiplusShutdown(gdiplustoken);
}