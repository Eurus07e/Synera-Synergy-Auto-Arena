//
// Created by 束宇轩 on 2026/5/20.
//

#ifndef SYNERA_STARTER_CHOGATH_H
#define SYNERA_STARTER_CHOGATH_H

#include "entity/unit/unit.h"

class ChoGath : public Unit
{
public:
    ChoGath();
    void castSkill(Game* game, Unit* target) override;
};

#endif // SYNERA_STARTER_CHOGATH_H
