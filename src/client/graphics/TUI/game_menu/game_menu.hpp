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

#ifndef TUI_GAME_MENU_HPP
#define TUI_GAME_MENU_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>

#include "../game_display/game_display.hpp"

#include "../../../../common/types/types.hpp"

#include "../../../../common/tetris_royal_lib/game_mode/game_mode.hpp"

class Controller;

namespace TUI {

    class MainTui;

    /**
     * @brief Join a friend or join a random a game.
     */
    enum class JoinType {
        FRIEND,
        RANDOM,
        SPECTATE_FRIEND,
        BACK,
        BACK_WAITING_SCREEN,
        NONE,
    };

    /**
     * @brief Create a new game or join an existing game.
     */
    enum class TypeGame {
        CREATE_GAME,
        JOIN_GAME,
        NONE,
    };

    /**
     * @brief Game menu class to show and select the game party
     *
     */
    class GameMenu {
      private:
        MainTui &mainTui_;

        Controller &controller_;

        std::unique_ptr<GameDisplay> gameDisplay_;

        JoinType joinType_;

        GameMode gameMode_;

        TypeGame typeGame_;

        bool quitMenu_;

        /**
         * @brief The button components that can be in the game menu
         *
         */
        ftxui::Component endlessButton_;
        ftxui::Component duelButton_;
        ftxui::Component classicButton_;
        ftxui::Component royalButton_;
        ftxui::Component backButton_;
        ftxui::Component backButtonWaintingScreen_;
        ftxui::Component quitMenuButton_;
        /*
         * @brief Rend the all the game present in the game menu
         */
        void renderAllGames();

        /*
         * @brief Rend the game menu with just the online games mods
         */
        void renderOnlineGames();

        /*
         * @brief Screen to choose between joining a random game, joining a
         * friend, or spectate a friend.
         */
        void joinTypeScreen();

        /*
         * @brief Handle the choice of the user in the game menu
         */
        void handleChoice();

        /*
         * @brief Screen when the user is joining a friend, a list of the online
         * friends is displayed and the user can select one
         */
        void joinFriendScreen();

        /*
         * @brief Screen when the is waiting for a random game
         */
        void joinRandomScreen();

        /*
         * @brief Screen when the user is waiting for the matchmaking
         */
        void createGameScreen();

        /*
         * @brief Make a button to add a friend
         *
         * @param userID The ID of the friend
         * @param friendName The name of the friend
         */
        ftxui::Component makeFriendButton(UserID userID,
                                          const std::string &friendName);

        /*
         * @brief Screen when the user has choosen the friend and now waiting
         * for the start of the game
         */
        void waitingFriendScreen();

        /*
         * @brief The screen to select the number of player for the game
         */
        void selectPlayerCountScreen();

      public:
        /*
         * @brief Construct a new Game Menu object
         *
         * @param mainTui The main TUI to render the components
         * @param controller The controller to interact with the server
         */
        GameMenu(MainTui &mainTui, Controller &controller);

        /*
         * @brief Destroy the Game Menu object
         */
        ~GameMenu() = default;

        /*
         * @brief Render the game menu
         *
         * @param typeGame The type of the game to render
         */
        void render(const TypeGame &typeGame);
    };

} // namespace TUI

#endif // TUI_GAME_MENU_HPP
