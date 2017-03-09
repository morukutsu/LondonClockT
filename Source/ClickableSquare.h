#ifndef CLICKABLESQUARE_H_INCLUDED
#define CLICKABLESQUARE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

struct GuiStore
{
	int id;
	bool isDrag, wasHover;
	int dragStartValue;
};

void startGui();
bool drawClickableSquare(bool* enabled, int x, int y, Graphics& g, int mx, int my, bool mouseDown, bool mouseClick);
bool drawKnobValue(int* value, int min, int max, int x, int y, Graphics& g, int mx, int my, bool isMouseDrag, int mouseDragDistanceY);



#endif  // CLICKABLESQUARE_H_INCLUDED
