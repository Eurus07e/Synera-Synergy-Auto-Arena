//
// Created by 束宇轩 on 2026/5/20.
//

#ifndef SYNERA_STARTER_RUMBLE_H
#define SYNERA_STARTER_RUMBLE_H

#include "entity/unit/unit.h"

class Rumble : public Unit
{
public:
    Rumble();
    void castSkill() override;
};

#endif // SYNERA_STARTER_RUMBLE_H
