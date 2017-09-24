#include "Gui.h"
#include <unordered_map>

int currentId = 0;
int currentDragId = -1;
std::string currentDragUUID = "";
bool isMouseOverKnob = false;
std::unordered_map<unsigned int, GuiStore> guiStore;

GuiStore& getGuiElement(unsigned int id)
{
	return guiStore[id];
}

unsigned int ImHash(const void* data, int data_size, unsigned int seed)
{
	static unsigned int crc32_lut[256] = { 0 };
	if (!crc32_lut[1])
	{
		const unsigned int polynomial = 0xEDB88320;
		for (unsigned int i = 0; i < 256; i++)
		{
			unsigned int crc = i;
			for (unsigned int j = 0; j < 8; j++)
				crc = (crc >> 1) ^ (unsigned int(-int(crc & 1)) & polynomial);
			crc32_lut[i] = crc;
		}
	}

	seed = ~seed;
	unsigned int crc = seed;
	const unsigned char* current = (const unsigned char*)data;

	if (data_size > 0)
	{
		// Known size
		while (data_size--)
			crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ *current++];
	}
	else
	{
		// Zero-terminated string
		while (unsigned char c = *current++)
		{
			// We support a syntax of "label###id" where only "###id" is included in the hash, and only "label" gets displayed.
			// Because this syntax is rarely used we are optimizing for the common case.
			// - If we reach ### in the string we discard the hash so far and reset to the seed.
			// - We don't do 'current += 2; continue;' after handling ### to keep the code smaller.
			if (c == '#' && current[0] == '#' && current[1] == '#')
				crc = seed;
			crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ c];
		}
	}
	return ~crc;
}

unsigned int getId(char* label)
{
	if (label == NULL)
		return currentId;
	else
		return ImHash(label, strlen(label), 1234);
}

void startGui()
{
	currentId = 0;
	isMouseOverKnob = false;
}

bool getInteractionsDisabled()
{
	return currentDragId != -1;
}

bool getIsMouseOverKnob()
{
	return isMouseOverKnob;
}

bool drawClickableSquare(bool* enabled, int x, int y, Graphics& g, int mx, int my, bool mouseDown, bool mouseClick, bool mousePressed)
{
	ignoreUnused(mouseClick);

	GuiStore& guiElement = getGuiElement(currentId);

	g.setColour(GREY_BG_COLOR);
	g.drawRect(x, y, 12, 12);

	bool disableInteractions = currentDragId != -1;
	bool hover = (mx >= x && my >= y && mx <= x + 12 && my <= y + 12);
	bool clicked = false;

	if (hover && !disableInteractions)
	{
		if (*enabled)
			g.setColour(HIGHLIGHT_HOVER_COLOR);
		else
			g.setColour(LIGHT_GREY_HOVER_COLOR);

		clicked = mousePressed;
	}
	else
	{
		if (*enabled)
			g.setColour(HIGHLIGHT_COLOR);
		else
			g.setColour(LIGHT_GREY_COLOR);
	}

	g.fillRect(x + 1, y + 1, 12 - 2, 12 - 2);

	if (mouseDown && hover && !guiElement.wasHover && !disableInteractions)
		clicked = true;

	if (clicked)
		*enabled = !*enabled;

	guiElement.wasHover = hover;

	currentId++;
	return clicked;
}

bool drawKnobValue(int* value, int min, int max, int x, int y, int w, int h, char* text, Graphics& g, int mx, int my, bool isMouseDrag, int mouseDragDistanceY, std::string key)
{
	GuiStore& guiElement = getGuiElement(currentId);

	bool hover = (mx >= x && my >= y && mx <= x + w && my <= y + h);
	if ((!isMouseOverKnob && hover) || currentDragId != -1)
		isMouseOverKnob = true;

	if (getInteractionsDisabled())
		hover = false;

	// Restore currentDragId using UUID
	if (currentDragUUID != "" && key != "" && isMouseDrag)
	{
		if (key == currentDragUUID)
		{
			GuiStore& dragElement = getGuiElement(currentDragId);
			if (currentDragId != -1)
				dragElement.isDrag = false;

			guiElement.isDrag = true;
			currentDragId = currentId;
		}
	}

	if (hover && isMouseDrag && !guiElement.isDrag && currentDragId == -1)
	{
		guiElement.isDrag = true;
		guiElement.dragStartValue = *value;
		currentDragId = currentId;

		if (key != "")
			currentDragUUID = key;
	}
	
	bool changed = false;

	if (guiElement.isDrag && currentDragId == currentId)
	{
		if (isMouseDrag)
		{
			const int scale = std::min(max - min, 16);
			const int mouseMaxDisplacement = 200;
			const int unit = mouseMaxDisplacement / scale;

			const int oldValue = *value;
			*value = guiElement.dragStartValue - (mouseDragDistanceY / unit);

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
		guiElement.isDrag = false;
		currentDragId = -1;

		if (key != "")
			currentDragUUID = "";
	}
	
	if (hover || currentDragId == currentId)
		g.setColour(HIGHLIGHT_HOVER_COLOR);
	else
		g.setColour(HIGHLIGHT_COLOR);

	g.fillRect(x, y, w, h);

	g.setColour(GREY_BG_COLOR);

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

bool drawButton(char* label, bool* enabled, char* text, int x, int y, int w, int h, Graphics& g, int mx, int my, bool mouseDown, bool mouseClick, bool mousePressed)
{
	ignoreUnused(mouseClick);

	GuiStore& guiElement = getGuiElement(getId(label) );

	bool disableInteractions = currentDragId != -1;
	bool hover = (mx >= x && my >= y && mx <= x + w && my <= y + h);
	bool clicked = false;

	if (hover && !disableInteractions)
	{
		if (*enabled)
			g.setColour(HIGHLIGHT_HOVER_COLOR);
		else
			g.setColour(LIGHT_GREY_HOVER_COLOR);

		clicked = mousePressed;
	}
	else
	{
		if (*enabled)
			g.setColour(HIGHLIGHT_COLOR);
		else
			g.setColour(LIGHT_GREY_COLOR);
	}

	if (mouseDown && hover && !guiElement.wasHover && !disableInteractions)
		clicked = true;

	if (clicked)
		*enabled = !*enabled;

	guiElement.wasHover = hover;

	// Draw code
	g.fillRect(x, y, w, h);

	g.setColour(GREY_BG_COLOR);

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