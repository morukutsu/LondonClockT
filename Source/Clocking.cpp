/*
  ==============================================================================

    Clocking.cpp
    Created: 1 Mar 2017 12:05:50am
    Author:  Moru

  ==============================================================================
*/

#include "Clocking.h"

Clocking::Clocking()
	: mTime(0)
{
	for (int i = 0; i < MAX_RHYTHMS; i++)
		mRhythms[i].enabled = false;

	mRhythms[0].enabled = true;
	mRhythms[1].enabled = true;

	mRhythmsCount = 2;
}

void Clocking::update(unsigned int transportSamplePos, unsigned int numSamples, MidiBuffer& midi)
{
	for (int i = 0; i < MAX_RHYTHMS; i++)
	{
		if (i >= mRhythmsCount)
			mRhythms[i].enabled = false;

		mRhythms[i].config(mBpm, mSampleRate);

		if (mRhythms[i].enabled)
		{
			for (unsigned int k = 0; k < numSamples; k++)
			{
				unsigned int timer = k + mTime;
				mRhythms[i].tick(k, timer, midi);
			}
		}
		

		mRhythms[i].update(transportSamplePos, numSamples);
	}

	mTime += numSamples;
}

void Clocking::config(double bpm, double sampleRate)
{
	mBpm = bpm;
	mSampleRate = sampleRate;
}

void Clocking::reset()
{
	mTime = 0;

	for (int i = 0; i < MAX_RHYTHMS; i++)
		mRhythms[i].reset();
}

void Clocking::serialize(MemoryOutputStream& stream)
{
	// Write MAGIC and VERSION number
	for (int i = 0; i < 4; i++)
		stream.writeByte(PLUGIN_MAGIC[i]);
	stream.writeInt(static_cast<int>(PluginFormatVersion::V1));

	// Write content
	stream.writeInt(mRhythmsCount);
	for (int i = 0; i < mRhythmsCount; i++)
		mRhythms[i].serialize(stream);
}

void Clocking::unserialize(MemoryInputStream& stream)
{
	PluginFormatVersion version = detectFormatVersion(stream);
	if (version == PluginFormatVersion::V0)
		stream.setPosition(0);

	mRhythmsCount = stream.readInt();
	jassert(mRhythmsCount <= MAX_RHYTHMS);

	for (int i = 0; i < mRhythmsCount; i++)
		mRhythms[i].unserialize(stream);
}

PluginFormatVersion Clocking::detectFormatVersion(MemoryInputStream& stream)
{
	stream.setPosition(0);

	// First format does not have the magic value inside
	char magic[4];
	for (int i = 0; i < 4; i++)
		magic[i] = stream.readByte();

	if (strncmp(PLUGIN_MAGIC, magic, 4) != 0)
		return PluginFormatVersion::V0;

	// Read version from the stream
	PluginFormatVersion version = static_cast<PluginFormatVersion>(stream.readInt());

	return version;
}


/*
	Rhythm class
*/

Rhythm::Rhythm()
	: time(0)
	, loopTime(0)
	, nextSampleTimestamp(0)
	, steps(4)
	, divisor(4)
{
	midiNote = 60;
	midiLevel = 127;
	nextStep = 0;

	for (int i = 0; i < MAX_STEPS; i++)
	{
		stepList[i].enabled = true;
		stepList[i].cc = 60;
		stepList[i].level = 127;
		stepList[i].offset = 0; // 1024 values
	}
}

void Rhythm::reset()
{
	time = 0;
	nextSampleTimestamp = 0;
	nextStep = 0;
	noteOffs.clear();
}

void Rhythm::config(double bpm, double sampleRate)
{
	if (bpm > 0)
	{
		double beatsPerSec = bpm / 60.0;
		if (beatsPerSec > 0)
		{
			loopTimeSecs = (unsigned int)(1.0 / beatsPerSec * sampleRate);

			loopTime = (unsigned int)(loopTimeSecs * ((double)divisor / steps));

			sync();
		}
	}
}

void Rhythm::update(unsigned int transportSamplePos, unsigned int numSamples)
{
	ignoreUnused(transportSamplePos);

	time += numSamples;

	// Clear sent note offs
	for (auto it = noteOffs.begin(); it != noteOffs.end(); )
	{
		if (time > (*it).timestamp)
			noteOffs.erase(it++);
		else
			++it;
	}
}

void Rhythm::tick(unsigned int currentSample, unsigned int timer, MidiBuffer& midi)
{
	int stepId = nextStep % steps;

	if (nextSampleTimestamp == timer)
	{
		if (stepList[stepId].enabled)
		{
			midi.addEvent(MidiMessage::noteOn(1, stepList[stepId].cc, (uint8)stepList[stepId].level), 
				currentSample);

			unsigned int noteOffTimestamp = nextSampleTimestamp + loopTime / 4;
			//noteOffTimestamp += stepList[stepId].offset;

			noteOffs.push_back({ (unsigned int)stepList[stepId].cc, noteOffTimestamp });

			// TODO: set to nextSampleTimestamp + loopTime - 1
			// clear note offs when the transport is stopped
			// send all outstanding note off
		}

		double roundedLoopTime = floor(loopTimeSecs * ((double)divisor / steps));
		nextSampleTimestamp = (unsigned int)(timer + roundedLoopTime);
		nextStep = (nextStep + 1) % steps;
	}

	// Send note offs if needed
	for (auto it = noteOffs.begin(); it != noteOffs.end(); it++)
	{
		if ((*it).timestamp == timer)
			midi.addEvent(MidiMessage::noteOff(1, (*it).cc, (uint8)0), currentSample);
	}
}

void Rhythm::sync()
{
	unsigned int completeLoopTime = (unsigned int)(loopTimeSecs * divisor);

	unsigned int progress = 0;
	if (completeLoopTime != 0)
		progress = time % completeLoopTime;

	if (completeLoopTime != 0)
		nextStep = (unsigned int)ceil(steps * (progress / (double)completeLoopTime));

	double roundedLoopTime = floor(loopTimeSecs * ((double)divisor / steps));

	unsigned int sequenceStartTimestamp = time - progress;
	nextSampleTimestamp = (unsigned int)(sequenceStartTimestamp + nextStep * roundedLoopTime);

	// Add step offset
	/*int stepId = nextStep % steps;
	nextSampleTimestamp += stepList[stepId].offset;*/
}

void Rhythm::stepValueSetter(StepValueType type, int index, int value)
{
	switch (type)
	{
		case LEVEL:
			stepList[index].level = value;
			break;

		case OFFSET:
			stepList[index].offset = value;
			break;
	}
}

int Rhythm::stepValueGetter(StepValueType type, int index)
{
	switch (type)
	{
		case LEVEL:
			return stepList[index].level;
			break;

		case OFFSET:
			return stepList[index].offset;
			break;
	}
}

void Rhythm::serialize(MemoryOutputStream& stream)
{
	stream.writeBool(enabled);
	stream.writeInt(midiNote);
	stream.writeInt(midiLevel);
	stream.writeInt(steps);
	stream.writeInt(divisor);

	for (int i = 0; i < MAX_STEPS; i++)
	{
		stream.writeBool(stepList[i].enabled);
		stream.writeInt(stepList[i].cc);
		stream.writeInt(stepList[i].level);
		stream.writeInt(stepList[i].duration);
	}
}

void Rhythm::unserialize(MemoryInputStream& stream)
{
	enabled   = stream.readBool();
	midiNote  = stream.readInt();
	midiLevel = stream.readInt();
	steps     = stream.readInt();
	divisor   = stream.readInt();

	for (int i = 0; i < MAX_STEPS; i++)
	{
		stepList[i].enabled = stream.readBool();
		stepList[i].cc = stream.readInt();
		stepList[i].level = stream.readInt();
		stepList[i].duration = stream.readInt();
	}
}
