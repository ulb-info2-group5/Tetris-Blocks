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

#ifndef TETRIS_HPP
#define TETRIS_HPP

#include "../board/board.hpp"
#include "../tetromino/abstract_tetromino.hpp"
#include "../tetromino_queue/tetromino_queue.hpp"
#include "tetris_observer.hpp"
#include "tetromino/tetromino_shapes.hpp"

#include <cstddef>
#include <memory>
#include <vector>

class TetrisTest;

constexpr uint32_t DEFAULT_LOCK_DELAY_TICKS_NUM = 1;

/**
 * @class Tetris
 * @brief Represents a Tetris game, composed essentially of a Board and a
 * Tetromino object.
 *
 * The Tetris class handles the game state, including the active Tetromino and
 * events like rotation, movement, and clockTick, making the Tetromino drop
 * one block down. Events are managed through an event-queue API.
 */
class Tetris {
  private:
    std::vector<TetrisObserverPtr> tetrisObservers_;

    TetrominoPtr activeTetromino_;
    TetrominoPtr previewTetromino_;
    Board board_;

    TetrominoPtr holdTetromino_;
    TetrominoQueue tetrominoQueue_;

    uint32_t lockDelayTicksNum_;
    uint32_t ticksSinceLockStart_;

    bool canHold_;

    /**
     * @brief Updates the preview tetromino.
     */
    void updatePreviewTetromino();

    /**
     * @brief Resets the lock delay.
     */
    void resetLockDelay();

    /**
     * @brief Checks whether the given tetromino can be droppped.
     *
     * @param tetromino The tetromino to be checked.
     */
    bool checkCanDrop(const ATetromino &tetromino) const;

    /**
     * @brief Places the active tetromino where it currently is in the grid
     * and sets the isAlive flag to false if it was placed outside of the
     * grid.
     */
    void placeActive();

    /**
     * @brief Checks whether the cell at the given position is empty.
     *
     * @param xCol The x-coordinate.
     * @param yRow The y-coordinate.
     */
    bool checkEmptyCell(size_t xCol, size_t yRow) const;

  public:
    // #### Constructors ####

    Tetris();
    Tetris(const Tetris &) = delete;
    Tetris(Tetris &&) = default;

    // #### Assignment ####

    Tetris &operator=(const Tetris &) = delete;
    Tetris &operator=(Tetris &&) = default;

    // #### Destructor ####

    ~Tetris() = default;

    // #### TetrisObserver ####

    /**
     * @brief Adds a new TetrisObserver.
     */
    void addObserver(TetrisObserverPtr pTetrisObserver);

    /**
     * @brief Removes the given TetrisObserver.
     */
    void removeObserver(TetrisObserverPtr pTetrisObserver);

    // #### Event API ####

    /**
     * @brief Makes the active Tetromino go down (and manages the lock delay).
     * Also updates the board to clear the fullRows and returns how many rows
     * were cleared.
     */
    size_t eventClockTick();

    /**
     * @brief Drops the active Tetromino until it hits the bottom or an occupied
     * cell.
     *
     * Also updates the board to clear the fullRows and returns how many rows
     * were cleared.
     */
    size_t eventBigDrop();

    /**
     * @brief Moves the active Tetromino in the given direction if possible.
     *
     * @param tetrominoMove The direction to move the Tetromino.
     */
    size_t eventTryMoveActive(TetrominoMove tetrominoMove);

    /**
     * @brief Rotates the active Tetromino if possible.
     *
     * @param rotateClockwise True to rotate clockwise, false for
     * counter-clockwise.
     */
    void eventTryRotateActive(bool rotateClockwise);

    /**
     * @brief Moves the next Tetromino from the queue to hold.
     *
     * If there was no hold tetromino, move the tetromino to hold.
     * If there was a hold tetromino, swap it with the tetromino to hold.
     */
    void eventHoldActiveTetromino();

    /**
     * @brief Adds penalty rows, sets isAlive flag to false if it made the
     * player lose.
     */
    void eventReceivePenaltyRows(int numPenalties);

    // #### Getters ####

    /**
     * @brief Returns how many Tetrominoes are waiting in the queue.
     *
     * @return The size of the queue.
     */
    size_t getTetrominoesQueueSize() const;

    /**
     * @brief Inserts the given tetromino at the front of the tetrominoes queue.
     */
    void insertNextTetromino(TetrominoPtr &&pTetromino);

    /**
     * @brief Creates an return a new Tetromino located at the top of the board.
     */
    static TetrominoPtr createTetromino(TetrominoShape tetrominoShape);

    /**
     * @brief Computes the initial anchor point for a Tetromino of the given
     * shape so that the Tetromino is positioned at the top of the board.
     */
    static Vec2 getTetrominoInitialAnchorPoint(TetrominoShape tetrominoShape);

    /**
     * @brief Destroys a random 2 by 2 square in
     * which all the cells are occupied in the board if found.
     */
    void destroy2By2Occupied();

    /* ------------------------------------------------
     *          Serialization
     * ------------------------------------------------*/

    nlohmann::json serializeSelf(bool emptyBoard = false) const;

    nlohmann::json serializeExternal() const;

    /* ------------------------------------------------
     *          Test Fixture Class
     * ------------------------------------------------*/

    friend TetrisTest;
};

using TetrisPtr = std::shared_ptr<Tetris>;

#endif
