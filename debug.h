
#ifndef _DEBUG_H
#define _DEBUG_H

#define DM_PIXEL			0
#define DM_CROSSHAIR		1
#define DM_XLINE			2
#define DM_YLINE			3
#define DM_BOX				4
#define DM_ABS_BOX          5


void debugbox(int x1, int y1, int x2, int y2, uchar r, uchar g, uchar b);
void debug_absbox(int x1, int y1, int x2, int y2, uchar r, uchar g, uchar b);
void debug(const char *fmt, ...);

#endif
