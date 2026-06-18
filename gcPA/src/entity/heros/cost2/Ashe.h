//
// Created by 束宇轩 on 2026/5/20.
//

#ifndef SYNERA_STARTER_ASHE_H
#define SYNERA_STARTER_ASHE_H

#include "entity/unit/unit.h"

class Ashe : public Unit
{
public:
    Ashe();
    void castSkill(Game* game, Unit* target) override;
};

#endif // SYNERA_STARTER_ASHE_H
