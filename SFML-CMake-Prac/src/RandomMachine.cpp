#include "RandomMachine.hpp"

RandomMachine::RandomMachine()
    : m_RandomDevice()
    , m_RandomEngine(m_RandomDevice())
{
}

int RandomMachine::getInt(int min, int max)
{
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(m_RandomEngine);
}

float RandomMachine::getFloat(float min, float max)
{
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(m_RandomEngine);
}

int RandomMachine::d2()
{
    return getInt(1, 2);
}

int RandomMachine::d20()
{
    return getInt(1, 20);
}

int RandomMachine::d100()
{
    return getInt(1, 100);
}