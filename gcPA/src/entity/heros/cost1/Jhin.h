//
// Created by 束宇轩 on 2026/5/18.
//

#ifndef SYNERA_STARTER_JHIN_H
#define SYNERA_STARTER_JHIN_H

#include "entity/unit/unit.h"

class Jhin : public Unit
{
public:
    Jhin();
    void castSkill(Game* game, Unit* target) override;
};

#endif // SYNERA_STARTER_JHIN_H
