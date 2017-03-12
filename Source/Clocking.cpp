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
	for (int i = 0; i < mRhythmsCount; i++)
	{
		if (!mRhythms[i].enabled)
			continue;

		mRhythms[i].config(mBpm, mSampleRate);

		for (unsigned int k = 0; k < numSamples; k++)
		{
			unsigned int timer = k + mTime;
			mRhythms[i].tick(k, timer, midi);
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
	stream.writeInt(mRhythmsCount);
	for (int i = 0; i < mRhythmsCount; i++)
		mRhythms[i].serialize(stream);
}

void Clocking::unserialize(MemoryInputStream& stream)
{
	mRhythmsCount = stream.readInt();
	for (int i = 0; i < mRhythmsCount; i++)
		mRhythms[i].unserialize(stream);
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
	}
}

void Rhythm::reset()
{
	time = 0;
	nextSampleTimestamp = 0;
	nextStep = 0;
}

void Rhythm::config(double bpm, double sampleRate)
{
	double beatsPerSec = bpm / 60.0;
	loopTime = (unsigned int)(1.0 / beatsPerSec * sampleRate);

	loopTime = loopTime * ((float)divisor / steps);

	sync();
}

void Rhythm::update(unsigned int transportSamplePos, unsigned int numSamples)
{
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
			midi.addEvent(MidiMessage::noteOn(1, stepList[stepId].cc, (uint8)stepList[stepId].level), currentSample);
			noteOffs.push_back({ (unsigned int)stepList[stepId].cc, nextSampleTimestamp + loopTime / 4 });
		}

		nextSampleTimestamp = timer + loopTime;
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
	unsigned int completeLoopTime = loopTime * steps;
	unsigned int progress = time % completeLoopTime;

	nextStep = (unsigned int)ceil(steps * (progress / (float)completeLoopTime));

	unsigned int sequenceStartTimestamp = time - progress;
	nextSampleTimestamp = sequenceStartTimestamp + nextStep * loopTime;
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
