
#ifndef _TEXTBOX_H
#define _TEXTBOX_H

#include "YesNoPrompt.h"
#include "ItemImage.h"
#include "StageSelect.h"
#include "SaveSelect.h"

#define MSG_W				244
#define MSG_H				64
#define MSG_X				((Graphics::SCREEN_WIDTH / 2) - (MSG_W / 2))
#define MSG_NORMAL_Y		((Graphics::SCREEN_HEIGHT - MSG_H) - 2)
#define MSG_UPPER_Y			24

#define MSG_NLINES			4
#define MSG_LINE_SPACING	16

enum TBFlags
{
	TB_DEFAULTS					= 0x00,
	
	TB_DRAW_AT_TOP				= 0x01,
	TB_NO_BORDER				= 0x02,
	
	TB_LINE_AT_ONCE				= 0x04,
	TB_VARIABLE_WIDTH_CHARS		= 0x08,
	TB_CURSOR_NEVER_SHOWN		= 0x10
};

class TextBox
{
public:
	bool Init();
	void Deinit();
	
	void SetVisible(bool enable, uint8_t flags = TB_DEFAULTS);
	void ResetState();
	
	void AddText(const char *str);
	void SetText(const char *str);
	void ClearText();
	
	void SetFace(int newface);
	
	void SetFlags(uint8_t flags, bool enable);
	void SetFlags(uint8_t flags);
	void ShowCursor(bool enable);
	
	TB_YNJPrompt YesNoPrompt;
	TB_ItemImage ItemImage;
	TB_StageSelect StageSelect;
	TB_SaveSelect SaveSelect;
	
	bool IsVisible();
	bool IsBusy();
	
	void Draw();
	static void DrawFrame(int x, int y, int w, int h);
	
	uint8_t GetFlags() { return fFlags; }
	void SetCanSpeedUp(bool newstate);
	
private:
	void DrawTextBox();
	int GetMaxLineLen();
	void AddNextChar();
	
	bool fVisible;
	uint8_t fFlags;
	
	uint8_t fFace;			// current NPC face or 0 if none
	int fFaceXOffset;		// for face slide-in animation
	
	// currently visible lines
	char fLines[MSG_NLINES][80];
	int fCurLine;
	int fCurLineLen;
	
	// handles scrolling lines off
	bool fScrolling;
	int fTextYOffset;
	
	// chars waiting to be added
	char fCharsWaiting[256];
	uint8_t fCWHead, fCWTail;
	
	int fTextTimer;
	bool fCanSpeedUp;
	
	// blinking cursor control
	bool fCursorVisible;
	int fCursorTimer;
	
	struct
	{
		int x, y;
		int w, h;
	}
	fCoords;
};

/*
#define MSG_NONE			0
#define MSG_NORMAL			1
#define MSG_UPPER_INVISIBLE	2
#define MSG_UPPER			3

#define MSG_NLINES			4

#define NOYESNO				0
#define YESNO_APPEAR		1
#define YESNO_WAIT			2
#define YESNO_YES			3
#define YESNO_NO			4

struct MessageBox
{
	int x, y, w, h;
	
	// 0 - no text box
	// 1 - normal text box
	// 2 - upper text box, box is invisible
	// 3 - upper text box, box is visible
	char displaystate;
	
	int line_spacing;
	int maxlinelen;
	char useautospacing;
	
	uchar face;				// current face or 0 if none
	int face_xoff;			// for face slide-in animation
	
	char instantline;		// instead of a char at a type, we do a line at a time
	
	char line[MSG_NLINES][80];
	int curline;
	int curlinelen;
	
	char scrolling;
	int text_yoffset;
	
	char chars_waiting[256];
	uchar cwhead, cwtail;
	int texttimer;
	char canspeedup;
	
	struct
	{
		char show;
		char inhibit;
		int timer;
	} cursor;
	
	struct
	{
		char state;
		int y;
		int timer;
		char answer;
	} yesno;
	
	struct
	{
		int x, y;
		int sprite, frame;
		int yoff;
	} item;
};

*/



#endif

