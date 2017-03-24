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
bool drawKnobValue(int* value, int min, int max, int x, int y, int w, int h, char* text, Graphics& g, int mx, int my, bool isMouseDrag, int mouseDragDistanceY, std::string key = "");
bool drawButton(bool* enabled, char* text, int x, int y, int w, int h, Graphics& g, int mx, int my, bool mouseDown, bool mouseClick);

bool getInteractionsDisabled();
bool getIsMouseOverKnob();

void noteToStr(char* str, int note);

// Plugin colors
const juce::Colour GREY_BG_COLOR = juce::Colour::fromRGB(69, 69, 69);
const juce::Colour LIGHT_GREY_COLOR = juce::Colour::fromRGB(165, 165, 165);
const juce::Colour HIGHLIGHT_COLOR = juce::Colour::fromRGB(255, 118, 118);
const juce::Colour LIGHT_GREY_HOVER_COLOR = juce::Colour::fromRGB(165 + 40, 165 + 40, 165 + 40);
const juce::Colour HIGHLIGHT_HOVER_COLOR = juce::Colour::fromRGB(255, 118 + 40, 118 + 40);
const juce::Colour MEDIUM_GREY_COLOR = juce::Colour::fromRGB(128, 128, 128);
const juce::Colour LOWLIGHT_COLOR = juce::Colour::fromRGB(229, 225, 143);

#endif  // CLICKABLESQUARE_H_INCLUDED
