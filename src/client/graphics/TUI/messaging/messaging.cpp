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

#include "messaging.hpp"

#include "../../../core/controller/controller.hpp"
#include "../ftxui_config/ftxui_config.hpp"
#include "../main_tui.hpp"
#include <ftxui/dom/elements.hpp>

namespace TUI {
    // ### constructor ###
    Messaging::Messaging(MainTui &mainTui, Controller &controller)
        : mainTui_(mainTui), controller_(controller) {
        userState_ = MessagingState::NONE;
        createButtons();
    }

    // ### protected methods ###

    void Messaging::createButtons() {
        sendButton_ =
            ftxui::Button(
                std::string(STR_SEND),
                [&] {
                    if (!newMessageBuffer_.empty()) {
                        getSelectedFriendId().transform([this](UserID userID) {
                            controller_.sendMessage(userID, newMessageBuffer_);
                            return 0;
                        });
                        newMessageBuffer_.clear();
                    }
                },
                GlobalButtonStyle())
            | ftxui::center;

        backButton_ = ftxui::Button(
                          std::string(STR_BACK),
                          [&] {
                              newMessageBuffer_.clear();
                              newFriendBuffer_.clear();
                              userState_ = MessagingState::BACK;
                              mainTui_.stopRender();
                          },
                          GlobalButtonStyle())
                      | ftxui::center;
    }

    void Messaging::drawInputUser() {
        newFriendBuffer_.clear();

        messageInput_ =
            ftxui::Input(&newMessageBuffer_, std::string(STR_WRITE_MESSAGE))
            | ftxui::center | ftxui::borderHeavy
            | ftxui::CatchEvent([&](ftxui::Event event) {
                  if (event == ftxui::Event::Return) {
                      if (!newMessageBuffer_.empty()) {
                          getSelectedFriendId().transform([&](UserID userID) {
                              controller_.sendMessage(userID,
                                                      newMessageBuffer_);
                              return 0;
                          });
                          newMessageBuffer_.clear();
                      }
                      return true;
                  }
                  return false;
              });
        ;
    }

    void Messaging::drawMenu() {
        drawInputUser();

        auto &friendsList = controller_.getFriendsList();

        friendNames_.clear();
        // NOTE: if we do not resize first, it will push_back out of the vector
        // -> memory problems
        friendNames_.resize(friendsList.size());
        std::transform(friendsList.begin(), friendsList.end(),
                       friendNames_.begin(),
                       [](const bindings::User &user) -> std::string {
                           return user.username;
                       });

        friendsMenu_ = ftxui::Menu(&friendNames_, &selectedFriend_);
    }

    void Messaging::drawDisplay() {
        drawMenu();

        sidebar_ = ftxui::Container::Vertical({
            friendsMenu_,
        });

        chatDisplay_ = ftxui::Renderer([&] {
            return getSelectedFriendId()
                .transform([this](auto id) -> ftxui::Element {
                    ftxui::Elements chat_elements;

                    auto [name, conversation] =
                        controller_.getConversationWith(id);

                    if (conversation.senderMessages.empty()) {
                        chat_elements.push_back(
                            ftxui::text(std::string("No messages yet")));
                    }

                    else {
                        for (auto &[senderId, message] :
                             conversation.senderMessages) {
                            if (senderId == id) {
                                chat_elements.push_back(
                                    ftxui::text(name + " : " + message)
                                    | ftxui::bold
                                    | ftxui::color(ftxui::Color::Blue));
                            } else {
                                chat_elements.push_back(
                                    ftxui::text("me : " + message) | ftxui::bold
                                    | ftxui::color(ftxui::Color::White));
                            }
                        }
                    }

                    return ftxui::vbox(chat_elements) | ftxui::flex;
                })
                .value_or(ftxui::text(std::string(STR_NO_CONVERSATION)));
        });
    }

    void Messaging::drawWindow() {
        drawDisplay();

        displayWindow_ = ftxui::Renderer(
            ftxui::Container::Horizontal({
                sidebar_,
                ftxui::Container::Vertical({
                    chatDisplay_,
                    messageInput_,
                    sendButton_,
                    backButton_,
                }),
            }),
            [&] {
                return ftxui::hbox({
                           ftxui::vbox({
                               ftxui::text(std::string(STR_FRIENDS_LIST_TITLE))
                                   | ftxui::bold
                                   | ftxui::color(ftxui::Color::Green)
                                   | ftxui::center,
                               ftxui::separator(),
                               friendsMenu_->Render(),

                           }) | ftxui::borderHeavy,

                           ftxui::vbox({
                               ftxui::text(std::string(STR_CONVERSATION_TITLE))
                                   | ftxui::bold
                                   | ftxui::color(ftxui::Color::Green)
                                   | ftxui::center,
                               ftxui::separator(),
                               chatDisplay_->Render() | ftxui::flex,
                               ftxui::separator(),
                               messageInput_->Render(),
                               ftxui::separator(),

                               ftxui::hbox({
                                   sendButton_->Render(),
                                   backButton_->Render(),
                               }) | ftxui::center,

                           }) | ftxui::borderHeavy
                               | ftxui::flex,
                       })
                       | ftxui::borderHeavy;
            });
    }

    std::optional<UserID> Messaging::getSelectedFriendId() {
        const std::vector<bindings::User> friendsList =
            controller_.getFriendsList();

        return (friendsList.empty())
                   ? std::nullopt
                   : std::make_optional(friendsList.at(selectedFriend_).userID);
    }

    // ### public methods ###
    void Messaging::render() {
        drawWindow();
        mainTui_.render(displayWindow_);
    }
} // namespace TUI
