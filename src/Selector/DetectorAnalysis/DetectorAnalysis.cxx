#include "DetectorAnalysis.h"

DetectorAnalysis::DetectorAnalysis(DetectorAnalysis::Inputs &in,
                                   Container                &d,
                                   DetectorConf             &c,
                                   DetectorHistograms       &dh)
    : input(in), data(d), detectorConf(c), detectorHistograms(dh),
      settings(Settings::getInstance()), conf(ConfManager::getInstance())
{
    SetupTimeWindow<DetectorConf>(detectorConf);
}

void DetectorAnalysis::Instantiate() {}

bool DetectorAnalysis::AllocateHistos()
{
    if (!detectorConf.enable_histos)
        return false;

    DetectorHistograms &h = detectorHistograms;

    Initialize<TH1D>(h.h_CR,
                     new TH1D("h_CR", "Event count rate of detector;Time [s];Counts/10s", 12 * 360,
                              0, 12 * 3600),
                     h.ptrs);
    Initialize<TH1D>(
        h.h_TdiffLastEvent,
        new TH1D("h_TdiffLastEvent",
                 "Distribution of time difference betweem following events;TS [10 ns];Counts/10 ns",
                 50000, 0, 50000),
        h.ptrs);
    return true;
}

void DetectorAnalysis::Clear()
{
    data.clear();
}

void DetectorAnalysis::Analyze() {}

bool DetectorAnalysis::FillHistos()
{
    if (!detectorConf.enable_histos)
        return false;

    DetectorHistograms &h = detectorHistograms;

    if (h.h_CR != nullptr)
    {
        for (unsigned int i{0}; i < data.Time.size(); ++i)
        {
            auto diff = unsignedDiff<double, unsigned long long>(data.TS, input.firstTS) *
                        detectorConf.time_unit;
            Fill(h.h_CR,
                 unsignedDiff<double, unsigned long long>(data.TS, input.firstTS) *
                     detectorConf.time_unit,
                 1. / h.h_CR->GetBinWidth(1));
        }
    }
    if (h.h_TdiffLastEvent != nullptr)
    {
        if (lastTS != 0)
        {
            Fill(h.h_TdiffLastEvent, unsignedDiff<double, unsigned long long>(data.TS, lastTS));
        }
    }
    lastTS = data.TS;
    return true;
}

void DetectorAnalysis::Finalize() {}

TLorentzVector DetectorAnalysis::SlowInTarget(TLorentzVector                p,
                                              const CoincidenceConf::DcPos &pos,
                                              const ReactionFragment::Ion  &ion) const
{
    double normAng = p.Angle(conf.target.targetNorm);

    auto degraderELossIt = input.degraderELoss.find(ion);
    auto targetELossIt   = input.targetELoss.find(ion);

    double Ek = p.E() - p.M();
    if (settings.verbosity_level > 10)
    {
        std::cout << "Target normal angle: " << conf.target.targetNorm.Theta() / UNITS::deg
                  << std::endl;
        std::cout << "Target phi: " << conf.target.targetNorm.Phi() / UNITS::deg << std::endl;
        std::cout << "Ion angle: " << p.Theta() / UNITS::deg << std::endl;
        std::cout << "Ion phi: " << p.Phi() / UNITS::deg << std::endl;
        std::cout << "SlowInTarget ini energy: " << Ek << "; Angle: " << normAng / UNITS::deg
                  << "; Z = " << ion.Z << std::endl;
    }

    switch (pos)
    {
    case CoincidenceConf::DcPos::BEGIN_TARGET:
        if (settings.verbosity_level > 10)
        {
            std::cout << "ELoss: begin target" << std::endl;
        }
        if (targetELossIt != input.targetELoss.end())
        {
            if (normAng > UNITS::CONSTANTS::pi / 2)
            {
                Ek = targetELossIt->second.SlowWithDensity(
                    Ek, conf.target.thickness * (conf.reaction.reaction_position), normAng);
            }
            else
            {
                Ek = targetELossIt->second.EvaluateInitialEnergyWithDensity(
                    Ek, conf.target.thickness * (1 - conf.reaction.reaction_position), normAng);
            }
        }
        break;
    case CoincidenceConf::DcPos::MID_TARGET:
        if (settings.verbosity_level > 10)
        {
            std::cout << "ELoss: mid target" << std::endl;
        }
        break;
    case CoincidenceConf::DcPos::END_TARGET:
        if (settings.verbosity_level > 10)
        {
            std::cout << "ELoss: end target" << std::endl;
        }
        if (targetELossIt != input.targetELoss.end())
        {
            if (normAng < UNITS::CONSTANTS::pi / 2)
            {
                Ek = targetELossIt->second.SlowWithDensity(
                    Ek, conf.target.thickness * (1 - conf.reaction.reaction_position), normAng);
            }
            else
            {
                Ek = targetELossIt->second.EvaluateInitialEnergyWithDensity(
                    Ek, conf.target.thickness * (conf.reaction.reaction_position), normAng);
            }
        }
        break;
    case CoincidenceConf::DcPos::MID_DEGRADER:
        if (settings.verbosity_level > 10)
        {
            std::cout << "ELoss: mid degrader" << std::endl;
        }
        if (targetELossIt != input.targetELoss.end())
        {
            if (normAng < UNITS::CONSTANTS::pi / 2)
            {
                Ek = targetELossIt->second.SlowWithDensity(
                    Ek, conf.target.thickness * (1 - conf.reaction.reaction_position), normAng);
            }
            else
            {
                Ek = targetELossIt->second.EvaluateInitialEnergyWithDensity(
                    Ek, conf.target.thickness * (conf.reaction.reaction_position), normAng);
            }
        }
        if ((conf.target.degPos == TargetConf::DegPos::AFTER &&
             normAng < UNITS::CONSTANTS::pi / 2) ||
            (conf.target.degPos == TargetConf::DegPos::BEFORE &&
             normAng > UNITS::CONSTANTS::pi / 2))
        {
            if (conf.target.degPresent && degraderELossIt != input.degraderELoss.end())
            {
                Ek = degraderELossIt->second.SlowWithDensity(Ek, conf.target.degThickness / 2.,
                                                             normAng);
            }
        }
        break;
    case CoincidenceConf::DcPos::END_DEGRADER:
        if (settings.verbosity_level > 10)
        {
            std::cout << "ELoss: end degrader" << std::endl;
        }
        if (targetELossIt != input.targetELoss.end())
        {
            if (normAng < UNITS::CONSTANTS::pi / 2)
            {
                Ek = targetELossIt->second.SlowWithDensity(
                    Ek, conf.target.thickness * (1 - conf.reaction.reaction_position), normAng);
            }
            else
            {
                Ek = targetELossIt->second.EvaluateInitialEnergyWithDensity(
                    Ek, conf.target.thickness * (conf.reaction.reaction_position), normAng);
            }
        }
        if ((conf.target.degPos == TargetConf::DegPos::AFTER &&
             normAng < UNITS::CONSTANTS::pi / 2) ||
            (conf.target.degPos == TargetConf::DegPos::BEFORE &&
             normAng > UNITS::CONSTANTS::pi / 2))
        {
            if (conf.target.degPresent && degraderELossIt != input.degraderELoss.end())
            {
                Ek = degraderELossIt->second.SlowWithDensity(Ek, conf.target.degThickness, normAng);
            }
        }
        break;
    default:
        throw std::runtime_error("DC pos makes no sense\n");
    }

    if (settings.verbosity_level > 10)
    {
        std::cout << "SlowInTarget final energy: " << Ek << std::endl;
    }
    double oldP = p.Vect().Mag();
    double newP = sqrt(Ek * Ek + 2 * Ek * p.M());
    p.SetPxPyPzE(p.X() * newP / oldP, p.Y() * newP / oldP, p.Z() * newP / oldP, p.M() + Ek);

    return p;
}

void DetectorAnalysis::EvalInTarget(TLorentzVector &p, const ReactionFragment::Ion &ion) const
{
    double normAng = p.Angle(conf.target.targetNorm);
    double Ek      = p.E() - p.M();

    auto degraderELossIt = input.degraderELoss.find(ion);
    auto targetELossIt   = input.targetELoss.find(ion);

    if (settings.verbosity_level > 10)
    {
        std::cout << "Target normal angle: " << conf.target.targetNorm.Theta() / UNITS::deg
                  << std::endl;
        std::cout << "Target phi: " << conf.target.targetNorm.Phi() / UNITS::deg << std::endl;
        std::cout << "Target vect print: " << std::endl;
        conf.target.targetNorm.Print();
        std::cout << "Ion angle: " << p.Theta() / UNITS::deg << std::endl;
        std::cout << "Ion phi: " << p.Phi() / UNITS::deg << std::endl;
        std::cout << "Ion vector: " << std::endl;
        p.Vect().Print();
        std::cout << "EvalInTarget ini energy: " << Ek << "; Angle: " << normAng / UNITS::deg
                  << "; Z = " << ion.Z << std::endl;
    }
    // Degrader energy loss if the ion passes through it.
    // The norm of the target should point in the direction of the beam
    // (positive Z)
    if (normAng < UNITS::CONSTANTS::pi / 2)
    {
        if (conf.target.degPresent && degraderELossIt != input.degraderELoss.end() &&
            conf.target.degPos == TargetConf::DegPos::AFTER)
        {
            Ek = degraderELossIt->second.EvaluateInitialEnergyWithDensity(
                Ek, conf.target.degThickness, normAng);
        }
        if (targetELossIt != input.targetELoss.end())
        {
            Ek = targetELossIt->second.EvaluateInitialEnergyWithDensity(
                Ek, conf.target.thickness * (1 - conf.reaction.reaction_position), normAng);
        }
    }
    else
    {
        if (conf.target.degPresent && degraderELossIt != input.degraderELoss.end() &&
            conf.target.degPos == TargetConf::DegPos::BEFORE)
        {
            Ek = degraderELossIt->second.EvaluateInitialEnergyWithDensity(
                Ek, conf.target.degThickness, normAng);
        }
        if (targetELossIt != input.targetELoss.end())
        {
            Ek = targetELossIt->second.EvaluateInitialEnergyWithDensity(
                Ek, conf.target.thickness * (conf.reaction.reaction_position), normAng);
        }
    }

    if (settings.verbosity_level > 10)
    {
        std::cout << "EvalInTarget final energy: " << Ek << std::endl;
    }

    double oldP = p.Vect().Mag();
    double newP = sqrt(Ek * Ek + 2 * Ek * p.M());
    p.SetPxPyPzE(p.X() * newP / oldP, p.Y() * newP / oldP, p.Z() * newP / oldP, p.M() + Ek);
}

bool DetectorAnalysis::FileIsPresent(const std::string &fileName)
{
    std::ifstream fileCheck(fileName);
    return fileCheck.good();
}
