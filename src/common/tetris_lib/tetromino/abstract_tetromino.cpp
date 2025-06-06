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

#include "abstract_tetromino.hpp"

#include "../vec2/vec2.hpp"
#include "cyclic_index/cyclic_index.hpp"
#include "tetromino_shapes.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

/*--------------------------------------------------
                    PROTECTED
--------------------------------------------------*/

// #### SRS Offsets Data Constants ####

const std::vector<std::vector<Vec2>> ATetromino::O_OFFSET_DATA = {
    {{0, 0}},
    {{0, -1}},
    {{-1, -1}},
    {{-1, 0}},
};

const std::vector<std::vector<Vec2>> ATetromino::I_OFFSET_DATA = {
    {{0, 0}, {-1, 0}, {2, 0}, {-1, 0}, {2, 0}},
    {{-1, 0}, {0, 0}, {0, 0}, {0, 1}, {0, -2}},
    {{-1, 1}, {1, 1}, {-2, 1}, {1, 0}, {-2, 0}},
    {{0, 1}, {0, 1}, {0, 1}, {0, -1}, {0, 2}},
};

const std::vector<std::vector<Vec2>> ATetromino::ZLSJT_OFFSET_DATA = {
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
    {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
    {{0, 0}, {1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
};

// #### Constructor ####

ATetromino::ATetromino(Vec2 &&anchorPoint, std::vector<Vec2> &&body,
                       const std::vector<std::vector<Vec2>> *offsetData,
                       TetrominoShape shape)
    : shape_(shape), anchorPoint_{std::move(anchorPoint)},
      body_{std::move(body)}, rotationIdx_(NUM_ROTATIONS),
      prevRotationIdx_(NUM_ROTATIONS), offsetData_(offsetData) {

    int minY = std::numeric_limits<int>::max();
    int maxY = std::numeric_limits<int>::min();
    int minX = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();

    for (const auto [x, y] : body_) {
        minY = std::min(minY, y);
        minX = std::min(minX, x);
        maxY = std::max(maxY, y);
        maxX = std::max(maxX, x);
    }

    height_ = static_cast<size_t>(maxY - minY + 1);
    width_ = static_cast<size_t>(maxX - minX + 1);
}

/*--------------------------------------------------
                     PUBLIC
--------------------------------------------------*/

// #### Factory ####

TetrominoPtr ATetromino::makeTetromino(TetrominoShape shape,
                                       Vec2 &&anchorPoint) {
    if (shape == TetrominoShape::NumBasicTetrominoShape
        || shape == TetrominoShape::NumTetrominoShape) {
        throw std::runtime_error(
            "invalid tetromino shape: NumBasicTetrominoShape");
    }

    TetrominoPtr ret;

    switch (shape) {
    case (TetrominoShape::Z):
        ret = std::make_unique<TetrominoZ>(std::move(anchorPoint));
        break;
    case (TetrominoShape::L):
        ret = std::make_unique<TetrominoL>(std::move(anchorPoint));
        break;
    case (TetrominoShape::O):
        ret = std::make_unique<TetrominoO>(std::move(anchorPoint));
        break;
    case (TetrominoShape::S):
        ret = std::make_unique<TetrominoS>(std::move(anchorPoint));
        break;
    case (TetrominoShape::I):
        ret = std::make_unique<TetrominoI>(std::move(anchorPoint));
        break;
    case (TetrominoShape::J):
        ret = std::make_unique<TetrominoJ>(std::move(anchorPoint));
        break;
    case (TetrominoShape::T):
        ret = std::make_unique<TetrominoT>(std::move(anchorPoint));
        break;
    case (TetrominoShape::MiniTetromino):
        ret = std::make_unique<MiniTetromino>(std::move(anchorPoint));
        break;
    default:
        break;
    }

    return ret;
}

// #### Getters ####

size_t ATetromino::getWidth() const noexcept { return width_; }

size_t ATetromino::getHeight() const noexcept { return height_; }

TetrominoShape ATetromino::getShape() const noexcept { return shape_; }

const Vec2 &ATetromino::getAnchorPoint() const noexcept { return anchorPoint_; }

const std::vector<Vec2> &ATetromino::getBody() const noexcept { return body_; }

unsigned ATetromino::getColorId() const noexcept {
    return static_cast<unsigned>(getShape());
}

const CyclicIndex &ATetromino::getRotationIndex() const noexcept {
    return rotationIdx_;
}

const CyclicIndex &ATetromino::getPrevRotationIndex() const noexcept {
    return prevRotationIdx_;
}

uint8_t ATetromino::getNumOffsetTests() const noexcept {
    return static_cast<uint8_t>(offsetData_->at(0).size());
}

TetrominoPtr ATetromino::getNthOffsetTest(uint8_t offsetIndex) const {
    TetrominoPtr copy = this->clone();

    Vec2 offsetVal1 = offsetData_->at(static_cast<size_t>(prevRotationIdx_))
                          .at(offsetIndex - 1);
    Vec2 offsetVal2 =
        offsetData_->at(static_cast<size_t>(rotationIdx_)).at(offsetIndex - 1);

    // Compute offset with offset data
    Vec2 offset = (offsetVal1 - offsetVal2);

    // Apply offset
    copy->anchorPoint_ += offset;

    return copy;
}

// #### Setters ####

void ATetromino::setAnchorPoint(const Vec2 &anchorPoint) {
    anchorPoint_ = anchorPoint;
}

// #### Tetromino Actions ####

void ATetromino::move(TetrominoMove tetrominoMove, bool reverse) {
    switch (tetrominoMove) {
    case TetrominoMove::Down:
        anchorPoint_.moveY(reverse ? +1 : -1);
        break;
    case TetrominoMove::Left:
        anchorPoint_.moveX(reverse ? +1 : -1);
        break;
    case TetrominoMove::Right:
        anchorPoint_.moveX(reverse ? -1 : +1);
        break;
    }
}

void ATetromino::rotate(bool rotateClockwise) {
    prevRotationIdx_ = rotationIdx_;
    rotationIdx_ += (rotateClockwise) ? 1 : -1;

    Vec2 center{0, 0}; // Convention : rotation center is always (0,0)

    for (Vec2 &coord : body_) {
        coord.rotateAround(center, rotateClockwise);
    }

    std::swap(height_, width_);
}

// #### Comparisons Operators ####

bool ATetromino::operator==(const ATetromino &other) const {
    return (
        std::equal(getBody().begin(), getBody().end(), other.getBody().begin())
        and getAnchorPoint() == other.getAnchorPoint()
        and getRotationIndex() == other.getRotationIndex()
        and getPrevRotationIndex() == other.getPrevRotationIndex()
        and getWidth() == other.getWidth() and getHeight() == other.getHeight()
        and getShape() == other.getShape());
}

bool ATetromino::operator!=(const ATetromino &other) const {
    return !(operator==(other));
}

// #### Output Stream ####

std::ostream &operator<<(std::ostream &os, const ATetromino &tetromino) {
    os << "anchor: " << tetromino.getAnchorPoint() << " body: {";
    for (const auto &coord : tetromino.getBody()) {
        std::cout << coord << " ";
    }
    std::cout << "}";
    return os;
}
