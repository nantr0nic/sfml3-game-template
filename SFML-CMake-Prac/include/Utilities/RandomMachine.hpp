#pragma once

#include <random>
#include <expected>
#include <string_view>

struct RandomError
{
    std::string_view message;
};

class RandomMachine
{
public:
    RandomMachine();
    RandomMachine(const RandomMachine&) = delete;
    RandomMachine& operator=(const RandomMachine&) = delete;
    ~RandomMachine() = default;

    std::expected<int, RandomError> getInt(int min, int max);
    std::expected<float, RandomError> getFloat(float min, float max);

    int d2();
    int d20();
    int d100();

private:
    std::random_device m_RandomDevice;
    std::mt19937 m_RandomEngine;
};