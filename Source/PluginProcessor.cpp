#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
LondonClockTAudioProcessor::LondonClockTAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

LondonClockTAudioProcessor::~LondonClockTAudioProcessor()
{
}

//==============================================================================
const String LondonClockTAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LondonClockTAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LondonClockTAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double LondonClockTAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LondonClockTAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LondonClockTAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LondonClockTAudioProcessor::setCurrentProgram (int index)
{
	ignoreUnused(index);
}

const String LondonClockTAudioProcessor::getProgramName (int index)
{
	ignoreUnused(index);
    return String();
}

void LondonClockTAudioProcessor::changeProgramName (int index, const String& newName)
{
	ignoreUnused(index, newName);
}

//==============================================================================
void LondonClockTAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
	ignoreUnused(sampleRate, samplesPerBlock);
}

void LondonClockTAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LondonClockTAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void LondonClockTAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
	if (AudioPlayHead* ph = getPlayHead())
	{
		AudioPlayHead::CurrentPositionInfo newTime;

		if (ph->getCurrentPosition(newTime))
		{
			lastPosInfo = newTime;  // Successfully got the current time from the host..
		}

		// Update cloking module parameters
		mClocking.config(newTime.bpm, getSampleRate() );
	}

	midiMessages.clear();

	if (lastPosInfo.isPlaying)
	{
		mClocking.update((unsigned int)lastPosInfo.timeInSamples, buffer.getNumSamples(), midiMessages);
	}
	else
	{
		mClocking.reset();
	}
}

//==============================================================================
bool LondonClockTAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* LondonClockTAudioProcessor::createEditor()
{
    return new LondonClockTAudioProcessorEditor (*this);
}

//==============================================================================
void LondonClockTAudioProcessor::getStateInformation (MemoryBlock& destData)
{
	MemoryOutputStream stream(destData, true);
	mClocking.serialize(stream);
}

void LondonClockTAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	MemoryInputStream stream(data, (size_t)sizeInBytes, false);
	mClocking.unserialize(stream);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LondonClockTAudioProcessor();
}
