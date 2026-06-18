//
// Created by 束宇轩 on 2026/5/20.
//

#ifndef SYNERA_STARTER_JINX_H
#define SYNERA_STARTER_JINX_H

#include "entity/unit/unit.h"

class Jinx : public Unit
{
public:
    Jinx();
    void castSkill(Game* game, Unit* target) override;
};

#endif // SYNERA_STARTER_JINX_H
