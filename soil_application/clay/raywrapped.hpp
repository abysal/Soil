#pragma once

// These functions conflict with the DirectX API names
#define CloseWindow Ray_CloseWindow
#define ShowCursor Ray_ShowCursor
#define Rectangle Ray_Rectangle
#define LoadImage Ray_LoadImage

#include <raylib.h>

#undef LoadImage
#undef CloseWindow
#undef ShowCursor
#undef Rectangle
