#include "SequencerStrip.h"
SequencerStrip::SequencerStrip(const int& initialColumns)
{
    using Grid = juce::Grid;
    grid.setGap(Grid::Px(0)); //the grid has no gaps between cells
    grid.autoFlow = Grid::AutoFlow::column;
    grid.autoRows = Grid::Fr(1);
    grid.autoColumns = Grid::Fr(1);

    grid.templateRows.add(Grid::Fr(1));

    grid.templateColumns.ensureStorageAllocated(initialColumns);
    grid.templateColumns.minimiseStorageOverheads();

    grid.items.ensureStorageAllocated(initialColumns);
    grid.items.minimiseStorageOverheads();

    for (auto column{ 0 }; column != initialColumns; ++column)
    {
        grid.templateColumns.add(grid.autoColumns);

        grid.items.add(new SequencerCell);

        auto item{ grid.items.getLast().associatedComponent };
        addAndMakeVisible(item);
        item->addMouseListener(this, true);
    }
}

SequencerStrip::~SequencerStrip()
{
    for (auto& item : grid.items)
        delete item.associatedComponent;
}

void SequencerStrip::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::black);
}

void SequencerStrip::resized()
{
	grid.performLayout(getLocalBounds());
}