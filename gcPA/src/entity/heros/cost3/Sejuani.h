//
// Created by 束宇轩 on 2026/5/20.
//

#ifndef SYNERA_STARTER_SEJUANI_H
#define SYNERA_STARTER_SEJUANI_H

#include "entity/unit/unit.h"

class Sejuani : public Unit
{
public:
    Sejuani();
    void castSkill(Game* game, Unit* target) override;
};

#endif // SYNERA_STARTER_SEJUANI_H
