#pragma once

#include <JuceHeader.h>
#include "SequencerCell.h"
#include "Globals.h"

using Pattern = std::array<std::vector<std::shared_ptr<SequencerCell>>, CONSTANTS::MIDI_PITCHES_SIZE>;

//the central UI element in which the user may sequence their drum patern
//TODO: make this an abstract base and make there be two different specalised
//classes for alpha/beta sequencers and tilt sequencer
class SequencerPanel : public juce::Component
{
public:

    enum SequencerMode
    {
        paintMode = 0,
        selectionMode = 1
    };

    SequencerPanel(const int& initialVisibleRows);

    SequencerPanel(const SequencerPanel& otherSequencerPanel);

    SequencerPanel(SequencerPanel&& otherSequencerPanel) noexcept;

    SequencerPanel& operator=(SequencerPanel otherSequencerPanel);

    SequencerPanel& operator=(SequencerPanel&& otherSequencerPanel) noexcept;

    ~SequencerPanel();

    void paint(juce::Graphics& g) override;

    void mouseMove(const juce::MouseEvent& event) override;

    void mouseUp(const juce::MouseEvent& event) override;

    void mouseExit(const juce::MouseEvent& event) override;

    void mouseDown(const juce::MouseEvent& event) override;

    void mouseDrag(const juce::MouseEvent& event) override;

    void resized() override;

    int getVisibleRows() { return grid.templateRows.size(); };

    //sets the number of visible rows on the panel, with referenceRow at the bottom
    void setNumberOfVisibleRows(const int& newVisibleRows);

    //returns the number of times the base columns layout is repeated
    int getRepeats() const { return repeats; };

    //sets the number of times the base columns layout is repeated
    void setRepeats(const int& newRepeats);

    //inserts a column onto the panel at startPosition and all repeats
    void insertColumn(float startPosition);

    //removes the column at index and all repeats
    void removeColumn(const int& index);

    //returns the total number of columns currently in the grid
    int columnsSize() const { return startPositions.size() * repeats; };
    
    //returns the number of rows in the grid
    int rowsSize() const { return pattern.size(); };

    //returns a copy of startPositions
    juce::Array<float> getStartPositions() const { return startPositions; };

    //returns the number of base columns (i.e. not counting repeats)
    int baseColumnsSize() const { return startPositions.size(); };

    //it is the responsiblity of the caller to ensure these are valid and in ascending order
    void shiftStartPositions(juce::Array<float> newStartPositions);

    //returns the minimum visible row (inclusive)
    int getReferenceRow() const { return referenceRow; };

    //returns the maximum visible row (inclusive)
    int getVisibleRowsMax() const { return referenceRow + numberOfVisibleRows - 1; };

    //shifts the visible rows up or down by shiftFactor
    void shiftVisibleRows(int shiftFactor = 1);

    inline SequencerMode getMode() const { return mode; };

    void setMode(const SequencerMode& newMode);
private:
    Pattern pattern;
    //a 2D matrix holding pointers to the SequencerCells which the grid formats on screen
    //Since juce::Grid stores GridItems in a 1D array, cells significantly simplifies
    //the manipulation of GridItems and ensures non-visible cells are still stored
    //generally, this means if you need to change the size of the rows in pattern
    //it is easier to do that before reflecting those changes in the grid

    SequencerMode mode;                            //stores the input behaviour mode of the sequencer (see enum SequencerMode)
    juce::Grid grid;                                      //the juce::Grid which handles the layout of Cells on screen
    int repeats{ 1 };                                     //the number of times the base columns layout is repeated
    juce::Array<float> startPositions{ 0 };               //the start positions of the base columns, in ascending order and in the range [0, 1)
    int numberOfVisibleRows{};                                    //the number of rows visible on screen at any time
    int referenceRow{ 60 };                               //the MIDI row which is at the bottom of the visible window (60 is C3)

    SequencerCell::State lastCellStateChange{ SequencerCell::State::off };  //stores the lastStateChange, used by mouseDown() and mouseDrag()
    SequencerCell* lastOverCell{ nullptr };                                 //stores the last cell changed, used by mouseOver()
    SequencerCell* mouseDownCell{ nullptr };                                //stores the last cell which received a mouseDown event, used by mouseDown() and mouseDrag()
    juce::Array<std::unique_ptr<SequencerCell>> rowSnapshot;                //stores a "snapshot" of a row in cells, populated in mouseDown() when on a cell edge and cleared in mouseUp()
    bool isDraggingLeftCellEdge{ false };                                   //true only if the user is currently dragging a cell edge left
    bool isDraggingRightCellEdge{ false };                                  //true only if the user is currently dragging a cell edge right
    juce::Array<std::shared_ptr<SequencerCell>> selectedCells;

    bool startPositionsIsValid(const juce::Array<float>& posiblyInvalidStartPositions) const;

    //returns a raw pointer to a grid item which could be nullptr
    const juce::GridItem* findGridItemPointer(const std::shared_ptr<SequencerCell>& cell) const;

    //does no bounds checking ;D
    std::shared_ptr<SequencerCell> getCellInPattern(const int& row, const int& column) const { return pattern[row][column]; };

    //does no bounds checking and could be null ;D
    SequencerCell* getCellPtr(const int& row, const int& column) const { return pattern[row][column].get(); };

    //adds a column at the end of the grid
    void addCell();

    //inserts a column at index in the grid
    void insertCell(const int& index);

    //finds all cells in the grid to be removed, and removes them
    void removeLastColumns(const int& numberOfCellsToRemove);

    //finds all cells in the grid to be removed, and removes them
    void removeCell(const int& index);

    //finds the index of a position if it were inserted into startPositions
    //this can cause problems if it returns 0 (which it never should)
    int findIndex(const float& startPosition);

    //because start positions are visible to the user, they can be greater than 1.f
    //helpfully, this function returns repeats as a float if index is equal to
    //baseWidths.size() * repeats
    float columnStartPosition(const int& index) const;

    //returns the width of a column as a fraction of the total width of the grid
    float columnWidth(const int& index) const;

    //returns the width of a column as a number of pixels
    int columnWidthAsGridFr(const int& index) const;

    //updates grid.templateColumns to match the current columns
    void updateTemplateColumns();

    //helper function for refreshTemplateColumns
    void makeIdealWidths(std::vector<int>& newTemplateColumns) const;

    //helper function for refreshTemplateColumns
    void distributeError(std::vector<int>& newTemplateColumns) const;

    //returns the index in grid.items of the cell at (row, column).
    //row and column refer to the visible rows and columns, rather
    //than the absolute rows and columns
    int gridItemsIndex(const int& row, const int& column) const;

    //returns the absolute (row, column) of the cell at grid.items index
    std::pair<int, int> gridItemsCoordinates(const int& gridItemsIndex) const;

    //returns the absolute (row, column) of this cell
    std::optional<std::pair<int, int>> getCellCoordinates(const SequencerCell* const cell) const;

    std::optional<int> getCellRow(const SequencerCell* const cell) const;

    std::optional<int> getCellColumn(const SequencerCell* const cell) const;

    //returns 2D cells matrix as a vector for ease of itteration
    std::vector<std::shared_ptr<SequencerCell>> cellsAsVector() const;

    //returns a pointer to the cell at location, or nullptr if no cell is there
    SequencerCell* getCellAtLocation(const juce::Point<int>& location);

    //returns bounds of left edge of cell
    juce::Rectangle<int> getLeftEdgeBounds(const SequencerCell* const cell) const;

    //returns bounds of right edge of cell
    juce::Rectangle<int> getRightEdgeBounds(const SequencerCell* const cell) const;

    //returns a pointer to the cell to the left of cell, or nullptr if there is none (though that shouldn't happen)
    SequencerCell* getLeftCell(const SequencerCell* const cell, const int& steps = 1) const;

    int getLeftColumn(const int& column, const int& steps = 1) const { return CUSTOM_FUNCTIONS::positiveMod(column - steps, columnsSize()); };

    //returns a pointer to the cell to the right of cell, or nullptr if there is none (though that shouldn't happen)
    SequencerCell* getRightCell(const SequencerCell* const cell, const int& steps = 1) const;

    int getRightColumn(const int& column, const int& steps = 1) const { return CUSTOM_FUNCTIONS::positiveMod(column + steps, columnsSize()); };

    //toggles the state of cell and updates lastCellStateChange
    SequencerCell* changeCellState(SequencerCell* const cell);

    //sets the state of cell to newState without changing lastCellStateChange
    SequencerCell* setCellState(SequencerCell* const cell, const SequencerCell::State& newState);

    //sets lastCellOver to nullptr
    void exitLastCellOver();

    bool eventIsContainedByADraggableLeftEdge(const SequencerCell* const cell, const juce::Point<int>& eventPosition) const;

    bool eventIsContainedByADraggableRightEdge(const SequencerCell* const cell, const juce::Point<int>& eventPosition) const;

    //returns true if the mouse is over an edge of cell
    bool eventIsContainedByADraggableEdge(const SequencerCell* const cell, const juce::Point<int>& eventPosition) const;

    //updates lastCellOver to cell
    void updateLastCellOver(SequencerCell* const cell);

    //shifts the grid item at itemIndex up or down by shiftFactor number of rows in grid.items
    void shiftGridItem(const int& itemIndex, const int& shiftFactor);

    //helper function called by setRepeats when newRepeats < repeats
    void handleNewRepeatsIsGreaterThanOld(const int& newRepeats);

    //helper function called by addCell, handles the addition of a cell and it's effect the Cells matrix
    void handleAdditionOfCellToPattern(const int& row, const std::shared_ptr<SequencerCell>& cell);

    //helper function called by insertCell, handles the insertion of a cell and it's effect the Cells matrix
    void handleInsertionOfCellToPattern(const int& row, const int& column, const int& preInsertionColumnsSize);

    //helper function called by handleInsertionOfCellToCells, handles how connections respond when a cell is inserted
    void handleCellConnectionsDuringInsertion(const int& row, const int& column, const int& preInsertionColumnsSize);

    //helper function called by insertCell, handles the removal of a cell and it's effect the Cells matrix and grid.items
    void handleRemovalOfCellFromCells(const int& row, const int& column, const int& preRemovalColumnsSize);

    //helper function called by handleRemovalOfCellFromCells,  handles how connections respond when a cell is removed
    void handleCellConnectionsDuringRemoval(const int& row, const int& column, const int& preRemovalColumnsSize);

    //takes a snapshot of a row which is stored as new unique pointers in rowSnapshot
    void snapshotRow(const int& row);

    bool isDraggingCellEdge() const { return isDraggingLeftCellEdge || isDraggingRightCellEdge; };

    //call this after dragging stops, sets dragging states to false and empties rowSnapshot
    void resetDraggingStates();

    //give whatever cell is being dragged a new left bound
    void setDraggedGreaterCellLeftBound(const int& row, const std::pair<int, int>& oldBounds, const int& newLeftBound);

    //give whatever cell is being dragged a new right bound
    void setDraggedGreaterCellRightBound(const int& row, const std::pair<int, int>& oldBounds, const int& newRightBound);

    //connects the whole row, this actually puts the row in an invalid state since there is no note beginning
    void connectWholeRow(const int& row);

    //find the index of a cell in grid items, returns nullopt if cell is not in grid items
    std::optional<int> findGridItemIndex(const std::shared_ptr<SequencerCell>& cell) const;

    //if dragging left and dragPosition is left of middle of cell then make the "greater" cell occupy only one cell,
    //else it is right of middle of cell and makes the greater cell occupy the whole row
    //if dragging right and dragPosition is right of middle of cell then make the "greater" cell occupy only one cell,
    //else it is left of middle of cell and makes the greater cell occupy the whole row
    void handleDragEdgeOfNonGreaterCell(SequencerCell* const cell, const juce::Point<int> dragPosition);

    //returned bound is inclusive
    std::optional<int> getLeftBoundOfGreaterCellContainingCell(const SequencerCell* const cell) const;

    //returned bound is exclusive
    std::optional<int> getRightBoundOfGreaterCellContainingCell(const SequencerCell* const cell) const;

    //the returned tupple<int, int, int> indicate the row, left bound, and right bound of the cell if it exists in cells, otherwise nullopt
    std::optional<std::tuple<int, int, int>> getCoordinatesOfGreaterCellContainingCell(const SequencerCell* const cell) const;

    //returns the left bound of  cell if it exists in cells, otherwise nullopt
    std::optional<int> getLeftBoundOfGreaterCellAtColumnInRowSnapshot(const int& column) const;

    //returns the right bound of  cell if it exists in cells, otherwise nullopt
    std::optional<int> getRightBoundOfGreaterCellAtColumnInRowSnapshot(const int& column) const;

    //the returned pair<int, int> indicate the left bound and right bound of the cell if it exists in cells, otherwise nullopt
    std::optional<std::pair<int, int>> getBoundsOfGreaterCellAtColumnInRowSnapshot(const int& column) const;

    //useful function to check if column is within bounds, as always the left bound is incluisve and the right bound is non-inclusive
    bool columnIsWithinBounds(const int& column, const std::pair<int, int>& bounds) const;

    //returns the modulo distance from originColumn to destinationColumn if counting cells in a rightward direction
    inline int rightwardDistance(const int& originColumn, const int& destinationColumn) const;

    //returns the modulo distance from originColumn to destinationColumn if counting cells in a leftward direction
    inline int leftwardDistance(const int& originColumn, const int& destinationColumn) const;

    //returns true only if the row is valid
    bool rowIsInValidState(const int& row) const;

    //called by mouseDrag() when isDraggingLeftCellEdge = true
    void handleDraggingLeftCellEdge(SequencerCell* const cell, const juce::Point<int>& dragPosition,
        const int& row, const std::pair<int, int>& currentBounds, const int& newLeftBound);

    //called by mouseDrag() when isDraggingRightCellEdge = true
    void handleDraggingRightCellEdge(SequencerCell* const cell, const juce::Point<int>& dragPosition,
        const int& row, const std::pair<int, int>& currentBounds, const int& newRightBound);

    //the returned pair<int, int> represents leftBound and rightBounds, bounds are live in the sense that a cell may be dragged when this is called
    //row is the row in cells where this greater cell is and column is a column which is contained by a greater cell
    std::optional<std::pair<int, int>> getLiveBoundsOfGreaterCell(const int& row, const int& column) const;

    //repaints a whole row in pattern
    void repaintRow(const int& row);

    //repaints the area within the coordinates, inclusive of leftBound and topBound, exclusive of rightBound and bottomBound
    void repaintRegion(const int& leftBound, const int& rightBound, const int& topBound, const int& bottomBound);

    //shuffles a row in pattern
    void shuffleRow(const int& row, const int& offset);

    void exitPaintMode();

    void exitSelectionMode();

    void enterPaintMode();

    void enterSelectionMode();

    //called by setNumberOfVisibleRows() and constructors
    void handleFillingGridItems(const int& newVisibleRows);

    void initialiseSequencerPanelInvariants();

    void resetToDefault();

    void setTemplateRows(const int& newNumberOfVisibleRows);

    //why can you access the private member variables like this??? oh well, seems to work
    void handShallowCopying(const SequencerPanel& otherSequencerPanel);
};