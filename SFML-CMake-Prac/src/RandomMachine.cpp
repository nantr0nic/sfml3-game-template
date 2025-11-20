#include "RandomMachine.hpp"

#include <cmath>
#include <limits>

RandomMachine::RandomMachine()
    : m_RandomDevice()
    , m_RandomEngine(m_RandomDevice())
{
}

std::expected<int, RandomError> RandomMachine::getInt(int min, int max)
{
    if (min > max)
    {
        return std::unexpected(RandomError{"<RandomMachine> Error: min > max in random!"});
    }
    
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(m_RandomEngine);
}

std::expected<float, RandomError> RandomMachine::getFloat(float min, float max)
{
    if (min > max)
    {
        return std::unexpected(RandomError{"<RandomMachine> Error: min > max in random!"});
    }

    // This will expand the range of returns to include the provided max
    // Meaning, if passed with (0,1) it will include "1" rather than just 0.9999
    float inclusiveMax = std::nextafter(max, std::numeric_limits<float>::max());
    
    std::uniform_real_distribution<float> distribution(min, inclusiveMax);
    return distribution(m_RandomEngine);
}

int RandomMachine::d2()
{
    return getInt(1, 2).value();
}

int RandomMachine::d20()
{
    return getInt(1, 20).value();
}

int RandomMachine::d100()
{
    return getInt(1, 100).value();
}