//------------------------------------------------------------------------
//  Author: Paul Roberts 2017
//------------------------------------------------------------------------

#ifndef _VIRTUALJOYPAD_H
#define _VIRTUALJOYPAD_H

#include <SDL.h>

struct JoyPad
{
	bool JoyPadUp;
	bool JoyPadDown;
	bool JoyPadRight;
	bool JoyPadLeft;

	JoyPad()
		: JoyPadUp(true)
		, JoyPadDown(false)
		, JoyPadLeft(false)
		, JoyPadRight(false)
	{
	}
};

//--------------------------------------------------------------------------------------------------
class VirtualJoypad
{
	
//--------------------------------------------------------------------------------------------------
public:
	~VirtualJoypad();

	static VirtualJoypad* Instance();

	void SetJoypadState(SDL_Event e);

	void SetJoypadState(JoyPad e, bool keyDown = true);

//--------------------------------------------------------------------------------------------------
private:
	VirtualJoypad();

//--------------------------------------------------------------------------------------------------
private:
	static VirtualJoypad* mInstance;

public:
	bool LeftArrow;
	bool RightArrow;
	bool UpArrow;
	bool DownArrow;
};

//--------------------------------------------------------------------------------------------------
#endif //_VIRTUALJOYPAD_H