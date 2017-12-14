#include <windows.h>
#include <windowsx.h>  // GET_X_LPARAM
#include <stdio.h>
#include <math.h>

#include "types.h"
#include "directx.cpp"
#include "text.cpp"


#define STB_IMAGE_IMPLEMENTATION
#include "lib\\stb_image.h"
void load_image(unsigned char *mem, int *w, int *h)
{
}
void render_mem_to_texture(IDirect3DTexture9 *t, unsigned char *mem, int w, int h)
{
    D3DLOCKED_RECT rect;
    HRESULT res = t->LockRect(0, &rect, 0, 0);
    if (res != D3D_OK) MessageBox(0,"error LockRect", 0, 0);
    memcpy(rect.pBits, mem, w*h*4);
    t->UnlockRect(0);
}



void fill_tex_with_pattern(IDirect3DTexture9 *t, float dt)
{
    static int running_t = 0; running_t += dt;
    D3DLOCKED_RECT rect;
    HRESULT res = t->LockRect(0, &rect, 0, 0);
    if (res != D3D_OK) MessageBox(0,"error LockRect", 0, 0);
    for (int x = 0; x < 400; x++)
    {
        for (int y = 0; y < 400; y++)
        {
            byte *b = (byte*)rect.pBits + ((400*y)+x)*4 + 0;
            byte *g = (byte*)rect.pBits + ((400*y)+x)*4 + 1;
            byte *r = (byte*)rect.pBits + ((400*y)+x)*4 + 2;
            byte *a = (byte*)rect.pBits + ((400*y)+x)*4 + 3;

            *a = 255;
            *r = y*(255.0/400.0);
            *g = ((-cos(running_t*2*3.141592 / 3000) + 1) / 2) * 255.0;
            *b = x*(255.0/400.0);
        }
    }
    t->UnlockRect(0);
}


d3d_textured_quad screen;
d3d_textured_quad hud;
d3d_textured_quad text;

HWND g_hwnd;

bool running = true;

DWORD WINAPI RunMainLoop( LPVOID lpParam )
{
    // {
        // unsigned char *mem = (unsigned char *)malloc(400*400*4);
        // ZeroMemory(mem, 400*400*4);
        // int w, h, bpp;
        // char *source = "C:\\Users\\cmmadmin\\~phil\\projects\\directx-archtest\\test.png";
        // mem = stbi_load(source, &w, &h, &bpp, 4);
        // if (!mem) MessageBox(0, "failed to load file", 0, 0);
        // hud.update(mem, w, h);
    // }
	
	{
		text = tt_create("hello world", 32, 255, true, true);
	}

    {
        u8 *mem = (u8*)malloc(400*400*4);
        ZeroMemory(mem, 400*400*4);
        screen.update(mem, 400, 400);
    }

    while (running)
    {
        fill_tex_with_pattern(screen.tex, 16);
        Sleep(16);
    }
    return 0;
}

UINT backdoor_render_timer_id;

const int RENDER_SLEEP = 1;

int cachedW;
int cachedH;

void render()
{
    if (!running) return;  // kinda smells

    RECT winRect; GetWindowRect(g_hwnd, &winRect);
    int sw = winRect.right-winRect.left;
    int sh = winRect.bottom-winRect.top;

    if (sw!=cachedW || sh!=cachedH)
    {
        cachedW = sw;
        cachedH = sh;
        d3d_resize(sw, sh);
    }

	// tt_print_nobake(sw/2, sh/2, "hello world", 32, sw, sh, 1, true, true, 1);
    // text.move_to_pixel_coords_TL(10, 10, sw, sh);
    text.move_to_pixel_coords_center(sw/2, sh/2, sw, sh);
					 
    // hud.update_with_pixel_coords(10, sh-10-200, 200, 200, sw, sh);

    screen.render();
    text.render();
    // hud.render();
    d3d_swap();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_CLOSE) running = false;
    if (uMsg == WM_NCHITTEST) {
        RECT win; if (!GetWindowRect(hwnd, &win)) return HTNOWHERE;
        POINT pos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        POINT pad = { GetSystemMetrics(SM_CXFRAME), GetSystemMetrics(SM_CYFRAME) };
        bool left   = pos.x < win.left   + pad.x;
        bool right  = pos.x > win.right  - pad.x -1;  // win.right 1 pixel beyond window, right?
        bool top    = pos.y < win.top    + pad.y;
        bool bottom = pos.y > win.bottom - pad.y -1;
        if (top && left)     return HTTOPLEFT;
        if (top && right)    return HTTOPRIGHT;
        if (bottom && left)  return HTBOTTOMLEFT;
        if (bottom && right) return HTBOTTOMRIGHT;
        if (left)            return HTLEFT;
        if (right)           return HTRIGHT;
        if (top)             return HTTOP;
        if (bottom)          return HTBOTTOM;
        return HTCAPTION;
    }
    if (uMsg == WM_SIZE) {
        // if (device) device->Present(0, 0, 0, 0);
    }
    if (uMsg == WM_ENTERSIZEMOVE) SetTimer(hwnd, backdoor_render_timer_id, RENDER_SLEEP, 0);
    if (uMsg == WM_EXITSIZEMOVE) KillTimer(hwnd, backdoor_render_timer_id);
    if (uMsg == WM_TIMER) if ((UINT)wParam == backdoor_render_timer_id) render();

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    timeBeginPeriod(1); // set resolution of sleep

    WNDCLASS wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "app";
    if (!RegisterClass(&wc)) { MessageBox(0, "RegisterClass failed", 0, 0); return 1; }

    HWND hwnd = CreateWindowEx(
        0, "app", "title",
        WS_POPUP | WS_VISIBLE,
        // WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, 0, 0, hInstance, 0);
    if (!hwnd) { MessageBox(0, "CreateWindowEx failed", 0, 0); return 1; }

    g_hwnd = hwnd;


    d3d_load();
    d3d_init(hwnd, 400, 400);
	
	tt_init_nobake();



    CreateThread(0, 0, RunMainLoop, 0, 0, 0);


    while(running)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        render();

        Sleep(RENDER_SLEEP);
    }

    d3d_cleanup();

    return 0;
}
