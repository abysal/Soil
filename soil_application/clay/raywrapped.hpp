#pragma once

// These functions conflict with the DirectX API names
#define CloseWindow Ray_CloseWindow
#define ShowCursor Ray_ShowCursor
#define Rectangle Ray_Rectangle
#define LoadImage Ray_LoadImage
#define DrawText Ray_DrawText
#define Color Ray_Color

#include <raylib.h>

#undef Color
#undef LoadImage
#undef CloseWindow
#undef ShowCursor
#undef Rectangle
#undef DrawText
