#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class LondonClockTAudioProcessorEditor  : public AudioProcessorEditor, public Timer
{
public:
    LondonClockTAudioProcessorEditor (LondonClockTAudioProcessor&);
    ~LondonClockTAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
	void timerCallback() override;

	int  paintChannelStatus(int x, int y, Graphics& g, Rhythm& rhythm, int index);
	void paintRhythm(int x, int y, Graphics& g, Rhythm& rhythm, int index);
	void paintEditMode(int x, int y, Graphics& g, Rhythm& rhythm, int index);

	void mouseDown(const MouseEvent &event) override;
	void mouseUp(const MouseEvent &event) override;
	void mouseDrag(const MouseEvent &event) override;

	bool isMouseDown, isMouseClicked, isMouseDrag;
	int mouseDragDistanceY;

	int editChannelId, editMode, baseCC;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LondonClockTAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LondonClockTAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
