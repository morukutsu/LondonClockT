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
		stepList[i] = true;
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
}

void Rhythm::tick(unsigned int currentSample, unsigned int timer, MidiBuffer& midi)
{
	if (nextSampleTimestamp == timer)
	{
		if (stepList[nextStep])
		{
			midi.addEvent(MidiMessage::noteOff(1, midiNote), currentSample);
			midi.addEvent(MidiMessage::noteOn(1, midiNote, (uint8)midiLevel), currentSample);
		}

		nextSampleTimestamp = timer + loopTime;
		nextStep = (nextStep + 1) % steps;
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


