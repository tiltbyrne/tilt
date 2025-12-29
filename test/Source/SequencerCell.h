#pragma once
#include <JuceHeader.h>

//a simple class representing a cell in Sequencer Pannel and Strip
class SequencerCell : public juce::Component
{
public:

    enum State
    {
        off = 0,
        on = 1
    };

    SequencerCell();

    SequencerCell(const SequencerCell& cell);

    void paint(juce::Graphics& g) override;

    SequencerCell* setCell(const SequencerCell& cell);

    SequencerCell* setIsLeftConnected(const bool& shouldBeConnected)
    {
        isLeftConnected = shouldBeConnected;
        return this;
    };

    SequencerCell* changeIsLeftConnected() { return setIsLeftConnected(!isLeftConnected); };

    inline bool getIsLeftConnected() const { return isLeftConnected; };

    SequencerCell* setIsRightConnected(const bool& shouldBeConnected)
    {
        isRightConnected = shouldBeConnected;
        return this;
    };

    SequencerCell* changeIsRightConnected() { return setIsRightConnected(!isRightConnected); };

    inline bool getIsRightConnected() const { return isRightConnected; };

    SequencerCell* setState(const State& newState)
    {
        state = newState;
        return this;
    };

    inline State getState() const { return state; };

    inline bool isOn() const { return state == on; };

    SequencerCell* changeState()
    {
        state = (state == off) ? on : off;
        return this;
    };

    SequencerCell* setMouseIsOverCell(const bool& shouldBeOver)
    {
        mouseIsOverCell = shouldBeOver;
        return this;
    };

    inline bool getMouseIsOverCell() const { return mouseIsOverCell; };

    SequencerCell* turnOff();

    inline bool getIsSelected() const { return isSelected; };

    void setIsSelected(const bool& shouldBeSelected) { isSelected = shouldBeSelected; };

    const int edgeWidth{ 3 };

private:
    State state{ off };

    bool mouseIsOverCell{ false },
        isLeftConnected{ false },
        isRightConnected{ false },
        isSelected{ false };
};