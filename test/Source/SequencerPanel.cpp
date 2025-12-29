#include "SequencerPanel.h"

SequencerPanel::SequencerPanel(const int& initialVisibleRows)
    : numberOfVisibleRows(initialVisibleRows > 0 ? initialVisibleRows : 1 )
{
    initialiseSequencerPanelInvariants();

    for (auto row{0}; row != rowsSize(); ++row)
        handleAdditionOfCellToPattern(row, std::shared_ptr<SequencerCell>(new SequencerCell));

    grid.templateColumns.add(grid.autoColumns);
    setTemplateRows(numberOfVisibleRows);
    handleFillingGridItems(numberOfVisibleRows);
}

SequencerPanel::SequencerPanel(const SequencerPanel& otherSequencerPanel)
    : numberOfVisibleRows(otherSequencerPanel.numberOfVisibleRows)
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

SequencerPanel::SequencerPanel(SequencerPanel&& otherSequencerPanel) noexcept
    : numberOfVisibleRows(otherSequencerPanel.numberOfVisibleRows)
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

SequencerPanel& SequencerPanel::operator=(SequencerPanel otherSequencerPanel)
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

            for (auto& cell : otherSequencerPanel.pattern[row])
                handleAdditionOfCellToPattern(row, std::shared_ptr<SequencerCell>(new SequencerCell(*cell)));
        }

        handleFillingGridItems(numberOfVisibleRows);
    }

    return *this;
}

SequencerPanel& SequencerPanel::operator=(SequencerPanel&& otherSequencerPanel) noexcept
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

void SequencerPanel::initialiseSequencerPanelInvariants()
{
    setInterceptsMouseClicks(true, false);

    using Grid = juce::Grid;
    grid.setGap(Grid::Px(0)); //the grid has no gaps between cells
    grid.autoFlow = Grid::AutoFlow::column;
    grid.autoRows = Grid::Fr(1);
    grid.autoColumns = Grid::Fr(1);
}

void SequencerPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void SequencerPanel::handleFillingGridItems(const int& newNumberOfVisibleRows)
{
    grid.items.resize(newNumberOfVisibleRows * columnsSize());
    grid.items.minimiseStorageOverheads();

    for (auto column{ 0 }; column != columnsSize(); ++column)
        for (auto visibleRow{ newNumberOfVisibleRows - 1 }; visibleRow >= 0; --visibleRow)
        {
            auto cell{ getCellInPattern(visibleRow + referenceRow, column).get() };

            grid.items.setUnchecked(newNumberOfVisibleRows - 1 - visibleRow + column * newNumberOfVisibleRows, cell);
            cell->setVisible(true);
        }
}

void SequencerPanel::setNumberOfVisibleRows(const int& newNumberOfVisibleRows)
{
    if (newNumberOfVisibleRows < 1 || referenceRow + newNumberOfVisibleRows - 1 >= rowsSize() || newNumberOfVisibleRows == numberOfVisibleRows)
        return;

    setTemplateRows(newNumberOfVisibleRows);

    if (newNumberOfVisibleRows < numberOfVisibleRows)//i.e. there are fewer visible rows on screen
        for (auto row{ referenceRow + newNumberOfVisibleRows }; row != referenceRow + numberOfVisibleRows; ++row)
            std::for_each(pattern[row].begin(), pattern[row].end(),[](auto& cell)
                { cell->setVisible(false); });

    handleFillingGridItems(newNumberOfVisibleRows);

    numberOfVisibleRows = newNumberOfVisibleRows;

    resized();
}

void SequencerPanel::setMode(const SequencerMode& newMode)
{
    if (newMode == mode)
        return;
    else
    {
        switch (mode)
        {
        case paintMode:
            exitPaintMode();
            break;
        case selectionMode:
            exitSelectionMode();
            break;
        }

        switch (newMode)
        {
        case paintMode:
            enterPaintMode();
            break;
        case selectionMode:
            enterSelectionMode();
            break;
        }

        mode = newMode;
    }
}

void SequencerPanel::exitPaintMode()
{
    if (isDraggingCellEdge())
        resetDraggingStates();
    
    exitLastCellOver();
    lastCellStateChange = SequencerCell::State::off;
}

void SequencerPanel::exitSelectionMode()
{
    selectedCells.clear();
}

void SequencerPanel::enterPaintMode()
{
}

void SequencerPanel::enterSelectionMode()
{
}

bool SequencerPanel::eventIsContainedByADraggableLeftEdge(const SequencerCell* const cell, const juce::Point<int>& eventPosition) const
{
    return cell->isOn() && getLeftEdgeBounds(cell).contains(eventPosition) && cell->getIsLeftConnected() == false;
}

bool SequencerPanel::eventIsContainedByADraggableRightEdge(const SequencerCell* const cell, const juce::Point<int>& eventPosition) const
{
    return cell->isOn() && getRightEdgeBounds(cell).contains(eventPosition) && cell->getIsRightConnected() == false;
}

bool SequencerPanel::eventIsContainedByADraggableEdge(const SequencerCell* const cell, const juce::Point<int>& eventPosition) const
{
    return eventIsContainedByADraggableLeftEdge(cell, eventPosition) || eventIsContainedByADraggableRightEdge(cell, eventPosition);
}

void SequencerPanel::updateLastCellOver(SequencerCell* const cell)
{
    if (cell)
    {
        exitLastCellOver();

        cell->setMouseIsOverCell(true);
        lastOverCell = cell;
    }
}

void SequencerPanel::resetDraggingStates()
{
    isDraggingLeftCellEdge = false;
    isDraggingRightCellEdge = false;
    mouseDownCell = nullptr;
    rowSnapshot.clear();
};

void SequencerPanel::mouseUp(const juce::MouseEvent& event)
{
    if (isDraggingCellEdge())
    {
        setMouseCursor(juce::MouseCursor::NormalCursor);
        updateLastCellOver(getCellAtLocation(event.getPosition()));

        resetDraggingStates();
    }
}

void SequencerPanel::mouseMove(const juce::MouseEvent& event)
{
    if (!isEnabled() || !isVisible() || isCurrentlyBlockedByAnotherModalComponent())
        return;

    using namespace juce;
    const auto eventPosition{ event.getPosition() };

    if (isDraggingCellEdge())
    {
        setMouseCursor(MouseCursor::LeftRightResizeCursor);
    }
    else if (const auto cell{ getCellAtLocation(eventPosition) })
    {
        if (eventIsContainedByADraggableEdge(cell, eventPosition))
            setMouseCursor(MouseCursor::LeftRightResizeCursor);
        else
            setMouseCursor(MouseCursor::NormalCursor);

        if (cell != lastOverCell)
        {
            updateLastCellOver(cell);
            cell->repaint();
        }
    }
    else
    {
        setMouseCursor(MouseCursor::NormalCursor);
        exitLastCellOver();
    }
}

void SequencerPanel::mouseExit(const juce::MouseEvent& event)
{
    exitLastCellOver();
    lastOverCell = nullptr;
}

SequencerCell* SequencerPanel::changeCellState(SequencerCell* cell)
{
    return setCellState(cell, cell->getState() == SequencerCell::State::on ? SequencerCell::State::off
                                                                  : SequencerCell::State::on);
}

SequencerCell* SequencerPanel::setCellState(SequencerCell* const cell, const SequencerCell::State& newState)
{
    cell->setState(newState);

    if (cell->getIsLeftConnected() && newState == SequencerCell::State::off)
    {
        cell->setIsLeftConnected(false);

        const auto leftCell{ getLeftCell(cell) };
        leftCell->setIsRightConnected(false)->repaint();
    }

    if (cell->getIsRightConnected() && newState == SequencerCell::State::off)
    {
        cell->setIsRightConnected(false);

        const auto rightCell{ getRightCell(cell) };
        rightCell->setIsLeftConnected(false)->repaint();
    }

    return cell;
}

bool SequencerPanel::columnIsWithinBounds(const int& column, const std::pair<int, int>& bounds) const
{
    const auto& [leftBound, rightBound] = bounds;

    if (leftBound < rightBound && column >= leftBound && column < rightBound)
        return true;
    else if (leftBound > rightBound && !(column >= rightBound && column < leftBound))
        return true;

    return false;
}

void SequencerPanel::mouseDown(const juce::MouseEvent& event)
{
    if (!isEnabled() || !isVisible() || isCurrentlyBlockedByAnotherModalComponent() || !contains(event.getPosition()))
        return;

    const auto eventPosition{ event.getPosition() };

    if (const auto cell{ getCellAtLocation(eventPosition) })
    {
        mouseDownCell = cell;
        const auto row{ getCellRow(cell) };
        const auto canDragLeft{ eventIsContainedByADraggableLeftEdge(cell, eventPosition) && row.has_value() }
        , canDragRight{ eventIsContainedByADraggableRightEdge(cell, eventPosition) && row.has_value() };

        if (canDragLeft || canDragRight)
            snapshotRow(row.value());

        if (canDragLeft)
        {
            isDraggingLeftCellEdge = true;
        }
        else if (canDragRight)
        {
            isDraggingRightCellEdge = true;
        }
        else
        {
            changeCellState(cell)->repaint();
            lastCellStateChange = cell->getState();
        }
    }
}

juce::Rectangle<int> SequencerPanel::getLeftEdgeBounds(const SequencerCell* const cell) const
{
    return getCellColumn(cell) != 0 ? cell->getBoundsInParent().withWidth(cell->edgeWidth)
        : cell->getBoundsInParent().withWidth(2 * cell->edgeWidth);
}

juce::Rectangle<int> SequencerPanel::getRightEdgeBounds(const SequencerCell* const cell) const
{
    const auto cellBounds{ cell->getBoundsInParent() };

    return getCellColumn(cell) != columnsSize() - 1 ? cellBounds.withTrimmedLeft(cellBounds.getWidth() - cell->edgeWidth)
        : cellBounds.withTrimmedLeft(cellBounds.getWidth() - 2 * cell->edgeWidth);
}

SequencerCell* SequencerPanel::getLeftCell(const SequencerCell* const cell, const int& steps) const
{
    if (const auto cellCoordinates{ getCellCoordinates(cell) })
        return getCellPtr(cellCoordinates.value().first, getLeftColumn(cellCoordinates.value().second, steps));
    else
        return nullptr;
}

SequencerCell* SequencerPanel::getRightCell(const SequencerCell* const cell, const int& steps) const
{
    if (const auto cellCoordinates{ getCellCoordinates(cell) })
        return getCellPtr(cellCoordinates.value().first, getRightColumn(cellCoordinates.value().second, steps));
    else
        return nullptr;
}

void SequencerPanel::connectWholeRow(const int& row)
{
    auto& patternRow{ pattern[row] };
    for (auto& cell : patternRow)
        cell->setState(SequencerCell::State::on)->setIsLeftConnected(true)->setIsRightConnected(true);
}

void SequencerPanel::handleDragEdgeOfNonGreaterCell(SequencerCell* const cell, const juce::Point<int> dragPosition)
{
    const auto row{ getCellRow(cell) };

    if (!row.has_value())
        return;

    auto& patternRow{ pattern[row.value()] };
    const auto middleOfCell{ cell->getBoundsInParent().getCentreX() };

    if ((isDraggingLeftCellEdge && dragPosition.getX() > middleOfCell) ||
        (isDraggingRightCellEdge && dragPosition.getX() < middleOfCell))
    {
        for (auto& rowCell : patternRow)
        {
            rowCell->setState(SequencerCell::State::on)->setIsLeftConnected(true)->setIsRightConnected(true);
        }

        if (isDraggingLeftCellEdge)
        {
            cell->setIsRightConnected(false);
            getRightCell(cell)->setIsLeftConnected(false);
        }
        else if (isDraggingRightCellEdge)
        {
            getLeftCell(cell)->setIsRightConnected(false);
            cell->setIsLeftConnected(false);
        }
    }
    else
    {
        const auto cellColumn{ getCellColumn(cell) };
        if (!cellColumn.has_value())
            return;

        const auto snapshotBounds{ getBoundsOfGreaterCellAtColumnInRowSnapshot(cellColumn.value()) };
        if (!snapshotBounds.has_value())
            return;

        for (auto column{ 0 }; column != columnsSize(); ++column)
        {
            auto& rowCell{ patternRow[column] };

            if (column != cellColumn.value() && columnIsWithinBounds(column, snapshotBounds.value()))
                rowCell->turnOff();
            else
                rowCell->setCell(*rowSnapshot.getReference(column));
        }

        patternRow[cellColumn.value()]->setIsLeftConnected(false)->setIsRightConnected(false);
    }
}

std::optional<int> SequencerPanel::getLeftBoundOfGreaterCellContainingCell(const SequencerCell* const cell) const
{
    if (!cell->getIsLeftConnected())
        return getCellColumn(cell);
    else
    {
        auto iteratingCell{ getLeftCell(cell) };
        while (iteratingCell != cell)
            if (!iteratingCell->getIsLeftConnected())
                return getCellColumn(iteratingCell);
            else
                iteratingCell = getLeftCell(iteratingCell);
    }

    return std::nullopt;
}

std::optional<int> SequencerPanel::getRightBoundOfGreaterCellContainingCell(const SequencerCell* const cell) const
{
    auto iteratingCell{ getRightCell(cell) };

    if (!cell->getIsRightConnected())
        return getCellColumn(iteratingCell);
    else
        while (iteratingCell != cell)
            if (!iteratingCell->getIsRightConnected())
                return getCellColumn(getRightCell(iteratingCell));
            else
                iteratingCell = getRightCell(iteratingCell);

    return std::nullopt;
}

std::optional<std::tuple<int, int, int>> SequencerPanel::getCoordinatesOfGreaterCellContainingCell(const SequencerCell* const cell) const
{
    const auto row{ getCellRow(cell) };
    if (!row.has_value())
        return std::nullopt;

    const auto leftBound{ getLeftBoundOfGreaterCellContainingCell(cell) };
    if (!leftBound.has_value())
        return std::nullopt;

    const auto rightBound{ getRightBoundOfGreaterCellContainingCell(cell) };
    if (!rightBound.has_value())
        return std::nullopt;

    return std::make_optional<std::tuple<int, int, int>>(row.value(), leftBound.value(), rightBound.value());
}

void SequencerPanel::mouseDrag(const juce::MouseEvent& event)
{
    if (!isEnabled() || !isVisible() || isCurrentlyBlockedByAnotherModalComponent() || !contains(event.getMouseDownPosition()))
        return;

    const auto eventPosition{ isDraggingCellEdge() ? event.getPosition().withY(event.getMouseDownPosition().getY())
                                                                        .withX(CUSTOM_FUNCTIONS::positiveMod(event.getPosition().getX(), getWidth()))
                                                   : event.getPosition() };

    auto cell{ getCellAtLocation(eventPosition) };

    if (!cell)
    {
        exitLastCellOver();
        return;
    }

    if (isDraggingCellEdge())
    {
        const auto mouseDownCellCoordinates{ getCellCoordinates(mouseDownCell) };
        if (!mouseDownCellCoordinates.has_value())
            return;

        const auto& [snapshotRow, mouseDownColumn] = mouseDownCellCoordinates.value();
        const auto currentBounds{ getLiveBoundsOfGreaterCell(snapshotRow, mouseDownColumn) };
        const auto cellColumn{ getCellColumn(cell) };

        if (!currentBounds.has_value() || !cellColumn.has_value())
            return;

        if (isDraggingLeftCellEdge)
        {
            handleDraggingLeftCellEdge(cell, eventPosition, snapshotRow,
                currentBounds.value(), getRightColumn(cellColumn.value()));
        }
        else if (isDraggingRightCellEdge)
        {
            handleDraggingRightCellEdge(cell, eventPosition, snapshotRow,
                currentBounds.value(), cellColumn.value());
        }
    }
    else
    {
        if (cell == lastOverCell)
            return;

        setCellState(cell, lastCellStateChange)->repaint();
    }

    updateLastCellOver(cell);
}

std::optional<std::pair<int, int>> SequencerPanel::getLiveBoundsOfGreaterCell(const int& row, const int& column) const
{
    std::optional<int> leftBound{}, rightBound{};

    if (isDraggingLeftCellEdge)
    {
        rightBound = getRightBoundOfGreaterCellAtColumnInRowSnapshot(column);
        if (!rightBound.has_value())
            return std::nullopt;

        leftBound = getLeftBoundOfGreaterCellContainingCell(getLeftCell(getCellPtr(row, rightBound.value())));
        if (!leftBound.has_value())
            return std::nullopt;
    }
    else if (isDraggingRightCellEdge)
    {
        leftBound = getLeftBoundOfGreaterCellAtColumnInRowSnapshot(column);
        if (!leftBound.has_value())
            return std::nullopt;

        rightBound = getRightBoundOfGreaterCellContainingCell(getCellPtr(row, leftBound.value()));
        if (!rightBound.has_value())
            return std::nullopt;
    }
    else
    {
        rightBound = getRightBoundOfGreaterCellContainingCell(getCellPtr(row, column));
        if (!rightBound.has_value())
            return std::nullopt;

        leftBound = getLeftBoundOfGreaterCellContainingCell(getCellPtr(row, column));
        if (!leftBound.has_value())
            return std::nullopt;
    }

    return std::make_optional<std::pair<int, int>>(leftBound.value(), rightBound.value());
}

void SequencerPanel::handleDraggingLeftCellEdge(SequencerCell* const cell, const juce::Point<int>& dragPosition,
    const int& row, const std::pair<int, int>& currentBounds, const int& newLeftBound)
{
    if (currentBounds.second == newLeftBound)
    {
        handleDragEdgeOfNonGreaterCell(cell, dragPosition);
        repaintRow(row);
    }
    else// if (oldLeftBounds != newLeftBound)
    {
        setDraggedGreaterCellLeftBound(row, currentBounds, newLeftBound);

        auto& oldLeftBound{ currentBounds.first };
        auto& rightBound{ currentBounds.second };

        repaintRegion(getLeftColumn(rightwardDistance(oldLeftBound, rightBound) > rightwardDistance(newLeftBound, rightBound)
                                    ? oldLeftBound : newLeftBound),
                      rightBound,
                      row, row - 1);
    }
}

void SequencerPanel::handleDraggingRightCellEdge(SequencerCell* const cell, const juce::Point<int>& dragPosition,
    const int& row, const std::pair<int, int>& currentBounds, const int& newRightBound)
{
    if (currentBounds.first == newRightBound)
    {
        handleDragEdgeOfNonGreaterCell(cell, dragPosition);
        repaintRow(row);
    }
    else// if (oldRightBound != newRightBound)
    {
        setDraggedGreaterCellRightBound(row, currentBounds, newRightBound);

        auto& leftBound{ currentBounds.first };
        auto& oldRightBound{ currentBounds.second };

        repaintRegion(leftBound,
                      getRightColumn(rightwardDistance(leftBound, oldRightBound) > rightwardDistance(leftBound, newRightBound)
                                     ? oldRightBound : newRightBound),
                      row, row - 1);
    }
}

void SequencerPanel::repaintRow(const int& row)
{
    for (auto& iteratorCell : pattern[row])
        iteratorCell->repaint();
}

void SequencerPanel::repaintRegion(const int& leftBound, const int& rightBound, const int& topBound, const int& bottomBound)
{
    if (leftBound < 0 || leftBound >= columnsSize() || rightBound < 0 || rightBound >= columnsSize() ||
        topBound < 0 || topBound >= rowsSize() || bottomBound < -1 || bottomBound >= rowsSize() - 1)
        return;

    for (auto row{ topBound }; row != bottomBound; --row)
    {
        auto column{ leftBound };
        auto notYetRepaintedColumnX{ true };

        while (column != rightBound || notYetRepaintedColumnX)
        {
            pattern[row][column]->repaint();

            column = getRightColumn(column);
            notYetRepaintedColumnX = false;
        }
    }
}

bool SequencerPanel::rowIsInValidState(const int& row) const
{
    auto& patternRow{ pattern[row] };

    for (const auto& cell : patternRow)
        if (cell->isOn())
        {
            if (cell->getIsLeftConnected() && !getLeftCell(cell.get())->getIsRightConnected())
            {
                DBG(cell->getName() + " is left-connected but shouldn't be.");
                return false;
            }
            if (cell->getIsRightConnected() && !getRightCell(cell.get())->getIsLeftConnected())
            {
                DBG(cell->getName() + " is right-connected but shouldn't be.");
                return false;
            }

        }
        else
        {
            if (cell->getIsLeftConnected())
            {
                DBG(cell->getName() + " is off but left-connected.");
                return false;
            }
            if (cell->getIsRightConnected())
            {
                DBG(cell->getName() + " is off but right-connected.");
                return false;
            }
        }

    return true;
}

SequencerPanel::~SequencerPanel()
{
    for (auto& row : pattern)
        std::for_each(row.begin(), row.end(),
            [this](auto& cell)
            {
                cell->removeMouseListener(this);
                removeChildComponent(cell.get());
            });
}

void SequencerPanel::resized()
{
    grid.performLayout(getLocalBounds());
}

std::vector<std::shared_ptr<SequencerCell>> SequencerPanel::cellsAsVector() const
{
    std::vector<std::shared_ptr<SequencerCell>> cellsAsVector;
    cellsAsVector.reserve(rowsSize() * columnsSize());

    for (auto& row : pattern)
        std::for_each(row.begin(), row.end(), [&cellsAsVector](auto& cell)
            { cellsAsVector.push_back(cell); });

    return cellsAsVector;
}

SequencerCell* SequencerPanel::getCellAtLocation(const juce::Point<int>& location)
{
    for (const auto& item : grid.items)
    {
        auto component{ item.associatedComponent };
        if (component && component->getBoundsInParent().contains(location))
            return static_cast<SequencerCell*>(component);

    }

    return nullptr;
}

std::optional<int> SequencerPanel::findGridItemIndex(const std::shared_ptr<SequencerCell>& cell) const
{
    auto& items = grid.items;
    for (int i = 0; i < items.size(); ++i)
        if (items.getUnchecked(i).associatedComponent == cell.get())
            return i;

    return -1;//could make this an optional, can't be bothered
}

void SequencerPanel::shiftVisibleRows(int shiftFactor)
{
    if (shiftFactor == 0)
        return;

    //handling edge cases
    if (referenceRow + shiftFactor < 0)
        shiftFactor = -referenceRow;
    else if (shiftFactor + getVisibleRowsMax() >= CONSTANTS::MIDI_PITCHES_SIZE)
        shiftFactor = CONSTANTS::MIDI_PITCHES_SIZE - getVisibleRowsMax() - 1;

    for (auto itemIndex{ 0 }; itemIndex != grid.items.size(); ++itemIndex)
        shiftGridItem(itemIndex, shiftFactor);

    referenceRow += shiftFactor;

    resized();
}

void SequencerPanel::shiftGridItem(const int& itemIndex, const int& shiftFactor)
{
    const auto [row, column] { gridItemsCoordinates(itemIndex) };

    if (row >= referenceRow + numberOfVisibleRows + shiftFactor || row <= referenceRow + shiftFactor)
        grid.items.getUnchecked(itemIndex).associatedComponent->setVisible(false);

    grid.items.setUnchecked(itemIndex, pattern[row + shiftFactor][column].get());
    grid.items.getUnchecked(itemIndex).associatedComponent->setVisible(true);
}

void SequencerPanel::shuffleRow(const int& row, const int& offset)
{
    if (row < 0 || row >= rowsSize() || offset == 0 || row + offset < 0 || row + offset >= rowsSize())
    {
        jassertfalse;
        return;
    }

    int newRow = row + offset;

    if (newRow > row) // Shift left
        std::move(pattern.begin() + row + 1, pattern.begin() + newRow + 1, pattern.begin() + row);
    else // Shift right
        std::move_backward(pattern.begin() + newRow, pattern.begin() + row, pattern.begin() + row + 1);

    pattern[newRow] = std::move(pattern[row]);
}

void SequencerPanel::setRepeats(const int& newRepeats)
{
    if (newRepeats < 1)
        return;

    if (newRepeats > repeats)
        handleNewRepeatsIsGreaterThanOld(newRepeats);
    else if (newRepeats < repeats)
        removeLastColumns((repeats - newRepeats) * baseColumnsSize());
    else
        return;

    repeats = newRepeats;

    updateTemplateColumns();
    resized();
}

void SequencerPanel::handleNewRepeatsIsGreaterThanOld(const int& newRepeats)
{
    const auto cashedBaseColumnsSize{ baseColumnsSize() };

    //allocating requisite storage for each row and setting edge connections
    for (auto& row : pattern)
    {
        row.reserve(newRepeats * cashedBaseColumnsSize);

        row[0]->setIsLeftConnected(false);
        row[row.size() - 1]->setIsRightConnected(false);
    }

    for (auto repeat{ repeats }; repeat != newRepeats; ++repeat)
        for (auto column{ 0 }; column != startPositions.size(); ++column)
            addCell();
}

void SequencerPanel::insertColumn(float startPosition)
{
    jassert(startPosition > 0 && startPosition < repeats);

    //since an inserted startPosition will be inserted across all repeats, we use 
    //a start position on the first repeat so the function's logic is uniform
    startPosition -= (int)startPosition;

    const auto index{ findIndex(startPosition) };
    const auto newBaseSize{ baseColumnsSize() + 1 };

    for (auto& row : pattern)
        row.reserve(newBaseSize * repeats);

    //adding 1 to each step of the loop to account for the inserted Cell
    for (auto repeatedIndex{ index % newBaseSize };
        repeatedIndex != index % newBaseSize + repeats * newBaseSize;
        repeatedIndex += newBaseSize)
    {
        insertCell(repeatedIndex);
    }

    startPositions.insert(index, startPosition);

    updateTemplateColumns();
    resized();
}

void SequencerPanel::removeColumn(const int& index)
{
    jassert(index > 0 && index < columnsSize() /*&& index % getBaseColumnsSize() != 0*/);

    const auto baseSize{ baseColumnsSize() };
    const auto newBaseSize{ baseSize - 1 };
    const auto baseIndex{ index % baseSize };

    //subtracting 1 from each step of the loop to account for the removed Cell
    for (auto repeatedIndex{ baseIndex };
        repeatedIndex != baseIndex + repeats * newBaseSize;
        repeatedIndex += newBaseSize)
        removeCell(repeatedIndex);

    startPositions.remove(baseIndex);

    updateTemplateColumns();
    resized();
}

bool SequencerPanel::startPositionsIsValid(const juce::Array<float>& posiblyInvalidStartPositions) const
{
    if (posiblyInvalidStartPositions.isEmpty())
        return true;

    if (std::find_if(posiblyInvalidStartPositions.begin(), posiblyInvalidStartPositions.end(),
            [](const float& position)
            {
                return position < 0.f || position > 1.f; //ensure positions are in range [0, 1]
            }) 
            == posiblyInvalidStartPositions.end() ||
        std::adjacent_find(posiblyInvalidStartPositions.begin(), posiblyInvalidStartPositions.end(),
            [](const float& positionA, const float& positionB)
            {
                return positionA >= positionB; //ensure positions are in ascending order
            })
            == posiblyInvalidStartPositions.end()
        )
        return true;
    
    return false;
}

void SequencerPanel::shiftStartPositions(juce::Array<float> newStartPositions)
{
    jassert(newStartPositions.size() == baseColumnsSize() - 1);

    jassert(startPositionsIsValid(newStartPositions));

    newStartPositions.insert(0, 0.f);
    startPositions = newStartPositions;

    updateTemplateColumns();
    resized();
}

float SequencerPanel::columnStartPosition(const int& column) const
{
    const auto cashedBaseColumnsSize{ baseColumnsSize() };
    jassert(column >= 0 && column <= cashedBaseColumnsSize * repeats);

    return startPositions.getUnchecked(column % cashedBaseColumnsSize) + column / cashedBaseColumnsSize;
}

float SequencerPanel::columnWidth(const int& column) const
{
    return columnStartPosition(column + 1) - columnStartPosition(column);
}

int SequencerPanel::columnWidthAsGridFr(const int& column) const
{
    return static_cast<int>(std::round((columnWidth(column) / repeats) * getLocalBounds().getWidth()));
}

void SequencerPanel::handleAdditionOfCellToPattern(const int& row, const std::shared_ptr<SequencerCell>& cell)
{
    if (!cell)
        return;

    pattern[row].push_back(cell);

    addChildComponent(cell.get());
    cell.get()->addMouseListener(this, true);
}

void SequencerPanel::addCell()
{
    for (auto row{ 0 }; row != rowsSize(); ++row)
        handleAdditionOfCellToPattern(row, std::shared_ptr<SequencerCell>(new SequencerCell));

    //adds this Cell pointer into grid.items, we have to add these backwards because Grid logic
    for (auto visibleRow{ numberOfVisibleRows - 1 }; visibleRow >= 0; --visibleRow)
    {
        auto& row{ pattern[visibleRow + referenceRow] };
        auto addedCell{ row[row.size() - 1].get()};

        grid.items.add(addedCell);
        addedCell->setVisible(true);
    }
}

void SequencerPanel::insertCell(const int& column)
{
    jassert(column > 0 && column <= columnsSize());

    const auto preInsertionColumnsSize{ columnsSize() };

    for (auto row{ 0 }; row != rowsSize(); ++row)
        handleInsertionOfCellToPattern(row, column, preInsertionColumnsSize);

    //inserts this Cell pointer into grid.items
    const auto index{ gridItemsIndex(numberOfVisibleRows - 1, column - 1) + 1 };
    for (auto visibleRow{ 0 }; visibleRow != numberOfVisibleRows; ++visibleRow)
    {
        auto insertedCell{ getCellInPattern(visibleRow + referenceRow, column).get()};

        grid.items.insert(index, insertedCell);
        insertedCell->setVisible(true);
    }
}

void SequencerPanel::handleInsertionOfCellToPattern(const int& row, const int& column, const int& preInsertionColumnsSize)
{
    auto& patternRow{ pattern[row] };

    //inserts a new Cell pointer at index in each row
    patternRow.insert(patternRow.begin() + column, std::shared_ptr<SequencerCell>(new SequencerCell));

    auto insertedCell{ patternRow[column].get()};

    handleCellConnectionsDuringInsertion(row, column, preInsertionColumnsSize);

    addChildComponent(insertedCell);
    insertedCell->addMouseListener(this, true);
}

void SequencerPanel::handleCellConnectionsDuringInsertion(const int& row, const int& column, const int& preInsertionColumnsSize)
{
    auto insertedCell{ getCellInPattern(row, column) };
    auto leftCell{ getCellInPattern(row, CUSTOM_FUNCTIONS::positiveMod(column - 1, preInsertionColumnsSize)) };

    if (leftCell && leftCell->getState() == SequencerCell::State::on)
    {
        insertedCell->setState(SequencerCell::State::on)->setIsLeftConnected(SequencerCell::State::on);

        if (leftCell->getIsRightConnected())
            insertedCell->setIsRightConnected(true);

        leftCell->setIsRightConnected(true);
    }
}

const juce::GridItem* SequencerPanel::findGridItemPointer(const std::shared_ptr<SequencerCell>& cell) const
{
    auto& items{ grid.items };
    auto iterator{ std::find_if(items.begin(), items.end(),
                                [&cell](auto& gridItem)
                                {
                                    return cell.get() == gridItem.associatedComponent;
                                }) };
    //chatGPT said use &(*iterator) but I don't understand why so ignored it ;P
    return iterator != items.end() ? iterator
        : nullptr;
}

void SequencerPanel::removeLastColumns(const int& numberOfCellsToRemove = 1)
{
    jassert(numberOfCellsToRemove > 0 && numberOfCellsToRemove <= columnsSize());

    for (auto row{ 0 }; row != rowsSize(); ++row)
    {
        auto& patternRow{ pattern[row] };

        if (row >= referenceRow && row <= getVisibleRowsMax())
            std::for_each(patternRow.end() - numberOfCellsToRemove, patternRow.end(),
                [this, &patternRow](auto& cell)
                {
                    cell->setVisible(false);
                    cell->removeMouseListener(this);

                    const auto index{ findGridItemIndex(cell) };
                    if (index.has_value())
                        grid.items.remove(index.value()); // remove by index, safe
                });

        patternRow.resize(patternRow.size() - numberOfCellsToRemove);

        //handling cell connections
        if (patternRow[0]->getIsLeftConnected())
            patternRow[0]->setIsLeftConnected(false);

        if (patternRow[patternRow.size() - 1]->getIsRightConnected())
        {
            patternRow[patternRow.size() - 1]->setIsRightConnected(false);
            patternRow[0]->setIsLeftConnected(false);
        }
    }
}

void SequencerPanel::handleRemovalOfCellFromCells(const int& row, const int& column, const int& preRemovalColumnsSize)
{
    auto& patternRow{ pattern[row] };

    //handleCellConnectionsDuringRemoval(row, column, preInsertionColumnsSize);
    auto removedCell{ patternRow[column] };

    //this function should not be moved from it's relative position inside this function
    handleCellConnectionsDuringRemoval(row, column, preRemovalColumnsSize);

    const auto index{ findGridItemIndex(removedCell) };
    if (index.has_value())
        grid.items.remove(index.value());

    removedCell->removeMouseListener(this);
    //patternRow.erase(find(patternRow.begin(), patternRow.end(), ))
    patternRow.erase(patternRow.begin() + column);
}

void SequencerPanel::handleCellConnectionsDuringRemoval(const int& row, const int& column, const int& preRemovalColumnsSize)
{
    auto leftCell{ getCellInPattern(row, CUSTOM_FUNCTIONS::positiveMod(column - 1, preRemovalColumnsSize)) };
    auto rightCell{ getCellInPattern(row, CUSTOM_FUNCTIONS::positiveMod(column + 1, preRemovalColumnsSize)) };

    if (leftCell->getIsRightConnected())
        if (!rightCell->getIsLeftConnected())
            leftCell->setIsRightConnected(false);
        else if (rightCell->getIsLeftConnected())
            rightCell->setIsLeftConnected(false);

    leftCell->repaint();
    rightCell->repaint();
}

void SequencerPanel::removeCell(const int& column)
{
    jassert(column > 0 && column <= columnsSize());

    //const auto cashedRowsSize{ rowsSize() };
    const auto cashedColumnsSize{ columnsSize() };

    for (auto row{ 0 }; row != rowsSize(); ++row)
        handleRemovalOfCellFromCells(row, column, cashedColumnsSize);
}

int SequencerPanel::findIndex(const float& startPosition)
{
    const auto baseSize{ baseColumnsSize() };

    for (auto index{ 1 }; index <= baseSize * repeats; ++index)
        if (startPosition > startPositions.getUnchecked((index - 1) % baseSize) + (index - 1) / baseSize &&
            startPosition < startPositions.getUnchecked(index % baseSize) + index / baseSize)
            return index;

    jassertfalse; //you never should have come here!
    return {};
}

int SequencerPanel::gridItemsIndex(const int& row, const int& column) const
{
    jassert(row >= 0 && row < numberOfVisibleRows &&
        column >= 0 && column < columnsSize());

    return row + column * numberOfVisibleRows;
}

std::pair<int, int> SequencerPanel::gridItemsCoordinates(const int& gridItemsIndex) const
{
    jassert(gridItemsIndex >= 0 && gridItemsIndex < grid.items.size());

    return { getVisibleRowsMax() - (gridItemsIndex % numberOfVisibleRows), //row
             gridItemsIndex / numberOfVisibleRows };                       //column
}

std::optional<std::pair<int, int>> SequencerPanel::getCellCoordinates(const SequencerCell* const cell) const
{
    jassert(cell != nullptr);

    if (cell == nullptr)//the cell does not exist
        return std::nullopt;

    for (auto row{ 0 }; row != rowsSize(); ++row)
        for (auto column{ 0 }; column != columnsSize(); ++column)
            if (getCellPtr(row, column) == cell)
                return std::make_pair(row, column);//the cell exists and is in the grid

    return std::nullopt;; //the cell exists but is not in the grid??
}

std::optional<int> SequencerPanel::getCellRow(const SequencerCell* const cell) const
{
    if (const auto coords = getCellCoordinates(cell))
        return coords->first;

    return std::nullopt;
}

std::optional<int> SequencerPanel::getCellColumn(const SequencerCell* const cell) const
{
    if (const auto coords = getCellCoordinates(cell))
        return coords->second;

    return std::nullopt;
}

void SequencerPanel::updateTemplateColumns()
{
    std::vector<int> newTemplateColumns;
    makeIdealWidths(newTemplateColumns);
    distributeError(newTemplateColumns);

    grid.templateColumns.resize(columnsSize());

    for (auto column{ 0 }; column != columnsSize(); ++column)
        grid.templateColumns.setUnchecked(column, juce::Grid::Px(newTemplateColumns[column]));
}

void SequencerPanel::makeIdealWidths(std::vector<int>& newTemplateColumns) const
{
    newTemplateColumns.reserve(columnsSize());

    for (auto column{ 0 }; column != columnsSize(); ++column)
        newTemplateColumns.push_back(column < baseColumnsSize() ? columnWidthAsGridFr(column)
            : newTemplateColumns[column % baseColumnsSize()]);
}

void SequencerPanel::distributeError(std::vector<int>& newTemplateColumns) const
{
    const auto error{ std::accumulate(newTemplateColumns.begin(), newTemplateColumns.end(), 0)
                      + (columnsSize() - 1) * grid.columnGap.pixels
                      - getLocalBounds().getWidth() };

    if (error == 0)
        return;

    if (const auto interval{ columnsSize() / std::abs(error) })
    {
        int index{ 0 };
        while (index < std::abs(error))
            newTemplateColumns[index++ * interval] -= error / std::abs(error);
    }
}

void SequencerPanel::snapshotRow(const int& row)
{
    jassert(rowSnapshot.isEmpty());

    rowSnapshot.clear();
    rowSnapshot.ensureStorageAllocated(rowsSize());

    for (const auto& rowCell : pattern[row])
        rowSnapshot.add(std::make_unique<SequencerCell>(*rowCell));
}

void SequencerPanel::exitLastCellOver()
{
    if (lastOverCell && lastOverCell->getMouseIsOverCell())
    {
        lastOverCell->setMouseIsOverCell(false);
        lastOverCell->repaint();
    }
}

std::optional<int> SequencerPanel::getLeftBoundOfGreaterCellAtColumnInRowSnapshot(const int& column) const
{
    if (!rowSnapshot.getReference(column)->getIsLeftConnected())
        return column;

    auto leftwardColumn{ getLeftColumn(column) };
    while (leftwardColumn != column)
        if (!rowSnapshot.getReference(leftwardColumn)->getIsLeftConnected())
            return leftwardColumn;
        else
            leftwardColumn = getLeftColumn(leftwardColumn);

    return std::nullopt;
}

std::optional<int> SequencerPanel::getRightBoundOfGreaterCellAtColumnInRowSnapshot(const int& column) const
{
    auto rightwardColumn{ getRightColumn(column) };

    if (!rowSnapshot.getReference(column)->getIsRightConnected())
        return rightwardColumn;

    while (rightwardColumn != column)
        if (!rowSnapshot.getReference(rightwardColumn)->getIsRightConnected())
            return getRightColumn(rightwardColumn);
        else
            rightwardColumn = getRightColumn(rightwardColumn);

    return std::nullopt;
}

std::optional<std::pair<int, int>> SequencerPanel::getBoundsOfGreaterCellAtColumnInRowSnapshot(const int& column) const
{
    if (rowSnapshot.isEmpty() || column >= rowSnapshot.size())
        return std::nullopt;

    std::pair<int, int> bounds;

    const auto leftBound{ getLeftBoundOfGreaterCellAtColumnInRowSnapshot(column) };
    if (!leftBound.has_value())
        return std::nullopt;

    const auto rightBound{ getRightBoundOfGreaterCellAtColumnInRowSnapshot(column) };
    if (!rightBound.has_value())
        return std::nullopt;

    return std::make_optional<std::pair<int, int>>(leftBound.value(), rightBound.value());
}

inline int SequencerPanel::rightwardDistance(const int& originColumn, const int& destinationColumn) const
{
    return CUSTOM_FUNCTIONS::positiveMod(destinationColumn - originColumn, columnsSize());
}

inline int SequencerPanel::leftwardDistance(const int& originColumn, const int& destinationColumn) const
{
    return CUSTOM_FUNCTIONS::positiveMod(originColumn - destinationColumn, columnsSize());
}

void SequencerPanel::setDraggedGreaterCellLeftBound(const int& row, const std::pair<int, int>& oldBounds, const int& newLeftBound)
{
    const auto& [oldLeftBound, rightBound] = oldBounds;

    const auto snapshotBounds{ getBoundsOfGreaterCellAtColumnInRowSnapshot(getLeftColumn(rightBound)) };
    if (!snapshotBounds.has_value())
        return;

    auto& patternRow{ pattern[row] };

    const auto greaterCellShrunk{ columnIsWithinBounds(newLeftBound, snapshotBounds.value()) };
    const auto leftOfRightBound{ getLeftColumn(rightBound) };

    for (auto column{ 0 }; column != columnsSize(); ++column)
    {
        auto cell{ patternRow[column] };

        //1: do this if cell is part of the dragged greater cell
        if (columnIsWithinBounds(column, { newLeftBound, rightBound }))
        {
            cell->setState(SequencerCell::State::on)->setIsLeftConnected(true)->setIsRightConnected(true);

            if (column == newLeftBound)
                cell->setIsLeftConnected(false);
            if (column == leftOfRightBound)
                cell->setIsRightConnected(false);

            continue;
        }

        //2: do this if cell was only part of the dragged greater cell when dragging began
        const auto& snapshotLeftBound{ snapshotBounds.value().first };
        if (greaterCellShrunk && columnIsWithinBounds(column, { snapshotLeftBound, getRightColumn(newLeftBound) }))
        {
            cell->turnOff();
            continue;
        }

        //3: do this if neither conditions are met
        cell->setCell(*rowSnapshot.getReference(column));
        if (column == getLeftColumn(newLeftBound))
            cell->setIsRightConnected(false);
    }
}

void SequencerPanel::setDraggedGreaterCellRightBound(const int& row, const std::pair<int, int>& oldBounds, const int& newRightBound)
{
    const auto& [leftBound, oldRightBound] = oldBounds;

    const auto snapshotBounds{ getBoundsOfGreaterCellAtColumnInRowSnapshot(leftBound) };
    if (!snapshotBounds.has_value())
        return;

    auto& patternRow{ pattern[row] };

    const auto greaterCellShrunk{ columnIsWithinBounds(newRightBound, snapshotBounds.value()) };
    const auto leftOfNewRightBound{ getLeftColumn(newRightBound) };

    for (auto column{ 0 }; column != columnsSize(); ++column)
    {
        auto cell{ patternRow[column] };

        //1: do this if cell is part of the dragged greater cell
        if (columnIsWithinBounds(column, { leftBound, newRightBound }))
        {
            cell->setState(SequencerCell::State::on)->setIsLeftConnected(true)->setIsRightConnected(true);

            if (column == leftBound)
                cell->setIsLeftConnected(false);
            if (column == leftOfNewRightBound)
                cell->setIsRightConnected(false);

            continue;
        }

        //2: do this if cell was only part of the dragged greater cell when dragging began
        const auto& snapshotRightBound{ snapshotBounds.value().second };
        if (greaterCellShrunk && columnIsWithinBounds(column, { newRightBound, snapshotRightBound }))
        {
            cell->turnOff();
            continue;
        }

        //3: do this if neither conditions are met
        cell->setCell(*rowSnapshot.getReference(column));
        if (column == newRightBound)
            cell->setIsLeftConnected(false);
    }
}

void SequencerPanel::setTemplateRows(const int& newNumberOfVisibleRows)
{
    grid.templateRows.ensureStorageAllocated(newNumberOfVisibleRows);
    grid.templateRows.minimiseStorageOverheads();

    for (auto row{ 0 }; row != newNumberOfVisibleRows; ++row)
        grid.templateRows.add(grid.autoRows);
}

void SequencerPanel::handShallowCopying(const SequencerPanel& otherSequencerPanel)
{
    mode = otherSequencerPanel.mode;
    repeats = otherSequencerPanel.repeats;
    startPositions = otherSequencerPanel.startPositions;
    startPositions.minimiseStorageOverheads();
    referenceRow = otherSequencerPanel.referenceRow;
    lastCellStateChange = otherSequencerPanel.lastCellStateChange;
    lastOverCell = nullptr; //there is no need to deep copy this
    mouseDownCell = nullptr; //there is no need to deep copy this
    rowSnapshot.clear(); //there is no need to deep copy these
    rowSnapshot.minimiseStorageOverheads();
    isDraggingLeftCellEdge = otherSequencerPanel.isDraggingLeftCellEdge;
    isDraggingRightCellEdge = otherSequencerPanel.isDraggingLeftCellEdge;
    selectedCells.clear(); //there is no need to deep copy these
    selectedCells.minimiseStorageOverheads();

    grid.templateColumns = otherSequencerPanel.grid.templateColumns;
    grid.templateColumns.minimiseStorageOverheads();
    grid.templateRows = otherSequencerPanel.grid.templateRows;
    grid.templateRows.minimiseStorageOverheads();
}