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

#include "client_link.hpp"

#include <iostream>

// ====== tcp connection class ======

// --- private ---

void ClientLink::read() {
    asio::async_read_until(
        socket_, streamBuffer_, bindings::PACKET_DELIMITER,
        [this](asio::error_code ec, std::size_t) {
            if (!ec) {

                handleReading();
            } else if (ec == asio::error::eof) {
                handleErrorReading();
            }
        });
}

bool ClientLink::checkPackage(std::string &package) {

    nlohmann::json checkJson;
    try {
        checkJson = nlohmann::json::parse(package);
    } catch (nlohmann::json::parse_error &e) {
        std::cerr << "Received packet is not valid JSON: " << e.what()
                  << std::endl;
        return false;
    }

    try {
        checkJson.at("type").get<bindings::BindingType>();
    } catch (nlohmann::json::exception &e) {
        std::cerr << "Received packet has no type  : " << e.what() << std::endl;
        return false;
    }

    return true;
}

void ClientLink::handleReading() {
    std::istream is(&streamBuffer_);
    std::string packet;
    std::getline(is, packet);
    // check if the package has the correct format
    if (!checkPackage(packet)) {
        return;
    }
    if (getUserState() == bindings::State::Offline) {
        handleAuthentication(packet);
    } else {
        packetHandler_(packet, clientId.value());
    }
    read();
}

void ClientLink::handleErrorReading() {
    mustBeDeletedFromTheWaitingForAuthList_ = true;
    removeClientCallback_(clientId);
}

void ClientLink::handleAuthentication(std::string &packet) {

    nlohmann::json jsonPacket = nlohmann::json::parse(packet);
    std::optional<nlohmann::json> response = authPacketHandler_(jsonPacket);
    if (response && !(response->is_null())) {
        sendPackage(response.value());

        if (response.value()
                    .at(bindings::PACKET_TYPE_FIELD)
                    .get<bindings::BindingType>()
                == bindings::BindingType::AuthenticationResponse
            && response.value().at("data").at("success").get<bool>()) {
            authSuccessCallback_(shared_from_this(), jsonPacket.at("data"));
            userState = bindings::State::Menu;
            mustBeDeletedFromTheWaitingForAuthList_ = true;
        }
    }
}

void ClientLink::writeSocket(std::string &content) {
    asio::async_write(
        socket_, asio::buffer(content),
        [this](asio::error_code ec, std::size_t length) {
            if (!ec) {
                buffer_.erase(0, length);
            }
        });
}

// --- public ---
ClientLink::ClientLink(tcp::socket socket, PacketHandler packetHandler,
                       AuthPacketHandler authPacketHandler,
                       AuthSuccessCallback authSuccessCallback,
                       RemoveClientCallback removeClientCallback)
    : socket_(std::move(socket)), userState(bindings::State::Offline),
      clientId(std::nullopt), gameMode_(std::nullopt),
      packetHandler_(packetHandler), authPacketHandler_(authPacketHandler),
      authSuccessCallback_(authSuccessCallback),
      removeClientCallback_(removeClientCallback) {
    start();
}

void ClientLink::start() { read(); }

void ClientLink::jointGame(const std::weak_ptr<GameServer> &gameServer) {
    pGame_ = gameServer;
}

void ClientLink::exitGame() {
    setUserState(bindings::State::Menu);
    setGameMode(std::nullopt);
    resetGame();
}

void ClientLink::resetGame() { pGame_.reset(); }

bool ClientLink::shouldItBeDeletedFromTheList() {
    return mustBeDeletedFromTheWaitingForAuthList_;
}

void ClientLink::sendPackage(nlohmann::json package) {
    buffer_ = package.dump().append(bindings::PACKET_DELIMITER);
    writeSocket(buffer_);
}

void ClientLink::setClientId(const int id) { clientId = id; }

void ClientLink::setUserState(bindings::State newState) {
    userState = newState;
}
void ClientLink::setGameMode(std::optional<GameMode> newGameMode) {
    gameMode_ = newGameMode;
}

std::optional<GameMode> ClientLink::getGameMode() { return gameMode_; }

bindings::State ClientLink::getUserState() { return userState; }

UserID ClientLink::getUserID() { return clientId.value(); }

std::weak_ptr<GameServer> ClientLink::getGameServer() { return pGame_; }
