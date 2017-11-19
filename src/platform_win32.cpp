#include <windows.h>
#include <windowsx.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "nps_common_defs.h"
#include "nps_string.h"
#include "nps_math.h"

#include "glcorearb.h"
#include "wglext.h"

#define WMCUSTOM_ACTIVATE WM_APP

struct app_input
{
    bool32 IsCursorDown;
    float CursorPosX, CursorPosY;

    bool32 Is1Down;
    bool32 Is2Down;
    bool32 Is3Down;

    bool32 IsDPressed;
};

global HANDLE LogFile;
global uint64_t GlobalPerfCountFrequency;
global char GeneralBuffer[512];
global bool32 GlobalRunning = true;
global bool32 GlobalHasFocus;

internal inline void
Printf(char *Format, ...)
{
    va_list VarArgs;
    va_start(VarArgs, Format);

    vsprintf(GeneralBuffer, Format, VarArgs);
    OutputDebugString(GeneralBuffer);

    va_end(VarArgs);
}

internal inline void
Log(char *Format, ...)
{
    va_list VarArgs;
    va_start(VarArgs, Format);

    vsprintf(GeneralBuffer, Format, VarArgs);

    OutputDebugString(GeneralBuffer);

    if(LogFile == INVALID_HANDLE_VALUE)
    {
        return;
    }

    DWORD BytesWritten;
    DWORD BytesToWrite = (uint32_t)strlen(GeneralBuffer);
    if(WriteFile(LogFile, GeneralBuffer, BytesToWrite, &BytesWritten, NULL) == FALSE ||
       BytesWritten != BytesToWrite)
    {
        Printf("failed to write to log file\n");
    }

    va_end(VarArgs);
}

internal void
Win32LogError(char *PreMessageFormat, ...)
{
    DWORD ErrorCode = GetLastError();

    char *Buffer;
    if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                     FORMAT_MESSAGE_FROM_SYSTEM,
                     NULL, 
                     ErrorCode, 
                     0, 
                     (LPTSTR)&Buffer, 
                     0, 
                     NULL) == 0)
    {
        Log("Error: Win32LogError()\n");
    }

    va_list VarArgs;
    va_start(VarArgs, PreMessageFormat);

    vsprintf(GeneralBuffer, PreMessageFormat, VarArgs);
    Log("%s: %s", GeneralBuffer, Buffer);

    va_end(VarArgs);
}

inline internal uint64_t
Win32GetCounter()
{
    LARGE_INTEGER LargeInteger;
    QueryPerformanceCounter(&LargeInteger);

    uint64_t Result = LargeInteger.QuadPart;
    return Result;
}

inline internal float
Win32GetSecondsElapsed(uint64_t EndCounter, uint64_t StartCounter)
{
    float Result = (float)(EndCounter - StartCounter) / (float)GlobalPerfCountFrequency;
    return Result;
}

inline internal int32_t
Win32GetCursorClientPosX(HWND WindowHandle)
{
    POINT CursorScreenPos;
    GetCursorPos(&CursorScreenPos);
    ScreenToClient(WindowHandle, &CursorScreenPos);
    return CursorScreenPos.x;
}

inline internal int32_t
Win32GetCursorClientPosY(HWND WindowHandle)
{
    POINT CursorScreenPos;
    GetCursorPos(&CursorScreenPos);
    ScreenToClient(WindowHandle, &CursorScreenPos);
    return CursorScreenPos.y;
}

inline internal int32_t
Win32GetCursorScreenPosX()
{
    POINT CursorScreenPos;
    GetCursorPos(&CursorScreenPos);
    return CursorScreenPos.x;
}

inline internal int32_t
Win32GetCursorScreenPosY()
{
    POINT CursorScreenPos;
    GetCursorPos(&CursorScreenPos);
    return CursorScreenPos.y;
}

#include "nps_glutils.h"

#if 1
#include "custom_lines.cpp"
#else
#include "scratch.cpp"
#include "template.cpp"
#endif

LRESULT CALLBACK
MainWindowProc(HWND hwnd,
               UINT uMsg,
               WPARAM wParam,
               LPARAM lParam)
{
    LRESULT Result = 0;

    switch(uMsg)
    {
        case WM_CLOSE:
        {
            GlobalRunning = false;
        } break;

        case WM_ACTIVATE:
        {
            GlobalHasFocus = (bool32)wParam;
        } break;

        default:
        {
            Result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }

    return Result;
}

int CALLBACK
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{
    LogFile = CreateFile("log.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(LogFile == INVALID_HANDLE_VALUE)
    {
        Printf("failed to create log file\n");
        Assert(0);
    }

    uint32_t WindowWidth = 1280;
    uint32_t WindowHeight = 800;

    uint32_t ScreenWidth = 1920;
    uint32_t ScreenHeight = 1080;

    LARGE_INTEGER QueryPerformanceFrequencyLargeInteger;
    QueryPerformanceFrequency(&QueryPerformanceFrequencyLargeInteger);
    GlobalPerfCountFrequency = QueryPerformanceFrequencyLargeInteger.QuadPart;

    WNDCLASS WindowClass = {};
    WindowClass.style = CS_OWNDC;
    WindowClass.lpfnWndProc = MainWindowProc;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = "Window Class";
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);

    if(!RegisterClass(&WindowClass))
    {
        Win32LogError("RegisterClass() for dummy window failed");
        Assert(0);
        return 1;
    }

    HWND WindowHandle = CreateWindowEx(0,
                                       WindowClass.lpszClassName,
                                       NULL,
                                       0,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       NULL,
                                       NULL,
                                       hInstance,
                                       NULL);

    if(WindowHandle == NULL)
    {
        Win32LogError("CreateWindowEx() for dummy window failed");
        Assert(0);
        return 1;
    }

    HDC DeviceContextHandle = GetDC(WindowHandle);
    if(DeviceContextHandle == NULL)
    {
        Win32LogError("GetDC() for dummy window failed");
        Assert(0);
        return 1;
    }

    PIXELFORMATDESCRIPTOR PixelFormatDescriptor = {};
    PixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    PixelFormatDescriptor.nVersion = 1;
    PixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    PixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
    PixelFormatDescriptor.cColorBits = 32;
    PixelFormatDescriptor.cDepthBits = 24;
    PixelFormatDescriptor.cStencilBits = 8;
    PixelFormatDescriptor.cAuxBuffers = 0;
    PixelFormatDescriptor.iLayerType = PFD_MAIN_PLANE;

    int PixelFormat = ChoosePixelFormat(DeviceContextHandle, &PixelFormatDescriptor);
    if(!PixelFormat)
    {
        Win32LogError("ChoosePixelFormat() for dummy window failed");
        Assert(0);
        return 1;
    }

    if(SetPixelFormat(DeviceContextHandle, PixelFormat, &PixelFormatDescriptor) == FALSE)
    {
        Win32LogError("SetPixelFormat() for dummy window failed");
        Assert(0);
        return 1;
    }

    HGLRC GLContextHandle = wglCreateContext(DeviceContextHandle);
    if(GLContextHandle == NULL)
    {
        Win32LogError("failed to create the dummy opengl context");
        Assert(0);
        return 1;
    }

    if(wglMakeCurrent(DeviceContextHandle, GLContextHandle) == FALSE)
    {
        Win32LogError("failed to make the dummy opengl context the current context");
        Assert(0);
        return 1;
    }

    wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)GetGLFunctionAddress("wglGetExtensionsStringARB");
    if(wglGetExtensionsStringARB == NULL)
    {
        Assert(0);
    }

    char *ExtensionsSupported = (char *)wglGetExtensionsStringARB(DeviceContextHandle);
    char *ExtensionsRequired[] = {"WGL_ARB_pixel_format", "WGL_ARB_create_context"};
    for(int ExtensionIdx = 0; ExtensionIdx < ArrayCount(ExtensionsRequired); ExtensionIdx++)
    {
        char *Extension = ExtensionsRequired[ExtensionIdx];
        if(!npsStringContains(ExtensionsSupported, Extension))
        {
            Assert(0);
        }
    }

    LoadGLFunctions();

    DestroyWindow(WindowHandle);

    RECT ClientRect = {0, 0, (LONG)WindowWidth, (LONG)WindowHeight};
    AdjustWindowRect(&ClientRect, WS_OVERLAPPEDWINDOW, FALSE);

    uint32_t ActualWindowWidth = ClientRect.right - ClientRect.left + 1;
    uint32_t ActualWindowHeight = ClientRect.bottom - ClientRect.top + 1;

    WindowHandle = CreateWindowEx(0,
                                  WindowClass.lpszClassName,
                                  "Window",
                                  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                  (ScreenWidth - ActualWindowWidth) / 2,
                                  (ScreenHeight - ActualWindowHeight) / 2,
                                  ActualWindowWidth,
                                  ActualWindowHeight,
                                  NULL,
                                  NULL,
                                  hInstance,
                                  NULL);

    if(WindowHandle == NULL)
    {
        Win32LogError("failed to create window");
        Assert(0);
        return 1;
    }

    DeviceContextHandle = GetDC(WindowHandle);
    if(DeviceContextHandle == NULL)
    {
        Win32LogError("failed to get DC");
        Assert(0);
        return 1;
    }

    int PixelFormatAttributeList[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
#if 0
        WGL_SAMPLE_BUFFERS_ARB, 1,
        WGL_SAMPLES_ARB, 4,
#endif
        0
    };

    uint32_t NumPixelFormats;
    wglChoosePixelFormatARB(DeviceContextHandle, PixelFormatAttributeList, 
                            NULL, 1, &PixelFormat, &NumPixelFormats);

    if(SetPixelFormat(DeviceContextHandle, PixelFormat, &PixelFormatDescriptor) == FALSE)
    {
        Win32LogError("SetPixelFormat() failed");
        Assert(0);
        return 1;
    }

    HGLRC DummyGLContextHandle = GLContextHandle;

    int ContextAttributeList[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 0,
        0
    };

    GLContextHandle = wglCreateContextAttribsARB(DeviceContextHandle, NULL, ContextAttributeList);
    if(GLContextHandle == NULL)
    {
        Win32LogError("wglCreateContextAttribsARB() failed");
        Assert(0);
        return 1;
    }

    if(wglDeleteContext(DummyGLContextHandle) == FALSE)
    {
        Win32LogError("wglDeleteContext() failed");
        Assert(0);
        return 1;
    }

    if(wglMakeCurrent(DeviceContextHandle, GLContextHandle) == FALSE)
    {
        Win32LogError("wglMakeCurrent() failed");
        Assert(0);
        return 1;
    }

    /* Begin application code. */

    npsGluInit(WindowWidth, WindowHeight);

    app_input AppInput = {};

    uint32_t FramesPerSecond = 60;
    float TimePerFrame =  1.0f / (float)FramesPerSecond;
    float dt = TimePerFrame;

    while(GlobalRunning)
    {
        uint64_t StartOfFrameCounter = Win32GetCounter();

        AppInput.IsDPressed = false;

        MSG Message;
        while(PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            switch(Message.message)
            {
                case WM_LBUTTONDOWN:
                {
                    AppInput.IsCursorDown = true;
                } break;

                case WM_LBUTTONUP:
                {
                    AppInput.IsCursorDown = false;
                } break;

                case WM_MOUSEMOVE:
                {
                    AppInput.CursorPosX = (float)GET_X_LPARAM(Message.lParam);
                    AppInput.CursorPosY = (float)(WindowHeight - GET_Y_LPARAM(Message.lParam));
                } break;

                case WM_KEYDOWN:
                {
                    switch(Message.wParam)
                    {
                        case '1':
                        {
                            AppInput.Is1Down = true;
                        } break;

                        case '2':
                        {
                            AppInput.Is2Down = true;
                        } break;

                        case '3':
                        {
                            AppInput.Is3Down = true;
                        } break;

                        case 'D':
                        {
                            AppInput.IsDPressed = true;
                        } break;
                    }
                } break;

                case WM_KEYUP:
                {
                    switch(Message.wParam)
                    {
                        case '1':
                        {
                            AppInput.Is1Down = false;
                        } break;

                        case '2':
                        {
                            AppInput.Is2Down = false;
                        } break;

                        case '3':
                        {
                            AppInput.Is3Down = false;
                        } break;
                    }
                } break;
            }
            DispatchMessage(&Message);
        }

        if(SwapBuffers(DeviceContextHandle) == FALSE)
        {
            Win32LogError("SwapBuffers() failed");
            Assert(0);
        }

        UpdateAndRender(WindowWidth, WindowHeight, &AppInput, dt);

        float FrameTimeElapsed = Win32GetSecondsElapsed(Win32GetCounter(), StartOfFrameCounter);
        if(FrameTimeElapsed < TimePerFrame)
        {
            int32_t SleepMillis = (int32_t)(1000.0f*(TimePerFrame - FrameTimeElapsed)) - 1;
            if(SleepMillis > 0)
            {
                Sleep(SleepMillis);
            }

            // spin lock
            do
            {
                FrameTimeElapsed = Win32GetSecondsElapsed(Win32GetCounter(), StartOfFrameCounter);
            } while(FrameTimeElapsed < TimePerFrame);
        }
        else
        {
            // we're late
        }
    }

    return 0;
}
