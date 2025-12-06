#include "Utilities/RandomMachine.hpp"
#include "Utilities/Logger.hpp"

#include <cmath>
#include <limits>
#include <format>

namespace utils
{
    RandomMachine::RandomMachine()
        : m_RandomDevice()
        , m_RandomEngine(m_RandomDevice())
    {
    }

    int RandomMachine::getInt(int min, int max, int fallback, const std::source_location& loc)
    {
        if (min > max)
        {
            logger::Error(std::format(
                "RandomMachine: Min ({}) is greater than max ({}). Falling back to {}.", min, max, fallback), loc
            );
            return fallback;
        }
        
        std::uniform_int_distribution<int> distribution(min, max);
        return distribution(m_RandomEngine);
    }

    float RandomMachine::getFloat(float min, float max, float fallback, const std::source_location& loc)
    {
        if (min > max)
        {
            logger::Error(std::format(
                "RandomMachine: Min ({}) is greater than max ({}). Falling back to {}.", min, max, fallback), loc
            );
            return fallback;
        }

        // This will expand the range of returns to include the provided max
        // Meaning, if passed with (0,1) it will include "1" rather than just 0.9999
        float inclusiveMax = std::nextafter(max, std::numeric_limits<float>::max());
        
        std::uniform_real_distribution<float> distribution(min, inclusiveMax);
        return distribution(m_RandomEngine);
    }

    int RandomMachine::d2()
    {
        return getInt(1, 2);
    }

    int RandomMachine::d4()
    {
        return getInt(1, 4);
    }

    int RandomMachine::d6()
    {
        return getInt(1, 6);
    }

    int RandomMachine::d8()
    {
        return getInt(1, 8);
    }

    int RandomMachine::d10()
    {
        return getInt(1, 10);
    }

    int RandomMachine::d12()
    {
        return getInt(1, 12);
    }

    int RandomMachine::d20()
    {
        return getInt(1, 20);
    }

    int RandomMachine::d100()
    {
        return getInt(1, 100);
    }

    float RandomMachine::zeroToOne()
    {
        return getFloat(0.0f, 1.0f);
    }

    float RandomMachine::negOneToOne()
    {
        return getFloat(-1.0f, 1.0f);
    }
}