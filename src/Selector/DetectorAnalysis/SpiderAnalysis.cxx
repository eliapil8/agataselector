#include "SpiderAnalysis.h"

#include "Spider.h"

SpiderAnalysis::SpiderAnalysis(DetectorAnalysis::Inputs in, Spider &s)
    : DetectorAnalysis(in, s, ConfManager::getInstance().spider, spiderHistograms),
      ExTheta(conf.replay.conf_path + conf.spider.exthetaPath),
      ExPhi(conf.replay.conf_path + conf.spider.exphiPath),
      ExID(conf.replay.conf_path + conf.spider.exidPath), spider(s)
{
    if (detectorConf.enable_treeFill && input.type == Inputs::Type::SIGNAL)
        input.tree->Branch("Spider", &spider);
}

bool SpiderAnalysis::AllocateHistos()
{
    SpiderHistograms &h = spiderHistograms;
    h.dir               = input.outFile.mkdir("Spider", "Spider histograms");
    h.dir->cd();
    if (!DetectorAnalysis::AllocateHistos())
        return false;

    // Setup deadlayer energy loss
    std::string sTemp =
        conf.replay.conf_path + "/EnergyLoss/Target-" + conf.spider.deadlayer_material + "_Ion-" +
        std::to_string(conf.spider.ion.A) + "-" + std::to_string(conf.spider.ion.Z) + ".txt";
    if (settings.verbosity_level > 0)
    {
        std::cout << "Spider dead layer energy loss: " << sTemp
                  << " - is present: " << FileIsPresent(sTemp) << std::endl;
    }
    if (FileIsPresent(sTemp))
    {
        spiderDeadlayerELoss = {true, EnergyLoss(sTemp, "SRIM", 100)};
    }

    Initialize<TH1D>(
        h.h_mult, new TH1D("h_mult", "SPIDER multiplicity;Multiplicity;Counts", 10, 0, 10), h.ptrs);

    Initialize<TH2D>(h.m_Energy_TS,
                     new TH2D("m_Energy_TS", "SPIDER TimeStamp vs energy;Time [s];Energy [MeV]",
                              4320, 0, 12 * 3600, 2000, 0, 200),
                     h.ptrs);
    Initialize<TH2D>(h.m_Theta_Phi,
                     new TH2D("m_Theta_Phi", "SPIDER Theta vs Phi;Theta [deg];Phi [deg]", 50, 120,
                              170, 60, -180, 180),
                     h.ptrs);

    Initialize<TH2D>(
        h.m_layout_XY,
        new TH2D("m_layout_XY", "SPIDER X vs Y;X[mm];Y[mm]", 100, -100, 100, 100, -100, 100),
        h.ptrs);

    Initialize<TH2D>(
        h.m_layout_XZ,
        new TH2D("m_layout_XZ", "SPIDER X vs Z;X[mm];Z[mm]", 100, -100, 100, 100, -100, 100),
        h.ptrs);

    Initialize<TH2D>(
        h.m_layout_YZ,
        new TH2D("m_layout_YZ", "SPIDER Y vs Z;Y[mm];Z[mm]", 100, -100, 100, 100, -100, 100),
        h.ptrs);

    Initialize<TH2D>(h.m_Theta_E,
                     new TH2D("m_Theta_E", "SPIDER theta vs energy;Theta [deg]; Energy [MeV]", 50,
                              120, 170, conf.spider.Ebins, conf.spider.Emin / UNITS::MeV,
                              conf.spider.Emax / UNITS::MeV),
                     h.ptrs);

    Initialize<TH2D>(h.m_spiderID_E,
                     new TH2D("m_spiderID_Energy",
                              "SPIDER energy vs spiderID;SPIDER segment; Energy [MeV]",
                              // 5*Lut::MAXCH, 0, 5*Lut::MAXCH,
                              100, -0.5, 99.5, conf.spider.Ebins, conf.spider.Emin / UNITS::MeV,
                              conf.spider.Emax / UNITS::MeV),
                     h.ptrs);
    SetupLabelsForChannelID(conf.spider, h.m_spiderID_E);

    Initialize<TH2D>(h.m_spiderID_E_uncal,
                     new TH2D("m_spiderID_Energy_uncal",
                              "SPIDER energy vs spiderID not "
                              "calibrated;SPIDER segment; Energy channel;",
                              100, -0.5, 99.5, 2000, 0, 20000),
                     h.ptrs);
    SetupLabelsForChannelID(conf.spider, h.m_spiderID_E_uncal);

    Initialize<TH1D>(h.h_Ex,
                     new TH1D("h_Ex",
                              "SPIDER excitation energy;Excitation Energy [MeV];Counts/0.1 MeV",
                              conf.spider.Exbins, conf.spider.Exmin / UNITS::MeV,
                              conf.spider.Exmax / UNITS::MeV),
                     h.ptrs);

    Initialize<TH2D>(
        h.m_Theta_Ex,
        new TH2D("m_Theta_Ex",
                 "SPIDER excitation energy;Excitation Energy [MeV]; Theta[deg]; Counts/0.1 MeV", 50,
                 120, 170, conf.spider.Exbins, conf.spider.Exmin / UNITS::MeV,
                 conf.spider.Exmax / UNITS::MeV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_Phi_Ex,
        new TH2D("m_Phi_Ex", "SPIDER excitation energy; Phi [deg]; Excitation Energy [MeV]; Counts",
                 60, -180, 180, conf.spider.Exbins, conf.spider.Exmin / UNITS::MeV,
                 conf.spider.Exmax / UNITS::MeV),
        h.ptrs);

    Initialize<TH2D>(h.m_spiderID_Ex,
                     new TH2D("m_spiderID_Ex",
                              "SPIDER energy vs spiderID;SPIDER segment; Energy [MeV]", 100, -0.5,
                              99.5, conf.spider.Exbins, conf.spider.Exmin / UNITS::MeV,
                              conf.spider.Exmax / UNITS::MeV),
                     h.ptrs);
    SetupLabelsForChannelID(conf.spider, h.m_spiderID_Ex);

    return true;
}

void SpiderAnalysis::Analyze()
{
    DetectorAnalysis::Analyze();

    if (!spiderReader.nbSPIDER)
        return;

    std::vector<unsigned int> idx;
    double                    maxEnergy = 0.;

    for (unsigned int i{0}; i < spiderReader.SPIDEREnergy->GetSize(); ++i)
    {
        unsigned int brd  = spiderReader.SPIDERBoard->At(i);
        unsigned int chan = spiderReader.SPIDERChannel->At(i);

        // check if it's not a broken channel
        for (const auto &it : conf.spider.broken_channels)
        {
            if (static_cast<int>(brd) == it.first && static_cast<int>(chan) == it.second)
                continue;
        }

        double energy{conf.spider.getEnergy(spiderReader.SPIDEREnergy->At(i), brd, chan)};

        Fill(spiderHistograms.m_spiderID_E_uncal, Lut::getID(brd, chan),
             spiderReader.SPIDEREnergy->At(i));
        Fill(spiderHistograms.m_spiderID_E, Lut::getID(brd, chan), energy);

        const auto &thr = conf.spider.getThr(brd, chan);

        if (energy < thr.first || energy > thr.second)
            continue;

        // in this case, we only keep the highest energy event
        if (conf.spider.keepHighEneEvent)
        {
            if (idx.empty())
                idx.push_back(i);
            else if (energy > maxEnergy)
            {
                idx.at(0) = i;
                maxEnergy = energy;
            }
        }
        else
            idx.push_back(i);

    } // end cycle on spider raw events

    // if no event is good, we are done
    if (idx.empty())
        return;

    spider.resize(idx.size());

    // start preparing the analyzed event to push forward to analysis

    // 1) assign fake TS to event, then find the lowest TS among all valid events
    spider.TS = std::numeric_limits<unsigned long long>::max();

    auto minTS = std::numeric_limits<unsigned long long>::max();
    for (unsigned int i{0}; i < idx.size(); ++i)
    {
        auto ts = spiderReader.TSSPIDER_Hit->At(idx[i]);
        if (ts > 0 && ts < minTS)
        {
            spider.TS = ts;
            minTS     = ts;
        }
    }

    for (unsigned int i{0}; i < idx.size(); ++i)
    {
        unsigned int brd  = spiderReader.SPIDERBoard->At(idx[i]);
        unsigned int chan = spiderReader.SPIDERChannel->At(idx[i]);

        // 1) assign ID to event
        spider.Id[i] = Lut::getID(brd, chan);

        // 2) calculate time from TS and cfd
        spider.Time[i] = unsignedDiff<double, unsigned long long>(
                             spiderReader.TSSPIDER_Hit->At(idx[i]), spider.TS) *
                             conf.spider.time_unit +
                         conf.spider.getTime(
                             spiderReader.SPIDERTime->At(idx[i]) * conf.spider.cfd_unit, brd, chan);

        // 3) Kinematical reconstruction
        auto reactionsIt = input.reactions.find(conf.spider.ion);
        if (reactionsIt == input.reactions.end())
            throw std::runtime_error("Ion in spider not found in reactions of "
                                     "interest! Check the conf file\n");

        // 3.0) Save the energy value in backup variable ESi
        spider.ESi[i] = spider.E[i] =
            conf.spider.getEnergy(spiderReader.SPIDEREnergy->At(idx[i]), brd, chan);

        // 3.1) correct by energy loss in dead layer of spider (unused, mostly)
        if (spiderDeadlayerELoss.first)
        {
            spider.E[i] = spiderDeadlayerELoss.second.EvaluateInitialEnergyWithDensity(
                spider.E[i], conf.spider.deadlayer_thickness, 0);
        }

        // 3.2) correct by the reaction
        reactionsIt->second.GetFixedFragment().Set_Ek(spider.E[i]);
        reactionsIt->second.GetFixedFragment().Set_Dir(conf.spider.getPos(brd, chan));

        TLorentzVector p = reactionsIt->second.GetFixedFragment().Get_P();
        EvalInTarget(p, conf.spider.ion);
        spider.E[i] = p.E() - p.M();

        // 3.3) calculate Ex; since this is dangerous, we put it in a try-catch statement
        try
        {
            spider.Ex[i] = static_cast<double>(
                reactionsIt->second.Set_Ek_Theta_Phi(spider.E[i], p.Theta(), p.Phi()));
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what();
            spider.Ex[i] = 0.;
        }

        // 4) correct for aberrations in the Ex
        // 4.1) correct with a cosine function depending on phi
        if (abs(conf.spider.cosAmpli) > 0.)
            spider.Ex[i] -= conf.spider.cosAmpli *
                            TMath::Cos(p.Phi() / conf.spider.cosPeriod / TMath::DegToRad() +
                                       conf.spider.cosPhase * TMath::DegToRad());

        // 4.2) correct a la prisma; maybe it's better to either correct on theta and phi,
        //      or on id, but what do i know
        //      also to cross check whether it's better a rescaling of a shift

        auto exIDEval  = ExID.Eval({0}, spider.Id[i]);
        auto exIDEval0 = ExID.Eval({0}, 33);
        if (exIDEval.second[0] != -1)
            spider.Ex[i] = spider.Ex[i] + (exIDEval0.first - exIDEval.first);

        auto exThetaEval  = ExTheta.Eval({0}, p.Theta() / UNITS::deg);
        auto exThetaEval0 = ExTheta.Eval({0}, 130.);
        if (exThetaEval.second[0] != -1)
            spider.Ex[i] = spider.Ex[i] + (exThetaEval0.first - exThetaEval.first);

        auto exPhiEval  = ExPhi.Eval({0}, p.Phi());
        auto exPhiEval0 = ExPhi.Eval({0}, 150.);
        if (exPhiEval.second[0] != -1)
            spider.Ex[i] = spider.Ex[i] + (exPhiEval0.first - exPhiEval.first);

        // 4.3) recalibrate with quadratic recalibration
        spider.Ex[i] = conf.spider.exShift + conf.spider.exGain * spider.Ex[i] +
                       conf.spider.exQuad * spider.Ex[i] * spider.Ex[i];

        // 5) assign momenta for the doppler correction
        spider.p3[i] = reactionsIt->second.GetFixedFragment().Get_P(); // Measured by spider
        spider.p4[i] = reactionsIt->second.GetFreeFragment().Get_P();  // Binary partner

        // 6) assign new Id, if present
        for (auto ch : conf.spider.aliasId)
            if (ch.first == spider.Id[i])
            {
                spider.Id[i] = ch.second;
                break;
            }
    }
}

bool SpiderAnalysis::FillHistos()
{
    if (!DetectorAnalysis::FillHistos())
        return false;

    // Filling histograms
    SpiderHistograms &h = spiderHistograms;

    Fill(h.h_mult, static_cast<int>(spider.E.size()));
    for (unsigned int i{0}; i < spider.E.size(); ++i)
    {

        Fill(h.m_Energy_TS,
             unsignedDiff<double, unsigned long long>(spider.TS, input.firstTS) / 1.e8,
             spider.ESi[i]);

        Fill(h.m_Theta_Phi, spider.p3[i].Theta() / UNITS::deg, spider.p3[i].Phi() / UNITS::deg);

        Fill(h.m_Theta_E, spider.p3[i].Theta() / UNITS::deg, spider.E[i]);

        TVector3 pos = spider.p3[i].Vect();
        pos.SetMag(90.);

        Fill(h.m_layout_XY, pos.x(), pos.y());
        Fill(h.m_layout_XZ, pos.x(), pos.z());
        Fill(h.m_layout_YZ, pos.y(), pos.z());

        Fill(h.h_Ex, spider.Ex[i]);

        Fill(h.m_Theta_Ex, spider.p3[i].Theta() / UNITS::deg, spider.Ex[i]);
        Fill(h.m_Phi_Ex, spider.p3[i].Phi() / UNITS::deg, spider.Ex[i]);

        Fill(h.m_spiderID_Ex, spider.Id[i], spider.Ex[i]);
    }
    return true;
}

void SpiderAnalysis::Instantiate()
{
    DetectorAnalysis::Instantiate();
    TTreeReader &fReader{input.fReader};

    if (conf.detectorsPresent.Spider)
    {
        if (input.treeToRead->FindLeaf("nbSPIDER") == nullptr)
            throw std::runtime_error("SPIDER enabled in the config but not found in the tree");

        spiderReader.nbSPIDER     = new TTreeReaderValue<Int_t>(fReader, "nbSPIDER");
        spiderReader.TSSPIDER     = new TTreeReaderValue<ULong64_t>(fReader, "TSSPIDER");
        spiderReader.TSSPIDER_Hit = new TTreeReaderArray<ULong64_t>(fReader, "TSSPIDER_Hit");

        if (input.treeToRead->FindLeaf("SPIDERChannelId") != nullptr)
            spiderReader.SPIDERChannel =
                new TTreeReaderArray<UShort_t>(fReader, "SPIDERChannel.SPIDERChannelId");
        else
            spiderReader.SPIDERChannel = new TTreeReaderArray<UShort_t>(fReader, "SPIDERChannel");

        if (input.treeToRead->FindLeaf("SPIDERBoardId") != nullptr)
            spiderReader.SPIDERBoard =
                new TTreeReaderArray<UShort_t>(fReader, "SPIDERBoard.SPIDERBoardId");
        else
            spiderReader.SPIDERBoard = new TTreeReaderArray<UShort_t>(fReader, "SPIDERBoard");

        spiderReader.SPIDEREnergy = new TTreeReaderArray<Float_t>(fReader, "SPIDEREnergy");
        spiderReader.SPIDERTime   = new TTreeReaderArray<Float_t>(fReader, "SPIDERTime");
    }
}

void SpiderAnalysis::Finalize()
{
    DetectorAnalysis::Finalize();
    NormalizeHistos<DetectorConf>(conf.spider);
}
