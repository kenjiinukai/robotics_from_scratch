// Copyright 2020 RT Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <math.h>
#include "../common/arm_parameter.h"
#include "myCX7_KDL_library.h"

static LINK_PARAM link_parameter_3dof[LINK_NUM_3DOF] = {0};
static JOINT_RANGE joint_range[JOINT_NUM] = {0};

void initParam()
{
    getLinkParam3Dof(link_parameter_3dof);
    getJointRange(joint_range);
}

int forwardKinematics2Dof(VECTOR_3D *p, double *theta)
{
    double L1 = link_parameter_3dof[1].length;
    double L2 = link_parameter_3dof[2].length;

    double C1 = cos(theta[1]);
    double C12 = cos(theta[1] + theta[3]);
    double S1 = sin(theta[1]);
    double S12 = sin(theta[1] + theta[3]);

    p->x = L1 * C1 + L2 * C12;
    p->y = L1 * S1 + L2 * S12;

    return 0;
}

int inverseKinematics2Dof(VECTOR_3D p, double *theta)
{
    double L1 = link_parameter_3dof[1].length;
    double L2 = link_parameter_3dof[2].length;

    double S2, C2;

    //手先位置の値が可動範囲の外であればエラー値を返す
    if (((pow(L1 - L2, 2)) > (p.x * p.x + p.y * p.y)) || ((p.x * p.x + p.y * p.y) > (pow(L1 + L2, 2))))
    {
        printf("Target position is out of range.\n");
        return 1;
    }

    //2軸目theta[1]と4軸目theta[3]以外は0 radで固定
    for (int i = 0; i < JOINT_NUM; i++)
    {
        theta[i] = 0;
    }

    //θ2の角度
    C2 = (p.x * p.x + p.y * p.y - L1 * L1 - L2 * L2) / (2 * L1 * L2);
    theta[3] = -acos(C2); //acosの戻り値[0:π]、CRANE-X7においてθ2の可動はマイナス方向のみ
    //θ1の角度
    S2 = sin(theta[3]);
    theta[1] = atan2((-L2 * S2 * p.x + (L1 + L2 * C2) * p.y), ((L1 + L2 * C2) * p.x + L2 * S2 * p.y));
 
    //得られた関節角度が可動範囲外であればエラーを返す
    for (int i = 0; i < JOINT_NUM; i++)
    {
        if (((joint_range[i].min) > (theta[i])) || ((theta[i]) > (joint_range[i].max)))
        {
            printf("Theta[ %d]　is out of range.\n", i);
            return 1;
        }
    }
    return 0;
}