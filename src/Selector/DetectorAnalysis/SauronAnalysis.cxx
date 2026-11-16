#include "SauronAnalysis.h"
#include "ReactionFragment.h"
#include "Sauron.h"

SauronAnalysis::SauronAnalysis(DetectorAnalysis::Inputs in,
                               Sauron                  &s,
                               SauronConf              &c,
                               const std::string       &gatesFolder)
    : DetectorAnalysis(in, s, c, sauronHistograms), sauron(s), sauronConf(c),
      particleGates(conf.replay.conf_path + gatesFolder)
{
}

SauronFwAnalysis::SauronFwAnalysis(DetectorAnalysis::Inputs in, Sauron &s)
    : SauronAnalysis(in, s, ConfManager::getInstance().sauronFw, "/CUT/SAURONFW/EImax")
{
    if (sauronConf.enable_treeFill)
        input.tree->Branch("SauronFw", &sauron);
}

SauronBwAnalysis::SauronBwAnalysis(DetectorAnalysis::Inputs in, Sauron &s)
    : SauronAnalysis(in, s, ConfManager::getInstance().sauronBw, "/CUT/SAURONBW/EImax")
{
    if (sauronConf.enable_treeFill)
        input.tree->Branch("SauronBw", &sauron);
}

bool SauronFwAnalysis::AllocateHistos()
{
    SauronHistograms &h = sauronHistograms;
    h.dir               = input.outFile.mkdir("SauronFw", "Sauron Fw angles histograms");
    h.dir->cd();
    if (!SauronAnalysis::AllocateHistos())
        return false;
    return true;
}

bool SauronBwAnalysis::AllocateHistos()
{
    SauronHistograms &h = sauronHistograms;
    h.dir               = input.outFile.mkdir("SauronBw", "Sauron Bw angles histograms");
    h.dir->cd();
    if (!SauronAnalysis::AllocateHistos())
        return false;
    return true;
}

bool SauronAnalysis::AllocateHistos()
{
    SauronHistograms &h = sauronHistograms;
    if (!DetectorAnalysis::AllocateHistos())
        return false;

    // Setup deadlayer energy loss
    std::string sTemp = conf.replay.conf_path + "/EnergyLoss/Target-" +
                        sauronConf.deadlayerMaterial + "_Ion-" + std::to_string(sauronConf.ion.A) +
                        "-" + std::to_string(sauronConf.ion.Z) + ".txt";
    if (settings.verbosity_level > 0)
    {
        std::cout << "Sauron dead layer energy loss: " << sTemp
                  << " - is present: " << FileIsPresent(sTemp) << std::endl;
    }
    if (FileIsPresent(sTemp))
    {
        sauronDeadlayerELoss = {true, EnergyLoss(sTemp, "SRIM", 100)};
    }

    sTemp = conf.replay.conf_path + "/EnergyLoss/Target-" + sauronConf.absorberMaterial + "_Ion-" +
            std::to_string(sauronConf.ion.A) + "-" + std::to_string(sauronConf.ion.Z) + ".txt";
    if (settings.verbosity_level > 0)
    {
        std::cout << "Sauron absorber energy loss: " << sTemp
                  << " - is present: " << FileIsPresent(sTemp) << std::endl;
    }
    if (FileIsPresent(sTemp))
    {
        sauronAbsorberELoss = {true, EnergyLoss(sTemp, "SRIM", 100)};
    }

    Initialize<TH1D>(
        h.h_mult, new TH1D("h_mult", "SAURON multiplicity;Multiplicity;Counts", 10, 0, 10), h.ptrs);

    Initialize<TH2D>(h.m_Energy_TS,
                     new TH2D("m_Energy_TS", "SAURON TimeStamp vs energy;Time [s];Energy [MeV]",
                              4320, 0, 12 * 3600, 2000, 0, 200),
                     h.ptrs);

    Initialize<TH2D>(h.m_Theta_Phi,
                     new TH2D("m_Theta_Phi", "SAURON Theta vs Phi;Theta [deg];Phi [deg]", 180, 0,
                              180, 90, -180, 180),
                     h.ptrs);

    Initialize<TH2D>(
        h.m_layout_XY,
        new TH2D("m_layout_XY", "SAURON X vs Y;X[mm];Y[mm]", 400, -100, 100, 400, -100, 100),
        h.ptrs);

    Initialize<TH2D>(
        h.m_layout_XZ,
        new TH2D("m_layout_XZ", "SAURON X vs Z;X[mm];Z[mm]", 400, -100, 100, 400, -100, 100),
        h.ptrs);

    Initialize<TH2D>(
        h.m_layout_YZ,
        new TH2D("m_layout_YZ", "SAURON Y vs Z;Y[mm];Z[mm]", 400, -100, 100, 400, -100, 100),
        h.ptrs);

    Initialize<TH2D>(
        h.m_Energy_Theta,
        new TH2D("m_Energy_Theta", "SAURON theta vs energy;Theta [deg]; Energy [MeV]", 180, 0, 180,
                 static_cast<int>(sauronConf.sauronEmax / UNITS::MeV / sauronConf.sauronbinWidth),
                 0, sauronConf.sauronEmax / UNITS::MeV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_Energy_sauronID,
        new TH2D("m_Energy_sauronID", "SAURON energy vs sauronID;SAURON segment; Energy [MeV]", 300,
                 0, 300,
                 static_cast<int>(sauronConf.sauronEmax / UNITS::MeV / sauronConf.sauronbinWidth) +
                     static_cast<int>(10 / sauronConf.sauronbinWidth),
                 -10, sauronConf.sauronEmax / UNITS::MeV),
        h.ptrs);
    SetupLabelsForChannelID(sauronConf, h.m_Energy_sauronID);

    Initialize<TH2D>(h.m_Energy_sauronID_uncal,
                     new TH2D("m_Energy_sauronID_uncal",
                              "SAURON energy vs sauronID not "
                              "calibrated;SAURON segment; Energy channel;",
                              300, 0, 300, 5000, 0, 50000),
                     h.ptrs);
    SetupLabelsForChannelID(sauronConf, h.m_Energy_sauronID_uncal);

    Initialize<TH2D>(h.m_sauronBrdCh_sauronBrdCh,
                     new TH2D("m_sauronBrdCh_sauronBrdCh",
                              "SAURON channel correlation matrix "
                              "SAURON id;SAURON id; Counts;",
                              300, 0, 300, 300, 0, 300),
                     h.ptrs);
    SetupLabelsForChannelID(sauronConf, h.m_sauronBrdCh_sauronBrdCh);

    Initialize<TH2D>(h.m_sauronBrdCh_sauronBrdCh_withcond,
                     new TH2D("m_sauronBrdCh_sauronBrdCh_withcond",
                              "SAURON channel correlation matrix with front+back condition"
                              "SAURON id;SAURON id; Counts;",
                              300, 0, 300, 300, 0, 300),
                     h.ptrs);
    SetupLabelsForChannelID(sauronConf, h.m_sauronBrdCh_sauronBrdCh_withcond);

    Initialize<TH1D>(
        h.h_Ex,
        new TH1D(
            "h_Ex", "SAURON excitation energy;Excitation Energy [MeV];Counts/0.1 MeV",
            static_cast<int>(sauronConf.sauronExmax / UNITS::MeV / sauronConf.sauronExbinWidth) +
                static_cast<int>(10 / sauronConf.sauronExbinWidth),
            -10, sauronConf.sauronExmax / UNITS::MeV),
        h.ptrs);

    if (sauronConf.pidHistograms)
    {
        h.pidHistograms.dir = h.dir->mkdir("PID", "PID histograms");
        h.pidHistograms.dir->cd();

        for (unsigned int brd{0}; brd < 20; ++brd)
        {
            for (unsigned int ch{0}; ch < Lut::MAXCH; ++ch)
            {
                unsigned int id{Lut::getID(brd, ch)};
                std::string  name{};
                try
                {
                    name = sauronConf.getName(brd, ch);
                }
                catch (std::runtime_error &e)
                {
                };
                if (name.empty())
                    continue;

                Initialize<TH2D>(h.pidHistograms.m_Imax_Energy[id],
                                 new TH2D(Form("m_Imax_Energy_%i_%i", brd, ch),
                                          Form("IMax vs Energy for PID of %s;Energy [MeV];Imax "
                                               "[];Counts",
                                               name.c_str()),
                                          static_cast<int>(sauronConf.sauronEmax / UNITS::MeV /
                                                           sauronConf.sauronbinWidth) +
                                              static_cast<int>(10 / sauronConf.sauronbinWidth),
                                          -10, sauronConf.sauronEmax / UNITS::MeV, 1000, 0, 2500),
                                 h.ptrs);
            }
        }
    }
    for (const auto &reaction : input.reactions)
    {
        SauronHistograms::ReactionHistograms &reactionHistos = h.reactionHistos[{reaction.first}];
        std::string                           histoName =
            "_A" + std::to_string(reaction.first.A) + "Z" + std::to_string(reaction.first.Z);
        std::string histoTitle =
            " A=" + std::to_string(reaction.first.A) + " Z=" + std::to_string(reaction.first.Z);

        reactionHistos.dir =
            h.dir->mkdir(Form("Reaction%s", histoName.c_str()),
                         Form("Spectra with cuts on the identified ion: %s", histoTitle.c_str()));
        reactionHistos.dir->cd();

        Initialize<TH1D>(reactionHistos.h_Ex,
                         new TH1D(Form("h_Ex%s", histoName.c_str()),
                                  "SAURON excitation energy;Excitation Energy "
                                  "[MeV];Counts/0.1 MeV",
                                  static_cast<int>(sauronConf.sauronExmax / UNITS::MeV /
                                                   sauronConf.sauronExbinWidth) +
                                      static_cast<int>(10 / sauronConf.sauronExbinWidth),
                                  -10, sauronConf.sauronExmax / UNITS::MeV),
                         reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_Energy_Theta,
            new TH2D(
                "m_Energy_Theta", "SAURON theta vs energy;Theta [deg]; Energy [MeV]", 180, 0, 180,
                static_cast<int>(sauronConf.sauronEmax / UNITS::MeV / sauronConf.sauronbinWidth), 0,
                sauronConf.sauronEmax / UNITS::MeV),
            reactionHistos.ptrs);
        Initialize<TH2D>(
            reactionHistos.m_Ex_Theta,
            new TH2D(
                "m_Ex_Theta",
                "SAURON theta vs excitation energy;Theta [deg]; Excitation Energy [MeV]", 180, 0,
                180,
                static_cast<int>(sauronConf.sauronExmax / UNITS::MeV / sauronConf.sauronExbinWidth),
                0, sauronConf.sauronExmax / UNITS::MeV),
            reactionHistos.ptrs);
    }
    for (const auto &reaction : input.nBodyReactions)
    {
        SauronHistograms::ReactionHistograms &reactionHistos = h.reactionHistos[reaction.first];
        std::string                           histoName;
        std::string                           histoTitle;

        for (const auto &it : reaction.first)
        {
            histoName += "_";
            histoName += "A" + std::to_string(it.A);
            histoName += "Z" + std::to_string(it.Z);

            histoTitle += " A=" + std::to_string(it.A);
            histoTitle += " Z=" + std::to_string(it.Z);
        }

        reactionHistos.dir = h.dir->mkdir(
            Form("Reaction%s", histoName.c_str()),
            Form("Gamma spectra with cuts on the identified ion: %s", histoTitle.c_str()));
        reactionHistos.dir->cd();

        Initialize<TH1D>(reactionHistos.h_Ex,
                         new TH1D(Form("h_Ex%s", histoName.c_str()),
                                  "SAURON excitation energy;Excitation Energy "
                                  "[MeV];Counts/0.1 MeV",
                                  static_cast<int>(sauronConf.sauronExmax / UNITS::MeV /
                                                   sauronConf.sauronExbinWidth) +
                                      static_cast<int>(10 / sauronConf.sauronExbinWidth),
                                  -10, sauronConf.sauronExmax / UNITS::MeV),
                         reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_Energy_Theta,
            new TH2D(
                "m_Energy_Theta", "SAURON theta vs energy;Theta [deg]; Energy [MeV]", 180, 0, 180,
                static_cast<int>(sauronConf.sauronEmax / UNITS::MeV / sauronConf.sauronbinWidth), 0,
                sauronConf.sauronEmax / UNITS::MeV),
            reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_Ex_Theta,
            new TH2D(
                "m_Ex_Theta",
                "SAURON theta vs excitation energy;Theta [deg]; Excitation Energy [MeV]", 180, 0,
                180,
                static_cast<int>(sauronConf.sauronExmax / UNITS::MeV / sauronConf.sauronExbinWidth),
                0, sauronConf.sauronExmax / UNITS::MeV),
            reactionHistos.ptrs);
    }
    return true;
}

void SauronAnalysis::Analyze()
{
    DetectorAnalysis::Analyze();
    // if (!sauronReader.nbSAURON || !sauronReader.nbSAURON->IsValid()) return;
    if (!sauronReader.nbSAURON)
        return;

    sauron.sauronGood = false;

    if (settings.verbosity_level > 1)
    {
        std::cout << "\n------------------------------------------------------"
                     "-------------------------------\n";
        std::cout << "NEW SAURON EVENT\n " << std::endl;
    }

    std::vector<unsigned int> idx;

    if (settings.verbosity_level > 10)
    {
        std::cout << "INPUT:\n";
        std::cout << "nbSAURON " << (**sauronReader.nbSAURON) << "\n";
        std::cout << "TSSAURON " << (**sauronReader.TSSAURON) << "\n";
        std::cout << "TSSAURON_Hit\t";
        for (unsigned int i{0}; i < sauronReader.TSSAURON_Hit->GetSize(); i++)
            std::cout << sauronReader.TSSAURON_Hit->At(i) << "\t";
        std::cout << "\n";
        std::cout << "SAURONChannel\t";
        for (unsigned int i{0}; i < sauronReader.SAURONChannel->GetSize(); i++)
            std::cout << sauronReader.SAURONChannel->At(i) << "\t";
        std::cout << "\n";
        std::cout << "SAURONBoard\t";
        for (unsigned int i{0}; i < sauronReader.SAURONBoard->GetSize(); i++)
            std::cout << sauronReader.SAURONBoard->At(i) << "\t";
        std::cout << "\n";
        std::cout << "SAURONEnergy\t";
        for (unsigned int i{0}; i < sauronReader.SAURONEnergy->GetSize(); i++)
            std::cout << sauronReader.SAURONEnergy->At(i) << "\t";
        std::cout << "\n";
        std::cout << "SAURONTime\t";
        for (unsigned int i{0}; i < sauronReader.SAURONTime->GetSize(); i++)
            std::cout << sauronReader.SAURONTime->At(i) << "\t";
        std::cout << "\n";
        std::cout << std::endl;
    }

    // Fill correlation matrix
    for (unsigned int i{0}; i < sauronReader.SAURONEnergy->GetSize(); ++i)
    {
        unsigned int brd  = sauronReader.SAURONBoard->At(i);
        unsigned int chan = sauronReader.SAURONChannel->At(i);
        for (unsigned int j{0}; j < i; ++j)
        {
            unsigned int brd2  = sauronReader.SAURONBoard->At(j);
            unsigned int chan2 = sauronReader.SAURONChannel->At(j);
            Fill(sauronHistograms.m_sauronBrdCh_sauronBrdCh, Lut::getID(brd, chan),
                 Lut::getID(brd2, chan2));
            Fill(sauronHistograms.m_sauronBrdCh_sauronBrdCh, Lut::getID(brd2, chan2),
                 Lut::getID(brd, chan));
        }
    }

    // Fill raw histograms before any condition
    for (unsigned int i{0}; i < sauronReader.SAURONEnergy->GetSize(); ++i)
    {
        unsigned int brd  = sauronReader.SAURONBoard->At(i);
        unsigned int chan = sauronReader.SAURONChannel->At(i);

        if (!sauronConf.isPresent(brd, chan))
            continue;
        double      energy{sauronConf.getEnergy(sauronReader.SAURONEnergy->At(i), brd, chan)};
        const auto &thr = sauronConf.getThr(brd, chan);
        Fill(sauronHistograms.m_Energy_sauronID, Lut::getID(brd, chan), energy);
        Fill(sauronHistograms.m_Energy_sauronID_uncal, Lut::getID(brd, chan),
             sauronReader.SAURONEnergy->At(i));

        // send event into analysis only under certain conditions

        // check channel is not broken
        bool id_ok = true;
        for (const auto &it : sauronConf.brokenChannels)
        {
            if (static_cast<int>(brd) == it.first && static_cast<int>(chan) == it.second)
                id_ok = false;
        }

        // check threshold
        if (id_ok && energy > thr.first && energy < thr.second)
            idx.push_back(i);
    }
    if (idx.empty())
        return;

    // sauron time gate
    auto minTS = std::numeric_limits<ULong64_t>::max();
    {
        // get min TS of the event
        int minTSidx = -1;
        for (auto i : idx)
        {
            if (minTS > sauronReader.TSSAURON_Hit->At(i))
            {
                minTS    = sauronReader.TSSAURON_Hit->At(i);
                minTSidx = i;
            }
        }
        auto timeMinTS = sauronConf.getTime(
            sauronReader.SAURONCfd->At(minTSidx) * sauronConf.cfd_unit,
            sauronReader.SAURONBoard->At(minTSidx), sauronReader.SAURONChannel->At(minTSidx));

        // apply time gate
        for (auto it{idx.begin()}; it != idx.end();)
        {
            unsigned int brd  = sauronReader.SAURONBoard->At(*it);
            unsigned int chan = sauronReader.SAURONChannel->At(*it);
            auto time = unsignedDiff<double, unsigned long long>(sauronReader.TSSAURON_Hit->At(*it),
                                                                 minTS) *
                            sauronConf.time_unit +
                        sauronConf.getTime(sauronReader.SAURONCfd->At(*it) * sauronConf.cfd_unit,
                                           brd, chan) -
                        timeMinTS;
            if (time / (10 * UNITS::ns) >= sauronConf.windowLeft &&
                time / (10 * UNITS::ns) < sauronConf.windowRight)
                it++;
            else
                it = idx.erase(it);
        }
    }
    if (idx.empty())
        return;

    // Sauron Hit structure
    struct SauronHit
    {
        TVector3              Pos;
        double                ESi;
        double                ESiNside;
        double                ESiPside;
        double                ISiNside;
        double                ISiPside;
        double                Time;
        unsigned int          IdPside;
        unsigned int          IdNside;
        ReactionFragment::Ion pIon;
        ReactionFragment::Ion nIon;
    };
    std::vector<SauronHit> hitList;
    SauronHit              hitTmp;

    if (sauronConf.singleSideDetector)
    {
        std::vector<unsigned int> idxSide;
        for (auto i : idx)
        {
            unsigned int brd  = sauronReader.SAURONBoard->At(i);
            unsigned int chan = sauronReader.SAURONChannel->At(i);

            // take only from P side
            if (sauronConf.isPside(brd, chan) && sauronConf.pSidePriority)
                idxSide.push_back(i);

            // take only from N side
            if (sauronConf.isNside(brd, chan) && !sauronConf.pSidePriority)
                idxSide.push_back(i);
        }

        for (auto i : idxSide)
        {
            unsigned int brd  = sauronReader.SAURONBoard->At(i);
            unsigned int chan = sauronReader.SAURONChannel->At(i);

            hitTmp.ESi = sauronConf.getEnergy(sauronReader.SAURONEnergy->At(i), brd, chan);

            if (sauronConf.pSidePriority)
            {
                hitTmp.IdPside  = Lut::getID(brd, chan);
                hitTmp.IdNside  = 1000;
                hitTmp.ESiPside = hitTmp.ESi;
                hitTmp.ESiNside = 0;
                hitTmp.ISiPside = sauronReader.SAURONImax->At(i);
                hitTmp.ISiNside = 0;
                auto idRes      = particleGates.Id({static_cast<int>(brd), static_cast<int>(chan)},
                                                   hitTmp.ESiPside, hitTmp.ISiPside);
                if (idRes.first)
                {
                    hitTmp.pIon = ReactionFragment::Ion{static_cast<unsigned int>(idRes.second[0]),
                                                        static_cast<unsigned int>(idRes.second[1])};
                }
                else
                {
                    hitTmp.pIon = ReactionFragment::Ion{0, 0};
                }
            }
            else
            {
                hitTmp.IdNside  = Lut::getID(brd, chan);
                hitTmp.IdPside  = 1000;
                hitTmp.ESiNside = hitTmp.ESi;
                hitTmp.ESiPside = 0;
                hitTmp.ISiNside = sauronReader.SAURONImax->At(i);
                hitTmp.ISiPside = 0;
                auto idRes      = particleGates.Id({static_cast<int>(brd), static_cast<int>(chan)},
                                                   hitTmp.ESiNside, hitTmp.ISiNside);
                if (idRes.first)
                {
                    hitTmp.nIon = ReactionFragment::Ion{static_cast<unsigned int>(idRes.second[0]),
                                                        static_cast<unsigned int>(idRes.second[1])};
                }
                else
                {
                    hitTmp.nIon = ReactionFragment::Ion{0, 0};
                }
            }

            hitTmp.Pos.SetXYZ(sauronConf.getRadius(brd, chan), 0, sauronConf.distanceFromTarget);
            hitTmp.Pos.RotateZ(sauronConf.getPhi(brd, chan));

            // Corrections to the nominal position

            hitTmp.Pos += TVector3(sauronConf.xShift, sauronConf.yShift, sauronConf.zShift);

            // Small variations from lookup table
            hitTmp.Pos.RotateZ(sauronConf.phi);
            hitTmp.Pos.RotateX(sauronConf.thetaX);
            hitTmp.Pos.RotateY(sauronConf.thetaY);

            // Major variation, most likely should be kept to 0 in the
            // selector.conf
            hitTmp.Pos.RotateY(sauronConf.angle);

            hitTmp.Time =
                unsignedDiff<double, unsigned long long>(sauronReader.TSSAURON_Hit->At(i), minTS) *
                    sauronConf.time_unit +
                sauronConf.getTime(sauronReader.SAURONCfd->At(i) * sauronConf.cfd_unit, brd, chan);

            hitList.push_back(hitTmp);
        }
    }
    else
    {
        // split indexes in Pside/Nside and quadrant by quadrant
        std::array<std::vector<unsigned int>, 4> idxPside;
        std::array<std::vector<unsigned int>, 4> idxNside;
        for (auto i : idx)
        {
            unsigned int brd  = sauronReader.SAURONBoard->At(i);
            unsigned int chan = sauronReader.SAURONChannel->At(i);
            if (sauronConf.isPside(brd, chan))
                idxPside.at(sauronConf.getSegment(brd, chan) / 16).push_back(i);
            if (sauronConf.isNside(brd, chan))
                idxNside.at(sauronConf.getSegment(brd, chan) / 4).push_back(i);
        }

        // build PN coincidences
        for (unsigned int q{0}; q < 4; q++)
        {
            std::vector<unsigned int> &idxP = idxPside.at(q);
            std::vector<unsigned int> &idxN = idxNside.at(q);
            if (idxP.size() == 1 && idxN.size() == 1)
            {
                unsigned int idP   = idxP.at(0);
                unsigned int idN   = idxN.at(0);
                unsigned int brdP  = sauronReader.SAURONBoard->At(idP);
                unsigned int brdN  = sauronReader.SAURONBoard->At(idN);
                unsigned int chanP = sauronReader.SAURONChannel->At(idP);
                unsigned int chanN = sauronReader.SAURONChannel->At(idN);
                double eneP = sauronConf.getEnergy(sauronReader.SAURONEnergy->At(idP), brdP, chanP);
                double eneN = sauronConf.getEnergy(sauronReader.SAURONEnergy->At(idN), brdN, chanN);

                // energy gate
                if (eneP - eneN > sauronConf.eneWindowLeft &&
                    eneP - eneN < sauronConf.eneWindowRight)
                {
                    hitTmp.IdPside  = Lut::getID(brdP, chanP);
                    hitTmp.IdNside  = Lut::getID(brdN, chanN);
                    hitTmp.ESiPside = eneP;
                    hitTmp.ESiNside = eneN;
                    hitTmp.ISiPside = sauronReader.SAURONImax->At(idP);
                    hitTmp.ISiNside = sauronReader.SAURONImax->At(idN);

                    auto idResP =
                        particleGates.Id({static_cast<int>(brdP), static_cast<int>(chanP)},
                                         hitTmp.ESiPside, hitTmp.ISiPside);
                    if (idResP.first)
                    {
                        hitTmp.pIon =
                            ReactionFragment::Ion{static_cast<unsigned int>(idResP.second[0]),
                                                  static_cast<unsigned int>(idResP.second[1])};
                    }
                    else
                    {
                        hitTmp.pIon = ReactionFragment::Ion{0, 0};
                    }
                    auto idResN =
                        particleGates.Id({static_cast<int>(brdN), static_cast<int>(chanN)},
                                         hitTmp.ESiNside, hitTmp.ISiNside);
                    if (idResN.first)
                    {
                        hitTmp.nIon =
                            ReactionFragment::Ion{static_cast<unsigned int>(idResN.second[0]),
                                                  static_cast<unsigned int>(idResN.second[1])};
                    }
                    else
                    {
                        hitTmp.nIon = ReactionFragment::Ion{0, 0};
                    }

                    hitTmp.Pos.SetXYZ(sauronConf.getRadius(brdP, chanP), 0,
                                      sauronConf.distanceFromTarget);
                    hitTmp.Pos.RotateZ(sauronConf.getPhi(brdN, chanN));

                    // Corrections to the nominal position

                    hitTmp.Pos += TVector3(sauronConf.xShift, sauronConf.yShift, sauronConf.zShift);

                    // Small variations from lookup table
                    hitTmp.Pos.RotateZ(sauronConf.phi);
                    hitTmp.Pos.RotateX(sauronConf.thetaX);
                    hitTmp.Pos.RotateY(sauronConf.thetaY);

                    // Major variation, most likely should be kept to 0 in the
                    // selector.conf
                    hitTmp.Pos.RotateY(sauronConf.angle);

                    if (sauronConf.pSidePriority)
                    {
                        hitTmp.Time =
                            unsignedDiff<double, unsigned long long>(
                                sauronReader.TSSAURON_Hit->At(idP), minTS) *
                                sauronConf.time_unit +
                            sauronConf.getTime(
                                sauronReader.SAURONCfd->At(idP) * sauronConf.cfd_unit, brdP, chanP);
                        hitTmp.ESi = eneP;
                    }
                    else
                    {
                        hitTmp.Time =
                            unsignedDiff<double, unsigned long long>(
                                sauronReader.TSSAURON_Hit->At(idN), minTS) *
                                sauronConf.time_unit +
                            sauronConf.getTime(
                                sauronReader.SAURONCfd->At(idN) * sauronConf.cfd_unit, brdN, chanN);
                        hitTmp.ESi = eneN;
                    }
                    hitList.push_back(hitTmp);
                }
            }
        }
    }

    if (settings.verbosity_level > 8)
    {
        std::cout << "hitList size: " << hitList.size() << std::endl;
        if (settings.verbosity_level > 10)
        {
            for (auto hit : hitList)
            {
                std::cout << "\nTime: " << hit.Time << "\n";
                std::cout << "IdPside: " << hit.IdPside << "\n";
                std::cout << "IdNside: " << hit.IdNside << "\n";
                std::cout << "ESi: " << hit.ESi << "\n";
                std::cout << "ESiNside: " << hit.ESiNside << "\n";
                std::cout << "ESiPside: " << hit.ESiPside << "\n";
                std::cout << "ISiNside: " << hit.ISiNside << "\n";
                std::cout << "ISiPside: " << hit.ISiPside << "\n";
                std::cout << "Pos: " << hit.Pos.X() << " " << hit.Pos.Y() << " " << hit.Pos.Z()
                          << "\n";
                std::cout << "pIon: " << hit.pIon.A << " " << hit.pIon.Z << "\n";
                std::cout << "nIon: " << hit.nIon.A << " " << hit.nIon.Z << "\n";
            }
            std::cout << std::endl;
        }
    }

    if (hitList.empty())
        return;
    // Fill correlation matrix
    for (auto hit : hitList)
    {
        Fill(sauronHistograms.m_sauronBrdCh_sauronBrdCh_withcond, hit.IdPside, hit.IdNside);
        Fill(sauronHistograms.m_sauronBrdCh_sauronBrdCh_withcond, hit.IdNside, hit.IdPside);
        Fill(sauronHistograms.pidHistograms.m_Imax_Energy[hit.IdNside], hit.ESiNside, hit.ISiNside);
        Fill(sauronHistograms.pidHistograms.m_Imax_Energy[hit.IdPside], hit.ESiPside, hit.ISiPside);
    }
    /*
        for(unsigned int i{0}; i < sauronReader.SAURONEnergy->GetSize(); ++i) {
            unsigned int brd  = sauronReader.SAURONBoard->At(i);
            unsigned int chan = sauronReader.SAURONChannel->At(i);
            for(unsigned int j{0}; j < i; ++j) {
                unsigned int brd2  = sauronReader.SAURONBoard->At(j);
                unsigned int chan2 = sauronReader.SAURONChannel->At(j);
                Fill(sauronHistograms.m_sauronBrdCh_sauronBrdCh_withcond,
                    Lut::getID(brd, chan),
                    Lut::getID(brd2, chan2));
                Fill(sauronHistograms.m_sauronBrdCh_sauronBrdCh_withcond,
                    Lut::getID(brd2, chan2),
                    Lut::getID(brd, chan));
            }
        }*/

    sauron.resize(hitList.size());
    sauron.TS = minTS * (sauronConf.time_unit / (10 * UNITS::ns));

    for (unsigned int i{0}; i < hitList.size(); i++)
    {
        sauron.IdPside[i] = hitList[i].IdPside;
        sauron.IdNside[i] = hitList[i].IdNside;
        sauron.Time[i]    = hitList[i].Time;
        sauron.ESi[i]     = hitList[i].ESi;
        sauron.Pos[i]     = hitList[i].Pos;

        if (sauronConf.pidMatch)
        {
            if (hitList[i].pIon == hitList[i].nIon)
            {
                sauron.A[i] = hitList[i].pIon.A;
                sauron.Z[i] = hitList[i].pIon.Z;
                if (sauron.A[i] != 0)
                    sauron.identified[i] = true;
            }
            else
            {
                sauron.identified[i] = false;
            }
        }
        else
        {
            if (sauronConf.pidFromPside)
            {
                sauron.A[i] = hitList[i].pIon.A;
                sauron.Z[i] = hitList[i].pIon.Z;
                if (sauron.A[i] != 0)
                    sauron.identified[i] = true;
            }
            else
            {
                sauron.A[i] = hitList[i].nIon.A;
                sauron.Z[i] = hitList[i].nIon.Z;
                if (sauron.A[i] != 0)
                    sauron.identified[i] = true;
            }
        }

        if (sauronDeadlayerELoss.first)
        {
            sauron.E[i] = sauronDeadlayerELoss.second.EvaluateInitialEnergyWithDensity(
                sauron.ESi[i], sauronConf.deadlayerThickness,
                sauron.Pos[i].Angle(TVector3(0, 0, 1)));
        }
        else
            sauron.E[i] = sauron.ESi[i];

        if (sauronAbsorberELoss.first)
        {
            sauron.E[i] = sauronAbsorberELoss.second.EvaluateInitialEnergyWithDensity(
                sauron.E[i], sauronConf.absorberThickness, sauron.Pos[i].Angle(TVector3(0, 0, 1)));
        }

        ReactionFragment::Ion ion;
        if (sauron.identified[i])
            ion = ReactionFragment::Ion{sauron.A[i], sauron.Z[i]};
        else
            ion = sauronConf.ion;
        if (settings.verbosity_level > 3)
        {
            std::cout << "Sauron Analysis - Reaction found for ion A " << ion.A << " Z " << ion.Z
                      << std::endl;
            for (const auto &reaction : input.reactions)
            {
                std::cout << "reactions: " << reaction.first.A << " " << reaction.first.Z
                          << std::endl;
            }
        }
        auto reactionsIt = input.reactions.find(ion);

        if (reactionsIt == input.reactions.end())
            throw std::runtime_error("Ion in sauron not found in reactions of "
                                     "interest! Check the conf file\n");

        auto reactionCopy = reactionsIt->second;
        reactionCopy.GetFixedFragment().Set_Ek(sauron.E[i]);
        reactionCopy.GetFixedFragment().Set_Dir(sauron.Pos[i]);

        TLorentzVector p = reactionCopy.GetFixedFragment().Get_P();
        EvalInTarget(p, ion);

        sauron.E[i] = p.E() - p.M();
        // std::cout<<"theta "<<p.Theta()<<" phi "<<p.Phi()<<std::endl;

        try
        {
            sauron.Ex[i] =
                static_cast<double>(reactionCopy.Set_Ek_Theta_Phi(sauron.E[i], p.Theta(), p.Phi()));
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what();
        }
        sauron.p3[i] = reactionCopy.GetFixedFragment().Get_P(); // Measured by sauron
        sauron.p4[i] = reactionCopy.GetFreeFragment().Get_P();  // Binary partner

        sauron.sauronGood = true;
        // std::cout<<"Sauron p3 SA "<<sauron.p3.at(i).Vect().x()<<"
        // "<<sauron.p3.at(i).Vect().y()<<" "<<sauron.p3.at(i).Vect().z()<<std::endl;
    }

    if (settings.verbosity_level > 5)
    {
        std::cout << "sauronGood " << sauron.sauronGood << std::endl;
        for (unsigned int i{0}; i < sauron.E.size(); ++i)
        {
            std::cout << "\nTS\t" << sauron.TS << "\t" << input.firstTS << "\n";
            std::cout << "Time\t" << sauron.Time.at(i) << "\n";
            std::cout << "IdP " << sauron.IdPside.at(i) << "\tIdN " << sauron.IdNside.at(i) << "\n";
            std::cout << "E\t" << sauron.ESi.at(i) << "\t" << sauron.E.at(i) << "\n";
            std::cout << "Ex\t" << sauron.Ex.at(i) << "\n";
            if (settings.verbosity_level > 5)
            {
                std::cout << "Pos\t( " << sauron.Pos.at(i).X() / UNITS::mm << " "
                          << sauron.Pos.at(i).Y() / UNITS::mm << " "
                          << sauron.Pos.at(i).Z() / UNITS::mm << " )\t";
                std::cout << "( " << sauron.Pos.at(i).Mag() / UNITS::mm << " "
                          << sauron.Pos.at(i).Theta() / UNITS::deg << " "
                          << sauron.Pos.at(i).Phi() / UNITS::deg << " )\n";
                std::cout << "p3\t( " << sauron.p3.at(i).X() / UNITS::mm << " "
                          << sauron.p3.at(i).Y() / UNITS::mm << " "
                          << sauron.p3.at(i).Z() / UNITS::mm << " )\t";
                std::cout << "( " << sauron.p3.at(i).Mag() << " "
                          << sauron.p3.at(i).Theta() / UNITS::deg << " "
                          << sauron.p3.at(i).Phi() / UNITS::deg << " )\n";
                std::cout << "p4\t( " << sauron.p4.at(i).X() / UNITS::mm << " "
                          << sauron.p4.at(i).Y() / UNITS::mm << " "
                          << sauron.p4.at(i).Z() / UNITS::mm << " )\t";
                std::cout << "( " << sauron.p4.at(i).Mag() << " "
                          << sauron.p4.at(i).Theta() / UNITS::deg << " "
                          << sauron.p4.at(i).Phi() / UNITS::deg << " )\n";
                std::cout << "Mass g.s. " << sauron.p4.at(i).Mag() - sauron.Ex.at(i) << std::endl;
            }
        }
    }
    if (settings.verbosity_level > 3)
    {
        std::cout << "sauronGood " << sauron.sauronGood << std::endl;
        for (unsigned int i{0}; i < sauron.E.size(); ++i)
        {

            std::cout << "p3\t( " << sauron.p3.at(i).Vect().x() << " " << sauron.p3.at(i).Vect().y()
                      << " " << sauron.p3.at(i).Vect().z() << " )\t";

            std::cout << "p4\t( " << sauron.p4.at(i).Vect().x() << " " << sauron.p4.at(i).Vect().y()
                      << " " << sauron.p4.at(i).Vect().z() << " )\t";
        }
    }

    if (settings.verbosity_level > 1)
        std::cout << "----------------------------------------------------------------------"
                  << std::endl;
}

bool SauronAnalysis::FillHistos()
{
    if (!DetectorAnalysis::FillHistos())
        return false;

    // Filling histograms
    SauronHistograms &h = sauronHistograms;

    Fill(h.h_mult, static_cast<int>(sauron.E.size()));

    for (unsigned int i{0}; i < sauron.E.size(); ++i)
    {
        /*       Fill(h.m_Energy_Time,
                 unsignedDiff<double, unsigned long long>(sauron.Time[i],
           timestampRangeSauron.first)/1.E8, sauron.E[i]);
         */
        Fill(h.m_Energy_TS,
             unsignedDiff<double, unsigned long long>(sauron.TS, input.firstTS) / 1e8,
             sauron.ESi[i]);

        Fill(h.m_Theta_Phi, sauron.p3[i].Theta() / UNITS::deg, sauron.p3[i].Phi() / UNITS::deg);

        Fill(h.m_Energy_Theta, sauron.p3[i].Theta() / UNITS::deg, sauron.E[i]);

        Fill(h.m_layout_XY, sauron.Pos[i].x() / UNITS::mm, sauron.Pos[i].y() / UNITS::mm);

        Fill(h.m_layout_XZ, sauron.Pos[i].x() / UNITS::mm, sauron.Pos[i].z() / UNITS::mm);

        Fill(h.m_layout_YZ, sauron.Pos[i].y() / UNITS::mm, sauron.Pos[i].z() / UNITS::mm);

        Fill(h.h_Ex, sauron.Ex[i]);

        // Particle identification histograms
        std::vector<ReactionFragment::Ion> reaction;
        for (unsigned int e{0}; e < sauron.E.size(); ++e)
        {
            if (!sauron.identified.at(e))
            {
                continue;
            }

            ReactionFragment::Ion idIon{sauron.A.at(e), sauron.Z.at(e)};
            reaction.emplace_back(idIon);
        }
        std::sort(reaction.begin(), reaction.end());

        auto reactionHist = h.reactionHistos.find(reaction);
        if (reactionHist != h.reactionHistos.end())
        {
            for (unsigned int s{0}; s < sauron.E.size(); ++s)
            {
                Fill(reactionHist->second.h_Ex, sauron.Ex.at(s));
                Fill(reactionHist->second.m_Ex_sauronId, sauron.IdPside[s], sauron.Ex[s]);
                Fill(reactionHist->second.m_Ex_sauronId, sauron.IdNside[s], sauron.Ex[s]);
                Fill(reactionHist->second.m_Ex_Theta, sauron.p3[s].Theta() / UNITS::deg,
                     sauron.Ex[s]);
                Fill(reactionHist->second.m_Energy_Theta, sauron.p3[s].Theta() / UNITS::deg,
                     sauron.E[s]);
            }
        }
    }

    return true;
}

void SauronAnalysis::Instantiate()
{
    DetectorAnalysis::Instantiate();
    TTreeReader &fReader{input.fReader};

    if (conf.detectorsPresent.SauronFw || conf.detectorsPresent.SauronBw)
    {
        if (input.treeToRead->FindLeaf("nbSAURON") == nullptr)
            throw std::runtime_error("SAURON enabled in the config but not found in the tree");

        sauronReader.nbSAURON     = new TTreeReaderValue<Int_t>(fReader, "nbSAURON");
        sauronReader.TSSAURON     = new TTreeReaderValue<ULong64_t>(fReader, "TSSAURON");
        sauronReader.TSSAURON_Hit = new TTreeReaderArray<ULong64_t>(fReader, "TSSAURON_Hit");

        if (input.treeToRead->FindLeaf("SAURONChannelId") != nullptr)
            sauronReader.SAURONChannel =
                new TTreeReaderArray<UShort_t>(fReader, "SAURONChannel.SAURONChannelId");
        else
            sauronReader.SAURONChannel = new TTreeReaderArray<UShort_t>(fReader, "SAURONChannel");

        if (input.treeToRead->FindLeaf("SAURONBoardId") != nullptr)
            sauronReader.SAURONBoard =
                new TTreeReaderArray<UShort_t>(fReader, "SAURONBoard.SAURONBoardId");
        else
            sauronReader.SAURONBoard = new TTreeReaderArray<UShort_t>(fReader, "SAURONBoard");

        sauronReader.SAURONEnergy = new TTreeReaderArray<Float_t>(fReader, "SAURONEnergy");
        sauronReader.SAURONTime   = new TTreeReaderArray<Float_t>(fReader, "SAURONTime");
        sauronReader.SAURONCfd    = new TTreeReaderArray<Float_t>(fReader, "SAURONCfd");
        sauronReader.SAURONImax   = new TTreeReaderArray<Float_t>(fReader, "SAURONImax");
        sauronReader.SAURONPid    = new TTreeReaderArray<Float_t>(fReader, "SAURONPid");
    }
}

void SauronAnalysis::Finalize()
{
    DetectorAnalysis::Finalize();
    NormalizeHistos<DetectorConf>(sauronConf);
}
