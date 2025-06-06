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

#include "friends_menu.hpp"

#include "../../../core/controller/controller.hpp"
#include "../qt_config/qt_config.hpp"
#include "../tetris_window.hpp"

#include <QMessageBox>


namespace GUI {

    FriendsMenu::FriendsMenu(Controller &controller, TetrisWindow &tetrisWindow,
                             QWidget *parent)
        : QWidget(parent), controller_(controller),
          tetrisWindow_(tetrisWindow) {
        setup();

        connect(&tetrisWindow_, &TetrisWindow::updateFriendsList, this,
                &FriendsMenu::refreshFriendsList);
        connect(&tetrisWindow_, &TetrisWindow::updateFriendRequestsList, this,
                &FriendsMenu::refreshFriendRequestsList);
    }

    void FriendsMenu::refreshFriendsList() { updateFriendsList(); }

    void FriendsMenu::refreshFriendRequestsList() {
        updateFriendRequestsList();
    }

    void FriendsMenu::showMainFriendsMenu() { stack_.setCurrentIndex(0); }

    void FriendsMenu::showAddFriendScreen() {
        friendNameInput_.clear();
        addFriendMsgLabel_.clear();
        stack_.setCurrentIndex(1);
        friendNameInput_.setFocus();
    }

    void FriendsMenu::showFriendRequestsScreen() { stack_.setCurrentIndex(2); }

    void FriendsMenu::sendFriendRequest() {
        QString name = friendNameInput_.text();
        if (name.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please enter a username");
            return;
        }

        controller_.sendFriendRequest(name.toStdString());
        addFriendMsgLabel_.setText("Request sent to " + name);
        friendNameInput_.clear();
    }

    void FriendsMenu::removeFriend() {
        QListWidgetItem *item = friendsList_.currentItem();
        if (!item) return;

        int userID = item->data(Qt::UserRole).toInt();
        QString username = item->text();

        int confirm =
            QMessageBox::question(this, "Remove Friend",
                                  "Are you sure you want to remove " + username
                                      + " from your friend list?");
        if (confirm == QMessageBox::Yes) {
            controller_.removeFriend(userID);
        }
    }

    void FriendsMenu::acceptRequest() {
        QListWidgetItem *item = friendRequestsList_.currentItem();
        if (!item) return;

        int userID = item->data(Qt::UserRole).toInt();
        controller_.acceptFriendRequest(userID);
    }

    void FriendsMenu::declineRequest() {
        QListWidgetItem *item = friendRequestsList_.currentItem();
        if (!item) return;

        int userID = item->data(Qt::UserRole).toInt();
        controller_.declineFriendRequest(userID);
    }

    /*----------------------------------------------------------
                        Private methods
    -----------------------------------------------------------*/

    void FriendsMenu::setup() {
        QPushButton *addFriendButton_ = new QPushButton(this);
        addFriendButton_->setAutoDefault(true);
        QPushButton *manageRequestsButton_ = new QPushButton(this);
        manageRequestsButton_->setAutoDefault(true);
        QPushButton *backToMainMenuButton_ = new QPushButton(this);
        backToMainMenuButton_->setAutoDefault(true);
        QPushButton *submitAdd_ = new QPushButton(this);
        submitAdd_->setAutoDefault(true);
        QPushButton *backFromAdd_ = new QPushButton(this);
        backFromAdd_->setAutoDefault(true);
        QPushButton *acceptButton_ = new QPushButton(this);
        acceptButton_->setAutoDefault(true);
        QPushButton *declineButton_ = new QPushButton(this);
        declineButton_->setAutoDefault(true);
        QPushButton *backFromReqButton_ = new QPushButton(this);
        backFromReqButton_->setAutoDefault(true);

        QWidget *mainWidget_ = new QWidget(this);
        QWidget *addFriendWidget_ = new QWidget(this);
        QWidget *friendRequestsWidget_ = new QWidget(this);

        // Main Friends Menu
        mainWidget_ = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget_);

        friendsList_.setFixedWidth(500);

        noFriendsLabel_.setText("You don't have any friends yet");
        noFriendsLabel_.setAlignment(Qt::AlignCenter);
        noFriendsLabel_.setVisible(true);

        addFriendButton_->setText("Add Friend");
        addFriendButton_->setFixedWidth(500);
        manageRequestsButton_->setText("Manage Friend Requests");
        manageRequestsButton_->setFixedWidth(500);
        backToMainMenuButton_->setText("Back to Main Menu");
        backToMainMenuButton_->setFixedWidth(500);

        connect(addFriendButton_, &QPushButton::clicked, this,
                &FriendsMenu::showAddFriendScreen);
        connect(manageRequestsButton_, &QPushButton::clicked, this,
                &FriendsMenu::showFriendRequestsScreen);
        connect(backToMainMenuButton_, &QPushButton::clicked, this,
                &FriendsMenu::backToMainMenu);
        connect(&friendsList_, &QListWidget::itemDoubleClicked, this,
                &FriendsMenu::removeFriend);

        mainLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                            QSizePolicy::Expanding));
        mainLayout->addWidget(createCenterBoldTitle("Your Friends List"));
        mainLayout->addWidget(&noFriendsLabel_);
        mainLayout->addWidget(&friendsList_, 0, Qt::AlignCenter);
        mainLayout->addWidget(addFriendButton_, 0, Qt::AlignCenter);
        mainLayout->addWidget(manageRequestsButton_, 0, Qt::AlignCenter);
        mainLayout->addWidget(backToMainMenuButton_, 0, Qt::AlignCenter);
        mainLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                            QSizePolicy::Expanding));

        // Add Friend Screen
        addFriendWidget_ = new QWidget(this);
        QVBoxLayout *addLayout = new QVBoxLayout(addFriendWidget_);

        friendNameInput_.setPlaceholderText("Friend Username");
        friendNameInput_.setFixedWidth(500);
        friendNameInput_.setAlignment(Qt::AlignCenter);

        submitAdd_->setText("Send Request");
        submitAdd_->setFixedWidth(500);
        backFromAdd_->setText("Back");
        backFromAdd_->setFixedWidth(500);

        addFriendMsgLabel_.setAlignment(Qt::AlignCenter);

        connect(submitAdd_, &QPushButton::clicked, this,
                &FriendsMenu::sendFriendRequest);
        connect(backFromAdd_, &QPushButton::clicked, this,
                &FriendsMenu::showMainFriendsMenu);

        addLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                           QSizePolicy::Expanding));
        addLayout->addWidget(createCenterBoldTitle("Add a Friend"));
        addLayout->addWidget(&addFriendMsgLabel_);
        addLayout->addWidget(&friendNameInput_, 0, Qt::AlignCenter);
        addLayout->addWidget(submitAdd_, 0, Qt::AlignCenter);
        addLayout->addWidget(backFromAdd_, 0, Qt::AlignCenter);
        addLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                           QSizePolicy::Expanding));

        // Friend Requests Screen
        friendRequestsWidget_ = new QWidget(this);
        QVBoxLayout *requestLayout = new QVBoxLayout(friendRequestsWidget_);

        friendRequestsList_.setFixedWidth(500);

        noRequestsLabel_.setText("No pending friend requests");
        noRequestsLabel_.setAlignment(Qt::AlignCenter);
        noRequestsLabel_.setVisible(true);

        acceptButton_->setText("Accept");
        acceptButton_->setFixedWidth(500);
        declineButton_->setText("Decline");
        declineButton_->setFixedWidth(500);
        backFromReqButton_->setText("Back");
        backFromReqButton_->setFixedWidth(500);

        connect(acceptButton_, &QPushButton::clicked, this,
                &FriendsMenu::acceptRequest);
        connect(declineButton_, &QPushButton::clicked, this,
                &FriendsMenu::declineRequest);
        connect(backFromReqButton_, &QPushButton::clicked, this,
                &FriendsMenu::showMainFriendsMenu);

        requestLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                               QSizePolicy::Expanding));
        requestLayout->addWidget(
            createCenterBoldTitle("Pending Friend Requests"));
        requestLayout->addWidget(&noRequestsLabel_);
        requestLayout->addWidget(&friendRequestsList_, 0, Qt::AlignCenter);
        requestLayout->addWidget(acceptButton_, 0, Qt::AlignCenter);
        requestLayout->addWidget(declineButton_, 0, Qt::AlignCenter);
        requestLayout->addWidget(backFromReqButton_, 0, Qt::AlignCenter);
        requestLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                               QSizePolicy::Expanding));

        // Add all pages to stacked widget
        stack_.addWidget(mainWidget_);           // 0
        stack_.addWidget(addFriendWidget_);      // 1
        stack_.addWidget(friendRequestsWidget_); // 2

        QVBoxLayout *mainLayoutGlobal = new QVBoxLayout(this);
        mainLayoutGlobal->addWidget(&stack_);
        mainLayoutGlobal->setAlignment(Qt::AlignCenter);

        // refresh friends list and friend requests list before showing the main
        // menu of friendsMenu
        refreshFriendsList();
        refreshFriendRequestsList();
        showMainFriendsMenu();
    }

    void FriendsMenu::updateFriendsList() {
        friendsList_.clear();

        const std::vector<bindings::User> friendsList =
            controller_.getFriendsList();

        if (friendsList.empty()) {
            noFriendsLabel_.setVisible(true);
        } else {
            noFriendsLabel_.setVisible(false);

            for (const auto &friendUser : friendsList) {
                QListWidgetItem *item = new QListWidgetItem(
                    QString::fromStdString(friendUser.username));
                item->setData(Qt::UserRole,
                              QVariant::fromValue(friendUser.userID));
                item->setTextAlignment(Qt::AlignCenter);
                friendsList_.addItem(item);
            }
        }
    }

    void FriendsMenu::updateFriendRequestsList() {
        friendRequestsList_.clear();

        const std::vector<bindings::User> pendingFriendsList =
            controller_.getPendingFriendRequests();

        if (pendingFriendsList.empty()) {
            noRequestsLabel_.setVisible(true);
        } else {
            noRequestsLabel_.setVisible(false);

            for (const auto &request : pendingFriendsList) {
                QListWidgetItem *item = new QListWidgetItem(
                    QString::fromStdString(request.username));
                item->setData(Qt::UserRole,
                              QVariant::fromValue(request.userID));
                item->setTextAlignment(Qt::AlignCenter);
                friendRequestsList_.addItem(item);
            }
        }
    }

} // namespace GUI
