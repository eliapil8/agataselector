#include <gtest/gtest.h>

#include <iostream>

#include "EnergyLoss.h"
#include "Units.h"

TEST(TESTEnergyLoss, ResultCheck) {
    EnergyLoss eloss("../../../../User/EXP/COM_002/Conf/EnergyLoss/"
                     "Target-116-50_Ion-59-29.txt",
                     "SRIM", 100);

    EXPECT_NEAR(eloss.Slow(100 * UNITS::MeV, 5 * UNITS::um, 0),
                35.706987510589954, 0);

    EXPECT_NEAR(
        eloss.EvaluateInitialEnergy(100 * UNITS::MeV, 5 * UNITS::um, 0),
        170.10695366190109, 0);

    EXPECT_NEAR(eloss.EvaluateMaterialThickness(100 * UNITS::MeV,
                                                35.706987510589954,
                                                5 * UNITS::um, 5 * UNITS::um),
                5 * UNITS::um, 0);
}
