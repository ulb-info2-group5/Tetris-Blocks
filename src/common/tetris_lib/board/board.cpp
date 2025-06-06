/*
 * This file is part of Royal Blocks.
 *
 * Royal Blocks is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Royal Blocks is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Royal Blocks.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "board.hpp"

#include "../tetromino/abstract_tetromino.hpp"
#include "../vec2/vec2.hpp"
#include "board_update.hpp"
#include "grid_cell.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <random>

/*--------------------------------------------------
                    PRIVATE
--------------------------------------------------*/

// #### Helpers ####

GridCell &Board::at(int xCol, int yRow) {
    return getRow(yRow).at(static_cast<size_t>(xCol));
}

std::array<GridCell, Board::width_> &Board::getRow(int yRow) {
    return grid_.at(getHeight() - 1 - static_cast<size_t>(yRow));
}

const std::array<GridCell, Board::width_> &Board::getRow(int yRow) const {
    return grid_.at(getHeight() - 1 - static_cast<size_t>(yRow));
}

void Board::dropRowsAbove(int yRow) {
    const int topRow = getHeight() - 1;

    for (int y = yRow; y < static_cast<int>(topRow); y++) {
        setRow(getRow(y + 1), static_cast<size_t>(y));
    }

    emptyRow(getHeight() - 1);
}

void Board::liftRowsFrom(int yRow, size_t numRows) {
    const int topRow = getHeight() - 1;

    for (size_t rowCount = 0; rowCount < numRows; rowCount++) {
        for (int y = topRow; y > static_cast<int>(yRow); y--) {
            setRow(getRow(y - 1), static_cast<size_t>(y));
        }
    }

    emptyRow(yRow);
}

void Board::setPenaltyRow(std::array<GridCell, width_> &row) {
    constexpr int firstCol = 0;
    const int lastCol = getWidth() - 1;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(firstCol, lastCol);

    // The empty block in the row
    size_t emptyIndex = static_cast<size_t>(distrib(gen));

    // Fill all the GridCell with penaltyBlocksColor except one (empty state)
    for (size_t xCol = 0; xCol < getWidth(); xCol++) {
        if (xCol == emptyIndex) {
            row.at(xCol).setEmpty();
        } else {
            row.at(xCol).setColorId(PENALTY_BLOCKS_COLOR_ID);
        }
    }
}

void Board::setRow(const std::array<GridCell, width_> &row, size_t yRow) {
    getRow(static_cast<int>(yRow)) = row;
}

bool Board::checkEmptyRow(int yRow) const {
    for (const GridCell &gridCell : getRow(yRow)) {
        if (!gridCell.isEmpty()) {
            return false;
        }
    }

    return true;
}

bool Board::checkFullRow(int yRow) const {
    for (const auto &cell : getRow(yRow)) {
        if (cell.isEmpty()) {
            return false;
        }
    }

    return true;
}

bool Board::checkFullCol(int xCol) const {
    for (int yRow = getHeight() - 1; yRow >= 0; yRow--) {
        if (get(xCol, yRow).isEmpty()) {
            return false;
        }
    }

    return true;
}

void Board::emptyRow(int yRow) {
    for (GridCell &gridCell : getRow(yRow)) {
        gridCell.setEmpty();
    }
}

void Board::emptyCol(int xCol) {
    for (int yRow = getHeight() - 1; yRow >= 0; yRow--) {
        at(xCol, yRow).setEmpty();
    }
}

void Board::gravity() {
    for (int xCol = 0; xCol < static_cast<int>(getWidth()); xCol++) {
        int writeY = 0;

        for (int yRow = 0; yRow < static_cast<int>(getHeight()); yRow++) {
            if (!get(xCol, yRow).isEmpty()) {
                if (yRow != writeY) {
                    at(xCol, writeY) = at(xCol, yRow);
                    at(xCol, yRow).setEmpty();
                }
                writeY++;
            }
        }
    }
}

/*--------------------------------------------------
                    PUBLIC
--------------------------------------------------*/

// #### Getters ####

const GridCell &Board::get(int xCol, int yRow) const {
    return getRow(yRow).at(static_cast<size_t>(xCol));
}

// #### Board Actions ####

void Board::placeTetromino(TetrominoPtr tetromino) {
    Vec2 anchor = tetromino->getAnchorPoint();

    for (const Vec2 &relativeCoord : tetromino->getBody()) {
        auto [x, y] = anchor + relativeCoord;
        at(x, y).setColorId(tetromino->getColorId());
    }
}

bool Board::checkInGrid(const Vec2 &vec) const {
    return                                            //
        vec.getX() >= 0                               //
        && vec.getX() < static_cast<int>(getWidth())  //
        && vec.getY() >= 0                            //
        && vec.getY() < static_cast<int>(getHeight()) //
        && (get(vec.getX(), vec.getY()).isEmpty());
}

bool Board::checkInGrid(const ATetromino &tetromino) const {
    Vec2 anchor = tetromino.getAnchorPoint();
    for (const Vec2 &relativeCoord : tetromino.getBody()) {
        Vec2 absoluteCoord = relativeCoord + anchor;
        if (!checkInGrid(absoluteCoord)) {
            return false;
        }
    }

    return true;
}

bool Board::check2By2Occupied(int x, int y) {
    constexpr int SQUARE_WIDTH = 2;

    bool ret = true;
    for (int xOffset = 0; xOffset < SQUARE_WIDTH; xOffset++)
        for (int yOffset = 0; yOffset < SQUARE_WIDTH; yOffset++) {
            if (get(x + xOffset, y + yOffset).isEmpty()) {
                ret = false;
            }
        };
    return ret;
}

void Board::empty2By2Square(int x, int y) {
    constexpr int SQUARE_WIDTH = 2;

    for (int xOffset = 0; xOffset < SQUARE_WIDTH; xOffset++)
        for (int yOffset = 0; yOffset < SQUARE_WIDTH; yOffset++) {
            at(x + xOffset, y + yOffset).setEmpty();
        }
}

void Board::destroy2By2Occupied() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(0, Board::getHeight()
                                                      * Board::getWidth());
    int tmp = distrib(gen);
    int startY =
        static_cast<int>(static_cast<size_t>(tmp) / Board::getHeight());
    int startX = static_cast<int>(static_cast<size_t>(tmp) % Board::getWidth());

    bool found2By2 = false;

    for (int yOffset = 0;
         yOffset < static_cast<int>(Board::getHeight() - 1) && !found2By2;
         yOffset++) {
        for (int xOffset = 0;
             xOffset < static_cast<int>(Board::getWidth() - 1) && !found2By2;
             xOffset++) {

            int x =
                (startX + xOffset) % static_cast<int>(Board::getWidth() - 1);
            int y =
                (startY + yOffset) % static_cast<int>(Board::getHeight() - 1);

            if (check2By2Occupied(x, y)) {
                found2By2 = true;
                empty2By2Square(x, y);
            }
        }
    }
}

// #### Penalty Rows ####

bool Board::receivePenaltyRows(size_t numPenaltyRows) {
    if (!checkEmptyRow(static_cast<int>(getHeight() - numPenaltyRows))) {
        return false;
    }

    constexpr int bottomRow = 0;
    // Lift rows from the bottom to make room for the penalty rows.
    liftRowsFrom(bottomRow, numPenaltyRows);

    // Fill the newly freed rows with penalty rows.
    for (size_t penaltyRowCount = 0; penaltyRowCount < numPenaltyRows;
         penaltyRowCount++) {
        setPenaltyRow(getRow(static_cast<int>(penaltyRowCount)));
    }

    return true;
}

// #### Update Board State ####

BoardUpdate Board::update() {
    BoardUpdate boardUpdate;

    for (int yRow = getHeight() - 1; yRow >= 0; yRow--) {
        if (checkFullRow(yRow)) {
            emptyRow(yRow);
            boardUpdate.incrementClearedRows();
            dropRowsAbove(yRow);
        }
    }

    return boardUpdate;
}

/* ------------------------------------------------
 *          Serialization
 * ------------------------------------------------*/

nlohmann::json Board::serialize() const {
    nlohmann::json j_grid = nlohmann::json::array();
    for (const auto &row : grid_) {
        nlohmann::json j_row = nlohmann::json::array();
        for (const auto &cell : row) {
            j_row.push_back(cell.serialize());
        }
        j_grid.push_back(j_row);
    }
    return j_grid;
}

void Board::deserialize(const nlohmann::json &j) {
    for (size_t y = 0; y < height_; ++y) {
        for (size_t x = 0; x < width_; ++x) {
            grid_.at(y).at(x).deserialize(j[y][x]);
        }
    }
}
