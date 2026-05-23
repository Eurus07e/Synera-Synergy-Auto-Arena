//
// Created by 束宇轩 on 2026/5/20.
//

#ifndef SYNERA_STARTER_SONA_H
#define SYNERA_STARTER_SONA_H

#include "entity/unit/unit.h"

class Sona : public Unit
{
public:
    Sona();
    void castSkill() override;
};

#endif // SYNERA_STARTER_SONA_H
