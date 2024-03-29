#ifndef CLOCKING_H_INCLUDED
#define CLOCKING_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <list>

const int MAX_RHYTHMS = 16;
const int MAX_STEPS = 16;

#define PLUGIN_MAGIC "LDCT"

enum PluginFormatVersion
{
	V0 = 0,
	V1 = 1,
};

struct NoteOff
{
	unsigned int cc, timestamp;
};

struct Step
{
	bool enabled;
	int cc, level, duration, offset;
};

enum StepValueType
{
	LEVEL,
	OFFSET,
	STEP_VALUE_TYPE_COUNT
};

class Rhythm
{
public:
	Rhythm();

	void reset();
	void config(double bpm, double sampleRate);
	void update(unsigned int transportSamplePos, unsigned int numSamples);
	void tick(unsigned int currentSample, unsigned int timer, MidiBuffer& midi);
	void sync();

	void stepValueSetter(StepValueType type, int index, int value);
	int  stepValueGetter(StepValueType type, int index);

	void serialize(MemoryOutputStream& stream);
	void unserialize(MemoryInputStream& stream);

public:
	bool enabled;
	unsigned int loopTime;
	double loopTimeSecs;
	unsigned int time;
	unsigned int nextSampleTimestamp, prevSampleTimestamp;
	unsigned int midiNote, midiLevel;
	unsigned int steps, divisor;

	Step stepList[MAX_STEPS];
	unsigned int nextStep;

private:
	std::list<NoteOff> noteOffs;
};


class Clocking
{
public:
	Clocking();

	void reset();
	void update(unsigned int transportSamplePos, unsigned int numSamples, MidiBuffer& midi);
	void config(double bpm, double sampleRate);

	void serialize(MemoryOutputStream& stream);
	void unserialize(MemoryInputStream& stream);
	PluginFormatVersion detectFormatVersion(MemoryInputStream& stream);

public:
	unsigned int mTime;
	double mBpm, mSampleRate;

	Rhythm mRhythms[MAX_RHYTHMS];
	int mRhythmsCount;
};

#endif  // CLOCKING_H_INCLUDED
