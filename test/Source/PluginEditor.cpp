#include "PluginProcessor.h"
#include "PluginEditor.h"

TestAudioProcessorEditor::TestAudioProcessorEditor(TestAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    //setWantsKeyboardFocus(true);
    //addKeyListener(this);

    addAndMakeVisible(sequencerPanel);
    addAndMakeVisible(alphaSequencerStrip);
    addAndMakeVisible(betaSequencerStrip);

    prepare(addVisibleRow);
    prepare(removeVisibleRow);
    prepare(setRepeats);
    prepare(insertColumn);
    prepare(removeColumn);
    prepare(setColumns);

    setSize(CONSTANTS::WINDOW_WIDTH, CONSTANTS::WINDOW_HEIGHT);
}

TestAudioProcessorEditor::~TestAudioProcessorEditor()
{
    addVisibleRow.removeListener(this);
    removeVisibleRow.removeListener(this);
    setRepeats.removeListener(this);
    insertColumn.removeListener(this);
    removeColumn.removeListener(this);
    setColumns.removeListener(this);
}

//==============================================================================
void TestAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::pink);
}

void TestAudioProcessorEditor::resized()
{
    addVisibleRow.setBounds(10, 40, 100, 20);
    removeVisibleRow.setBounds(100, 10, 100, 20);
    setRepeats.setBounds(100, 40, 100, 20);
    insertColumn.setBounds(200, 10, 100, 20);
    removeColumn.setBounds(200, 40, 100, 20);
    setColumns.setBounds(300, 40, 100, 20);

    const auto& localBounds{ getLocalBounds() };
    const auto& localHeight{ localBounds.getHeight() };

    auto alphaSequencerStripBounds{ localBounds };
    alphaSequencerStripBounds.setY(localHeight / 6);
    alphaSequencerStripBounds.setHeight(localHeight / 6);

    alphaSequencerStrip.setBounds(alphaSequencerStripBounds);
    betaSequencerStrip.setBounds(alphaSequencerStripBounds.withY(2 * localHeight / 3));

    auto tiltSequencerPanelBounds{ localBounds };
    tiltSequencerPanelBounds.setY(localHeight / 3);
    tiltSequencerPanelBounds.setHeight(localHeight / 3);

    sequencerPanel.setBounds(tiltSequencerPanelBounds);
}

void TestAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &addVisibleRow)
    {
        const auto newVisibleRows{ sequencerPanel.getVisibleRows() == sequencerPanel.rowsSize()
            ? sequencerPanel.getVisibleRows() : sequencerPanel.getVisibleRows() + 1 };
        sequencerPanel.setNumberOfVisibleRows(newVisibleRows);
	}
    if (button == &removeVisibleRow)
    {
        const auto newVisibleRows{ sequencerPanel.getVisibleRows() == 1
            ? 1 : sequencerPanel.getVisibleRows() - 1 };
        sequencerPanel.setNumberOfVisibleRows(newVisibleRows);
    }
    if (button == &setRepeats)
    {
        sequencerPanel.setRepeats(12);
    }
    if (button == &insertColumn)
    {
        //const auto startPosition{  };
        sequencerPanel.insertColumn(((1 + juce::Random::getSystemRandom().nextFloat()) / 3) * sequencerPanel.getRepeats());
    }
    if (button == &removeColumn)
    {
        if (sequencerPanel.baseColumnsSize() > 1)
        {
            const auto indexBasePart{ juce::Random::getSystemRandom().nextInt(sequencerPanel.baseColumnsSize() - 1) + 1 };
            const auto indexRepeatPart{ juce::Random::getSystemRandom().nextInt(sequencerPanel.getRepeats()) * sequencerPanel.baseColumnsSize() };
            sequencerPanel.removeColumn(indexBasePart + indexRepeatPart);
        }
    }
    if (button == &setColumns)
    {
        juce::Array<float> newStartPositions;
        newStartPositions.ensureStorageAllocated(sequencerPanel.baseColumnsSize() - 1);
        for (auto i{ 0 }; i != sequencerPanel.baseColumnsSize() - 1; ++i)
            newStartPositions.add(((1 + juce::Random::getSystemRandom().nextFloat()) / 3));

        std::sort(newStartPositions.begin(), newStartPositions.end());
        sequencerPanel.shiftStartPositions(newStartPositions);
    }
}

void TestAudioProcessorEditor::prepare(juce::Button& button)
{
    button.addListener(this);
    addAndMakeVisible(button);
}