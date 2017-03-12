#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "Gui.h"

//==============================================================================
LondonClockTAudioProcessorEditor::LondonClockTAudioProcessorEditor (LondonClockTAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(500, 300);
	startTimerHz(60);

	/*addAndMakeVisible(btn);
	btn.setButtonText("test");
	btn.setBounds(0, 10, 60, 20);
	btn.setOpaque(true);*/

	isMouseDown = isMouseClicked = isMouseDrag = false;
	editChannelId = -1;
}

LondonClockTAudioProcessorEditor::~LondonClockTAudioProcessorEditor()
{
}

void LondonClockTAudioProcessorEditor::buttonClicked(Button* button)
{
}

//==============================================================================
void LondonClockTAudioProcessorEditor::paint (Graphics& g)
{
	startGui();

	juce::Point<int> mousePos = getMouseXYRelative();
    g.fillAll (juce::Colour::fromRGB(69, 69, 69) );

	int currentY = 0;

	// Draw all the rhythms controllers
	const int rhythmHeight = 22;
	for (int i = 0; i < processor.mClocking.mRhythmsCount; i++)
	{
		if (editChannelId == -1)
			paintRhythm(i * rhythmHeight, g, processor.mClocking.mRhythms[i], i);

		// Edit
		bool isEdit = editChannelId == i;
		if (drawButton(&isEdit, "EDIT", 438, i * rhythmHeight + 20, g, mousePos.x, mousePos.y, isMouseButtonDown(), isMouseClicked))
		{
			if (editChannelId == i)
				editChannelId = -1; // Disable edit mode when pressing edit mode if on
			else
				editChannelId = i;
		}
	}

	// Edit mode
	if (editChannelId != -1)
	{
		paintRhythm(0, g, processor.mClocking.mRhythms[editChannelId], editChannelId);
		paintEditMode(40, g, processor.mClocking.mRhythms[editChannelId], editChannelId);
	}

	currentY += (processor.mClocking.mRhythmsCount + 1) * rhythmHeight;

	if (editChannelId == -1)
	{
		// Additional controls at the bottom
		if (drawKnobValue(&processor.mClocking.mRhythmsCount, 1, 16, 10, currentY, NULL, g, mousePos.x, mousePos.y, isMouseDrag, mouseDragDistanceY))
		{
			processor.mClocking.mRhythms[processor.mClocking.mRhythmsCount - 1].enabled = true;
		}
	}

	isMouseClicked = false;
}

void LondonClockTAudioProcessorEditor::paintRhythm(int yPos, Graphics& g, Rhythm& rhythm, int rhythmIndex)
{
	juce::Point<int> mousePos = getMouseXYRelative();

	float x = 10;
	float y = yPos + 20;
	float width = 300;
	float height = 16;
	char buf[32];

	// Channel status
	sprintf(buf, "%d %s", (rhythmIndex + 1), rhythm.enabled ? "ON" : "OFF");
	drawButton(&rhythm.enabled, buf, x, y, g, mousePos.x, mousePos.y, isMouseButtonDown(), isMouseClicked);
	x += 38;

	// Staps & divisor
	drawKnobValue((int*)&rhythm.steps, 1, 16, x, y, NULL, g, mousePos.x, mousePos.y, isMouseDrag, mouseDragDistanceY);
	x += 34;

	drawKnobValue((int*)&rhythm.divisor, 1, 16, x, y, NULL, g, mousePos.x, mousePos.y, isMouseDrag, mouseDragDistanceY);

	// Background and progess bar
	float xBar = 38;
	x = x + xBar;

	g.setColour(juce::Colour::fromRGB(165, 165, 165));
	g.fillRect(x, y, width, height);

	unsigned int completeLoopTime = rhythm.loopTime * rhythm.steps;
	float progressWidth = (width / (completeLoopTime)) * fmodf(rhythm.time, completeLoopTime);

	g.setColour(juce::Colour::fromRGB(229, 225, 143));
	g.fillRect(x, y, progressWidth, height);

	// Steps
	for (unsigned int i = 0; i < rhythm.steps; i++)
	{
		float stepX = i * (width / rhythm.steps);
		drawClickableSquare(&rhythm.stepList[i].enabled, 2 + x + stepX, 2 + y, g, mousePos.x, mousePos.y, isMouseButtonDown(), isMouseClicked);
	}

	x += 300;

	// Other tmp controls
	/*noteToStr(buf, rhythm.midiNote);
	drawKnobValue((int*)&rhythm.midiNote, 0, 127, x, y, buf, g, mousePos.x, mousePos.y, isMouseDrag, mouseDragDistanceY);

	x += 38;*/

	

	x += 38;
}

void LondonClockTAudioProcessorEditor::paintEditMode(int yPos, Graphics& g, Rhythm& rhythm, int index)
{
	juce::Point<int> m = getMouseXYRelative();

	float x = 119;
	float y = yPos;
	float width = 301;
	float height = 150;

	// BG
	g.setColour(juce::Colour::fromRGB(165, 165, 165));
	g.fillRect(x, y, width, height);

	// VEL EDITOR
	for (unsigned int i = 0; i < rhythm.steps; i++)
	{
		float stepX = i * (width / rhythm.steps);
		float level = (height / 128) * rhythm.stepList[i].level;

		float squareX = stepX + x;
		float squareY = y + (height - level);
		float squareW = width / rhythm.steps;
		float squareH = level;

		bool hover = (m.x >= squareX && m.y >= y && m.x <= squareX + squareW && m.y <= squareY + height);
		if (!hover)
			g.setColour(juce::Colour::fromRGB(255, 118, 118));
		else
			g.setColour(juce::Colour::fromRGB(255, 118 + 40, 118 + 40));

		// Modification of vel by mouse
		if (hover && isMouseButtonDown())
		{
			int l = (m.y - y) * (128 / height);
			if (l < 0)
				l = 0;
			if (l > 127)
				l = 127;

			l = 127 - l;
			rhythm.stepList[i].level = l;
		}

		g.fillRect(squareX, squareY, squareW, squareH);
	}

	// Lines
	g.setColour(juce::Colour::fromRGB(64, 64, 64));
	for (unsigned int i = 0; i < rhythm.steps; i++)
	{
		float stepX = i * (width / rhythm.steps);
		g.drawVerticalLine((int)stepX + x, yPos, yPos + height);
	}
}

void LondonClockTAudioProcessorEditor::resized()
{
}

void LondonClockTAudioProcessorEditor::timerCallback()
{
	repaint();
}

void LondonClockTAudioProcessorEditor::mouseDown(const MouseEvent &event)
{
	isMouseDown = true;
}

void LondonClockTAudioProcessorEditor::mouseUp(const MouseEvent &event)
{
	if (isMouseDown)
	{
		isMouseClicked = true;
	}

	isMouseDown = false;
	isMouseDrag = false;
}

void LondonClockTAudioProcessorEditor::mouseDrag(const MouseEvent &event)
{
	isMouseDrag = true;

	mouseDragDistanceY = event.getDistanceFromDragStartY();
}
