#pragma once
#include <JuceHeader.h>
#include "SequencerCell.h"

class SequencerStrip : public juce::Component
{
public:
	SequencerStrip(const int& initialColumns);

	//SequencerStrip(const SequencerStrip& otherSequencerStrip);

	//SequencerStrip(SequencerStrip&& otherSequencerStrip) noexcept;

	//SequencerStrip& operator=(SequencerStrip otherSequencerStrip);
	
	//SequencerStrip& operator=(SequencerStrip&& otherSequencerStrip) noexcept;

	~SequencerStrip();

	void initialiseSequencerStripInvariants();

	void setColumns(const int& newColumns);

	void paint(juce::Graphics& g) override;

	void resized() override;

private:
	juce::Grid grid;
};