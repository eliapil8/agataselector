#include "UserConf.h"

UserConf::UserConf() : Conf("USER_CONF") {
    parD.emplace("ATC00_phi", Conf::Property<double&>{
                                  ATC00_phi, "deg", "Phi rotation of ATC 00"});
    parD.emplace("ATC01_phi", Conf::Property<double&>{
                                  ATC01_phi, "deg", "Phi rotation of ATC 01"});
    parD.emplace("ATC02_phi", Conf::Property<double&>{
                                  ATC02_phi, "deg", "Phi rotation of ATC 02"});
    parD.emplace("ATC03_phi", Conf::Property<double&>{
                                  ATC02_phi, "deg", "Phi rotation of ATC 03"});
    parD.emplace("ATC04_phi", Conf::Property<double&>{
                                  ATC04_phi, "deg", "Phi rotation of ATC 04"});
    parD.emplace("ATC05_phi", Conf::Property<double&>{
                                  ATC05_phi, "deg", "Phi rotation of ATC 05"});
    parD.emplace("ATC06_phi", Conf::Property<double&>{
                                  ATC06_phi, "deg", "Phi rotation of ATC 06"});
    parD.emplace("ATC07_phi", Conf::Property<double&>{
                                  ATC07_phi, "deg", "Phi rotation of ATC 07"});
    parD.emplace("ATC08_phi", Conf::Property<double&>{
                                  ATC08_phi, "deg", "Phi rotation of ATC 08"});
    parD.emplace("ATC09_phi", Conf::Property<double&>{
                                  ATC09_phi, "deg", "Phi rotation of ATC 09"});
    parD.emplace("ATC10_phi", Conf::Property<double&>{
                                  ATC10_phi, "deg", "Phi rotation of ATC 10"});
    parD.emplace("ATC11_phi", Conf::Property<double&>{
                                  ATC11_phi, "deg", "Phi rotation of ATC 11"});
    parD.emplace("ATC12_phi", Conf::Property<double&>{
                                  ATC12_phi, "deg", "Phi rotation of ATC 12"});
    parD.emplace("ATC13_phi", Conf::Property<double&>{
                                  ATC13_phi, "deg", "Phi rotation of ATC 13"});
    parD.emplace("ATC14_phi", Conf::Property<double&>{
                                  ATC14_phi, "deg", "Phi rotation of ATC 14"});
    parD.emplace("ATC15_phi", Conf::Property<double&>{
                                  ATC15_phi, "deg", "Phi rotation of ATC 15"});
    parD.emplace("ATC16_phi", Conf::Property<double&>{
                                  ATC16_phi, "deg", "Phi rotation of ATC 16"});
    parD.emplace("ATC17_phi", Conf::Property<double&>{
                                  ATC17_phi, "deg", "Phi rotation of ATC 17"});
    parD.emplace("ATC18_phi", Conf::Property<double&>{
                                  ATC18_phi, "deg", "Phi rotation of ATC 18"});
    parD.emplace("ATC19_phi", Conf::Property<double&>{
                                  ATC19_phi, "deg", "Phi rotation of ATC 19"});
    parD.emplace("ATC20_phi", Conf::Property<double&>{
                                  ATC20_phi, "deg", "Phi rotation of ATC 20"});
    parD.emplace("ATC21_phi", Conf::Property<double&>{
                                  ATC21_phi, "deg", "Phi rotation of ATC 21"});
    parD.emplace("ATC22_phi", Conf::Property<double&>{
                                  ATC22_phi, "deg", "Phi rotation of ATC 22"});
    parD.emplace("ATC23_phi", Conf::Property<double&>{
                                  ATC23_phi, "deg", "Phi rotation of ATC 23"});
    parD.emplace("ATC24_phi", Conf::Property<double&>{
                                  ATC24_phi, "deg", "Phi rotation of ATC 24"});
    parD.emplace("ATC25_phi", Conf::Property<double&>{
                                  ATC25_phi, "deg", "Phi rotation of ATC 25"});
    parD.emplace("ATC26_phi", Conf::Property<double&>{
                                  ATC26_phi, "deg", "Phi rotation of ATC 26"});
    parD.emplace("ATC27_phi", Conf::Property<double&>{
                                  ATC27_phi, "deg", "Phi rotation of ATC 27"});
    parD.emplace("ATC28_phi", Conf::Property<double&>{
                                  ATC28_phi, "deg", "Phi rotation of ATC 28"});
    parD.emplace("ATC29_phi", Conf::Property<double&>{
                                  ATC29_phi, "deg", "Phi rotation of ATC 29"});
    parD.emplace("ATC30_phi", Conf::Property<double&>{
                                  ATC30_phi, "deg", "Phi rotation of ATC 30"});
    parD.emplace("ATC31_phi", Conf::Property<double&>{
                                  ATC31_phi, "deg", "Phi rotation of ATC 31"});
    parD.emplace("ATC32_phi", Conf::Property<double&>{
                                  ATC32_phi, "deg", "Phi rotation of ATC 32"});
    parD.emplace("ATC33_phi", Conf::Property<double&>{
                                  ATC33_phi, "deg", "Phi rotation of ATC 33"});
    parD.emplace("ATC34_phi", Conf::Property<double&>{
                                  ATC34_phi, "deg", "Phi rotation of ATC 34"});
    parD.emplace("ATC35_phi", Conf::Property<double&>{
                                  ATC35_phi, "deg", "Phi rotation of ATC 35"});
    parD.emplace("ATC36_phi", Conf::Property<double&>{
                                  ATC36_phi, "deg", "Phi rotation of ATC 36"});
    parD.emplace("ATC37_phi", Conf::Property<double&>{
                                  ATC37_phi, "deg", "Phi rotation of ATC 37"});
    parD.emplace("ATC38_phi", Conf::Property<double&>{
                                  ATC38_phi, "deg", "Phi rotation of ATC 38"});
    parD.emplace("ATC39_phi", Conf::Property<double&>{
                                  ATC39_phi, "deg", "Phi rotation of ATC 39"});
    parD.emplace("ATC40_phi", Conf::Property<double&>{
                                  ATC40_phi, "deg", "Phi rotation of ATC 40"});
    parD.emplace("ATC41_phi", Conf::Property<double&>{
                                  ATC41_phi, "deg", "Phi rotation of ATC 41"});
    parD.emplace("ATC42_phi", Conf::Property<double&>{
                                  ATC42_phi, "deg", "Phi rotation of ATC 42"});

    parD.emplace("ATC00_theta",
                 Conf::Property<double&>{ATC00_theta, "deg",
                                         "Theta rotation of ATC 00"});
    parD.emplace("ATC01_theta",
                 Conf::Property<double&>{ATC01_theta, "deg",
                                         "Theta rotation of ATC 01"});
    parD.emplace("ATC02_theta",
                 Conf::Property<double&>{ATC02_theta, "deg",
                                         "Theta rotation of ATC 02"});
    parD.emplace("ATC03_theta",
                 Conf::Property<double&>{ATC02_theta, "deg",
                                         "Theta rotation of ATC 03"});
    parD.emplace("ATC04_theta",
                 Conf::Property<double&>{ATC04_theta, "deg",
                                         "Theta rotation of ATC 04"});
    parD.emplace("ATC05_theta",
                 Conf::Property<double&>{ATC05_theta, "deg",
                                         "Theta rotation of ATC 05"});
    parD.emplace("ATC06_theta",
                 Conf::Property<double&>{ATC06_theta, "deg",
                                         "Theta rotation of ATC 06"});
    parD.emplace("ATC07_theta",
                 Conf::Property<double&>{ATC07_theta, "deg",
                                         "Theta rotation of ATC 07"});
    parD.emplace("ATC08_theta",
                 Conf::Property<double&>{ATC08_theta, "deg",
                                         "Theta rotation of ATC 08"});
    parD.emplace("ATC09_theta",
                 Conf::Property<double&>{ATC09_theta, "deg",
                                         "Theta rotation of ATC 09"});
    parD.emplace("ATC10_theta",
                 Conf::Property<double&>{ATC10_theta, "deg",
                                         "Theta rotation of ATC 10"});
    parD.emplace("ATC11_theta",
                 Conf::Property<double&>{ATC11_theta, "deg",
                                         "Theta rotation of ATC 11"});
    parD.emplace("ATC12_theta",
                 Conf::Property<double&>{ATC12_theta, "deg",
                                         "Theta rotation of ATC 12"});
    parD.emplace("ATC13_theta",
                 Conf::Property<double&>{ATC13_theta, "deg",
                                         "Theta rotation of ATC 13"});
    parD.emplace("ATC14_theta",
                 Conf::Property<double&>{ATC14_theta, "deg",
                                         "Theta rotation of ATC 14"});
    parD.emplace("ATC15_theta",
                 Conf::Property<double&>{ATC15_theta, "deg",
                                         "Theta rotation of ATC 15"});
    parD.emplace("ATC16_theta",
                 Conf::Property<double&>{ATC16_theta, "deg",
                                         "Theta rotation of ATC 16"});
    parD.emplace("ATC17_theta",
                 Conf::Property<double&>{ATC17_theta, "deg",
                                         "Theta rotation of ATC 17"});
    parD.emplace("ATC18_theta",
                 Conf::Property<double&>{ATC18_theta, "deg",
                                         "Theta rotation of ATC 18"});
    parD.emplace("ATC19_theta",
                 Conf::Property<double&>{ATC19_theta, "deg",
                                         "Theta rotation of ATC 19"});
    parD.emplace("ATC20_theta",
                 Conf::Property<double&>{ATC20_theta, "deg",
                                         "Theta rotation of ATC 20"});
    parD.emplace("ATC21_theta",
                 Conf::Property<double&>{ATC21_theta, "deg",
                                         "Theta rotation of ATC 21"});
    parD.emplace("ATC22_theta",
                 Conf::Property<double&>{ATC22_theta, "deg",
                                         "Theta rotation of ATC 22"});
    parD.emplace("ATC23_theta",
                 Conf::Property<double&>{ATC23_theta, "deg",
                                         "Theta rotation of ATC 23"});
    parD.emplace("ATC24_theta",
                 Conf::Property<double&>{ATC24_theta, "deg",
                                         "Theta rotation of ATC 24"});
    parD.emplace("ATC25_theta",
                 Conf::Property<double&>{ATC25_theta, "deg",
                                         "Theta rotation of ATC 25"});
    parD.emplace("ATC26_theta",
                 Conf::Property<double&>{ATC26_theta, "deg",
                                         "Theta rotation of ATC 26"});
    parD.emplace("ATC27_theta",
                 Conf::Property<double&>{ATC27_theta, "deg",
                                         "Theta rotation of ATC 27"});
    parD.emplace("ATC28_theta",
                 Conf::Property<double&>{ATC28_theta, "deg",
                                         "Theta rotation of ATC 28"});
    parD.emplace("ATC29_theta",
                 Conf::Property<double&>{ATC29_theta, "deg",
                                         "Theta rotation of ATC 29"});
    parD.emplace("ATC30_theta",
                 Conf::Property<double&>{ATC30_theta, "deg",
                                         "Theta rotation of ATC 30"});
    parD.emplace("ATC31_theta",
                 Conf::Property<double&>{ATC31_theta, "deg",
                                         "Theta rotation of ATC 31"});
    parD.emplace("ATC32_theta",
                 Conf::Property<double&>{ATC32_theta, "deg",
                                         "Theta rotation of ATC 32"});
    parD.emplace("ATC33_theta",
                 Conf::Property<double&>{ATC33_theta, "deg",
                                         "Theta rotation of ATC 33"});
    parD.emplace("ATC34_theta",
                 Conf::Property<double&>{ATC34_theta, "deg",
                                         "Theta rotation of ATC 34"});
    parD.emplace("ATC35_theta",
                 Conf::Property<double&>{ATC35_theta, "deg",
                                         "Theta rotation of ATC 35"});
    parD.emplace("ATC36_theta",
                 Conf::Property<double&>{ATC36_theta, "deg",
                                         "Theta rotation of ATC 36"});
    parD.emplace("ATC37_theta",
                 Conf::Property<double&>{ATC37_theta, "deg",
                                         "Theta rotation of ATC 37"});
    parD.emplace("ATC38_theta",
                 Conf::Property<double&>{ATC38_theta, "deg",
                                         "Theta rotation of ATC 38"});
    parD.emplace("ATC39_theta",
                 Conf::Property<double&>{ATC39_theta, "deg",
                                         "Theta rotation of ATC 39"});
    parD.emplace("ATC40_theta",
                 Conf::Property<double&>{ATC40_theta, "deg",
                                         "Theta rotation of ATC 40"});
    parD.emplace("ATC41_theta",
                 Conf::Property<double&>{ATC41_theta, "deg",
                                         "Theta rotation of ATC 41"});
    parD.emplace("ATC42_theta",
                 Conf::Property<double&>{ATC42_theta, "deg",
                                         "Theta rotation of ATC 42"});
}
