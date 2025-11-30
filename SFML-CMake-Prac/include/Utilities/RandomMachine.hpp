#pragma once

#include <random>
#include <source_location>

namespace utils
{
    class RandomMachine
    {
    public:
        RandomMachine();
        RandomMachine(const RandomMachine&) = delete;
        RandomMachine& operator=(const RandomMachine&) = delete;
        ~RandomMachine() = default;

        int getInt(int min, int max, int fallback = 0,
            const std::source_location& loc = std::source_location::current());
        float getFloat(float min, float max, float fallback = 0.0f,
            const std::source_location& loc = std::source_location::current());

        int d2();
        int d4();
        int d6();
        int d8();
        int d10();
        int d12();
        int d20();
        int d100();

        float zeroToOne();
        float negOneToOne();

    private:
        std::random_device m_RandomDevice;
        std::mt19937 m_RandomEngine;
    };
}