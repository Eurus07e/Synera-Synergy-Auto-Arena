//
// Created by 束宇轩 on 2026/5/20.
//

#ifndef SYNERA_STARTER_LORIS_H
#define SYNERA_STARTER_LORIS_H

#include "entity/unit/unit.h"

class Loris : public Unit
{
public:
    Loris();
    void castSkill() override;
};

#endif // SYNERA_STARTER_LORIS_H
