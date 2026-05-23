//
// Created by 束宇轩 on 2026/5/20.
//

#ifndef SYNERA_STARTER_JARVANIV_H
#define SYNERA_STARTER_JARVANIV_H

#include "entity/unit/unit.h"

class JarvanIV : public Unit
{
public:
    JarvanIV();
    void castSkill(Game* game, Unit* target) override;
};

#endif // SYNERA_STARTER_JARVANIV_H
