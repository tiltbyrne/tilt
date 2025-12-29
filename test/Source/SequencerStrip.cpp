#include "SequencerStrip.h"

SequencerStrip::SequencerStrip(const int& initialColumns)
{
    initialiseSequencerStripInvariants();
    setColumns(initialColumns);
}

/*
SequencerStrip::SequencerStrip(const SequencerStrip& otherSequencerStrip)
{
    initialiseSequencerPanelInvariants();

    handShallowCopying(otherSequencerPanel);

    //deep copy stuff--------------------------------------------------------
    for (auto row{ 0 }; row != rowsSize(); ++row)
    {
        pattern[row].reserve(otherSequencerPanel.columnsSize());

        for (auto& cell : otherSequencerPanel.pattern[row])
            handleAdditionOfCellToPattern(row, std::shared_ptr<SequencerCell>(new SequencerCell(*cell)));
    }

    handleFillingGridItems(numberOfVisibleRows);
}

SequencerStrip::SequencerStrip(SequencerStrip&& otherSequencerStrip) noexcept
{
    initialiseSequencerPanelInvariants();

    handShallowCopying(otherSequencerPanel);

    //deep copy stuff--------------------------------------------------------
    for (auto row{ 0 }; row != rowsSize(); ++row)
    {
        auto& thisRow{ pattern[row] };

        thisRow.clear();
        thisRow.reserve(otherSequencerPanel.columnsSize());

        auto& otherRow{ otherSequencerPanel.pattern[row] };

        thisRow = otherRow;

        otherRow.clear();
    }

    handleFillingGridItems(numberOfVisibleRows);
}

SequencerStrip& SequencerStrip::operator=(SequencerStrip otherSequencerStrip)
{
    if (this != &otherSequencerStrip)
    {
        handShallowCopying(otherSequencerPanel);

        //deep copy stuff--------------------------------------------------------
        for (auto row{ 0 }; row != rowsSize(); ++row)
        {
            auto& thisRow{ pattern[row] };

            thisRow.clear();
            thisRow.reserve(otherSequencerPanel.columnsSize());

            for (auto& cell : otherSequencerPanel.pattern[row])
                handleAdditionOfCellToPattern(row, std::shared_ptr<SequencerCell>(new SequencerCell(*cell)));
        }

        handleFillingGridItems(numberOfVisibleRows);
    }

    return *this;
}

SequencerStrip& SequencerStrip::operator=(SequencerStrip&& otherSequencerStrip) noexcept
{
    if (this != &otherSequencerPanel)
    {
        handShallowCopying(otherSequencerPanel);

        //deep copy stuff--------------------------------------------------------
        for (auto row{ 0 }; row != rowsSize(); ++row)
        {
            auto& thisRow{ pattern[row] };

            thisRow.clear();
            thisRow.reserve(otherSequencerPanel.columnsSize());

            auto& otherRow{ otherSequencerPanel.pattern[row] };

            thisRow = otherRow;

            otherRow.clear();
        }

        handleFillingGridItems(numberOfVisibleRows);
    }

    return *this;
}

*/

SequencerStrip::~SequencerStrip()
{
    for (auto& item : grid.items)
		delete item.associatedComponent;
}

void SequencerStrip::initialiseSequencerStripInvariants()
{
    setInterceptsMouseClicks(true, false);

    using Grid = juce::Grid;
    grid.setGap(Grid::Px(0)); //the grid has no gaps between cells
    grid.autoFlow = Grid::AutoFlow::column;
    grid.autoRows = Grid::Fr(1);
    grid.autoColumns = Grid::Fr(1);

    grid.templateRows.add(grid.autoColumns);
}

void SequencerStrip::setColumns(const int& newColumns)
{
    if (newColumns == grid.templateColumns.size())
        return;

    auto& gridColumns{ grid.templateColumns };
    auto& gridItems{ grid.items };
    
    auto oldColumns{ gridColumns.size() };

    auto newColumnssGreaterThanOldColumns{ newColumns > oldColumns };

    if (newColumnssGreaterThanOldColumns)
    {
        gridColumns.ensureStorageAllocated(newColumns);
        gridItems.ensureStorageAllocated(newColumns);

        for (auto column{ oldColumns }; column != newColumns; ++column)
        {
            gridColumns.add(grid.autoRows);
            gridItems.add(new SequencerCell);
			auto& cell{ gridItems.getReference(column).associatedComponent };

            addAndMakeVisible(cell);
            cell->addMouseListener(this, true);
        }
    }
	else//newColumns is less than oldColumns
    {
        gridColumns.resize(newColumns);
        gridColumns.minimiseStorageOverheads();

        std::for_each(gridItems.begin() + newColumns, gridItems.end(), [this](auto& item)
            {
                auto& cell{ item.associatedComponent };

                cell->removeMouseListener(this);
                delete item.associatedComponent;
            });
            
        gridItems.resize(newColumns);
        gridItems.minimiseStorageOverheads();
    }
}

void SequencerStrip::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::black);
}

void SequencerStrip::resized()
{
	grid.performLayout(getLocalBounds());
}