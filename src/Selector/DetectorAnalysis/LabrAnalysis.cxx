#include "LabrAnalysis.h"

#include "Labr.h"

#include <numeric>

LabrAnalysis::LabrAnalysis(DetectorAnalysis::Inputs in, Labr &l)
    : DetectorAnalysis(in, l, ConfManager::getInstance().labr, labrHistograms), labr(l)
{
    if (detectorConf.enable_treeFill && input.type == Inputs::Type::SIGNAL)
        input.tree->Branch("labr", &labr);
}

bool LabrAnalysis::AllocateHistos()
{
    LabrHistograms &h = labrHistograms;

    h.dir = input.outFile.mkdir("Labr", "Labr standalone histograms");
    h.dir->cd();
    if (!DetectorAnalysis::AllocateHistos())
        return false;

    Initialize<TH2D>(
        h.m_layout_XY,
        new TH2D("m_layoutXY", "gamma position XY;X[mm];Y[mm]", 800, -400, 400, 800, -400, 400),
        h.ptrs);
    Initialize<TH2D>(
        h.m_layout_YZ,
        new TH2D("m_layoutYZ", "gamma position YZ;Y[mm];Z[mm]", 800, -400, 400, 800, -400, 400),
        h.ptrs);
    Initialize<TH2D>(
        h.m_layout_XZ,
        new TH2D("m_layoutXZ", "gamma position XZ;X[mm];Z[mm]", 800, -400, 400, 800, -400, 400),
        h.ptrs);
    Initialize<TH3D>(h.m_layout_XYZ,
                     new TH3D("m_layoutXYZ", "gamma position XYZ;X[mm];Y[mm];Z[mm]", 200, -400, 400,
                              200, -400, 400, 200, -400, 400),
                     h.ptrs);

    // 1-D histrograms
    Initialize<TH1D>(h.h_E_labr_raw,
                     new TH1D("h_E_raw", "gamma raw LaBr;ADC channels [-];Counts/4 channels",
                              32768 / 4, 0, 32768),
                     h.ptrs);

    Initialize<TH1D>(
        h.h_E_labr,
        new TH1D("h_E", Form("gamma LaBr;Energy [keV];Counts/%d keV", conf.labr.binWidth),
                 static_cast<int>(conf.labr.gammaEmax / UNITS::keV / conf.labr.binWidth), 0,
                 conf.labr.gammaEmax / UNITS::keV),
        h.ptrs);

    // 2-D histrograms
    Initialize<TH2D>(
        h.m_EE_labr,
        new TH2D("m_Labr_E_E", "gamma-gamma Labr;Energy [keV];Energy [keV]",
                 static_cast<int>(conf.labr.gammaEmax / UNITS::keV / conf.labr.binWidth), 0,
                 conf.labr.gammaEmax / UNITS::keV,
                 static_cast<int>(conf.labr.gammaEmax / UNITS::keV / conf.labr.binWidth), 0,
                 conf.labr.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_E_Tdiff,
        new TH2D("m_E_Tdiff",
                 "Labr Energy vs time difference of Labrs;Time "
                 "difference [ns];Energy [keV]",
                 4000, -200, 200,
                 static_cast<int>(conf.labr.gammaEmax / UNITS::keV / conf.labr.binWidth), 0,
                 conf.labr.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH2D>(
        h.m_Energy_TS,
        new TH2D("m_Energy_TS", "LaBr Time vs energy;Time [s];Energy [MeV]", 12 * 360, 0, 12 * 3600,
                 static_cast<int>(conf.labr.gammaEmax / UNITS::keV / conf.labr.binWidth), 0,
                 conf.labr.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH1D>(
        h.h_monitor,
        new TH1D("h_monitor", Form("monitor;Energy [keV];Counts/%d keV", conf.labr.binWidth),
                 static_cast<int>(conf.labr.gammaEmax / UNITS::keV / conf.labr.binWidth), 0,
                 conf.labr.gammaEmax / UNITS::keV),
        h.ptrs);
    Initialize<TH2D>(h.m_labrID_tdiff,
                     new TH2D("m_labrID_tdiff", "Time alignement LaBr3; LaBr ID; #Delta T [ns]",
                              2 * Lut::MAXCH, 0, 2 * Lut::MAXCH, 1024, -128, 128),
                     h.ptrs);
    SetupLabelsForChannelID(conf.labr, h.m_labrID_tdiff);

    {
        h.ana.dir = h.dir->mkdir("Ana", "Labr spectra with time condition");
        h.ana.dir->cd();
        Initialize<TH1D>(
            h.ana.h_EDC_avg_labr,
            new TH1D("h_EDC_avg_labr",
                     "Histogram with average Doppler correction Labr;Energy "
                     "[keV];Energy [keV]",
                     static_cast<int>(conf.labr.gammaEmax / UNITS::keV / conf.labr.binWidth), 0,
                     conf.labr.gammaEmax / UNITS::keV),
            h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_E_labrID,
            new TH2D("m_E_labrID",
                     Form("Energy vs Labr ID;Energy "
                          "[keV];Counts/ %d",
                          conf.labr.binWidth),
                     2 * Lut::MAXCH, 0, 2 * Lut::MAXCH,
                     static_cast<int>(conf.labr.gammaEmax / UNITS::keV / conf.labr.binWidth), 0,
                     conf.labr.gammaEmax / UNITS::keV),
            h.ana.ptrs);
        SetupLabelsForChannelID(conf.labr, h.ana.m_E_labrID);

        Initialize<TH2D>(
            h.ana.m_EDC_EDC_avg_labr,
            new TH2D("m_EDC_EDC_avg",
                     "gamma-gamma with average Doppler correction;Energy "
                     "[keV];Energy [keV]",
                     static_cast<int>(conf.labr.gammaEmax / UNITS::keV / conf.labr.binWidth), 0,
                     conf.labr.gammaEmax / UNITS::keV,
                     static_cast<int>(conf.labr.gammaEmax / UNITS::keV / conf.labr.binWidth), 0,
                     conf.labr.gammaEmax / UNITS::keV),
            h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_Theta_E_avg_labr,
            new TH2D("m_Theta_E_avg_labr",
                     "Angle with respect to beta avg vector vs gamma energy "
                     "labr;Angle [deg];Energy [keV]",
                     100, 0, 180,
                     static_cast<int>(conf.labr.gammaEmax / UNITS::keV / conf.labr.binWidth), 0,
                     conf.labr.gammaEmax / UNITS::keV),
            h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_Theta_EDC_avg_labr,
            new TH2D("m_Theta_EDC_avg_labr",
                     "Angle with respect to beta avg vector vs Doppler "
                     "corrected gamma energy labr;Angle [deg];Energy DC[keV]",
                     100, 0, 180,
                     static_cast<int>(conf.labr.gammaEmax / UNITS::keV / conf.labr.binWidth), 0,
                     conf.labr.gammaEmax / UNITS::keV),
            h.ana.ptrs);
        Initialize<TH2D>(
            h.ana.m_montdiff_EDClabr,
            new TH2D("m_montdiff_EDClabr",
                     "Time difference between a labr and the monitor; "
                     "#Delta t [ns];Energy DC[keV]",
                     1000, -200, 200,
                     static_cast<int>(conf.labr.gammaEmax / UNITS::keV / conf.labr.binWidth), 0,
                     conf.labr.gammaEmax / UNITS::keV),
            h.ana.ptrs);
    }
    return true;
}

void LabrAnalysis::Analyze()
{
    DetectorAnalysis::Analyze();

    // if (!labrReader.nbLABR || !labrReader.nbLABR->IsValid()) return;
    if (!labrReader.nbLABR)
        return;

    std::vector<unsigned int> idx;
    int                       monId{-1};

    for (unsigned int i{0}; i < labrReader.LABREnergy->GetSize(); ++i)
    {
        unsigned int brd  = labrReader.LABRBoard->At(i);
        unsigned int chan = labrReader.LABRChannel->At(i);
        double       energy{conf.labr.getEnergy(labrReader.LABREnergy->At(i), brd, chan)};
        const auto  &thr = conf.labr.getThr(brd, chan);
        if (energy > thr.first && energy < thr.second)
        {
            if (conf.labr.getName(brd, chan).compare(0, 7, "monitor") == 0)
            {
                monId = i;
            }
            else
            {
                idx.push_back(i);
            }
        }
    }

    labr.resize(idx.size());

    // Find lowest TS
    if (!labr.E.empty())
    {
        labr.TS = std::numeric_limits<unsigned long long>::max();
    }

    for (unsigned int i{0}; i < idx.size(); ++i)
    {
        auto ts = labrReader.TSLABR_Hit->At(i);
        if (ts > 0 && ts < labr.TS)
        {
            labr.TS = ts;
        }
    }

    if (monId != -1)
    {
        unsigned int brd  = labrReader.LABRBoard->At(monId);
        unsigned int chan = labrReader.LABRChannel->At(monId);

        labr.MonE = conf.labr.getEnergy(labrReader.LABREnergy->At(monId), brd, chan);

        labr.MonTime =
            unsignedDiff<double, unsigned long long>(labrReader.TSLABR_Hit->At(monId), labr.TS) *
                conf.labr.time_unit +
            conf.labr.getTime(labrReader.LABRTime->At(monId) * conf.labr.cfd_unit, brd, chan);
    }

    for (unsigned int i{0}; i < idx.size(); ++i)
    {
        unsigned int brd  = labrReader.LABRBoard->At(idx[i]);
        unsigned int chan = labrReader.LABRChannel->At(idx[i]);
        labr.Id[i]        = Lut::getID(brd, chan);
        labr.E[i]         = conf.labr.getEnergy(labrReader.LABREnergy->At(idx[i]), brd, chan);

        labr.Qshort[i] = conf.labr.getQshort(labrReader.LABRQshort->At(idx[i]), brd, chan);

        labr.Time[i] =
            unsignedDiff<double, unsigned long long>(labrReader.TSLABR_Hit->At(idx[i]), labr.TS) *
                conf.labr.time_unit +
            conf.labr.getTime(labrReader.LABRTime->At(idx[i]) * conf.labr.cfd_unit, brd, chan);
        labr.Pos[i] += conf.labr.getPos(brd, chan);

        labr.P[i] = TLorentzVector(0, 0, labr.E.at(i), labr.E.at(i));
        labr.P[i].SetTheta(labr.Pos.at(i).Theta());
        labr.P[i].SetPhi(labr.Pos.at(i).Phi());
    }
    switch (conf.labr.spectraType)
    {
    case LabrConf::SpectraType::CORE: {
        // All ok, the previous analysis is ok
        break;
    }
    case LabrConf::SpectraType::CALORIMETER: {
        // Here instead we sum up all the labrs and take the highest energy hit
        // as the first point for doppler correction
        if (labr.E.empty())
            break;
        auto maxIdx{std::distance(labr.E.begin(), std::max_element(labr.E.begin(), labr.E.end()))};
        auto totalEne{std::accumulate(labr.E.begin(), labr.E.end(), 0)};
        auto totalQshort{std::accumulate(labr.Qshort.begin(), labr.Qshort.end(), 0)};

        std::swap(labr.Id[0], labr.Id[maxIdx]);
        labr.E[0]      = totalEne;
        labr.Qshort[0] = totalQshort;
        std::swap(labr.Time[0], labr.Time[maxIdx]);
        std::swap(labr.Pos[0], labr.Pos[maxIdx]);
        labr.P[0] = TLorentzVector(0, 0, labr.E.at(0), labr.E.at(0));
        labr.P[0].SetTheta(labr.Pos.at(0).Theta());
        labr.P[0].SetPhi(labr.Pos.at(0).Phi());
        labr.resize(1);
        break;
    }
    default:
        throw std::runtime_error("Labr can only handle CORE or CALORIMETER modes\n");
    }
}

bool LabrAnalysis::FillHistos()
{
    if (!DetectorAnalysis::FillHistos())
        return false;

    LabrHistograms &h = labrHistograms;

    // Filling histograms
    for (unsigned int i{0}; i < labr.Id.size(); ++i)
    {
        if (static_cast<int>(labr.Id.at(i)) == conf.labr.reference)
        {
            for (unsigned int j{0}; j < labr.Id.size(); ++j)
            {
                Fill(h.m_labrID_tdiff, labr.Id.at(j),
                     (labr.Time.at(i) - labr.Time.at(j)) / UNITS::ns);
            }
        }
    }
    std::vector<TLorentzVector> pDC;
    for (unsigned int i{0}; i < labr.E.size(); ++i)
    {
        pDC.push_back(labr.P.at(i));
        pDC.back().Boost(-conf.labr.betaAvg);
    }

    Fill(labrHistograms.h_monitor, labr.MonE);
    for (unsigned int i{0}; i < labr.E.size(); ++i)
    {
        TVector3 posRan(ranGen.Rndm(), ranGen.Rndm(), ranGen.Rndm());
        while (((posRan.X() - 0.5) * (posRan.X() - 0.5) + (posRan.Y() - 0.5) * (posRan.Y() - 0.5)) >
               0.5 * 0.5)
        {
            posRan.SetX(ranGen.Rndm());
            posRan.SetY(ranGen.Rndm());
        }

        posRan.SetX((posRan.X() - 0.5) * 75);
        posRan.SetY((posRan.Y() - 0.5) * 75);
        posRan.SetZ(ranGen.Rndm() * 100 + 230);

        posRan.RotateY(labr.Pos.at(i).Theta());
        posRan.RotateZ(labr.Pos.at(i).Phi());

        Fill(h.m_layout_XY, (posRan.X()), (posRan.Y()));
        Fill(h.m_layout_XZ, (posRan.X()), (posRan.Z()));
        Fill(h.m_layout_YZ, (posRan.Y()), (posRan.Z()));
        Fill(h.m_layout_XYZ, (posRan.X()), (posRan.Y()), (posRan.Z()));

        Fill(h.h_E_labr_raw, labrReader.LABREnergy->At(i));
        Fill(h.h_E_labr, labr.E[i]);

        Fill(h.m_Energy_TS, unsignedDiff<double, unsigned long long>(labr.TS, input.firstTS) / 1.e8,
             labr.E[i]);

        // Ana histograms
        Fill(h.ana.h_EDC_avg_labr, pDC.at(i).E());
        Fill(h.ana.m_E_labrID, labr.Id.at(i), labr.E.at(i));
        Fill(h.ana.m_Theta_E_avg_labr, conf.labr.betaAvg.Angle(labr.Pos.at(i)) / UNITS::deg,
             labr.E.at(i));
        Fill(h.ana.m_Theta_EDC_avg_labr, conf.labr.betaAvg.Angle(labr.Pos.at(i)) / UNITS::deg,
             pDC.at(i).E());

        if (labr.MonE > 0)
        {
            Fill(h.ana.m_montdiff_EDClabr, (labr.Time[i] - labr.MonTime) / UNITS::ns,
                 pDC.at(i).E());
        }
        for (unsigned int j{0}; j < i; ++j)
        {
            Fill(h.ana.m_EDC_EDC_avg_labr, pDC.at(i).E(), pDC.at(j).E());
            Fill(h.ana.m_EDC_EDC_avg_labr, pDC.at(j).E(), pDC.at(i).E());

            Fill(h.m_EE_labr, labr.E[i], labr.E[j]);
            Fill(h.m_EE_labr, labr.E[j], labr.E[i]);

            Fill(h.m_E_Tdiff, (labr.Time[i] - labr.Time[j]) / UNITS::ns, labr.E[i]);
            Fill(h.m_E_Tdiff, (labr.Time[j] - labr.Time[i]) / UNITS::ns, labr.E[j]);
        }
    }

    return true;
}

void LabrAnalysis::Instantiate()
{
    DetectorAnalysis::Instantiate();
    TTreeReader &fReader{input.fReader};

    if (conf.detectorsPresent.Labr)
    {
        if (input.treeToRead->FindLeaf("nbLABR") == nullptr)
            throw std::runtime_error("LaBr enabled in the config but not found in the tree");
        labrReader.nbLABR     = new TTreeReaderValue<Int_t>(fReader, "nbLABR");
        labrReader.TSLABR     = new TTreeReaderValue<ULong64_t>(fReader, "TSLABR");
        labrReader.TSLABR_Hit = new TTreeReaderArray<ULong64_t>(fReader, "TSLABR_Hit");

        if (input.treeToRead->FindLeaf("LABRChannelId") != nullptr)
            labrReader.LABRChannel =
                new TTreeReaderArray<UShort_t>(fReader, "LABRChannel.LABRChannelId");
        else
            labrReader.LABRChannel = new TTreeReaderArray<UShort_t>(fReader, "LABRChannel");

        if (input.treeToRead->FindLeaf("LABRBoardId") != nullptr)
            labrReader.LABRBoard = new TTreeReaderArray<UShort_t>(fReader, "LABRBoard.LABRBoardId");
        else
            labrReader.LABRBoard = new TTreeReaderArray<UShort_t>(fReader, "LABRBoard");

        labrReader.LABREnergy = new TTreeReaderArray<Float_t>(fReader, "LABREnergy");
        labrReader.LABRQshort = new TTreeReaderArray<Float_t>(fReader, "LABRQshort");
        labrReader.LABRTime   = new TTreeReaderArray<Float_t>(fReader, "LABRTime");
    }
}

void LabrAnalysis::Finalize()
{
    DetectorAnalysis::Finalize();
    NormalizeHistos<DetectorConf>(conf.labr);
}
