#include "EuclidesAnalysis.h"

#include "Euclides.h"

EuclidesAnalysis::EuclidesAnalysis(DetectorAnalysis::Inputs in, Euclides &e)
    : DetectorAnalysis(in, e, ConfManager::getInstance().euclides, euclidesHistograms), euclides(e)
{
    if (detectorConf.enable_treeFill && input.type == Inputs::Type::SIGNAL)
        input.tree->Branch("Euclides", &euclides);
}

bool EuclidesAnalysis::AllocateHistos()
{
    EuclidesHistograms &h = euclidesHistograms;
    h.dir                 = input.outFile.mkdir("Euclides", "Euclides histograms");
    h.dir->cd();
    if (!DetectorAnalysis::AllocateHistos())
        return false;

    Initialize<TH1D>(h.h_mult,
                     new TH1D("h_mult", "EUCLIDES multiplicity;Multiplicity;Counts", 10, 0, 10),
                     h.ptrs);

    Initialize<TH1D>(h.h_mon, new TH1D("h_mon", "monitor;Energy [ch];Counts", 400, 0, 16000),
                     h.ptrs);

    Initialize<TH2D>(h.m_EuclidesID_timediff_dE,
                     new TH2D("m_EuclidesID_timediff_dE",
                              "Euclides dE vs Time difference of any other; "
                              "Euclides ID; TS difference [10 ns]",
                              6 * Lut::MAXCH, 0, 6 * Lut::MAXCH, 200, -50, 150),
                     h.ptrs);
    SetupLabelsForChannelID(conf.euclides, h.m_EuclidesID_timediff_dE);

    Initialize<TH2D>(h.m_EuclidesID_timediff_E,
                     new TH2D("m_EuclidesID_timediff_E",
                              "Euclides dE vs Time difference of any other; "
                              "Euclides ID; TS difference [10 ns]",
                              6 * Lut::MAXCH, 0, 6 * Lut::MAXCH, 300, -150, 150),
                     h.ptrs);
    SetupLabelsForChannelID(conf.euclides, h.m_EuclidesID_timediff_E);

    Initialize<TH2D>(h.m_Energy_TS,
                     new TH2D("m_Energy_TS", "EUCLIDES TimeStamp vs energy;Time [s];Energy [MeV]",
                              /*"EUCLIDES TimeStamp vs energy;Time [s];Energy [ch]",*/ 4320, 0,
                              12 * 3600, 2000, 0, 40),
                     h.ptrs);

    Initialize<TH2D>(h.m_Theta_Phi,
                     new TH2D("m_Theta_Phi", "EUCLIDES Theta vs Phi;Theta [deg];Phi [deg]", 60, 10,
                              130, 180, -180, 180),
                     h.ptrs);

    Initialize<TH2D>(
        h.m_layout_XY,
        new TH2D("m_layout_XY", "EUCLIDES X vs Y;X[mm];Y[mm]", 100, -100, 100, 100, -100, 100),
        h.ptrs);

    Initialize<TH2D>(
        h.m_layout_XZ,
        new TH2D("m_layout_XZ", "EUCLIDES X vs Z;X[mm];Z[mm]", 100, -100, 100, 100, -100, 100),
        h.ptrs);

    Initialize<TH2D>(
        h.m_layout_YZ,
        new TH2D("m_layout_YZ", "EUCLIDES Y vs Z;Y[mm];Z[mm]", 100, -100, 100, 100, -100, 100),
        h.ptrs);

    Initialize<TH2D>(
        h.m_Energy_Theta,
        new TH2D("m_Energy_Theta", "EUCLIDES theta vs energy;Theta [deg]; Energy [MeV]",
                 /*"EUCLIDES theta vs energy;Theta [deg]; Energy [ch]",*/ 60, 10, 130, 2000, 0, 40),
        h.ptrs);

    Initialize<TH2D>(h.m_Energy_euclidesID,
                     new TH2D("m_Energy_ID",
                              "EUCLIDES energy vs EuclidesID;EUCLIDES segment; Energy [MeV]",
                              /*"EUCLIDES energy vs detector;EUCLIDES segment; Energy [ch]",*/
                              6 * Lut::MAXCH, 0, 6 * Lut::MAXCH, 2000, 0, 40),
                     h.ptrs);

    SetupLabelsForChannelID(conf.euclides, h.m_Energy_euclidesID);

    // we need good calibration to change the ch to MeV
    Initialize<TH2D>(h.m_E_dE,
                     new TH2D("m_E_dE", "EUCLIDES E vs dE;E_Energy [MeV];dE_Energy [MeV]", 1000, 0,
                              40, 2000, 0, 40),
                     h.ptrs);

    Initialize<TH1D>(h.h_Ex,
                     new TH1D("h_Ex",
                              "EUCLIDES excitation energy;Excitation Energy "
                              "[MeV];Counts/0.1 MeV",
                              1000, -10, 90),
                     h.ptrs);

    // this should be removed when calibration is done in proper way
    h.ede.dir = h.dir->mkdir("E_dE", "E_dE_matrix");
    h.ede.dir->cd();
    for (const auto &it : conf.euclides.getListOfNames_dE())
    {
        Initialize<TH2D>(
            h.ede.m_E_dE[it.first],
            new TH2D(Form("m_E_dE_%s", it.second.c_str()),
                     Form("EUCLIDES_%s E vs dE;E_Energy [MeV];dE_Energy [MeV]", it.second.c_str()),
                     1000, 0, 40, 1000, 0, 40),
            h.ede.ptrs);
        // cms change 08/02/2024
        SetupLabelsForChannelID(conf.euclides, h.ede.m_E_dE[it.first]);
    }
    return true;
}

void EuclidesAnalysis::Analyze()
{
    DetectorAnalysis::Analyze();

    std::vector<unsigned int> idxdE;
    std::vector<unsigned int> idxE;

    // if (!euclidesReader.nbEUCLIDES || !euclidesReader.nbEUCLIDES->IsValid()) return;
    if (!euclidesReader.nbEUCLIDES)
        return;

    for (unsigned int i{0}; i < euclidesReader.EUCLIDESEnergy->GetSize(); ++i)
    {
        unsigned int brd  = euclidesReader.EUCLIDESBoard->At(i);
        unsigned int chan = euclidesReader.EUCLIDESChannel->At(i);
        double       energy{0};

        energy          = conf.euclides.getEnergy(euclidesReader.EUCLIDESEnergy->At(i),
                                                  euclidesReader.EUCLIDESBoard->At(i),
                                                  euclidesReader.EUCLIDESChannel->At(i));
        const auto &thr = conf.euclides.getThr(euclidesReader.EUCLIDESBoard->At(i),
                                               euclidesReader.EUCLIDESChannel->At(i));
        if (energy > thr.first && energy < thr.second)
        {
            // Only consider dE events
            if (conf.euclides.isE(brd, chan))
            {
                idxE.push_back(i);
            }
            else if (conf.euclides.isdE(brd, chan))
            {
                idxdE.push_back(i);
            }
        }
        Fill(euclidesHistograms.m_Energy_euclidesID, Lut::getID(brd, chan), energy);
    }
    if (idxdE.empty())
        return;

    // Euclides time gate
    {
        auto minTSidx =
            std::min_element(idxdE.begin(), idxdE.end(), [&](const auto &a, const auto &b) {
                return euclidesReader.TSEUCLIDES_Hit->At(a) < euclidesReader.TSEUCLIDES_Hit->At(b);
            });
        euclides.TS    = euclidesReader.TSEUCLIDES_Hit->At(*minTSidx);
        auto timeMinTS = conf.euclides.getTime(euclidesReader.EUCLIDESTime->At(*minTSidx) *
                                                   conf.euclides.cfd_unit,
                                               euclidesReader.EUCLIDESBoard->At(*minTSidx),
                                               euclidesReader.EUCLIDESChannel->At(*minTSidx));

        // Euclides time gate
        // dE layer
        for (auto it{idxdE.begin()}; it != idxdE.end();)
        {
            unsigned int brd  = euclidesReader.EUCLIDESBoard->At(*it);
            unsigned int chan = euclidesReader.EUCLIDESChannel->At(*it);

            auto time =
                unsignedDiff<double, unsigned long long>(euclidesReader.TSEUCLIDES_Hit->At(*it),
                                                         euclides.TS) *
                    conf.euclides.time_unit +
                conf.euclides.getTime(euclidesReader.EUCLIDESTime->At(*it) * conf.euclides.cfd_unit,
                                      brd, chan) -
                timeMinTS;

            // cms change 08/02/2024 - get EUCLIDES ID from LUT instead of conf data structure
            // Fill(euclidesHistograms.m_EuclidesID_timediff_dE,
            //      conf.euclides.getName(brd, chan).c_str(),
            //      time / (10 * UNITS::ns));

            Fill(euclidesHistograms.m_EuclidesID_timediff_dE, Lut::getID(brd, chan),
                 time / (10 * UNITS::ns));

            if (time / (10 * UNITS::ns) >= windowLeft && time / (10 * UNITS::ns) < windowRight)
            {
                it++;
            }
            else
            {
                it = idxdE.erase(it);
            }
        }
        if (idxdE.empty())
            return;
        // E layer
        for (auto it{idxE.begin()}; it != idxE.end();)
        {
            unsigned int brd  = euclidesReader.EUCLIDESBoard->At(*it);
            unsigned int chan = euclidesReader.EUCLIDESChannel->At(*it);

            auto time =
                unsignedDiff<double, unsigned long long>(euclidesReader.TSEUCLIDES_Hit->At(*it),
                                                         euclides.TS) *
                    conf.euclides.time_unit +
                conf.euclides.getTime(euclidesReader.EUCLIDESTime->At(*it) * conf.euclides.cfd_unit,
                                      brd, chan) -
                timeMinTS;

            // cms change 08/02/2024 - get EUCLIDES ID from LUT instead of conf data structure
            Fill(euclidesHistograms.m_EuclidesID_timediff_E, Lut::getID(brd, chan),
                 time / (10 * UNITS::ns));

            if (time / (10 * UNITS::ns) >= windowLeft && time / (10 * UNITS::ns) < windowRight)
            {
                it++;
            }
            else
            {
                it = idxE.erase(it);
            }
        }
    }

    euclides.resize(idxdE.size());

    for (unsigned int i{0}; i < idxdE.size(); ++i)
    {
        unsigned int brd  = euclidesReader.EUCLIDESBoard->At(idxdE[i]);
        unsigned int chan = euclidesReader.EUCLIDESChannel->At(idxdE[i]);
        euclides.Id[i]    = conf.euclides.getMapName(brd, chan);
        euclides.dESi[i] =
            conf.euclides.getEnergy(euclidesReader.EUCLIDESEnergy->At(idxdE[i]), brd, chan);
        euclides.ESi[i] = euclides.dESi[i];

        // Find the E detector associated to this dE
        unsigned int idxofEofInterest{euclides.Id[i] % 1000 + 1000};

        for (unsigned int j{0}; j < idxE.size(); ++j)
        {
            unsigned int brdTmp  = euclidesReader.EUCLIDESBoard->At(idxE[j]);
            unsigned int chanTmp = euclidesReader.EUCLIDESChannel->At(idxE[j]);

            if (conf.euclides.getMapName(brdTmp, chanTmp) == idxofEofInterest)
            {
                euclides.ESi[i] =
                    euclides.dESi[i] +
                    conf.euclides.getEnergy(euclidesReader.EUCLIDESEnergy->At(idxE[j]),
                                            euclidesReader.EUCLIDESBoard->At(idxE[j]),
                                            euclidesReader.EUCLIDESChannel->At(idxE[j]));
                break;
            }
        }

        euclides.Time[i] =
            unsignedDiff<double, unsigned long long>(euclidesReader.TSEUCLIDES_Hit->At(idxdE[i]),
                                                     euclides.TS) *
                conf.euclides.time_unit +
            conf.euclides.getTime(
                euclidesReader.EUCLIDESTime->At(idxdE[i]) * conf.euclides.cfd_unit, brd, chan);

        euclides.pos[i] = conf.euclides.getPos(brd, chan);

        // if(brd == 5 && chan == 0) // TODO: this is experiment dependent
        //     Fill(euclidesHistograms.h_mon,
        //          euclidesReader.EUCLIDESEnergy->At(idxdE[i]));

        // if(euclides.Time[i] > 0
        //    && euclides.Time[i] < euclides.TS) {
        //     euclides.TS = euclides.Time[i];
        // }

        ReactionFragment::Ion detectedIon{conf.euclides.idParticle(
            euclides.ESi.at(i), euclides.dESi.at(i), euclidesReader.EUCLIDESBoard->At(idxdE.at(i)),
            euclidesReader.EUCLIDESChannel->At(idxdE.at(i)))};
        euclides.A[i] = detectedIon.A;
        euclides.Z[i] = detectedIon.Z;

        if (euclides.A[i] != 0)
            euclides.identified[i] = true;

        if (euclides.identified[i])
        {
            auto reactionsIt = input.reactions.find(detectedIon);
            if (reactionsIt == input.reactions.end())
                throw std::runtime_error("Ion in euclides not found in reactions of interest! "
                                         "Check the conf file\n");

            reactionsIt->second.GetFixedFragment().Set_Ek(euclides.ESi.at(i));
            reactionsIt->second.GetFixedFragment().Set_Dir(euclides.pos.at(i));
            euclides.pFragments[i] = reactionsIt->second.GetFixedFragment().Get_P();
            EvalInTarget(euclides.pFragments[i], detectedIon);
            euclides.E[i] = euclides.pFragments[i].E() - euclides.pFragments[i].M();
        }
    }

    if (settings.verbosity_level > 2)
        std::cout << "Calculating reactions" << std::endl;
    unsigned int multiplicity =
        std::count(euclides.identified.begin(), euclides.identified.end(), true);
    if (multiplicity == 1)
    {
        if (settings.verbosity_level > 2)
            std::cout << "Multiplicity 1" << std::endl;
        if (settings.verbosity_level == -10)
            std::cout << "Multiplicity 1" << std::endl;
        // Multiplicity 1 in euclides, assuming binary reaction

        auto it = std::find(euclides.identified.begin(), euclides.identified.end(), true);
        if (it != euclides.identified.end())
        {
            long                  i{it - euclides.identified.begin()};
            ReactionFragment::Ion detectedIon{euclides.A[i], euclides.Z[i]};

            auto reactionsIt = input.reactions.find(detectedIon);
            if (reactionsIt != input.reactions.end())
            {
                if (settings.verbosity_level == -10)
                {
                    std::cout << "Euclides energy: " << euclides.E.at(i) / UNITS::MeV
                              << " Theta: " << euclides.pFragments[i].Theta() / UNITS::deg;
                }
                try
                {
                    euclides.Ex = static_cast<double>(reactionsIt->second.Set_Ek_Theta_Phi(
                        euclides.E.at(i), euclides.pFragments[i].Theta(),
                        euclides.pFragments[i].Phi()));
                    if (settings.verbosity_level == -10)
                    {
                        std::cout << " Ex: " << euclides.Ex / UNITS::MeV << std::endl;
                    }
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what();
                }
                euclides.p3 = reactionsIt->second.GetFreeFragment().Get_P(); // Measured by euclides
            }
        }
    }
    else if (multiplicity > 1)
    {
        // Multiplicity more than one, N body reaction
        if (settings.verbosity_level > 2)
            std::cout << "Multiplicity >1" << std::endl;
        std::vector<ReactionFragment::Ion> fragments{};
        std::vector<long double>           Ek;
        std::vector<long double>           Theta;
        std::vector<long double>           Phi;
        for (unsigned int i{0}; i < euclides.identified.size(); ++i)
        {
            if (euclides.identified[i])
            {
                fragments.emplace_back(euclides.A[i], euclides.Z[i]);
                Ek.emplace_back(euclides.E[i]);
                Theta.emplace_back(euclides.pFragments[i].Theta());
                Phi.emplace_back(euclides.pFragments[i].Phi());
            }
        }
        std::sort(fragments.begin(), fragments.end());
        auto reactionsIt = input.nBodyReactions.find(fragments);
        if (reactionsIt != input.nBodyReactions.end())
        {
            euclides.Ex = static_cast<double>(reactionsIt->second.Set_Ek_Theta_Phi(Ek, Theta, Phi));
            euclides.p3 = reactionsIt->second.Get_p3().Get_P();
        }
    }
}

bool EuclidesAnalysis::FillHistos()
{
    if (!DetectorAnalysis::FillHistos())
        return false;

    // Filling histograms
    EuclidesHistograms &h = euclidesHistograms;

    if (settings.verbosity_level > 2)
        std::cout << "Filling Euclides output" << std::endl;

    if (settings.verbosity_level > 2)
        std::cout << "Filling histograms" << std::endl;
    Fill(h.h_mult, static_cast<int>(euclides.E.size()));
    for (unsigned int i{0}; i < euclides.E.size(); ++i)
    {
        Fill(h.m_E_dE, euclides.ESi[i] / UNITS::MeV, euclides.dESi[i] / UNITS::MeV);
        Fill(h.ede.m_E_dE.at(euclides.Id.at(i)), euclides.ESi[i] / UNITS::MeV,
             euclides.dESi[i] / UNITS::MeV);

        Fill(h.m_Energy_TS,
             unsignedDiff<double, unsigned long long>(euclides.TS, input.firstTS) / 1.e8,
             euclides.E[i]);

        Fill(h.m_Theta_Phi, euclides.pos[i].Theta() / UNITS::deg,
             euclides.pos[i].Phi() / UNITS::deg);

        Fill(h.m_Energy_Theta, euclides.pos[i].Theta() / UNITS::deg, euclides.E[i]);

        TVector3 &pos = euclides.pos[i];

        Fill(h.m_layout_XY, pos.x(), pos.y());

        Fill(h.m_layout_XZ, pos.x(), pos.z());

        Fill(h.m_layout_YZ, pos.y(), pos.z());
    }
    Fill(h.h_Ex, euclides.Ex);

    if (settings.verbosity_level > 2)
        std::cout << "Finished euclides analysis" << std::endl;

    return true;
}

void EuclidesAnalysis::Instantiate()
{
    DetectorAnalysis::Instantiate();
    TTreeReader &fReader{input.fReader};

    if (conf.detectorsPresent.Euclides)
    {
        if (input.treeToRead->FindLeaf("nbEUCLIDES") == nullptr)
            throw std::runtime_error("EUCLIDES enabled in the config but not found in the tree");

        euclidesReader.nbEUCLIDES     = new TTreeReaderValue<Int_t>(fReader, "nbEUCLIDES");
        euclidesReader.TSEUCLIDES     = new TTreeReaderValue<ULong64_t>(fReader, "TSEUCLIDES");
        euclidesReader.TSEUCLIDES_Hit = new TTreeReaderArray<ULong64_t>(fReader, "TSEUCLIDES_Hit");

        if (input.treeToRead->FindLeaf("EUCLIDESChannelId") != nullptr)
            euclidesReader.EUCLIDESChannel =
                new TTreeReaderArray<UShort_t>(fReader, "EUCLIDESChannel.EUCLIDESChannelId");
        else
            euclidesReader.EUCLIDESChannel =
                new TTreeReaderArray<UShort_t>(fReader, "EUCLIDESChannel");

        if (input.treeToRead->FindLeaf("EUCLIDESBoardId") != nullptr)
            euclidesReader.EUCLIDESBoard =
                new TTreeReaderArray<UShort_t>(fReader, "EUCLIDESBoard.EUCLIDESBoardId");
        else
            euclidesReader.EUCLIDESBoard = new TTreeReaderArray<UShort_t>(fReader, "EUCLIDESBoard");

        euclidesReader.EUCLIDESEnergy = new TTreeReaderArray<Float_t>(fReader, "EUCLIDESEnergy");
        euclidesReader.EUCLIDESTime   = new TTreeReaderArray<Float_t>(fReader, "EUCLIDESTime");
    }
}

void EuclidesAnalysis::Finalize()
{
    DetectorAnalysis::Finalize();
    NormalizeHistos<DetectorConf>(conf.euclides);
}
