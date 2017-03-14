#include "Gui.h"

int currentId = 0;
GuiStore store[128];
int currentDragId = -1;
std::string currentDragUUID = "";

void startGui()
{
	currentId = 0;
}

bool drawClickableSquare(bool* enabled, int x, int y, Graphics& g, int mx, int my, bool mouseDown, bool mouseClick)
{
	g.setColour(juce::Colour::fromRGB(64, 64, 64));
	g.drawRect(x, y, 12, 12);

	bool disableInteractions = currentDragId != -1;
	bool hover = (mx >= x && my >= y && mx <= x + 12 && my <= y + 12);
	bool clicked = false;

	if (hover && !disableInteractions)
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

	if (mouseDown && hover && !store[currentId].wasHover && !disableInteractions)
		clicked = true;

	if (clicked)
		*enabled = !*enabled;

	store[currentId].wasHover = hover;

	currentId++;
	return clicked;
}

bool drawKnobValue(int* value, int min, int max, int x, int y, int w, int h, char* text, Graphics& g, int mx, int my, bool isMouseDrag, int mouseDragDistanceY, std::string key)
{
	bool hover = (mx >= x && my >= y && mx <= x + w && my <= y + h);

	// Restore currentDragId using UUID
	if (currentDragUUID != "" && key != "" && isMouseDrag)
	{
		if (key == currentDragUUID)
		{
			if (currentDragId != -1)
				store[currentDragId].isDrag = false;
			store[currentId].isDrag = true;
			currentDragId = currentId;
		}
	}

	if (hover && isMouseDrag && !store[currentId].isDrag && currentDragId == -1)
	{
		store[currentId].isDrag = true;
		store[currentId].dragStartValue = *value;
		currentDragId = currentId;

		if (key != "")
			currentDragUUID = key;
	}
	
	bool changed = false;

	if (store[currentId].isDrag && currentDragId == currentId)
	{
		if (isMouseDrag)
		{
			const int scale = std::min(max - min, 16);
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

			if (key != "")
				currentDragUUID = "";
		}
	}

	if (!isMouseDrag)
	{
		store[currentId].isDrag = false;
		currentDragId = -1;

		if (key != "")
			currentDragUUID = "";
	}
	
	if (hover || currentDragId == currentId)
		g.setColour(juce::Colour::fromRGB(255, 102 + 40, 102 + 40));
	else
		g.setColour(juce::Colour::fromRGB(255, 102, 102));

	g.fillRect(x, y, w, h);

	g.setColour(juce::Colour::fromRGB(69, 69, 69));

	char str[32];
	if (text == NULL)
	{
		sprintf(str, "%d", *value);
		g.drawText(str, x, y, w, h, juce::Justification::centred);
	}
	else
	{
		g.drawText(text, x, y, w, h, juce::Justification::centred);
	}

	currentId++;
	return changed;
}

bool drawButton(bool* enabled, char* text, int x, int y, int w, int h, Graphics& g, int mx, int my, bool mouseDown, bool mouseClick)
{
	bool disableInteractions = currentDragId != -1;
	bool hover = (mx >= x && my >= y && mx <= x + w && my <= y + h);
	bool clicked = false;

	if (hover && !disableInteractions)
	{
		if (*enabled)
			g.setColour(juce::Colour::fromRGB(255, 102 + 40, 102 + 40));
		else
			g.setColour(juce::Colour::fromRGB(165 + 40, 165 + 40, 165 + 40));

		clicked = mouseClick;
	}
	else
	{
		if (*enabled)
			g.setColour(juce::Colour::fromRGB(255, 102, 102));
		else
			g.setColour(juce::Colour::fromRGB(165, 165, 165));
	}

	if (mouseDown && hover && !store[currentId].wasHover && !disableInteractions)
		clicked = true;

	if (clicked)
		*enabled = !*enabled;

	store[currentId].wasHover = hover;

	// Draw code
	g.fillRect(x, y, w, h);

	g.setColour(juce::Colour::fromRGB(69, 69, 69));

	g.drawText(text, x, y, w, h, juce::Justification::centred);

	currentId++;
	return clicked;
}

void noteToStr(char* str, int note)
{
	char* notes[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	if (note >= 0 && note <= 127)
	{
		int octave    = note / 12;
		int noteIndex = note % 12;

		sprintf(str, "%s%d", notes[noteIndex], octave - 2);
	}
}