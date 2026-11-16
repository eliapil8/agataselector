#include "PrismaDanteConf.h"

PrismaDanteConf::PrismaDanteConf() : CoincidenceConf("PRISMADANTE_CONF") {
    parD.emplace("MAX_THETA_DISCR",
                 Conf::Property<double&>{
                     max_theta_discr, "deg",
                     "Maximum discrepancy in theta between the prisma "
                     "reconstriction and the dante measurement"});
    parD.emplace("MAX_PHI_DISCR",
                 Conf::Property<double&>{
                     max_phi_discr, "deg",
                     "Maximum discrepancy in phi between the prisma "
                     "reconstriction and the dante measurement"});
}