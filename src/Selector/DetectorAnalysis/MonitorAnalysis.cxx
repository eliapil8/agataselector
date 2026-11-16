#include "MonitorAnalysis.h"

#include "Monitor.h"

MonitorAnalysis::MonitorAnalysis(DetectorAnalysis::Inputs in, Monitor &s)
    : DetectorAnalysis(in, s, ConfManager::getInstance().monitor, monitorHistograms), monitor(s)
{
    if (detectorConf.enable_treeFill && input.type == Inputs::Type::SIGNAL)
        input.tree->Branch("Monitor", &monitor);
}

bool MonitorAnalysis::AllocateHistos()
{
    MonitorHistograms &h = monitorHistograms;
    h.dir                = input.outFile.mkdir("Monitor", "Monitor histograms");
    h.dir->cd();
    if (!DetectorAnalysis::AllocateHistos())
        return false;

    // Setup deadlayer energy loss
    std::string sTemp =
        conf.replay.conf_path + "/EnergyLoss/Target-" + conf.monitor.deadlayer_material + "_Ion-" +
        std::to_string(conf.monitor.ion.A) + "-" + std::to_string(conf.monitor.ion.Z) + ".txt";
    if (settings.verbosity_level > 0)
    {
        std::cout << "Monitor dead layer energy loss: " << sTemp
                  << " - is present: " << FileIsPresent(sTemp) << std::endl;
    }
    if (FileIsPresent(sTemp))
    {
        monitorDeadlayerELoss = {true, EnergyLoss(sTemp, "SRIM", 100)};
    }

    Initialize<TH2D>(h.m_Energy_TS,
                     new TH2D("m_Energy_TS", "MONITOR TimeStamp vs energy;Time [s];Energy [MeV]",
                              4320, 0, 12 * 3600, 2000, 0, 300),
                     h.ptrs);

    Initialize<TH2D>(
        h.m_Energy_monitorID,
        new TH2D(
            "m_Energy_monitorID", "MONITOR energy vs monitorID;MONITOR segment; Energy [MeV]", 20,
            0, 20,
            static_cast<int>(conf.monitor.monitorEmax / UNITS::MeV / conf.monitor.monitorbinWidth) +
                static_cast<int>(10 / conf.monitor.monitorbinWidth),
            0, conf.monitor.monitorEmax / UNITS::MeV),
        h.ptrs);
    // SetupLabelsForChannelID(conf.monitor, h.m_Energy_monitorID);

    Initialize<TH2D>(h.m_Energy_monitorID_uncal,
                     new TH2D("m_Energy_monitorID_uncal",
                              "MONITOR energy vs monitorID not "
                              "calibrated;MONITOR segment; Energy channel;",
                              20, 0, 20, 4000, 0, 12000),
                     h.ptrs);
    // SetupLabelsForChannelID(conf.monitor, h.m_Energy_monitorID_uncal);

    Initialize<TH1D>(h.h_E,
                     new TH1D("h_E", "Monitor energy",
                              static_cast<int>(conf.monitor.monitorEmax / UNITS::MeV /
                                               conf.monitor.monitorbinWidth) +
                                  static_cast<int>(10 / conf.monitor.monitorbinWidth),
                              0, conf.monitor.monitorEmax / UNITS::MeV),
                     h.ptrs);

    return true;
}

void MonitorAnalysis::Analyze()
{
    DetectorAnalysis::Analyze();

    if (settings.verbosity_level > 2)
    {
        std::cout << "\n------------------------------------------------------"
                     "-------------------------------\n";
        std::cout << "NEW Monitor EVENT\n " << std::endl;
    }

    std::vector<unsigned int> idx;
    double                    maxEnergy{0};
    // Fill raw histograms before any condition
    for (unsigned int i{0}; i < monitorReader.MONITOREnergy->GetSize(); ++i)
    {

        unsigned int brd  = monitorReader.MONITORBoard->At(i);
        unsigned int chan = monitorReader.MONITORChannel->At(i);

        double energy{conf.monitor.getEnergy(monitorReader.MONITOREnergy->At(i),
                                             monitorReader.MONITORBoard->At(i),
                                             monitorReader.MONITORChannel->At(i))};

        const auto &thr = conf.monitor.getThr(monitorReader.MONITORBoard->At(i),
                                              monitorReader.MONITORChannel->At(i));

        std::string chanName = conf.monitor.getName(brd, chan);
        std::transform(chanName.begin(), chanName.end(), chanName.begin(), ::toupper);

        // this define the detector number
        // returns the first number in the string M1
        unsigned int monNr;
        monNr = static_cast<unsigned int>(std::stoi(chanName.substr(
            chanName.find_first_not_of('M'), chanName.find_first_not_of("M") - chanName.size())));
        Fill(monitorHistograms.m_Energy_monitorID, monNr, energy);
        Fill(monitorHistograms.m_Energy_monitorID_uncal, monNr, monitorReader.MONITOREnergy->At(i));
        Fill(monitorHistograms.h_E, energy);

        if (settings.verbosity_level > 2)
            std::cout << "Monitor " << chanName << " ID " << monNr << std::endl;
        // send event into analysis if the energy
        // is in threshold
        if (energy > thr.first && energy < thr.second)
        {
            if (idx.empty())
            {
                idx.push_back(i);
            }
            else
            {
                if (energy > maxEnergy)
                {
                    idx.at(0) = i;
                    maxEnergy = energy;
                }
            }
        }
    }

    if (settings.verbosity_level > 2)
    {
        std::cout << "Monitor E size " << monitor.ESi.size() << std::endl;
    }

    monitor.resize(idx.size());

    // Build monitor events with the right conditions
    if (!monitor.ESi.empty())
        monitor.TS = std::numeric_limits<unsigned long long>::max();

    // Find the lowest ts of monitor
    for (unsigned int i{0}; i < idx.size(); ++i)
    {
        auto ts = monitorReader.TSMONITOR_Hit->At(i) +
                  conf.monitor.getTime(monitorReader.MONITORTime->At(i),
                                       monitorReader.MONITORBoard->At(i),
                                       monitorReader.MONITORChannel->At(i));
        if (ts > 0 && ts < monitor.TS)
        {
            monitor.TS = ts;
        }
    }

    for (unsigned int i{0}; i < idx.size(); ++i)
    {
        unsigned int brd  = monitorReader.MONITORBoard->At(idx[i]);
        unsigned int chan = monitorReader.MONITORChannel->At(idx[i]);

        std::string chanName = conf.monitor.getName(brd, chan);
        std::transform(chanName.begin(), chanName.end(), chanName.begin(), ::toupper);

        // this define the detector number
        // returns the first number in the string M1

        unsigned int monNr;
        monNr = static_cast<unsigned int>(std::stoi(chanName.substr(
            chanName.find_first_not_of('M'), chanName.find_first_not_of("M") - chanName.size())));
        if (settings.verbosity_level > 2)
            std::cout << "Monitor " << chanName << " ID " << monNr << std::endl;

        monitor.Id[i]   = monNr;
        monitor.Time[i] = monitorReader.TSMONITOR_Hit->At(idx[i]) * conf.monitor.time_unit +
                          conf.monitor.getTime(monitorReader.MONITORTime->At(idx[i]), brd, chan) *
                              conf.monitor.time_unit;
        // std::cout<<"Monitor Time "<<monitor.Time[i]<<" mon TS "<<
        // monitorReader.TSMONITOR_Hit->At(idx[i])
        //         * conf.monitor.time_unit<<std::endl;

        auto reactionsIt = input.reactions.find(conf.monitor.ion);
        if (reactionsIt == input.reactions.end())
            throw std::runtime_error("Ion in monitor not found in reactions of "
                                     "interest! Check the conf file\n");

        monitor.ESi[i] = monitor.E[i] =
            conf.monitor.eShift +
            conf.monitor.eGain *
                conf.monitor.getEnergy(monitorReader.MONITOREnergy->At(idx[i]), brd, chan);

        if (monitorDeadlayerELoss.first)
        {
            monitor.E[i] = monitorDeadlayerELoss.second.EvaluateInitialEnergyWithDensity(
                monitor.E[i], conf.monitor.deadlayer_thickness, 0);
        }
        // reactionsIt->second.GetFixedFragment().Set_Ek(monitor.E[i]);
        // reactionsIt->second.GetFixedFragment().Set_Dir(
        //     conf.monitor.getPos(brd, chan));
        //
        // TLorentzVector p = reactionsIt->second.GetFixedFragment().Get_P();
        // EvalInTarget(p, conf.monitor.ion);
        // monitor.ESi[i] = p.E() - p.M();
        //
        //
        // monitor.p3[i] = reactionsIt->second.GetFixedFragment()
        //                   .Get_P(); // Measured by monitor
        // monitor.p4[i]
        //    = reactionsIt->second.GetFreeFragment().Get_P(); // Binary partner
    }
    if (settings.verbosity_level > 2)
    {
        std::cout << "Monitor E size " << monitor.ESi.size() << std::endl;
    }
}

bool MonitorAnalysis::FillHistos()
{
    if (!DetectorAnalysis::FillHistos())
        return false;

    if (settings.verbosity_level > 2)
    {
        std::cout << "Monitor E size Histo " << monitor.ESi.size() << std::endl;
    }

    // Filling histograms
    MonitorHistograms &h = monitorHistograms;

    for (unsigned int i{0}; i < monitor.ESi.size(); ++i)
    {

        Fill(h.m_Energy_TS,
             unsignedDiff<double, unsigned long long>(monitor.TS, input.firstTS) *
                 conf.monitor.time_unit,
             monitor.ESi[i]);

        // TVector3 pos = monitor.p3[i].Vect();
        // pos.SetMag(90.);
    }
    return true;
}

void MonitorAnalysis::Instantiate()
{
    DetectorAnalysis::Instantiate();
    TTreeReader &fReader{input.fReader};

    if (conf.detectorsPresent.Monitor)
    {
        if (input.treeToRead->FindLeaf("nbMONITOR") == nullptr)
            throw std::runtime_error("MONITOR enabled in the config but not found in the tree");

        monitorReader.nbMONITOR     = new TTreeReaderValue<Int_t>(fReader, "nbMONITOR");
        monitorReader.TSMONITOR     = new TTreeReaderValue<ULong64_t>(fReader, "TSMONITOR");
        monitorReader.TSMONITOR_Hit = new TTreeReaderArray<ULong64_t>(fReader, "TSMONITOR_Hit");

        if (input.treeToRead->FindLeaf("MONITORChannelId") != nullptr)
            monitorReader.MONITORChannel =
                new TTreeReaderArray<UShort_t>(fReader, "MONITORChannel.MONITORChannelId");
        else
            monitorReader.MONITORChannel =
                new TTreeReaderArray<UShort_t>(fReader, "MONITORChannel");

        if (input.treeToRead->FindLeaf("MONITORBoardId") != nullptr)
            monitorReader.MONITORBoard =
                new TTreeReaderArray<UShort_t>(fReader, "MONITORBoard.MONITORBoardId");
        else
            monitorReader.MONITORBoard = new TTreeReaderArray<UShort_t>(fReader, "MONITORBoard");

        //    monitorReader.MONITORChannel = new TTreeReaderArray<UShort_t>(
        //        fReader, "MONITORChannel.MONITORChannel");
        //    monitorReader.MONITORBoard = new TTreeReaderArray<UShort_t>(
        //        fReader, "MONITORBoard.MONITORBoard");
        monitorReader.MONITOREnergy = new TTreeReaderArray<Float_t>(fReader, "MONITOREnergy");
        monitorReader.MONITORTime   = new TTreeReaderArray<Float_t>(fReader, "MONITORTime");
    }
}

void MonitorAnalysis::Finalize()
{
    DetectorAnalysis::Finalize();
}
