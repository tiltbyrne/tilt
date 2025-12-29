#include "SequencerCell.h"

SequencerCell::SequencerCell()
{
    setInterceptsMouseClicks(false, false);
}

SequencerCell::SequencerCell(const SequencerCell& cell)
    : state{ cell.getState() }
    , isLeftConnected{ cell.getIsLeftConnected() }
    , isRightConnected{ cell.getIsRightConnected() }
{
    //setName(cell.getName());
    setInterceptsMouseClicks(false, false);
}

SequencerCell* SequencerCell::setCell(const SequencerCell& cell)
{
    //setName(cell.getName());

    return setState(cell.getState())->
        setIsLeftConnected(cell.getIsLeftConnected())->
        setIsRightConnected(cell.getIsRightConnected());
}



void SequencerCell::paint(juce::Graphics& g)
{
    using namespace juce;
    const auto localBounds{ getLocalBounds() };

    Path outline;
    const auto outlineReduction{ 2 };
    const auto cornerSize{ 3 };
    outline.addRoundedRectangle(!isLeftConnected * outlineReduction - isLeftConnected,
        outlineReduction,
        localBounds.getWidth() - (!isLeftConnected + !isRightConnected) * outlineReduction
        + isLeftConnected + isRightConnected,
        localBounds.getHeight() - 2 * outlineReduction,
        cornerSize, cornerSize,
        !isLeftConnected, !isRightConnected,
        !isLeftConnected, !isRightConnected);

    const auto outlineColour{ Colours::white };
    g.setColour(outlineColour);
    g.strokePath(outline, PathStrokeType(1.f));

    const auto currentColour{ isSelected ? Colours::antiquewhite
                                         : state ? Colours::lightcoral
                                                 : Colours::darkgrey };

    g.setColour(currentColour.brighter(!isSelected * mouseIsOverCell * 0.4));
    g.fillPath(outline);
}

SequencerCell* SequencerCell::turnOff()
{
    return setState(off)->setIsLeftConnected(false)->setIsRightConnected(false);
}