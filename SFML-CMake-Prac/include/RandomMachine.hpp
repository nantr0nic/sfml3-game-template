#pragma once

#include <random>

class RandomMachine
{
public:
    RandomMachine();
    RandomMachine(const RandomMachine&) = delete;
    RandomMachine& operator=(const RandomMachine&) = delete;
    ~RandomMachine() = default;

    int getInt(int min, int max);
    float getFloat(float min, float max);

    int d2();
    int d20();
    int d100();

private:
    std::random_device m_RandomDevice;
    std::mt19937 m_RandomEngine;
};