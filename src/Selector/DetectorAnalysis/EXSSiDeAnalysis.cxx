#include "EXSSiDeAnalysis.h"
#include "EXSSiDe.h"
#include "EXSSiDeConf.h"
#include "ReactionFragment.h"

EXSSiDeAnalysis::EXSSiDeAnalysis(DetectorAnalysis::Inputs in, EXSSiDe &s)
    : DetectorAnalysis(in, s, ConfManager::getInstance().exsside, exssideHistograms), exsside(s)
{
    if (detectorConf.enable_treeFill && input.type == Inputs::Type::SIGNAL)
        input.tree->Branch("EXSSiDe", &exsside);
}

bool EXSSiDeAnalysis::AllocateHistos()
{

    EXSSiDeHistograms &h = exssideHistograms;

    h.dir = input.outFile.mkdir("EXSSiDe", "EXSSiDe histograms");
    h.dir->cd();
    if (!DetectorAnalysis::AllocateHistos())
        return false;

    // Setup deadlayer energy loss
    std::string sTemp =
        conf.replay.conf_path + "/EnergyLoss/Target-" + conf.exsside.deadlayerMaterial + "_Ion-" +
        std::to_string(conf.exsside.ion.A) + "-" + std::to_string(conf.exsside.ion.Z) + ".txt";
    if (settings.verbosity_level > 0)
    {
        std::cout << "EXSSiDe dead layer energy loss: " << sTemp
                  << " - is present: " << FileIsPresent(sTemp) << std::endl;
    }
    if (FileIsPresent(sTemp))
    {
        exssideDeadlayerELoss = {true, EnergyLoss(sTemp, "SRIM", 100)};
    }

    sTemp = conf.replay.conf_path + "/EnergyLoss/Target-" + conf.exsside.absorberMaterial +
            "_Ion-" + std::to_string(conf.exsside.ion.A) + "-" +
            std::to_string(conf.exsside.ion.Z) + ".txt";
    if (settings.verbosity_level > 0)
    {
        std::cout << "EXSSiDe absorber energy loss: " << sTemp
                  << " - is present: " << FileIsPresent(sTemp) << std::endl;
    }
    if (FileIsPresent(sTemp))
    {
        exssideAbsorberELoss = {true, EnergyLoss(sTemp, "SRIM", 100)};
    }

    // Initialize<TH1D>(h.h_mult,
    //                  new TH1D("h_mult",
    //                           "EXSSiDe multiplicity;Multiplicity;Counts", 10, 0,
    //                           10),
    //                  h.ptrs);

    Initialize<TH2D>(h.m_exsside_TSdiff,
                     new TH2D("m_exsside_TSdiff",
                              "EXSSiDe Back Channel vs TS diff;#DeltaT [10 ns]; Channel", 1000, -50,
                              50, 64, 0, 64),
                     h.ptrs);

    Initialize<TH2D>(h.m_Energy_TS,
                     new TH2D("m_Energy_TS", "EXSSiDe Energy vs Time;T [s]; Energy [MeV]", 12 * 360,
                              0, 12 * 3600, 1000, 0, 20000),
                     h.ptrs);

    Initialize<TH2D>(h.m_layout_XY,
                     new TH2D("m_layout_XY", "EXSSiDe X vs Y;(AGATA)<-(-X)[mm];Y[mm]", 50, -49.5,
                              50.5, 50, -49.5, 50.5),
                     h.ptrs);

    Initialize<TH2D>(h.m_layout_XZ,
                     new TH2D("m_layout_XZ", "EXSSiDe X vs Z;(AGATA)<-(-X)[mm];Z->[mm]", 50, -49.5,
                              50.5, 200, -6900., -6700.),
                     h.ptrs);

    Initialize<TH2D>(h.m_layout_YZ,
                     new TH2D("m_layout_YZ", "EXSSiDe Y vs Z;Y[mm];Z->[mm]", 50, -49.5, 50.5, 200,
                              -6900., -6700.),
                     h.ptrs);

    Initialize<TH2D>(
        h.m_Energy_Theta,
        new TH2D(
            "m_Energy_Theta", "EXSSiDe theta vs energy;Theta [deg]; Energy [MeV]", 180, 0, 180,
            static_cast<int>(conf.exsside.exssideEmax / UNITS::MeV / conf.exsside.exssidebinWidth),
            0, conf.exsside.exssideEmax / UNITS::MeV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_Energy_exssideID,
        new TH2D(
            "m_Energy_exssideID", "EXSSiDe energy vs exssideID;EXSSiDe segment; Energy [MeV]", 64,
            0, 64,
            static_cast<int>(conf.exsside.exssideEmax / UNITS::MeV / conf.exsside.exssidebinWidth) +
                static_cast<int>(10 / conf.exsside.exssidebinWidth),
            -10, conf.exsside.exssideEmax / UNITS::MeV),
        h.ptrs);
    SetupLabelsForChannelID(conf.exsside, h.m_Energy_exssideID);

    Initialize<TH2D>(h.m_Energy_exssideID_uncal,
                     new TH2D("m_Energy_exssideID_uncal",
                              "EXSSiDe energy vs exssideID not "
                              "calibrated;EXSSiDe segment; Energy channel;",
                              200, 0, 200, 2000, 0, 20000),
                     h.ptrs);
    SetupLabelsForChannelID(conf.exsside, h.m_Energy_exssideID_uncal);

    Initialize<TH2D>(h.m_exssideBrdCh_exssideBrdCh,
                     new TH2D("m_exssideBrdCh_exssideBrdCh",
                              "EXSSiDe channel correlation matrix "
                              "Front strip nr; Strip nr; Counts;",
                              200, 0, 200, 200, 0, 200),
                     h.ptrs);
    SetupLabelsForChannelID(conf.exsside, h.m_exssideBrdCh_exssideBrdCh);

    Initialize<TH2D>(h.m_exssideBrdCh_exssideBrdCh_withcond,
                     new TH2D("m_exssideBrdCh_exssideBrdCh_withcond",
                              "EXSSiDe channel correlation matrix with front+back condition"
                              "Front strip nr; Strip nr; Counts;",
                              200, 0, 200, 200, 0, 200),
                     h.ptrs);
    SetupLabelsForChannelID(conf.exsside, h.m_exssideBrdCh_exssideBrdCh_withcond);

    Initialize<TH1F>(h.h_exxside_EDiff,
                     new TH1F("h_exxside_EDiff",
                              "Back E-front E strip"
                              "E [MeV]; Counts;",
                              2000, -7000, 10000),
                     h.ptrs);

    for (const auto &reaction : input.reactions)
    {
        EXSSiDeHistograms::ReactionHistograms &reactionHistos = h.reactionHistos[{reaction.first}];
        std::string                            histoName =
            "_A" + std::to_string(reaction.first.A) + "Z" + std::to_string(reaction.first.Z);
        std::string histoTitle =
            " A=" + std::to_string(reaction.first.A) + " Z=" + std::to_string(reaction.first.Z);

        reactionHistos.dir = h.dir->mkdir(
            Form("Reaction%s", histoName.c_str()),
            Form("Gamma spectra with cuts on the identified ion: %s", histoTitle.c_str()));
        reactionHistos.dir->cd();

        Initialize<TH2D>(reactionHistos.m_Energy_Theta,
                         new TH2D("m_Energy_Theta",
                                  "EXSSiDe theta vs energy;Theta [deg]; Energy [MeV]", 180, 0, 180,
                                  static_cast<int>(conf.exsside.exssideEmax / UNITS::MeV /
                                                   conf.exsside.exssidebinWidth),
                                  0, conf.exsside.exssideEmax / UNITS::MeV),
                         reactionHistos.ptrs);
    }
    for (const auto &reaction : input.nBodyReactions)
    {
        EXSSiDeHistograms::ReactionHistograms &reactionHistos = h.reactionHistos[reaction.first];
        std::string                            histoName;
        std::string                            histoTitle;

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

        Initialize<TH2D>(reactionHistos.m_Energy_Theta,
                         new TH2D("m_Energy_Theta",
                                  "EXSSiDe theta vs energy;Theta [deg]; Energy [MeV]", 180, 0, 180,
                                  static_cast<int>(conf.exsside.exssideEmax / UNITS::MeV /
                                                   conf.exsside.exssidebinWidth),
                                  0, conf.exsside.exssideEmax / UNITS::MeV),
                         reactionHistos.ptrs);
    }
    return true;
}

void EXSSiDeAnalysis::Analyze()
{
    DetectorAnalysis::Analyze();

    // if (!exssideReader.nbEXSSiDe || !exssideReader.nbEXSSiDe->IsValid()) return;
    if (!exssideReader.nbEXSSiDe)
        return;

    if (settings.verbosity_level > 8)
    {
        std::cout << "\n------------------------------------------------------"
                     "-------------------------------\n";
        std::cout << "NEW EXSSiDe EVENT\n " << std::endl;
    }

    std::vector<unsigned int> idx;

    if (settings.verbosity_level > 10)
    {
        std::cout << "INPUT:\n";
        std::cout << "nbEXSSiDe " << (**exssideReader.nbEXSSiDe) << "\n";
        std::cout << "TSEXSSiDe " << (**exssideReader.TSEXSSiDe) << "\n";
        std::cout << "TSEXSSiDe_Hit\t";
        for (unsigned int i{0}; i < exssideReader.TSEXSSiDe_Hit->GetSize(); i++)
            std::cout << exssideReader.TSEXSSiDe_Hit->At(i) << "\t";
        std::cout << "\n";
        std::cout << "EXSSiDeChannel\t";
        for (unsigned int i{0}; i < exssideReader.EXSSiDeChannel->GetSize(); i++)
            std::cout << exssideReader.EXSSiDeChannel->At(i) << "\t";
        std::cout << "\n";
        std::cout << "EXSSiDeBoard\t";
        for (unsigned int i{0}; i < exssideReader.EXSSiDeBoard->GetSize(); i++)
            std::cout << exssideReader.EXSSiDeBoard->At(i) << "\t";
        std::cout << "\n";
        std::cout << "EXSSiDeEnergy\t";
        for (unsigned int i{0}; i < exssideReader.EXSSiDeEnergy->GetSize(); i++)
            std::cout << exssideReader.EXSSiDeEnergy->At(i) << "\t";
        std::cout << "\n";
        std::cout << "EXSSiDeTime\t";
        for (unsigned int i{0}; i < exssideReader.EXSSiDeTime->GetSize(); i++)
            std::cout << exssideReader.EXSSiDeTime->At(i) << "\t";
        std::cout << "\n";
        std::cout << std::endl;
    }

    // Fill correlation matrix
    for (unsigned int i{0}; i < exssideReader.EXSSiDeEnergy->GetSize(); ++i)
    {
        unsigned int brd  = exssideReader.EXSSiDeBoard->At(i);
        unsigned int chan = exssideReader.EXSSiDeChannel->At(i);
        double       ene1 = exssideReader.EXSSiDeEnergy->At(i);
        for (unsigned int j{0}; j < i; ++j)
        {
            unsigned int brd2  = exssideReader.EXSSiDeBoard->At(j);
            unsigned int chan2 = exssideReader.EXSSiDeChannel->At(j);
            double       ene2  = exssideReader.EXSSiDeEnergy->At(j);
            if (ene1 >= conf.exsside.minE && ene2 >= conf.exsside.minE)
            {
                Fill(exssideHistograms.m_exssideBrdCh_exssideBrdCh, Lut::getID(brd, chan),
                     Lut::getID(brd2, chan2));
                Fill(exssideHistograms.m_exssideBrdCh_exssideBrdCh, Lut::getID(brd2, chan2),
                     Lut::getID(brd, chan));
            }
        }
    }

    // Fill raw histograms before any condition
    for (unsigned int i{0}; i < exssideReader.EXSSiDeEnergy->GetSize(); ++i)
    {
        unsigned int brd  = exssideReader.EXSSiDeBoard->At(i);
        unsigned int chan = exssideReader.EXSSiDeChannel->At(i);

        if (!conf.exsside.isPresent(brd, chan))
            continue;

        double      energy{conf.exsside.getEnergy(exssideReader.EXSSiDeEnergy->At(i), brd, chan)};
        const auto &thr = conf.exsside.getThr(brd, chan);
        // Fill(exssideHistograms.m_Energy_exssideID, Lut::getID(brd,chan),
        //      energy);
        Fill(exssideHistograms.m_Energy_exssideID_uncal, Lut::getID(brd, chan),
             exssideReader.EXSSiDeEnergy->At(i));

        // send event into analysis only under certain conditions

        // check channel is not broken
        bool id_ok = true;
        for (const auto &it : conf.exsside.brokenChannels)
        {
            if (static_cast<int>(brd) == it.first && static_cast<int>(chan) == it.second)
            {
                id_ok = false;
                if (settings.verbosity_level > 3)
                {
                    std::cout << "Removing broken channel " << chan << " of board " << brd
                              << std::endl;
                    std::cout << it.second << " of board " << it.first << std::endl;
                }
            }
        }
        id_ok = true;

        // check threshold
        if (id_ok && energy > thr.first && energy < thr.second)
            idx.push_back(i);
    }
    if (idx.empty())
        return;
    // now only events within energy threshold and not for broken channels

    if (settings.verbosity_level > 2)
        std::cout << "EXSSiDe event multiplicity (energy thr) = " << idx.size() << std::endl;

    // exsside time gate
    auto minTS = std::numeric_limits<ULong64_t>::max();

    // get min TS of the event
    for (auto i : idx)
    {
        if (minTS > exssideReader.TSEXSSiDe_Hit->At(i))
        {
            minTS = exssideReader.TSEXSSiDe_Hit->At(i) +
                    conf.exsside.getTime(exssideReader.EXSSiDeTime->At(i),
                                         exssideReader.EXSSiDeBoard->At(i),
                                         exssideReader.EXSSiDeChannel->At(i));
        }
    }
    auto timeMinTS = minTS; //+conf.exsside.getTime(
                            // exssideReader.EXSSiDeTime->At(minTSidx),
                            // exssideReader.EXSSiDeBoard->At(minTSidx),
                            // exssideReader.EXSSiDeChannel->At(minTSidx));

    auto tref = unsignedDiff<double, unsigned long long>(
                    exssideReader.TSEXSSiDe_Hit->At(conf.exsside.refch), minTS) *
                    conf.exsside.time_unit +
                conf.exsside.getTime(exssideReader.EXSSiDeTime->At(conf.exsside.refch),
                                     exssideReader.EXSSiDeBoard->At(conf.exsside.refch),
                                     conf.exsside.refch) *
                    conf.exsside.cfd_unit;

    // apply time gate
    for (auto it{idx.begin()}; it != idx.end();)
    {
        unsigned int brd  = exssideReader.EXSSiDeBoard->At(*it);
        unsigned int chan = exssideReader.EXSSiDeChannel->At(*it);
        auto         time =
            unsignedDiff<double, unsigned long long>(exssideReader.TSEXSSiDe_Hit->At(*it), minTS) *
                conf.exsside.time_unit +
            conf.exsside.getTime(exssideReader.EXSSiDeTime->At(*it), brd, chan) *
                conf.exsside.cfd_unit;
        auto diff = time - tref;
        Fill(exssideHistograms.m_exsside_TSdiff, diff / (10 * UNITS::ns), chan);
        if (time / (10 * UNITS::ns) >= windowLeft && time / (10 * UNITS::ns) < windowRight)
            it++;
        else
            it = idx.erase(it);
    }

    if (idx.empty())
        return;
    // now only GOOD events

    if (settings.verbosity_level > 2)
        std::cout << "EXSSiDe event multiplicity (energy and time gate) = " << idx.size()
                  << std::endl;

    // EXSSiDe Hit structure
    struct EXSSiDeHit
    {
        TVector3                  Pos;
        double                    ESi;
        std::pair<double, double> Eraw;
        double                    ESiNside;
        double                    ESiPside;
        double                    Time;
        unsigned int              IdPside;
        unsigned int              IdNside;
        bool                      goodev = false;
    };
    std::vector<EXSSiDeHit> hitList;
    EXSSiDeHit              hitTmp;

    if (conf.exsside.singleSideDetector)
    {
        std::vector<unsigned int> idxSide;
        for (auto i : idx)
        {
            unsigned int brd  = exssideReader.EXSSiDeBoard->At(i);
            unsigned int chan = exssideReader.EXSSiDeChannel->At(i);

            // take only from P side
            if (conf.exsside.isPside(brd, chan) && conf.exsside.pSidePriority)
                idxSide.push_back(i);

            // take only from N side
            if (conf.exsside.isNside(brd, chan) && !conf.exsside.pSidePriority)
                idxSide.push_back(i);
        }
        try
        {
            if (idxSide.size() > 1)
                throw std::runtime_error(
                    "More than one good event for single-side detector configuration");
        }
        catch (...)
        {
            return;
        }

        for (auto i : idxSide)
        {
            hitTmp.goodev     = false;
            unsigned int brd  = exssideReader.EXSSiDeBoard->At(i);
            unsigned int chan = exssideReader.EXSSiDeChannel->At(i);

            hitTmp.ESi = conf.exsside.getEnergy(exssideReader.EXSSiDeEnergy->At(i), brd, chan);
            if (conf.exsside.isPside(brd, chan))
                hitTmp.Eraw = {0, exssideReader.EXSSiDeEnergy->At(i)};
            else
                hitTmp.Eraw = {exssideReader.EXSSiDeEnergy->At(i), 0};
            // I still don't know if PSIDE is vertical or viceversa
            // Assuming PSIDE is vertical

            if (conf.exsside.pSidePriority)
            {
                hitTmp.IdPside  = Lut::getID(brd, chan);
                hitTmp.IdNside  = 1000;
                hitTmp.ESiPside = hitTmp.ESi;
                hitTmp.ESiNside = 0;
                hitTmp.Pos      = conf.exsside.getPosition(chan, conf.exsside.getAvgY());
            }
            else
            {
                hitTmp.IdNside  = Lut::getID(brd, chan);
                hitTmp.IdPside  = 1000;
                hitTmp.ESiNside = hitTmp.ESi;
                hitTmp.ESiPside = 0;
                hitTmp.Pos      = conf.exsside.getPosition(conf.exsside.getAvgX(), chan);
            }

            // Major variation, most likely should be kept to 0 in the
            // selector.conf
            hitTmp.Pos.RotateY(conf.exsside.angle);

            hitTmp.Time = unsignedDiff<double, unsigned long long>(
                              exssideReader.TSEXSSiDe_Hit->At(i), minTS) *
                              conf.exsside.time_unit +
                          conf.exsside.getTime(exssideReader.EXSSiDeTime->At(i), brd, chan) *
                              conf.exsside.cfd_unit;

            hitList.push_back(hitTmp);

        } // onlyOneSide
    }
    else
    {
        // split indexes in Pside/Nside
        std::vector<unsigned int> idxPside;
        std::vector<unsigned int> idxNside;
        for (auto i : idx)
        {
            hitTmp.goodev     = false;
            unsigned int brd  = exssideReader.EXSSiDeBoard->At(i);
            unsigned int chan = exssideReader.EXSSiDeChannel->At(i);
            if (conf.exsside.isPside(brd, chan))
                idxPside.push_back(i);
            if (conf.exsside.isNside(brd, chan))
                idxNside.push_back(i);
        }
        try
        {
            if (idxPside.size() > 1 || idxNside.size() > 1)
            {
                throw std::runtime_error(
                    "More than one good event for double-side detector configuration");
            }
        }
        catch (...)
        {
            return;
        }
        // build PN coincidences
        if (idxPside.size() == 1 && idxNside.size() == 1)
        {
            unsigned int idP   = idxPside.at(0);
            unsigned int idN   = idxNside.at(0);
            unsigned int brdP  = exssideReader.EXSSiDeBoard->At(idP);
            unsigned int brdN  = exssideReader.EXSSiDeBoard->At(idN);
            unsigned int chanP = exssideReader.EXSSiDeChannel->At(idP);
            unsigned int chanN = exssideReader.EXSSiDeChannel->At(idN);
            double eneP = conf.exsside.getEnergy(exssideReader.EXSSiDeEnergy->At(idP), brdP, chanP);
            double eneN = conf.exsside.getEnergy(exssideReader.EXSSiDeEnergy->At(idN), brdN, chanN);
            // energy gate
            if (eneN - eneP > conf.exsside.eneWindowLeft &&
                eneN - eneP < conf.exsside.eneWindowRight)
                if (eneN >= conf.exsside.minE && eneP >= conf.exsside.minE)
                {

                    hitTmp.goodev   = true;
                    hitTmp.IdPside  = Lut::getID(brdP, chanP);
                    hitTmp.IdNside  = Lut::getID(brdN, chanN);
                    hitTmp.ESiPside = eneP;
                    hitTmp.ESiNside = eneN;
                    hitTmp.Pos      = conf.exsside.getPosition(chanN, chanP);

                    if (settings.verbosity_level > 10)
                        std::cout << chanN << "\t" << chanP << "\t" << hitTmp.Pos.x() << "\t"
                                  << hitTmp.Pos.y() << "\t" << hitTmp.Pos.z() << std::endl;

                    // Small variations from lookup table
                    hitTmp.Pos.RotateZ(conf.exsside.phi);
                    hitTmp.Pos.RotateX(conf.exsside.thetaX);
                    hitTmp.Pos.RotateY(conf.exsside.thetaY);
                    hitTmp.Pos.RotateY(conf.exsside.angle);

                    hitTmp.Eraw = {exssideReader.EXSSiDeEnergy->At(idN),
                                   exssideReader.EXSSiDeEnergy->At(idP)};
                    hitTmp.ESi  = eneN;

                    if (settings.verbosity_level > 3)
                        if (chanN == 0)
                            std::cout << "chanN =" << chanN << " " << hitTmp.Pos.x() << " "
                                      << hitTmp.Pos.y() << std::endl;

                    Fill(exssideHistograms.m_Energy_exssideID, chanN, eneN);
                    Fill(exssideHistograms.m_Energy_exssideID, chanP, eneP);

                    Fill(exssideHistograms.h_exxside_EDiff, eneN - eneP);

                    if (conf.exsside.pSidePriority)
                    {
                        hitTmp.ESi = eneP;
                        hitTmp.Time =
                            unsignedDiff<double, unsigned long long>(
                                exssideReader.TSEXSSiDe_Hit->At(idP), minTS) *
                                conf.exsside.time_unit +
                            conf.exsside.getTime(exssideReader.EXSSiDeTime->At(idP), brdP, chanP) *
                                conf.exsside.cfd_unit;

                        Fill(exssideHistograms.m_Energy_TS,
                             unsignedDiff<double, unsigned long long>(
                                 exssideReader.TSEXSSiDe_Hit->At(idP), input.firstTS) *
                                 conf.exsside.time_unit,
                             hitTmp.ESi);
                    }
                    else
                    {
                        hitTmp.Time =
                            unsignedDiff<double, unsigned long long>(
                                exssideReader.TSEXSSiDe_Hit->At(idN), minTS) *
                                conf.exsside.time_unit +
                            conf.exsside.getTime(exssideReader.EXSSiDeTime->At(idN), brdN, chanN) *
                                conf.exsside.cfd_unit;

                        Fill(exssideHistograms.m_Energy_TS,
                             unsignedDiff<double, unsigned long long>(
                                 exssideReader.TSEXSSiDe_Hit->At(idN), input.firstTS) *
                                 conf.exsside.time_unit,
                             hitTmp.ESi);
                        //    std::cout<<"Input firstTS "<<input.firstTS<<"
                        //    "<<exssideReader.TSEXSSiDe_Hit->At(idN)<<std::endl;
                    }
                    hitList.push_back(hitTmp);
                }
        }
    }

    if (settings.verbosity_level > 2)
    {
        std::cout << "hitList multipliticy = " << hitList.size() << std::endl;
        if (settings.verbosity_level > 10)
        {
            for (auto hit : hitList)
            {
                std::cout << "\nTime: " << hit.Time << "\n";
                std::cout << "IdPside: " << hit.IdPside << "\n";
                std::cout << "IdNside: " << hit.IdNside << "\n";
                std::cout << "Eraw Nside: " << hit.Eraw.first << "\n";
                std::cout << "Eraw Pside: " << hit.Eraw.second << "\n";
                std::cout << "ESi: " << hit.ESi << "\n";
                std::cout << "ESiNside: " << hit.ESiNside << "\n";
                std::cout << "ESiPside: " << hit.ESiPside << "\n";
                std::cout << "Pos: " << hit.Pos.X() << " " << hit.Pos.Y() << " " << hit.Pos.Z()
                          << "\n";
            }
            std::cout << std::endl;
        }
    }

    if (hitList.empty())
        return;

    // Fill correlation matrix
    // for(auto hit : hitList) {
    //    Fill(exssideHistograms.m_exssideBrdCh_exssideBrdCh_withcond, hit.IdPside,
    //         hit.IdNside);
    //    Fill(exssideHistograms.m_exssideBrdCh_exssideBrdCh_withcond, hit.IdNside,
    //         hit.IdPside);
    //}
    exsside.resize(hitList.size());

    exsside.TS = timeMinTS; // * (conf.exsside.time_unit / (10 * UNITS::ns));

    for (unsigned int i{0}; i < hitList.size(); i++)
    {
        exsside.IdPside = hitList[i].IdPside;
        exsside.IdNside = hitList[i].IdNside;
        exsside.Time[i] = hitList[i].Time;
        exsside.ESi     = hitList[i].ESi;
        exsside.Pos     = hitList[i].Pos;
        exsside.E       = {hitList[i].Eraw.first, hitList[i].Eraw.second};
        exsside.good    = hitList[i].goodev;

        if (settings.verbosity_level > 10)
            std::cout << "EXSSIDE good events " << exsside.Pos.x() << "\t" << exsside.Pos.y()
                      << "\t" << exsside.Pos.z() << std::endl;

        if (exssideDeadlayerELoss.first)
        {
            exsside.ESi = exssideDeadlayerELoss.second.EvaluateInitialEnergyWithDensity(
                exsside.ESi, conf.exsside.deadlayerThickness, exsside.Pos.Angle(TVector3(0, 0, 1)));
        }
        if (exssideAbsorberELoss.first)
        {
            exsside.ESi = exssideAbsorberELoss.second.EvaluateInitialEnergyWithDensity(
                exsside.ESi, conf.exsside.absorberThickness, exsside.Pos.Angle(TVector3(0, 0, 1)));
        }

        /*   const ReactionFragment::Ion& ion{conf.exsside.ion};
           auto reactionsIt = input.reactions.find(ion);

           if(reactionsIt == input.reactions.end())
               throw std::runtime_error("Ion in exsside not found in reactions of "
                                        "interest! Check the conf file\n");

       //    if(reactionsIt != input.reactions.end()) {
       //        auto beam = reactionsIt->second.GetReactionFragment(1);
       //        TVector3
       beta(dir.X()/sqrt(mcp.TOF.first),dir.Y()/sqrt(mcp.TOF.first),dir.Z()/sqrt(mcp.TOF.first));
       //        beam.Set_Beta(beta);
       //        beam.Set_Theta_Phi(dir.Theta(), dir.Phi());
       //        mcp.p1 = beam.Get_P();
       //        EvalInTarget(mcp.p1, ion);
       //    }

           reactionsIt->second.GetFixedFragment().Set_Ek(exsside.E[i]);
           reactionsIt->second.GetFixedFragment().Set_Dir(exsside.Pos[i]);

           TLorentzVector p = reactionsIt->second.GetFixedFragment().Get_P();
           EvalInTarget(p, ion);

           exsside.E[i] = p.E() - p.M();

           try {
               exsside.Ex[i]
                   = static_cast<double>(reactionsIt->second.Set_Ek_Theta_Phi(
                       exsside.E[i], p.Theta(), p.Phi()));
           } catch(const std::exception& e) { std::cerr << e.what(); }
           exsside.p3[i] = reactionsIt->second.GetFixedFragment()
                              .Get_P(); // Measured by exsside
           exsside.p4[i]
               = reactionsIt->second.GetFreeFragment().Get_P(); // Binary partner
           */
    }

    if (settings.verbosity_level > 10)
    {
        std::cout << "\nTS\t" << exsside.TS << "\t" << input.firstTS << "\n";
        std::cout << "Time\t" << exsside.Time.at(0) << "\n";
        std::cout << "E\t" << exsside.ESi << "\t" << exsside.E.first << " " << exsside.E.second
                  << "\n";
        std::cout << "Pos\t( " << exsside.Pos.X() / UNITS::mm << " " << exsside.Pos.Y() / UNITS::mm
                  << " " << exsside.Pos.Z() / UNITS::mm << " )\t";
        std::cout << "( " << exsside.Pos.Mag() / UNITS::mm << " "
                  << exsside.Pos.Theta() / UNITS::deg << " " << exsside.Pos.Phi() / UNITS::deg
                  << " )\n";
    }
}

bool EXSSiDeAnalysis::FillHistos()
{
    if (!DetectorAnalysis::FillHistos())
        return false;

    // Filling histograms
    EXSSiDeHistograms &h = exssideHistograms;

    // Fill(h.h_mult, static_cast<int>(exsside.E.size()));

    // Fill correlation matrix
    Fill(exssideHistograms.m_exssideBrdCh_exssideBrdCh_withcond, exsside.IdPside, exsside.IdNside);
    Fill(exssideHistograms.m_exssideBrdCh_exssideBrdCh_withcond, exsside.IdNside, exsside.IdPside);
    if (!exsside.Time.empty())
    {
        //    Fill(h.m_Energy_TS,
        //         unsignedDiff<double, unsigned long long>(exsside.Time.at(0),
        //         input.firstTS*conf.exsside.time_unit)/ conf.exsside.time_unit, exsside.ESi);
        //    std::cout<<"Time\t"<< exsside.Time.at(0) <<"\t"<<input.firstTS<<std::endl;
    }
    // Fill(h.m_Energy_Theta, exsside.p3.Theta() / UNITS::deg, exsside.E)
    Fill(h.m_layout_XY, -exsside.Pos.x(), exsside.Pos.y());
    Fill(h.m_layout_XZ, -exsside.Pos.x(), exsside.Pos.z());
    Fill(h.m_layout_YZ, exsside.Pos.y(), exsside.Pos.z());

    // Particle identification histograms
    // std::vector<ReactionFragment::Ion> reaction;
    // for(unsigned int e{0}; e < exsside.E.size(); ++e) {
    //    if(!exsside.identified.at(e)) { continue; }
    //
    //    ReactionFragment::Ion idIon{exsside.A.at(e), exsside.Z.at(e)};
    //    reaction.emplace_back(idIon);
    //}
    // std::sort(reaction.begin(), reaction.end());
    //
    // auto reactionHist = h.reactionHistos.find(reaction);
    // if(reactionHist != h.reactionHistos.end()) {
    //    for(unsigned int s{0}; s < exsside.E.size(); ++s) {
    //        Fill(reactionHist->second.m_Energy_Theta,
    //             exsside.p3[s].Theta() / UNITS::deg, exsside.E[s]);
    //    }
    //}

    return true;
}

void EXSSiDeAnalysis::Instantiate()
{
    DetectorAnalysis::Instantiate();
    TTreeReader &fReader{input.fReader};

    if (conf.detectorsPresent.EXSSiDe)
    {
        if (input.treeToRead->FindLeaf("nbEXSSiDe") == nullptr)
            throw std::runtime_error("EXSSiDe enabled in the config but not found in the tree");

        exssideReader.nbEXSSiDe     = new TTreeReaderValue<Int_t>(fReader, "nbEXSSiDe");
        exssideReader.TSEXSSiDe     = new TTreeReaderValue<ULong64_t>(fReader, "TSEXSSiDe");
        exssideReader.TSEXSSiDe_Hit = new TTreeReaderArray<ULong64_t>(fReader, "TSEXSSiDe_Hit");

        if (input.treeToRead->FindLeaf("EXSSiDeChannelId") != nullptr)
            exssideReader.EXSSiDeChannel =
                new TTreeReaderArray<UShort_t>(fReader, "EXSSiDeChannel.EXSSiDeChannelId");
        else
            exssideReader.EXSSiDeChannel =
                new TTreeReaderArray<UShort_t>(fReader, "EXSSiDeChannel");

        if (input.treeToRead->FindLeaf("EXSSiDeBoardId") != nullptr)
            exssideReader.EXSSiDeBoard =
                new TTreeReaderArray<UShort_t>(fReader, "EXSSiDeBoard.EXSSiDeBoardId");
        else
            exssideReader.EXSSiDeBoard = new TTreeReaderArray<UShort_t>(fReader, "EXSSiDeBoard");

        exssideReader.EXSSiDeEnergy = new TTreeReaderArray<Float_t>(fReader, "EXSSiDeEnergy");
        exssideReader.EXSSiDeTime   = new TTreeReaderArray<Float_t>(fReader, "EXSSiDeTime");
        exssideReader.EXSSiDeCfd    = new TTreeReaderArray<Float_t>(fReader, "EXSSiDeCfd");
    }
}

void EXSSiDeAnalysis::Finalize()
{
    DetectorAnalysis::Finalize();
    NormalizeHistos<DetectorConf>(conf.exsside);
}
