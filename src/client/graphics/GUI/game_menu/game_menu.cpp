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

// Authors: Jonas, Ernest, Rafaou

#include "game_menu.hpp"

#include "../../../core/controller/controller.hpp"
#include "../main_gui.hpp"
#include "../qt_config/qt_config.hpp"
#include "graphics/GUI/game_display/game_display.hpp"

#include <QTimer>

namespace GUI {

    GameMenu::GameMenu(Controller &controller, TetrisWindow &tetrisWindow,
                       QWidget *parent)
        : QWidget(parent), controller_(controller), tetrisWindow_(tetrisWindow),
          playerCount_(4), isCreateGame_(false) {
        setup();
    }

    void GameMenu::run(bool isCreateGame) {
        isCreateGame_ = isCreateGame;
        showSelectModeScreen();
    }

    void GameMenu::setup() {
        stack_.setParent(this);

        QPushButton *endlessButton = new QPushButton(this);
        endlessButton->setAutoDefault(true);
        QPushButton *dualButton = new QPushButton(this);
        dualButton->setAutoDefault(true);
        QPushButton *classicButton = new QPushButton(this);
        classicButton->setAutoDefault(true);
        QPushButton *royalButton = new QPushButton(this);
        royalButton->setAutoDefault(true);
        QPushButton *selectModeBackButton = new QPushButton(this);
        selectModeBackButton->setAutoDefault(true);
        QPushButton *playerCountBackButton = new QPushButton(this);
        playerCountBackButton->setAutoDefault(true);
        QPushButton *joinTypeBackButton = new QPushButton(this);
        joinTypeBackButton->setAutoDefault(true);
        QPushButton *friendsListBackButton = new QPushButton(this);
        friendsListBackButton->setAutoDefault(true);
        QPushButton *waitingBackButton = new QPushButton(this);
        waitingBackButton->setAutoDefault(true);
        QPushButton *confirmButton = new QPushButton(this);
        confirmButton->setAutoDefault(true);
        QPushButton *joinRandomButton = new QPushButton(this);
        joinRandomButton->setAutoDefault(true);
        QPushButton *joinFriendButton = new QPushButton(this);
        joinFriendButton->setAutoDefault(true);
        QPushButton *spectateFriendButton = new QPushButton(this);
        spectateFriendButton->setAutoDefault(true);

        // Configure buttons
        endlessButton->setText("Endless Mode");
        endlessButton->setFixedWidth(500);
        dualButton->setText("Dual Mode");
        dualButton->setFixedWidth(500);
        classicButton->setText("Classic Mode");
        classicButton->setFixedWidth(500);
        royalButton->setText("Royal Competition Mode");
        royalButton->setFixedWidth(500);

        // Set up separate back buttons for each screen
        selectModeBackButton->setText("Back");
        selectModeBackButton->setFixedWidth(500);
        playerCountBackButton->setText("Back");
        playerCountBackButton->setFixedWidth(500);
        joinTypeBackButton->setText("Back");
        joinTypeBackButton->setFixedWidth(500);
        friendsListBackButton->setText("Back");
        friendsListBackButton->setFixedWidth(500);
        waitingBackButton->setText("Back");
        waitingBackButton->setFixedWidth(500);

        confirmButton->setText("Confirm");
        confirmButton->setFixedWidth(500);
        joinRandomButton->setText("Join Random Game");
        joinRandomButton->setFixedWidth(500);
        joinFriendButton->setText("Join Friend's Game");
        joinFriendButton->setFixedWidth(500);
        spectateFriendButton->setText("Spectate Friend's Game");
        spectateFriendButton->setFixedWidth(500);

        // Connect signals for game mode buttons
        connect(endlessButton, &QPushButton::clicked, this,
                &GameMenu::onEndlessButtonClicked);
        connect(dualButton, &QPushButton::clicked, this,
                &GameMenu::onDualButtonClicked);
        connect(classicButton, &QPushButton::clicked, this,
                &GameMenu::onClassicButtonClicked);
        connect(royalButton, &QPushButton::clicked, this,
                &GameMenu::onRoyalButtonClicked);

        // Connect all back buttons to the same slot
        connect(selectModeBackButton, &QPushButton::clicked, this,
                &GameMenu::onBackButtonClicked);
        connect(playerCountBackButton, &QPushButton::clicked, this,
                &GameMenu::onBackButtonClicked);
        connect(joinTypeBackButton, &QPushButton::clicked, this,
                &GameMenu::onBackButtonClicked);
        connect(friendsListBackButton, &QPushButton::clicked, this,
                &GameMenu::onBackButtonClicked);
        connect(waitingBackButton, &QPushButton::clicked, this,
                &GameMenu::onBackButtonClicked);

        connect(confirmButton, &QPushButton::clicked, this,
                &GameMenu::onConfirmButtonClicked);
        connect(joinRandomButton, &QPushButton::clicked, this,
                &GameMenu::onJoinRandomButtonClicked);
        connect(joinFriendButton, &QPushButton::clicked, this,
                &GameMenu::onJoinFriendButtonClicked);
        connect(spectateFriendButton, &QPushButton::clicked, this,
                &GameMenu::onSpectateFriendButtonClicked);

        // Setup select mode screen
        QVBoxLayout *selectModeLayout = new QVBoxLayout(&selectModeWidget_);
        selectModeLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                                  QSizePolicy::Expanding));
        selectModeLayout->addWidget(createCenterBoldTitle("Select Game Mode"));
        selectModeLayout->addWidget(endlessButton, 0, Qt::AlignCenter);
        selectModeLayout->addWidget(dualButton, 0, Qt::AlignCenter);
        selectModeLayout->addWidget(classicButton, 0, Qt::AlignCenter);
        selectModeLayout->addWidget(royalButton, 0, Qt::AlignCenter);
        selectModeLayout->addWidget(selectModeBackButton, 0, Qt::AlignCenter);
        selectModeLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                                  QSizePolicy::Expanding));

        // Setup player count screen
        QVBoxLayout *playerCountLayout = new QVBoxLayout(&playerCountWidget_);
        playerCountSlider_.setParent(this);
        playerCountSlider_.setOrientation(Qt::Orientation::Horizontal);
        playerCountSlider_.setMinimum(MIN_NUM_PLAYERS_CLASSIC_ROYAL);
        playerCountSlider_.setMaximum(MAX_NUM_PLAYERS_CLASSIC_ROYAL);
        playerCountSlider_.setValue(MAX_NUM_PLAYERS_CLASSIC_ROYAL);
        playerCountSlider_.setFixedWidth(500);
        playerCountLabel_.setAlignment(Qt::AlignCenter);
        connect(&playerCountSlider_, &QSlider::valueChanged, this,
                &GameMenu::onPlayerCountChanged);

        playerCountLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                                   QSizePolicy::Expanding));
        playerCountLayout->addWidget(
            createCenterBoldTitle("Select Player Count"));
        playerCountLayout->addWidget(&playerCountLabel_, 0, Qt::AlignCenter);
        playerCountLayout->addWidget(&playerCountSlider_, 0, Qt::AlignCenter);
        playerCountLayout->addWidget(confirmButton, 0, Qt::AlignCenter);
        playerCountLayout->addWidget(playerCountBackButton, 0, Qt::AlignCenter);
        playerCountLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                                   QSizePolicy::Expanding));

        // Setup join type screen
        QVBoxLayout *joinTypeLayout = new QVBoxLayout(&joinTypeWidget_);
        joinTypeLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                                QSizePolicy::Expanding));
        joinTypeLayout->addWidget(createCenterBoldTitle("Join Game"));
        joinTypeLayout->addWidget(joinRandomButton, 0, Qt::AlignCenter);
        joinTypeLayout->addWidget(joinFriendButton, 0, Qt::AlignCenter);
        joinTypeLayout->addWidget(spectateFriendButton, 0, Qt::AlignCenter);
        joinTypeLayout->addWidget(joinTypeBackButton, 0, Qt::AlignCenter);
        joinTypeLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                                QSizePolicy::Expanding));

        // Setup friends list screen
        QVBoxLayout *friendsListLayout = new QVBoxLayout(&friendsListWidget_);
        friendsList_.setFixedWidth(500);
        friendsListLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                                   QSizePolicy::Expanding));
        friendsListLayout->addWidget(createCenterBoldTitle("Select a Friend"));
        friendsListLayout->addWidget(&friendsList_, 0, Qt::AlignCenter);
        friendsListLayout->addWidget(friendsListBackButton, 0, Qt::AlignCenter);
        friendsListLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                                   QSizePolicy::Expanding));
        connect(&friendsList_, &QListWidget::itemDoubleClicked, this,
                &GameMenu::onFriendSelected);

        // Setup waiting screen
        QVBoxLayout *waitingLayout = new QVBoxLayout(&waitingWidget_);
        waitingLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                               QSizePolicy::Expanding));
        waitingLayout->addWidget(createCenterBoldTitle("Waiting for Game"));
        QLabel *waitingLabel =
            new QLabel("Please wait while we find players for your game...");
        waitingLabel->setAlignment(Qt::AlignCenter);
        waitingLayout->addWidget(waitingLabel, 0, Qt::AlignCenter);
        waitingLayout->addWidget(waitingBackButton, 0, Qt::AlignCenter);
        waitingLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                               QSizePolicy::Expanding));

        // Add widgets to stack
        stack_.addWidget(&selectModeWidget_);
        stack_.addWidget(&playerCountWidget_);
        stack_.addWidget(&joinTypeWidget_);
        stack_.addWidget(&friendsListWidget_);
        stack_.addWidget(&waitingWidget_);

        // Set main layout
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(&stack_);
        setLayout(mainLayout);
    }

    // Screen display methods
    void GameMenu::showSelectModeScreen() {
        stack_.setCurrentWidget(&selectModeWidget_);
    }

    void GameMenu::showPlayerCountScreen() {
        stack_.setCurrentWidget(&playerCountWidget_);
    }

    void GameMenu::showJoinTypeScreen() {
        stack_.setCurrentWidget(&joinTypeWidget_);
    }

    void GameMenu::showFriendsListScreen() {
        updateFriendsList();
        stack_.setCurrentWidget(&friendsListWidget_);
    }

    void GameMenu::showWaitingScreen() {
        stack_.setCurrentWidget(&waitingWidget_);

        // Check periodically if we've joined a game
        QTimer *gameCheckTimer = new QTimer(this);
        connect(gameCheckTimer, &QTimer::timeout, [this, gameCheckTimer]() {
            if (controller_.inGame()) {
                gameCheckTimer->stop();
                gameCheckTimer->deleteLater();
                createAndShowGameDisplay();
            }
        });
        gameCheckTimer->start(100); // Check every 100ms
    }

    void GameMenu::updateFriendsList() {
        friendsList_.clear();

        const std::vector<bindings::User> friendsList =
            controller_.getFriendsList();
        for (const auto &friendUser : friendsList) {
            if (friendUser.isJoinable()
                && friendUser.gameMode == selectedGameMode_) {
                QListWidgetItem *item = new QListWidgetItem(
                    QString::fromStdString(friendUser.username));
                item->setData(Qt::UserRole,
                              QVariant::fromValue(friendUser.userID));
                item->setTextAlignment(Qt::AlignCenter);
                friendsList_.addItem(item);
            }
        }

        if (friendsList_.count() == 0) {
            QListWidgetItem *noFriendsItem =
                new QListWidgetItem("No friends available to join");
            noFriendsItem->setFlags(noFriendsItem->flags()
                                    & ~Qt::ItemIsEnabled);
            noFriendsItem->setTextAlignment(Qt::AlignCenter);
            friendsList_.addItem(noFriendsItem);
        }
    }

    void GameMenu::createAndShowGameDisplay() {
        gameDisplay_ = new GameDisplay(controller_, tetrisWindow_, this);
        connect(gameDisplay_, &GameDisplay::backToMainMenu, this, [this] {
            stack_.removeWidget(gameDisplay_);
            gameDisplay_->deleteLater();
            gameDisplay_.clear(); 
            QTimer::singleShot(0, this, [this]() {
                emit backToMainMenu();
            });
        });
        stack_.addWidget(gameDisplay_);
        stack_.setCurrentWidget(gameDisplay_);
        gameDisplay_->setFocus(Qt::ActiveWindowFocusReason);
    }

    // Slot implementations
    void GameMenu::onEndlessButtonClicked() {
        selectedGameMode_ = GameMode::Endless;

        controller_.joinGame(selectedGameMode_, std::nullopt);

        createAndShowGameDisplay();
    }

    void GameMenu::onDualButtonClicked() {
        selectedGameMode_ = GameMode::Dual;

        if (isCreateGame_) {
            // dual always has 2 players
            controller_.createGame(selectedGameMode_, 2);
            showWaitingScreen();
        } else {
            showJoinTypeScreen();
        }
    }

    void GameMenu::onClassicButtonClicked() {
        selectedGameMode_ = GameMode::Classic;

        if (isCreateGame_) {
            showPlayerCountScreen();
        } else {
            showJoinTypeScreen();
        }
    }

    void GameMenu::onRoyalButtonClicked() {
        selectedGameMode_ = GameMode::RoyalCompetition;

        if (isCreateGame_) {
            showPlayerCountScreen();
        } else {
            showJoinTypeScreen();
        }
    }

    void GameMenu::onBackButtonClicked() {
        if (stack_.currentWidget() == &selectModeWidget_) {
            QTimer::singleShot(0, this, [this]() {
                emit backToMainMenu();
            });
        } else if (stack_.currentWidget() == &playerCountWidget_) {
            showSelectModeScreen();
        } else if (stack_.currentWidget() == &joinTypeWidget_) {
            showSelectModeScreen();
        } else if (stack_.currentWidget() == &friendsListWidget_) {
            showJoinTypeScreen();
        } else if (stack_.currentWidget() == &waitingWidget_) {
            controller_.abortMatchmaking(); 
            showPlayerCountScreen();
        }
    }

    void GameMenu::onConfirmButtonClicked() {
        controller_.createGame(selectedGameMode_, playerCount_);
        showWaitingScreen();
    }

    void GameMenu::onJoinRandomButtonClicked() {
        controller_.joinGame(selectedGameMode_, std::nullopt);
        showWaitingScreen();
    }

    void GameMenu::onJoinFriendButtonClicked() {
        joinFriendType_ = JoinFriendType::JoinFriend;
        showFriendsListScreen();
    }

    void GameMenu::onSpectateFriendButtonClicked() {
        joinFriendType_ = JoinFriendType::SpectateFriend;
        showFriendsListScreen();
    }

    void GameMenu::onPlayerCountChanged(int value) {
        playerCount_ = value;
        playerCountLabel_.setText(QString("Player Count: %1").arg(value));
    }

    void GameMenu::onFriendSelected(QListWidgetItem *item) {
        if (item && item->flags() & Qt::ItemIsEnabled) {
            UserID friendId = item->data(Qt::UserRole).toULongLong();

            switch (joinFriendType_) {
            case JoinFriendType::JoinFriend:
                controller_.joinGame(selectedGameMode_, friendId);
                break;
            case JoinFriendType::SpectateFriend:
                controller_.joinGameAsViewer(friendId);
                break;
            }

            showWaitingScreen();
        }
    }

} // namespace GUI
