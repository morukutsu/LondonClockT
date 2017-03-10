#include "Gui.h"

int currentId = 0;
GuiStore store[128];
int currentDragId = -1;

void startGui()
{
	currentId = 0;
}

bool drawClickableSquare(bool* enabled, int x, int y, Graphics& g, int mx, int my, bool mouseDown, bool mouseClick)
{
	g.setColour(juce::Colour::fromRGB(64, 64, 64));
	g.drawRect(x, y, 12, 12);

	bool hover = (mx >= x && my >= y && mx <= x + 12 && my <= y + 12);
	bool clicked = false;

	if (hover)
	{
		if (*enabled)
			g.setColour(juce::Colour::fromRGB(255, 118 + 40, 118 + 40));
		else
			g.setColour(juce::Colour::fromRGB(165 + 40, 165 + 40, 165 + 40));

		clicked = mouseClick;
	}
	else
	{
		if (*enabled)
			g.setColour(juce::Colour::fromRGB(255, 118, 118));
		else
			g.setColour(juce::Colour::fromRGB(165, 165, 165));
	}

	g.fillRect(x + 1, y + 1, 12 - 2, 12 - 2);

	if (mouseDown && hover && !store[currentId].wasHover)
		clicked = true;

	if (clicked)
		*enabled = !*enabled;

	store[currentId].wasHover = hover;

	currentId++;
	return clicked;
}

bool drawKnobValue(int* value, int min, int max, int x, int y, Graphics& g, int mx, int my, bool isMouseDrag, int mouseDragDistanceY)
{
	bool hover = (mx >= x && my >= y && mx <= x + 32 && my <= y + 18);

	if (hover && isMouseDrag && !store[currentId].isDrag && currentDragId == -1)
	{
		store[currentId].isDrag = true;
		store[currentId].dragStartValue = *value;
		currentDragId = currentId;
	}
	
	bool changed = false;

	if (store[currentId].isDrag && currentDragId == currentId)
	{
		if (isMouseDrag)
		{
			const int scale = max - min;
			const int mouseMaxDisplacement = 200;
			const int unit = mouseMaxDisplacement / scale;

			const int oldValue = *value;
			*value = store[currentId].dragStartValue - (mouseDragDistanceY / unit);

			changed = oldValue != *value;

			if (*value < min)
				*value = min;

			if (*value > max)
				*value = max;
		}
		else
		{
			currentDragId = -1;
		}
	}

	if (!isMouseDrag)
	{
		store[currentId].isDrag = false;
		currentDragId = -1;
	}
	

	g.setColour(juce::Colour::fromRGB(255, 102, 102));

	g.fillRect(x, y, 32, 16);

	char str[32];
	sprintf(str, "%d", *value);

	g.setColour(juce::Colour::fromRGB(69, 69, 69));
	g.drawText(str, x, y, 32, 16, juce::Justification::centred);

	currentId++;
	return changed;
}