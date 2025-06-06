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

#include "game_state.hpp"
#include "effect/bonus/bonus_type.hpp"
#include "effect_price/effect_price.hpp"
#include "game_engine/game_engine.hpp"
#include "game_mode/game_mode.hpp"
#include "nlohmann/json_fwd.hpp"
#include "player_state/player_state.hpp"
#include "player_tetris/player_tetris.hpp"

#include <memory>
#include <optional>
#include <vector>

GameState::GameState(GameMode gameMode, std::vector<PlayerState> &&playerStates)
    : isFinished_{false}, gameMode_{gameMode} {

    size_t numPlayers = playerStates.size();
    for (size_t i = 0; i < playerStates.size(); i++) {
        PlayerState &playerState = playerStates.at(i);

        playerState.setPenaltyTarget(
            playerStates.at((i + 1) % numPlayers).getUserID());

        playerState.toggleEffects(GameEngine::checkFeatureEnabled(
            gameMode, GameEngine::GameModeFeature::Effects));

        userToPlayerTetris_.emplace(
            playerState.getUserID(),
            PlayerTetris{std::make_shared<PlayerState>(playerState)});
    }
}

GameMode GameState::getGameMode() const { return gameMode_; }

std::optional<UserID> GameState::getWinner() const {
    if (gameMode_ == GameMode::Endless) {
        // There can't be any winner in Endless mode
        return std::nullopt;
    }

    std::optional<UserID> winner;

    for (const auto &[_, playerStateTetris] : userToPlayerTetris_) {
        if (playerStateTetris.pPlayerState->isAlive()) {
            if (winner.has_value()) {
                // had already found a player that is
                // still alive -> more than one player
                // alive -> no winner yet.
                return std::nullopt;
            }

            winner = playerStateTetris.pPlayerState->getUserID();
        }
    }

    return winner;
}

PlayerStatePtr GameState::getPlayerState(UserID userID) {
    auto it = userToPlayerTetris_.find(userID);
    return it != userToPlayerTetris_.end() ? it->second.pPlayerState : nullptr;
}

TetrisPtr GameState::getTetris(UserID userID) {
    auto it = userToPlayerTetris_.find(userID);
    return it != userToPlayerTetris_.end() ? it->second.pTetris : nullptr;
}

std::map<UserID, PlayerTetris> &GameState::getUserToPlayerTetris() {
    return userToPlayerTetris_;
}

void GameState::setIsFinished(bool isFinished) { isFinished_ = isFinished; }

/* ------------------------------------------------
 *          Serialization
 * ------------------------------------------------*/

nlohmann::json GameState::serializeForPlayer(UserID userID) const {
    nlohmann::json j;
    j["isFinished"] = isFinished_;
    j["gameMode"] = gameMode_;
    j["externals"] = nlohmann::json::array();

    for (const auto &[_, playerTetris] : userToPlayerTetris_) {
        if (playerTetris.pPlayerState->getUserID() == userID) {
            j["self"] = playerTetris.serializeSelf();
        } else {
            j["externals"].push_back(playerTetris.serializeExternal());
        }
    }

    // add the effects that the players can buy with their price
    j["bonusToPrice"] = nlohmann::json::array();
    j["penaltyToPrice"] = nlohmann::json::array();

    if (GameEngine::checkFeatureEnabled(gameMode_,
                                        GameEngine::GameModeFeature::Effects)) {
        // bonuses
        for (size_t i = 0; i < static_cast<size_t>(BonusType::NumBonusType);
             i++) {
            BonusType bonusType = static_cast<BonusType>(i);
            j["bonusToPrice"].push_back({bonusType, getEffectPrice(bonusType)});
        }
        // penalties
        for (size_t i = 0; i < static_cast<size_t>(PenaltyType::NumPenaltyType);
             i++) {
            PenaltyType penaltyType = static_cast<PenaltyType>(i);
            j["penaltyToPrice"].push_back(
                {penaltyType, getEffectPrice(penaltyType)});
        }
    }

    return j;
}

nlohmann::json GameState::serializeForViewer() const {
    nlohmann::json j;

    j["isFinished"] = isFinished_;
    j["gameMode"] = gameMode_;

    j["externals"] = nlohmann::json::array();

    for (const auto &[_, playerTetris] : userToPlayerTetris_) {
        j["externals"].push_back(playerTetris.serializeExternal());
    }

    return j;
}
