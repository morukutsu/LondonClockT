#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class LondonClockTAudioProcessorEditor  : public AudioProcessorEditor, public Timer, public Button::Listener
{
public:
    LondonClockTAudioProcessorEditor (LondonClockTAudioProcessor&);
    ~LondonClockTAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
	void timerCallback() override;

	void paintRhythm(int y, Graphics& g, Rhythm& rhythm, int index);

	void buttonClicked(Button* button) override;
	void mouseDown(const MouseEvent &event) override;
	void mouseUp(const MouseEvent &event) override;
	void mouseDrag(const MouseEvent &event) override;

	bool isMouseDown, isMouseClicked, isMouseDrag;
	int mouseDragDistanceY;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LondonClockTAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LondonClockTAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
