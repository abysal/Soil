#pragma once

#ifndef _DEBUG
#define DEBUG_CODE(...)
#else
#define DEBUG_CODE(...) __VA_ARGS__
#endif