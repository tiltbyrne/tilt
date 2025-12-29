#pragma once
#include <JuceHeader.h>
#include "SequencerCell.h"

class SequencerStrip : public juce::Component
{
public:
	SequencerStrip(const int& initialColumns);
	
	~SequencerStrip();

	void paint(juce::Graphics& g) override;

	void resized() override;

private:
	juce::Grid grid;

};