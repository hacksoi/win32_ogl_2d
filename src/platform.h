#ifndef PLATFORM_H
#define PLATFORM_H

internal inline void Printf(char *Format, ...);
internal inline void Log(char *Format, ...);

struct app_input
{
    bool32 IsCursorDown;
    float CursorPosX, CursorPosY;

    bool32 Is1Down;
    bool32 Is2Down;
    bool32 Is3Down;

    bool32 IsDPressed;
};

#endif
