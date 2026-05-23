//
// Created by 束宇轩 on 2026/5/20.
//

#ifndef SYNERA_STARTER_XINZHAO_H
#define SYNERA_STARTER_XINZHAO_H

#include "entity/unit/unit.h"

class XinZhao : public Unit
{
public:
    XinZhao();
    void castSkill() override;
};

#endif // SYNERA_STARTER_XINZHAO_H
