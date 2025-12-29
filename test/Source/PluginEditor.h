#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SequencerPanel.h"
#include "SequencerStrip.h"
#include "Globals.h"

class TestAudioProcessorEditor  : public juce::AudioProcessorEditor
                                , private juce::Button::Listener
                                //, private juce::KeyListener
{
public:
    TestAudioProcessorEditor (TestAudioProcessor&);
    ~TestAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;

private:
    TestAudioProcessor& audioProcessor;
    SequencerPanel sequencerPanel{ 8 };
    SequencerStrip alphaSequencerStrip{ 3 },
                   betaSequencerStrip{ 4 };

    //to implement these I will need to make several getter functions
    juce::TextButton addVisibleRow{ "add one to visibleRows" },
                     removeVisibleRow{ "take one from visibleRows" },
                     setRepeats{ "setRepeats" },
                     insertColumn{ "insertColumn" },
                     removeColumn{ "removeColumn" },
                     setColumns{ "setColumns" };

    void prepare(juce::Button& button);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestAudioProcessorEditor)
};