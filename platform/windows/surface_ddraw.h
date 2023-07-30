/*
 *   2D surfaces via DirectDraw
 *
 *   Based on GDI surfaces because DirectDraw surfaces need to be both
 *   initialized and restored via BitBlt().
 */

#include "platform/windows/surface_gdi.h"

struct IDirectDrawSurface;

struct SURFACE_DDRAW : public SURFACE_GDI {
	IDirectDrawSurface* surf = nullptr;
	int width;
	int height;
};
