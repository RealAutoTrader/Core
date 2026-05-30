#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>

// 최근 체결 가격을 고정 크기 배열로 관리하는 Ring Buffer
class PriceRingBuffer {
private:
    static constexpr std::size_t CAPACITY = 60;

    std::array<double, CAPACITY> data{};
    std::size_t head = 0;   // 가장 오래된 값의 위치
    std::size_t count = 0;  // 현재 저장된 개수

public:
    void push(double value) {
        if (count < CAPACITY) {
            std::size_t index = (head + count) % CAPACITY;
            data[index] = value;
            count++;
        }
        else {
            // 버퍼가 가득 찬 경우 가장 오래된 위치에 덮어쓰기
            data[head] = value;
            head = (head + 1) % CAPACITY;
        }
    }

    std::size_t size() const {
        return count;
    }

    bool empty() const {
        return count == 0;
    }

    static constexpr std::size_t capacity() {
        return CAPACITY;
    }

    double back() const {
        if (count == 0) {
            throw std::runtime_error("PriceRingBuffer is empty");
        }

        std::size_t index = (head + count - 1) % CAPACITY;
        return data[index];
    }

    // 최신 값 기준 offset 위치의 값을 가져온다.
    // offset = 0 → 가장 최신 값
    // offset = 1 → 바로 이전 값
    // offset = count - 1 → 가장 오래된 값
    double nthFromBack(std::size_t offset) const {
        if (offset >= count) {
            throw std::runtime_error("PriceRingBuffer offset out of range");
        }

        std::size_t index = (head + count - 1 - offset + CAPACITY) % CAPACITY;
        return data[index];
    }

    // 오래된 값부터 접근하고 싶을 때 사용
    double atChronological(std::size_t index) const {
        if (index >= count) {
            throw std::runtime_error("PriceRingBuffer index out of range");
        }

        std::size_t real_index = (head + index) % CAPACITY;
        return data[real_index];
    }
};