#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "Gui.h"

//==============================================================================
LondonClockTAudioProcessorEditor::LondonClockTAudioProcessorEditor (LondonClockTAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(470, 300);
	startTimerHz(60);

	isMouseDown = isMouseClicked = isMouseDrag = false;
	editChannelId = -1;
	editMode = 0;
	baseCC = 60;
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

	Font& f = g.getCurrentFont();
	f.setBold(true);
	g.setFont(f);

	int currentY = 0;
	int currentX = 10;
	int channelStatusWidth;
	const int EDIT_MODE_WIDTH = 300 + 2;
	const int PADDING_TOP = 10;

	// Draw all the rhythms controllers
	const int rhythmHeight = 20;
	for (int i = 0; i < processor.mClocking.mRhythmsCount; i++)
	{
		channelStatusWidth = paintChannelStatus(currentX, PADDING_TOP + i * rhythmHeight, g, processor.mClocking.mRhythms[i], i);
		 
		if (editChannelId == -1)
			paintRhythm(channelStatusWidth, PADDING_TOP + i * rhythmHeight, g, processor.mClocking.mRhythms[i], i);

		// Edit
		bool isEdit = editChannelId == i;
		if (drawButton(&isEdit, "EDIT", channelStatusWidth + EDIT_MODE_WIDTH, PADDING_TOP + i * rhythmHeight, 36, 16, g, mousePos.x, mousePos.y, isMouseButtonDown(), isMouseClicked))
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
		paintRhythm(channelStatusWidth, PADDING_TOP, g, processor.mClocking.mRhythms[editChannelId], editChannelId);
		paintEditMode(channelStatusWidth, PADDING_TOP + 20, g, processor.mClocking.mRhythms[editChannelId], editChannelId);
	}

	currentY += (processor.mClocking.mRhythmsCount + 1) * rhythmHeight;

	if (editChannelId == -1)
	{
		// Additional controls at the bottom
		bool dummy = true;
		if (drawButton(&dummy, "-", 10, currentY, 38, 16, g, mousePos.x, mousePos.y, isMouseButtonDown(), isMouseClicked))
		{
			if (processor.mClocking.mRhythmsCount > 1)
			{
				processor.mClocking.mRhythmsCount--;
				processor.mClocking.mRhythms[processor.mClocking.mRhythmsCount - 1].enabled = true;
			}
		}

		dummy = true;
		if (drawButton(&dummy, "+", 10 + 40, currentY, 38, 16, g, mousePos.x, mousePos.y, isMouseButtonDown(), isMouseClicked))
		{
			if (processor.mClocking.mRhythmsCount < 16)
			{
				processor.mClocking.mRhythmsCount++;
				processor.mClocking.mRhythms[processor.mClocking.mRhythmsCount - 1].enabled = true;
			}
		}
	}

	isMouseClicked = false;
}

int LondonClockTAudioProcessorEditor::paintChannelStatus(int x, int y, Graphics& g, Rhythm& rhythm, int rhythmIndex)
{
	juce::Point<int> mousePos = getMouseXYRelative();

	const float width = 300;
	const float height = 16;
	char buf[32];

	const int KNOB_WIDTH = 38;
	const int SPACE = 2;
	const int KNOB_WIDTH_SMALL = 32;

	// Channel status
	sprintf(buf, "%d %s", (rhythmIndex + 1), rhythm.enabled ? "ON" : "OFF");
	drawButton(&rhythm.enabled, buf, x, y, KNOB_WIDTH, 16, g, mousePos.x, mousePos.y, isMouseButtonDown(), isMouseClicked);
	x += KNOB_WIDTH + SPACE;

	// Staps & divisor
	drawKnobValue((int*)&rhythm.steps, 1, 16, x, y, KNOB_WIDTH_SMALL, 16, NULL, g, mousePos.x, mousePos.y, isMouseDrag, mouseDragDistanceY);
	x += KNOB_WIDTH_SMALL + SPACE;

	drawKnobValue((int*)&rhythm.divisor, 1, 16, x, y, KNOB_WIDTH_SMALL, 16, NULL, g, mousePos.x, mousePos.y, isMouseDrag, mouseDragDistanceY);
	x += KNOB_WIDTH_SMALL + SPACE;
	
	return x;
}

void LondonClockTAudioProcessorEditor::paintRhythm(int x, int yPos, Graphics& g, Rhythm& rhythm, int rhythmIndex)
{
	juce::Point<int> mousePos = getMouseXYRelative();

	float y = yPos;
	float width = 300;
	float height = 16;
	char buf[32];

	// Background and progess bar
	g.setColour(juce::Colour::fromRGB(165, 165, 165));
	g.fillRect((float)x, y, width, height);

	unsigned int completeLoopTime = rhythm.loopTime * rhythm.steps;
	float progressWidth = (width / (completeLoopTime)) * fmodf(rhythm.time, completeLoopTime);

	g.setColour(juce::Colour::fromRGB(229, 225, 143));
	g.fillRect((float)x, y, progressWidth, height);

	// Steps
	for (unsigned int i = 0; i < rhythm.steps; i++)
	{
		float stepX = i * (width / rhythm.steps);
		drawClickableSquare(&rhythm.stepList[i].enabled, 2 + x + stepX, 2 + y, g, mousePos.x, mousePos.y, isMouseButtonDown(), isMouseClicked);
	}

	x += 300;

}

void LondonClockTAudioProcessorEditor::paintEditMode(int x, int yPos, Graphics& g, Rhythm& rhythm, int index)
{
	juce::Point<int> m = getMouseXYRelative();

	float y = yPos;
	float width = 300;
	float height = 150;

	// BG
	g.setColour(juce::Colour::fromRGB(165, 165, 165));
	g.fillRect((float)x, y, width, height);

	// VEL EDITOR
	if (editMode == 1)
	{
		for (unsigned int i = 0; i < rhythm.steps; i++)
		{
			float stepX = i * (width / rhythm.steps);
			float level = (height / 128) * rhythm.stepList[i].level;

			float squareX = stepX + x;
			float squareY = y + (height - level);
			float squareW = width / rhythm.steps;
			float squareH = level;

			bool hover = (m.x >= squareX && m.y >= y && m.x <= squareX + squareW && m.y <= squareY + height) && !getInteractionsDisabled();
			if (!hover)
				g.setColour(juce::Colour::fromRGB(255, 118, 118));
			else
				g.setColour(juce::Colour::fromRGB(255, 118 + 40, 118 + 40));

			// Modification of vel by mouse
			if (hover && isMouseButtonDown() && !getInteractionsDisabled() )
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
	}
	else if (editMode == 0)
	{
		// NOTE EDITOR
		char buf[32];
		float noteHeight = height / 12;

		// Draw note squares
		g.setColour(juce::Colour::fromRGB(255, 118, 118));
		for (unsigned int i = 0; i < rhythm.steps; i++)
		{
			// Mouse input
			float stepX = i * (width / rhythm.steps);
			float squareX = stepX + x;
			float squareW = width / rhythm.steps;
			float squareH = noteHeight;

			bool hover = (m.x >= squareX && m.y >= y && m.x <= squareX + squareW && m.y <= y + height) && !getInteractionsDisabled();

			if (hover && isMouseButtonDown() && !getInteractionsDisabled())
			{
				int mouseIndex = (m.y - y) / noteHeight;
				rhythm.stepList[i].cc = baseCC - mouseIndex;
			}

			// Check if the note can be displayed in the current screen
			int noteIndex = baseCC - rhythm.stepList[i].cc;
			if (noteIndex >= 0 && noteIndex < 12)
			{
				float squareY = y + noteIndex * noteHeight;

				// Mouse interaction
				if (!hover)
					g.setColour(juce::Colour::fromRGB(255, 118, 118));
				else
					g.setColour(juce::Colour::fromRGB(255, 118 + 40, 118 + 40));

				// Display
				g.fillRect(squareX, squareY, squareW, squareH);
			}
		}

		// Hover with note names
		g.setColour(juce::Colour::fromRGB(64, 64, 64));
		for (int k = 0; k < 12; k++)
		{
			noteToStr(buf, baseCC - k);
			g.drawText(buf, x + 2, y + k * noteHeight, 32, noteHeight, juce::Justification::left);
		}

		// Change base CC
		int octave = (baseCC / 12) - 2;
		sprintf(buf, "OCT %d", octave);
		if (drawKnobValue(&octave, 1, 8, x, y + height + 4 + 18, 40, 16, buf, g, m.x, m.y, isMouseDrag, mouseDragDistanceY))
		{
			baseCC = (octave + 2) * 12;
		}
	}
	

	// Lines
	g.setColour(juce::Colour::fromRGB(128, 128, 128));
	for (unsigned int i = 0; i < rhythm.steps; i++)
	{
		float stepX = i * (width / rhythm.steps);
		g.drawVerticalLine((int)stepX + x, yPos, yPos + height);
	}

	// Buttons
	bool isNoteEdit = editMode == 0;
	bool isVelEdit  = editMode == 1;

	if (drawButton(&isNoteEdit, "NOTE", x, y + height + 4, 40, 16, g, m.x, m.y, isMouseButtonDown(), isMouseClicked))
		editMode = 0;
	
	if (drawButton(&isVelEdit, "VEL", x + 42, y + height + 4, 40, 16, g, m.x, m.y, isMouseButtonDown(), isMouseClicked))
		editMode = 1;
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
