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
		paintRhythm(i * rhythmHeight, g, processor.mClocking.mRhythms[i], i);

	currentY += (processor.mClocking.mRhythmsCount + 1) * rhythmHeight;

	// Additional controls at the bottom
	if (drawKnobValue(&processor.mClocking.mRhythmsCount, 1, 16, 10, currentY, NULL, g, mousePos.x, mousePos.y, isMouseDrag, mouseDragDistanceY))
	{
		processor.mClocking.mRhythms[processor.mClocking.mRhythmsCount - 1].enabled = true;
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
		drawClickableSquare(&rhythm.stepList[i], 2 + x + stepX, 2 + y, g, mousePos.x, mousePos.y, isMouseButtonDown(), isMouseClicked);
	}

	x += 300;

	// Other tmp controls
	noteToStr(buf, rhythm.midiNote);
	drawKnobValue((int*)&rhythm.midiNote, 0, 127, x, y, buf, g, mousePos.x, mousePos.y, isMouseDrag, mouseDragDistanceY);
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
