
#include "../nx.h"
#include "message.h"
#include "message.fdh"
using namespace Options;
extern FocusStack optionstack;
extern int last_sdl_key;	// from inputs.cpp

#define MESSAGE_X		((Graphics::SCREEN_WIDTH / 2) - 112)
#define MESSAGE_Y		((Graphics::SCREEN_HEIGHT / 2) - 30)
#define MESSAGE_W		244
#define MESSAGE_H		48

Message::Message(const char *msg, const char *msg2)
{
	rawKeyReturn = NULL;
	on_dismiss = NULL;
	last_sdl_key = -1;
	
	fMsg = strdup(msg);
	fMsg2 = strdup(msg2 ? msg2 : "");
	
	fMsgX = MESSAGE_X + ((MESSAGE_W / 2) - (GetFontWidth(fMsg, 0) / 2));
	
	if (fMsg2[0])
	{
		fMsgY = MESSAGE_Y + 10;
		fMsg2X = MESSAGE_X + ((MESSAGE_W / 2) - (GetFontWidth(fMsg2, 0) / 2));
		fMsg2Y = fMsgY + GetFontHeight() + 4;
		fShowDelay = 0;
	}
	else
	{
		fMsgY = (MESSAGE_Y + ((MESSAGE_H / 2) - (GetFontHeight() / 2))) - 1;
		fShowDelay = 4;
	}
	
	optionstack.AddItem(this);
}

Message::~Message()
{
	optionstack.RemoveItem(this);
	free(fMsg);
	free(fMsg2);
}

/*
void c------------------------------() {}
*/

void Message::Draw()
{
	TextBox::DrawFrame(MESSAGE_X, MESSAGE_Y, MESSAGE_W, MESSAGE_H);
	
	if (fShowDelay > 0)
	{
		fShowDelay--;
		return;
	}
	
	font_draw(fMsgX, fMsgY, fMsg, 0);
	
	if (fMsg2[0])
		font_draw(fMsg2X, fMsg2Y, fMsg2, 0);
}


void Message::RunInput()
{
	if (last_sdl_key != -1)
	{
		if (rawKeyReturn) *rawKeyReturn = last_sdl_key;
		if (on_dismiss)   (*on_dismiss)(this);
		
		delete this;
	}
}




