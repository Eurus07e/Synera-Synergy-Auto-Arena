//
// Created by 束宇轩 on 2026/5/18.
//

#ifndef SYNERA_STARTER_AHRI_H
#define SYNERA_STARTER_AHRI_H
#include "entity/unit/unit.h"


class Ahri : public Unit
{
public:
    Ahri();
    void castSkill() override;
};


#endif //SYNERA_STARTER_AHRI_H
