#pragma once

#include "base_types.h"

class NuidGenerator
{
private:
  static Nuid m_nuid;
  static Nuid InitNuid();
  friend Nuid GenNuid();
};

Nuid GenNuid();
