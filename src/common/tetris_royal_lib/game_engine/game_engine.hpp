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

#ifndef GAME_ENGINE_HPP
#define GAME_ENGINE_HPP

#include <array>
#include <bitset>
#include <initializer_list>
#include <optional>
#include <stddef.h>

#include "../../types/types.hpp"
#include "../game_mode/game_mode.hpp"
#include "../game_state/game_state.hpp"
#include "effect/effect_type.hpp"
#include "player_state/player_state.hpp"
#include "tetromino/abstract_tetromino.hpp"

class Tetris;

enum class PenaltyType;

struct PlayerTetris;

class GameEngine {
  public:
    enum class GameModeFeature {
        PenaltyRows,
        Effects,
        SelectPenaltyTarget,
        NumGameModeFeature,
    };

    static constexpr size_t numFeatures =
        static_cast<size_t>(GameModeFeature::NumGameModeFeature);

    static constexpr size_t numGameMode =
        static_cast<size_t>(GameMode::NumGameMode);

    /**
     * @brief Types used for constant-time feature lookup per GameMode.
     */
    using FeaturesBitset = std::bitset<GameEngine::numFeatures>;
    using FeaturesMap = std::array<FeaturesBitset, numGameMode>;

  private:
    // #### GameState ####

    GameStatePtr pGameState_;

    // #### Features-related ####

    /**
     * @brief Generates a FeaturesBitsets at compile time.
     */
    static constexpr auto genFeaturesBitset =
        [](std::initializer_list<GameEngine::GameModeFeature> enabledFeatures)
        -> GameEngine::FeaturesBitset {
        GameEngine::FeaturesBitset bitset;

        for (GameEngine::GameModeFeature features : enabledFeatures) {
            bitset.set(static_cast<size_t>(features));
        }

        return bitset;
    };

    constexpr static FeaturesMap featuresBitsets =
        []() -> GameEngine::FeaturesMap {
        GameEngine::FeaturesMap featuresPerGameMode;

        featuresPerGameMode.at(static_cast<size_t>(GameMode::Endless)) =
            genFeaturesBitset({
                // Endless features go here
            });

        featuresPerGameMode.at(static_cast<size_t>(GameMode::Dual)) =
            genFeaturesBitset({
                // Dual features go here
                GameModeFeature::PenaltyRows,
            });

        featuresPerGameMode.at(static_cast<size_t>(GameMode::Classic)) =
            genFeaturesBitset({
                // Classic features go here
                GameModeFeature::PenaltyRows,
                GameModeFeature::SelectPenaltyTarget,
            });

        featuresPerGameMode.at(static_cast<size_t>(
            GameMode::RoyalCompetition)) = genFeaturesBitset({
            // Royal features go here
            GameModeFeature::Effects,
            GameModeFeature::SelectPenaltyTarget,
        });

        return featuresPerGameMode;
    }();

    /**
     * @brief Checks whether the given feature is enabled for the current
     * GameMode.
     */
    bool checkFeatureEnabled(GameModeFeature gameModeFeature) const;

    /**
     * @brief Returns the number of points awarded for clearing
     * the given number of rows.
     */
    Score calculatePointsClearedRows(size_t numClearedRows) const;

    /**
     * @brief Returns the amount of Energy awarded to the player for
     * clearing the given number of rows.
     */
    Energy calculateEnergyClearedRows(size_t numClearedRows) const;

    /**
     * @brief Checks that the given player is alive.
     */
    bool checkAlive(const PlayerStatePtr &pPlayerState) const;

    /**
     * @brief Handles energy and PenaltyRows for the given player when his
     * Tetromino gets placed.
     */
    void onTetrominoPlaced(PlayerState &playerState, size_t numClearedRows);

    /**
     * @brief Makes an engine tick happen for the given player.
     */
    void tick(PlayerTetris &playerTetris);

    // #### Effects Helpers ####

    /**
     * @brief Handles the timed bonus for the given player (one tick).
     */
    void handlePlayerTimedBonus(PlayerTetris &playerTetris);

    /**
     * @brief Handles the timed penalty for the given player (one tick).
     */
    void handlePlayerTimedPenalty(PlayerTetris &playerTetris);

    /**
     * @brief Handles the timed effects for the given player (one tick).
     */
    void handlePlayerTimedEffect(PlayerTetris &playerTetris);

    /**
     * @brief Returns true if the given player currently has the inverted
     * controls penalty.
     */
    bool shouldReverseControls(const PlayerState &playerState) const;

    /**
     * @brief Returns true if the given player currently has the input lock
     * penalty.
     */
    bool shouldLockInput(const PlayerState &playerState) const;

    /**
     * @brief Returns the opposite move of the given TetrominioMove
     */
    TetrominoMove invertTetrominoMove(TetrominoMove tetrominoMove) const;

    /**
     * @brief Returns true if the tick should be ignored for the given
     * player. (Due to slowdown bonus).
     */
    bool shouldIgnoreTick(const PlayerState &playerState) const;

    /**
     * @brief Sends the given penalty to the sender's selected target.
     */
    void sendPenaltyEffect(const PlayerState &playerStateSender,
                           PenaltyType penaltyType);

    /**
     * @brief Makes the sender send penalty rows to its chosen target.
     */
    void sendPenaltyRows(const PlayerState &playerStateSender, size_t numRows);

    /**
     * @brief Returns true if the given player has enough energy to buy the
     * given effect; false otherwise.
     */
    bool checkCanBuyEffect(const PlayerState &playerState,
                           EffectType effectType) const;

    /**
     * @brief Inserts two mini tetrominoes at the front of the given
     * player's tetrominoes queue.
     */
    void handleMiniTetrominoes(Tetris &tetris);

    /**
     * @brief Destroys a 2x2 block in a random position in the player's grid
     * if there one was found;otherwise, doesn't do anything.
     */
    void handleLightning(Tetris &tetris);

  public:
    /**
     * @brief Constructor
     *
     * @param pGameState A shared pointer to the GameState that the
     * GameEngine should manage.
     */
    GameEngine(const GameStatePtr &pGameState);
    GameEngine(const GameEngine &) = default;
    GameEngine(GameEngine &&) = default;
    GameEngine &operator=(const GameEngine &) = default;
    GameEngine &operator=(GameEngine &&) = default;

    ~GameEngine() = default;

    /**
     * @brief Makes the given player buy the given effect if he has enough
     * energy for it, stashes the effect for later or activates it
     * instantly.
     */
    void tryBuyEffect(UserID userID, EffectType effectType,
                      bool stashForLater = false);

    /**
     * @brief Changes the given player's target to the new target.
     * @param userID The player whose target will be changed.
     * @param target The new target.
     */
    void selectTarget(UserID userID, UserID target);

    /**
     * @brief Moves the player's active Tetromino in the given direction if
     * possible.
     */
    void tryMoveActive(UserID userID, TetrominoMove tetrominoMove);

    /**
     * @brief Drops the active Tetromino until it hits the bottom or an
     * occupied cell.
     */
    void bigDrop(UserID userID);

    /**
     * @brief Holds the active tetromino.
     */
    void holdActiveTetromino(UserID userID);

    /**
     * @brief Rotates the player's active Tetromino in the given direction
     * if possible.
     */
    void tryRotateActive(UserID userID, bool rotateClockwise);

    /**
     * @brief Sends all the penalties that were stashed.
     */
    void emptyPenaltyStash(UserID userID);

    /**
     * @brief Creates an engine tick, making everything update.
     */
    void tick();

    /**
     * @brief Returns the winner's userID if there is one, nullopt otherwise.
     */
    std::optional<UserID> getWinner() const;

    /**
     * @brief Checks that the given player is alive.
     */
    bool checkAlive(UserID userID) const;

    /**
     * @brief Returns true if the game is finished.
     */
    bool gameIsFinished() const;

    /**
     * @brief Checks whether the given feature is enabled for the given
     * GameMode.
     */
    static bool checkFeatureEnabled(GameMode gameMode,
                                    GameModeFeature gameModeFeature);

    /**
     * @brief Quits the game for the given player.
     */
    void quitGame(const UserID userID);
};

#endif // GAME_ENGINE_HPP
