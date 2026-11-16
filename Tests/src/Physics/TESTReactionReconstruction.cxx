#include <gtest/gtest.h>

#include <iostream>

#include "ReactionReconstruction.h"
#include "Units.h"

TEST(TESTReactionReconstruction, ResultCheck) {
    {
        ReactionReconstruction2body<long double> reaction(
            {ReactionFragment::FragmentSettings(48, 20, 0, 100 * UNITS::MeV,
                                                0 * UNITS::MeV),
             ReactionFragment::FragmentSettings(48, 20, 0, 0 * UNITS::MeV,
                                                0 * UNITS::MeV),
             ReactionFragment::FragmentSettings(49, 20, 0, 0 * UNITS::MeV,
                                                0 * UNITS::MeV),
             ReactionFragment::FragmentSettings(47, 20, 0, 0 * UNITS::MeV,
                                                0 * UNITS::MeV)},
            "../../../../User/EXP/Template/Conf/NuclearData/nutab12");
        reaction.ChooseFixed(3);

        reaction.Set_Ek(80 * UNITS::MeV);
        EXPECT_NEAR(reaction.GetFreeFragment().Get_P().Theta() / UNITS::deg,
                    64.488345783516252, 1e-10);
        EXPECT_NEAR(reaction.GetFreeFragment().Get_Ek() / UNITS::MeV,
                    15.19387000001333, 1e-10);
        EXPECT_NEAR(reaction.GetFixedFragment().Get_P().Theta() / UNITS::deg,
                    22.646885106214722, 1e-10);
        EXPECT_NEAR(reaction.GetFixedFragment().Get_Ek() / UNITS::MeV,
                    79.999999999996618, 1e-10);
    }
    {
        ReactionReconstruction2body<long double> reaction(
            {ReactionFragment::FragmentSettings(34, 14, 0, 1300 * UNITS::MeV,
                                                0 * UNITS::MeV),
             ReactionFragment::FragmentSettings(1, 1, 0, 0 * UNITS::MeV,
                                                0 * UNITS::MeV),
             ReactionFragment::FragmentSettings(2, 1, 0, 0 * UNITS::MeV,
                                                0 * UNITS::MeV),
             ReactionFragment::FragmentSettings(33, 14, 0, 0 * UNITS::MeV,
                                                0 * UNITS::MeV)},
            "../../../../User/EXP/Template/Conf/NuclearData/nutab12");
        reaction.ChooseFixed(3);

        reaction.Set_Ek(10 * UNITS::MeV);
        EXPECT_NEAR(reaction.GetFreeFragment().Get_P().Theta() / UNITS::deg,
                    0.31183242612128081, 1e-10);
        EXPECT_NEAR(reaction.GetFreeFragment().Get_Ek() / UNITS::MeV,
                    1284.710549000001, 1e-10);
        EXPECT_NEAR(reaction.GetFixedFragment().Get_P().Theta() / UNITS::deg,
                    14.588863438026516, 1e-10);
        EXPECT_NEAR(reaction.GetFixedFragment().Get_Ek() / UNITS::MeV,
                    9.9999999999999449, 1e-10);


        reaction.Set_Ek(180 * UNITS::MeV);
        EXPECT_NEAR(reaction.GetFreeFragment().Get_P().Theta() / UNITS::deg,
                    1.3947855216492366, 1e-10);
        EXPECT_NEAR(reaction.GetFreeFragment().Get_Ek() / UNITS::MeV,
                    1114.710549000001, 1e-10);
        EXPECT_NEAR(reaction.GetFixedFragment().Get_P().Theta() / UNITS::deg,
                    13.980918053308208, 1e-10);
        EXPECT_NEAR(reaction.GetFixedFragment().Get_Ek() / UNITS::MeV,
                    179.99999999999994, 1e-10);


        reaction.GetExFreeFragment().Set_Ex(15 * UNITS::MeV);
        reaction.Set_Ek(140 * UNITS::MeV);
        EXPECT_NEAR(reaction.GetFreeFragment().Get_P().Theta() / UNITS::deg,
                    1.1411268205051504, 1e-10);
        EXPECT_NEAR(reaction.GetFreeFragment().Get_Ek() / UNITS::MeV,
                    1139.710549000001, 1e-10);
        EXPECT_NEAR(reaction.GetFixedFragment().Get_P().Theta() / UNITS::deg,
                    13.173193292658549, 1e-10);
        EXPECT_NEAR(reaction.GetFixedFragment().Get_Ek() / UNITS::MeV,
                    139.99999999999994, 1e-10);
        //reaction.Set_Ek_Theta()

        auto ex = reaction.Set_Ek_Theta(140 * UNITS::MeV, 13.173193292658549 * UNITS::deg);
        EXPECT_NEAR(reaction.GetFreeFragment().Get_P().Theta() / UNITS::deg,
                    1.1411268205051504, 1e-10);
        EXPECT_NEAR(reaction.GetFreeFragment().Get_Ek() / UNITS::MeV,
                    1139.710549000001, 1e-10);
        EXPECT_NEAR(reaction.GetFixedFragment().Get_P().Theta() / UNITS::deg,
                    13.173193292658549, 1e-10);
        EXPECT_NEAR(reaction.GetFixedFragment().Get_Ek() / UNITS::MeV,
                    139.99999999999994, 1e-10);
        EXPECT_NEAR(ex / UNITS::MeV,
                    15 * UNITS::MeV, 1e-10);

        ex = reaction.Set_Ek_Theta(14 * UNITS::MeV, 17 * UNITS::deg);
        EXPECT_NEAR(reaction.GetFreeFragment().Get_P().Theta() / UNITS::deg,
                    0.43002807843191881, 1e-10);
        EXPECT_NEAR(reaction.GetFreeFragment().Get_Ek() / UNITS::MeV,
                    1275.6134782437859, 1e-10);
        EXPECT_NEAR(reaction.GetFixedFragment().Get_P().Theta() / UNITS::deg,
                    17.000000000000167, 1e-10);
        EXPECT_NEAR(reaction.GetFixedFragment().Get_Ek() / UNITS::MeV,
                    13.999999999999945, 1e-10);
        EXPECT_NEAR(ex / UNITS::MeV,
                    5.0970707562115258, 1e-10);
    }
    //{
    //    ReactionReconstruction2body<long double> reaction(
    //        {ReactionFragment::FragmentSettings(64, 30, 0, 220 * UNITS::MeV,
    //                                            0 * UNITS::MeV),
    //         ReactionFragment::FragmentSettings(208, 82, 0, 0 * UNITS::MeV,
    //                                            0 * UNITS::MeV),
    //         ReactionFragment::FragmentSettings(64, 30, 0, 0 * UNITS::MeV,
    //                                            0 * UNITS::MeV),
    //         ReactionFragment::FragmentSettings(208, 82, 0, 0 * UNITS::MeV,
    //                                            0 * UNITS::MeV)},
    //        "../../../../User/EXP/Template/Conf/NuclearData/nutab12");
    //    reaction.ChooseFixed(3);
    //    reaction.ChooseExFixed(3);

    //    auto ex = reaction.Set_Theta_Theta(40 * UNITS::deg, 62.2 * UNITS::deg, true);

    //    EXPECT_NEAR(ex / UNITS::MeV,
    //                10.45029726, 1e-6);
    //}
}
