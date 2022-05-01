#include "nuid.h"
#include <ctime>
#include <random>

Nuid NuidGenerator::m_nuid = NuidGenerator::InitNuid();
Nuid NuidGenerator::InitNuid()
{
	srand((unsigned)std::time(NULL));
	return rand();
}

Nuid GenNuid()
{
	return NuidGenerator::m_nuid++;
}
