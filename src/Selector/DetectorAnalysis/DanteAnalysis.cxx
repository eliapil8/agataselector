#include "DanteAnalysis.h"

#include "Dante.h"
#include "Lut.h"

#include <iomanip>

DanteAnalysis::DanteAnalysis(DetectorAnalysis::Inputs in, Dante &d)
    : DetectorAnalysis(in, d, ConfManager::getInstance().dante, danteHistograms), dante(d),
      TOFgate(conf.replay.conf_path + "/CUT/DANTE/")
{
    if (detectorConf.enable_treeFill && input.type == Inputs::Type::SIGNAL)
        input.tree->Branch("Dante", &dante);
}

bool DanteAnalysis::AllocateHistos()
{
    DanteHistograms &h = danteHistograms;
    h.dir              = input.outFile.mkdir("Dante", "Dante histograms");
    h.dir->cd();
    if (!DetectorAnalysis::AllocateHistos())
        return false;

    Initialize<TH3D>(h.m_layout_XYZ,
                     new TH3D("m_layout_XYZ", "DANTE X vs Y vs Z;X[mm];Y[mm];Z[mm]", 100, -100, 100,
                              100, -100, 100, 100, -100, 100),
                     h.ptrs);

    Initialize<TH2D>(h.m_layout_front,
                     new TH2D("m_layout_front",
                              // "DANTE back (beam dump) view;<-PRISMA | AGATA-> X[mm];Y[mm]",
                              "DANTE front (beam) view;<-AGATA | Prisma->(-X)[mm];Y[mm]", 1000,
                              -100, 100, 1000, -100, 100),
                     h.ptrs);

    Initialize<TH2D>(h.m_layout_top,
                     new TH2D("m_layout_top",
                              // "DANTE top view;<-PRISMA | AGATA-> X[mm]; beam dir Z[mm]",
                              "DANTE top view;<-AGATA | Prisma->(-X)[mm];beam->(Z)[mm]", 1000, -100,
                              100, 1000, -100, 100),
                     h.ptrs);

    Initialize<TH2D>(h.m_layout_side,
                     new TH2D("m_layout_side",
                              // "DANTE side (PRISMA) view;Y[mm];beam dir Z[mm]",
                              "DANTE side (AGATA) view;<-beam (-Z)[mm];Y[mm]", 1000, -100, 100,
                              1000, -100, 100),
                     h.ptrs);
    Initialize<TH1D>(
        h.h_danteID,
        new TH1D("h_danteID", "Total counts for each Dante detector;ID;Counts", 10, 0, 10), h.ptrs);

    Initialize<TH2D>(h.m_goodEvtsStat_danteID,
                     new TH2D("m_goodEvtsStat_danteID",
                              "Statistics of good vs incomplete events for "
                              "Dante ID;ID;Goodness;Counts",
                              5, 0, 5, 11, 0, 11),
                     h.ptrs);
    if (h.m_goodEvtsStat_danteID != nullptr)
    {
        h.m_goodEvtsStat_danteID->GetYaxis()->SetBinLabel(1, "OK");
        h.m_goodEvtsStat_danteID->GetYaxis()->SetBinLabel(2, "Missing X");
        h.m_goodEvtsStat_danteID->GetYaxis()->SetBinLabel(3, "Missing Y");
        h.m_goodEvtsStat_danteID->GetYaxis()->SetBinLabel(4, "Missing T");
        h.m_goodEvtsStat_danteID->GetYaxis()->SetBinLabel(5, "Missing X and Y");
        h.m_goodEvtsStat_danteID->GetYaxis()->SetBinLabel(6, "Missing X and T");
        h.m_goodEvtsStat_danteID->GetYaxis()->SetBinLabel(7, "Missing Y and T");
        h.m_goodEvtsStat_danteID->GetYaxis()->SetBinLabel(8, "Missing TOF");
        h.m_goodEvtsStat_danteID->GetYaxis()->SetBinLabel(9, "Missing X and TOF");
        h.m_goodEvtsStat_danteID->GetYaxis()->SetBinLabel(10, "Missing Y and TOF");
    }

    for (const auto &id : conf.dante.getAllDanteId())
    {
        auto brdchanX = conf.dante.getBrdChanFromId(id, DanteLut::X);
        auto brdchanY = conf.dante.getBrdChanFromId(id, DanteLut::Y);
        auto xThr     = conf.dante.getThr(brdchanX.first, brdchanX.second);
        auto yThr     = conf.dante.getThr(brdchanY.first, brdchanY.second);
        Initialize<TH2D>(h.m_dante[id],
                         new TH2D(Form("m_dante_%i", id),
                                  Form("Total counts for Dante detector %i;X;Y;Counts", id), 1000,
                                  xThr.first, xThr.second, 1000, yThr.first, yThr.second),
                         h.ptrs);

        if (conf.dante.hasTof(id))
        {
            auto brdchanTOF = conf.dante.getBrdChanFromId(id, DanteLut::TOF);
            auto tofThr     = conf.dante.getThr(brdchanTOF.first, brdchanTOF.second);
            Initialize<TH1D>(
                h.h_TOF[id],
                new TH1D(Form("h_tof_%i", id), Form("Tof of detector %i;Tof;Counts", id), 1000,
                         conf.dante.getTof(tofThr.first, id), conf.dante.getTof(tofThr.second, id)),
                h.ptrs);
            Initialize<TH2D>(h.h_TOF_Theta[id],
                             new TH2D(Form("h_tof_Theta_%i_0", id),
                                      Form("Tof of detector %i vs Theta;Tof [ns];Theta [deg]", id),
                                      1000, conf.dante.getTof(tofThr.first, id),
                                      conf.dante.getTof(tofThr.second, id), 1000, 0, 180),
                             h.ptrs);
        }
    }

    // dante - dante coindence
    Initialize<TH2D>(h.m_dante_dante_theta,
                     new TH2D("m_dante_dante_theta", "dante  dante coindence  ;theta;theta;Counts",
                              1000, 0, 180, 1000, 0, 180),
                     h.ptrs);
    Initialize<TH2D>(
        h.m_dante_dante_theta_tof,
        new TH2D(
            "m_dante_dante_theta_tof",
            Form("dante  dante coindence with tof condition;theta A=%i Z=%i;theta A=%i Z=%i;Counts",
                 conf.dante.ion.A, conf.dante.ion.Z, conf.dante.ion2.A, conf.dante.ion2.Z),
            1000, 0, 180, 1000, 0, 180),
        h.ptrs);
    Initialize<TH2D>(h.m_dante_dante_phi,
                     new TH2D("m_dante_dante_phi", "dante  dante coindence  ;phi;phi;Counts", 1000,
                              -180, 180, 1000, -180, 180),
                     h.ptrs);

    for (const auto &id : conf.dante.getAllDanteId())
    {
        auto &d = dantePresentMask[id];

        //-1 means mandatory, -2 means not mandatory
        if (conf.dante.mandatoryX)
            d[DanteLut::DanteType::X] = -1;
        else
            d[DanteLut::DanteType::X] = -2;

        if (conf.dante.mandatoryY)
            d[DanteLut::DanteType::Y] = -1;
        else
            d[DanteLut::DanteType::Y] = -2;

        if (conf.dante.mandatoryTime)
            d[DanteLut::DanteType::T] = -1;
        else
            d[DanteLut::DanteType::T] = -2;

        if (conf.dante.mandatoryTOF)
            d[DanteLut::DanteType::TOF] = -1;
        else
            d[DanteLut::DanteType::TOF] = -2;

        if (std::any_of(d.begin(), d.end(), [](const auto &p) { return p > 0; }))
            throw std::runtime_error("Dante should have at least one mandatory channel");
    }
    return true;
}

void DanteAnalysis::Analyze()
{
    DetectorAnalysis::Analyze();

    if (settings.verbosity_level > 2)
        std::cout << "Dante analyze() " << std::endl;

    auto dantePresent = dantePresentMask;

    // cms edit accessing arrays below only valid operation if nbDante > 0
    // if (!danteReader.nbDANTE || !danteReader.nbDANTE->IsValid()) return;
    if (!danteReader.nbDANTE)
        return;
    if (settings.verbosity_level > 2)
        danteReader.printDanteReader();

    // Find what is present
    for (unsigned int i{0}; i < danteReader.DANTEEnergy->GetSize(); ++i)
    {
        const int          &brd  = danteReader.DANTEBoard->At(i);
        const int          &ch   = danteReader.DANTEChannel->At(i);
        const unsigned int &id   = conf.dante.getDanteId(brd, ch);
        const unsigned int &type = conf.dante.getDanteType(brd, ch);

        // Energy threshold
        if (danteReader.DANTEEnergy->At(i) > conf.dante.getThr(brd, ch).first &&
            danteReader.DANTEEnergy->At(i) < conf.dante.getThr(brd, ch).second)
        {
            dantePresent[id][type] = static_cast<int>(i);
        }
    }

    for (const auto &dantePresentIt : dantePresent)
    {
        if (dantePresentIt.second.at(DanteLut::DanteType::X) >= 0 &&
            dantePresentIt.second.at(DanteLut::DanteType::Y) >= 0 &&
            dantePresentIt.second.at(DanteLut::DanteType::T) >= 0 &&
            dantePresentIt.second.at(DanteLut::DanteType::TOF) >= 0)
            Fill(danteHistograms.m_goodEvtsStat_danteID, dantePresentIt.first, "OK");

        else if (dantePresentIt.second.at(DanteLut::DanteType::X) < 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::Y) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::T) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::TOF) >= 0)
            Fill(danteHistograms.m_goodEvtsStat_danteID, dantePresentIt.first, "Missing X");
        else if (dantePresentIt.second.at(DanteLut::DanteType::X) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::Y) < 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::T) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::TOF) >= 0)
            Fill(danteHistograms.m_goodEvtsStat_danteID, dantePresentIt.first, "Missing Y");
        else if (dantePresentIt.second.at(DanteLut::DanteType::X) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::Y) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::T) < 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::TOF) >= 0)
            Fill(danteHistograms.m_goodEvtsStat_danteID, dantePresentIt.first, "Missing T");
        else if (dantePresentIt.second.at(DanteLut::DanteType::X) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::Y) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::T) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::TOF) < 0)
            Fill(danteHistograms.m_goodEvtsStat_danteID, dantePresentIt.first, "Missing TOF");
        else if (dantePresentIt.second.at(DanteLut::DanteType::X) < 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::Y) < 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::T) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::TOF) >= 0)
            Fill(danteHistograms.m_goodEvtsStat_danteID, dantePresentIt.first, "Missing X and Y");
        else if (dantePresentIt.second.at(DanteLut::DanteType::X) < 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::Y) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::T) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::TOF) < 0)
            Fill(danteHistograms.m_goodEvtsStat_danteID, dantePresentIt.first, "Missing X and TOF");
        else if (dantePresentIt.second.at(DanteLut::DanteType::X) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::Y) < 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::T) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::TOF) < 0)
            Fill(danteHistograms.m_goodEvtsStat_danteID, dantePresentIt.first, "Missing Y and TOF");
        else if (dantePresentIt.second.at(DanteLut::DanteType::X) < 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::Y) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::T) < 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::TOF) >= 0)
            Fill(danteHistograms.m_goodEvtsStat_danteID, dantePresentIt.first, "Missing X and T");
        else if (dantePresentIt.second.at(DanteLut::DanteType::X) >= 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::Y) < 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::T) < 0 &&
                 dantePresentIt.second.at(DanteLut::DanteType::TOF) >= 0)
            Fill(danteHistograms.m_goodEvtsStat_danteID, dantePresentIt.first, "Missing Y and T");
    }

    // Remove incomplete events
    for (auto it{dantePresent.begin()}; it != dantePresent.end();)
    {
        if ((it->second[DanteLut::DanteType::X] == -1 && conf.dante.mandatoryX) ||
            (it->second[DanteLut::DanteType::Y] == -1 && conf.dante.mandatoryY) ||
            (it->second[DanteLut::DanteType::T] == -1 && conf.dante.mandatoryTime) ||
            (it->second[DanteLut::DanteType::TOF] == -1 && conf.dante.mandatoryTOF))
        {
            dantePresent.erase(it++);
        }
        else
            ++it;
    }

    if (dantePresent.empty())
        return;

    // Get timestamp
    if (settings.verbosity_level > 2)
        std::cout << "Setting timestamps " << std::endl;
    std::unordered_map<unsigned int, std::pair<unsigned long long, double>> timestamps;
    for (const auto &d : dantePresent)
    {
        if (conf.dante.mandatoryTime && d.second.at(DanteLut::DanteType::T) >= 0)
        { // Take time from T
            if (settings.verbosity_level > 2)
                std::cout << "Time from T " << std::endl;
            timestamps[d.first] = {
                danteReader.TSDANTE_Hit->At(d.second.at(DanteLut::DanteType::T)),
                conf.dante.getTime(
                    danteReader.DANTETime->At(d.second.at(DanteLut::DanteType::T)) *
                        conf.dante.cfd_unit,
                    danteReader.DANTEBoard->At(d.second.at(DanteLut::DanteType::T)),
                    danteReader.DANTEChannel->At(d.second.at(DanteLut::DanteType::T)))};
        }
        else if ((conf.dante.mandatoryX && d.second.at(DanteLut::DanteType::X) >= 0) &&
                 (conf.dante.mandatoryY && d.second.at(DanteLut::DanteType::Y) >= 0))
        {
            if (settings.verbosity_level > 2)
                std::cout << "Time from X or Y " << std::endl;
            auto tx = danteReader.TSDANTE_Hit->At(d.second.at(DanteLut::DanteType::X));
            auto ty = danteReader.TSDANTE_Hit->At(d.second.at(DanteLut::DanteType::Y));
            if (tx < ty)
                timestamps[d.first] = {
                    tx, conf.dante.getTime(
                            danteReader.DANTETime->At(d.second.at(DanteLut::DanteType::X)) *
                                conf.dante.cfd_unit,
                            danteReader.DANTEBoard->At(d.second.at(DanteLut::DanteType::X)),
                            danteReader.DANTEChannel->At(d.second.at(DanteLut::DanteType::X)))};
            else
                timestamps[d.first] = {
                    ty, conf.dante.getTime(
                            danteReader.DANTETime->At(d.second.at(DanteLut::DanteType::Y)) *
                                conf.dante.cfd_unit,
                            danteReader.DANTEBoard->At(d.second.at(DanteLut::DanteType::Y)),
                            danteReader.DANTEChannel->At(d.second.at(DanteLut::DanteType::Y)))};
        }
        else if (conf.dante.mandatoryX && d.second.at(DanteLut::DanteType::X) >= 0)
        { // Take time from X
            if (settings.verbosity_level > 2)
                std::cout << "Time from x " << std::endl;
            timestamps[d.first] = {
                danteReader.TSDANTE_Hit->At(d.second.at(d.second.at(DanteLut::DanteType::X))) *
                    conf.dante.cfd_unit,
                conf.dante.getTime(
                    danteReader.DANTETime->At(d.second.at(DanteLut::DanteType::X)),
                    danteReader.DANTEBoard->At(d.second.at(DanteLut::DanteType::X)),
                    danteReader.DANTEChannel->At(d.second.at(DanteLut::DanteType::X)))};
        }
        else if (conf.dante.mandatoryY && d.second.at(DanteLut::DanteType::Y) >= 0)
        { // Take time from Y
            if (settings.verbosity_level > 2)
                std::cout << "time from y " << std::endl;
            // std::cout<<"Y "<<danteReader.TSDANTE_Hit->At(d.second.at(1))<<std::endl;
            timestamps[d.first] = {
                danteReader.TSDANTE_Hit->At(d.second.at(DanteLut::DanteType::Y)),
                conf.dante.getTime(
                    danteReader.DANTETime->At(d.second.at(DanteLut::DanteType::Y)) *
                        conf.dante.cfd_unit,
                    danteReader.DANTEBoard->At(d.second.at(DanteLut::DanteType::Y)),
                    danteReader.DANTEChannel->At(d.second.at(DanteLut::DanteType::Y)))};
        }
        else if (conf.dante.mandatoryTOF && d.second.at(DanteLut::DanteType::TOF) >= 0)
        { // Take time from T
            if (settings.verbosity_level > 2)
                std::cout << "Time from TOF " << std::endl;
            timestamps[d.first] = {
                danteReader.TSDANTE_Hit->At(d.second.at(DanteLut::DanteType::TOF)),
                conf.dante.getTime(
                    danteReader.DANTETime->At(d.second.at(DanteLut::DanteType::TOF)) *
                        conf.dante.cfd_unit,
                    danteReader.DANTEBoard->At(d.second.at(DanteLut::DanteType::TOF)),
                    danteReader.DANTEChannel->At(d.second.at(DanteLut::DanteType::TOF)))};
        }
        else
        {
            throw std::runtime_error("Something is wrong with dante conditions\n");
        }
    }

    dante.TS =
        std::min_element(timestamps.begin(), timestamps.end(), [](const auto &l, const auto &r) {
            return l.second.first < r.second.first;
        })->second.first;

    // Time gate
    if (settings.verbosity_level > 2)
        std::cout << "Setting time gate " << std::endl;
    for (const auto &ts : timestamps)
    {
        if (unsignedDiff<int, unsigned long long>(ts.second.first, dante.TS) > windowRight)
            dantePresent.erase(dantePresent.find(ts.first));
    }

    if (dantePresent.empty())
        return;
    else
        dante.resize(dantePresent.size());

    for (auto dantePresentIt{dantePresent.begin()}; dantePresentIt != dantePresent.end();
         ++dantePresentIt)
    {
        auto idx = std::distance(dantePresent.begin(), dantePresentIt);
        if (settings.verbosity_level > 2)
            std::cout << "Dante id with coincidence " << dantePresentIt->first << std::endl;
        // Position
        if (dantePresentIt->second[DanteLut::DanteType::X] >= 0 &&
            dantePresentIt->second[DanteLut::DanteType::Y] >= 0)
        {
            // X,Y both present
            dante.Pos.at(idx) = conf.dante.getPosition(
                danteReader.DANTEEnergy->At(dantePresentIt->second[DanteLut::DanteType::X]),
                danteReader.DANTEEnergy->At(dantePresentIt->second[DanteLut::DanteType::Y]),
                dantePresentIt->first);
        }
        else if (dantePresentIt->second[DanteLut::DanteType::X] >= 0 &&
                 dantePresentIt->second[DanteLut::DanteType::Y] < 0)
        {
            // X only present
            dante.Pos.at(idx) = conf.dante.getPosition(
                danteReader.DANTEEnergy->At(dantePresentIt->second[DanteLut::DanteType::X]),
                conf.dante.getAvgY(dantePresentIt->first), dantePresentIt->first);
        }
        else if (dantePresentIt->second[DanteLut::DanteType::X] < 0 &&
                 dantePresentIt->second[DanteLut::DanteType::Y] >= 0)
        {
            // Y only present
            dante.Pos.at(idx) = conf.dante.getPosition(
                conf.dante.getAvgX(dantePresentIt->first),
                danteReader.DANTEEnergy->At(dantePresentIt->second[DanteLut::DanteType::Y]),
                dantePresentIt->first);
        }
        else
        {
            // Neither is present
            dante.Pos.at(idx) = conf.dante.getPosition(conf.dante.getAvgX(dantePresentIt->first),
                                                       conf.dante.getAvgY(dantePresentIt->first),
                                                       dantePresentIt->first);
        }
        dante.Id.at(idx) = dantePresentIt->first;

        Fill(danteHistograms.m_dante.at(dantePresentIt->first),
             danteReader.DANTEEnergy->At(dantePresentIt->second[DanteLut::DanteType::X]),
             danteReader.DANTEEnergy->At(dantePresentIt->second[DanteLut::DanteType::Y]));

        dante.Time.at(idx) = timestamps.at(dantePresentIt->first).second;
        dante.Time.at(idx) = unsignedDiff<double, unsigned long long>(
                                 timestamps.at(dantePresentIt->first).first, dante.TS) *
                                 conf.dante.time_unit +
                             timestamps.at(dantePresentIt->first).second;

        if (conf.dante.hasTof(dantePresentIt->first))
        {
            dante.TOF.at(idx) = {
                conf.dante.getTof(
                    danteReader.DANTEEnergy->At(dantePresentIt->second[DanteLut::DanteType::TOF]),
                    dantePresentIt->first) *
                    UNITS::ns,
                static_cast<Dante::TofType>(conf.dante.tofType(dantePresentIt->first))};
        }
    }

    if (settings.verbosity_level > 2)
        std::cout << "Calculating ion " << std::endl;
    // const ReactionFragment::Ion& detectedIon{conf.dante.ion};

    for (size_t d1{0}; d1 < dante.Pos.size(); ++d1)
    {
        dante.ions.at(d1) = conf.dante.ion;
    }

    // Check for DANTE-DANTE coincidences
    for (size_t d1{0}; d1 < dante.Pos.size(); ++d1)
    {
        for (size_t d2{0}; d2 < dante.Pos.size(); ++d2)
        {
            if (d1 == d2)
                continue;
            // Check if binary reaction
            if (abs(dante.Pos.at(d1).Phi() + dante.Pos.at(d2).Phi()) < conf.dante.phiCoinc)
            {
                if (dante.TOF.at(d1).second == Dante::TofType::DANTE)
                {

                    // dante.matchingDanteIndex.at(d2)=d1;

                    /*
                                                     if(dante.TOF.at(d1).first/UNITS::ns > 0){
                                                                 dante.ions.at(d1) = conf.dante.ion;
                                                                 dante.ions.at(d2) =
                       conf.dante.ion2;
                                                     }
                                                     else if(dante.TOF.at(d1).first/UNITS::ns < 0){
                                                            dante.ions.at(d2) = conf.dante.ion;
                                                               dante.ions.at(d1) = conf.dante.ion2;
                                                     }


                                             */
                    auto res = TOFgate.Id({static_cast<int>(dante.Id.at(d1))},
                                          dante.TOF.at(d1).first / UNITS::ns,
                                          dante.Pos.at(d1).Theta() / UNITS::deg);
                    if (res.first)
                    {

                        dante.matchingDanteIndex.at(d1) = d2;

                        if (res.second[0] == 0)
                        {
                            dante.TOFregion.at(d1) = 0;
                            dante.TOFregion.at(d2) = 0;
                            dante.ions.at(d1)      = conf.dante.ion;
                            dante.ions.at(d2)      = conf.dante.ion2;
                        }
                        else if (res.second[0] == 1)
                        {
                            dante.TOFregion.at(d1) = 1;
                            dante.TOFregion.at(d2) = 1;
                            dante.ions.at(d2)      = conf.dante.ion;
                            dante.ions.at(d1)      = conf.dante.ion2;
                        }
                    }
                }
            }
        }
    }

    for (size_t a{0}; a < dante.Pos.size(); ++a)
    {
        // If it finds an ion2, it means that there has been a dante-dante coincidence
        if (dante.matchingDanteIndex.at(a) == -1)
        {
            // Single dante case, here we calculate the reaction based on a single dante
            const ReactionFragment::Ion &detectedIon{dante.ions.at(a)};
            // Reaction reconstruction
            auto reactionsIt = input.reactions.find(detectedIon);
            if (reactionsIt != input.reactions.end())
            {

                // std::cout <<"Test found Ion"<< std::endl;

                // auto ion = reactionsIt->second.GetFixedFragment();
                // ion.Set_Beta(conf.dante.betaAvg);
                // ion.Set_Theta_Phi(dante.Pos.at(a).Theta(), dante.Pos.at(a).Phi());

                reactionsIt->second.Set_Theta_Phi(dante.Pos.at(a).Theta(), dante.Pos.at(a).Phi(),
                                                  true);
                {
                    TLorentzVector p = reactionsIt->second.GetFixedFragment().Get_P();
                    EvalInTarget(p, detectedIon);
                    dante.p4.at(a) = p;
                }
                {
                    TLorentzVector p = reactionsIt->second.GetFreeFragment().Get_P();
                    EvalInTarget(p, conf.dante.ion2);
                    dante.p3.at(a) = p;
                }
            }
        }
        else if (dante.matchingDanteIndex.at(a) != -1)
        {
            // Two dante coinc since we have found one ion2, here we use both dantes for the
            // kinematics reconstruction
            const ReactionFragment::Ion &detectedIon{dante.ions.at(a)};
            // Reaction reconstruction
            auto reactionsIt = input.reactions.find(detectedIon);
            if (reactionsIt != input.reactions.end())
            {
                // std::cout << "Theta is "  << std::setprecision(5)<<
                // dante.Pos.at(a).Theta()/UNITS::deg << std::endl; std::cout << "Ohi is "    <<
                // std::setprecision(5)<< dante.Pos.at(a).Phi()/UNITS::deg << std::endl; std::cout
                // << "Theta2 is " << std::setprecision(5)<<
                // dante.Pos.at(dante.matchingDanteIndex[a]).Theta()/UNITS::deg << std::endl;
                // std::cout << "Ohi2 is "    << std::setprecision(5)<<
                // dante.Pos.at(dante.matchingDanteIndex[a]).Phi()/UNITS::deg << std::endl;

                auto Qval = reactionsIt->second.Set_Theta_Phi_Theta(
                    dante.Pos.at(a).Theta(), dante.Pos.at(a).Phi(),
                    dante.Pos.at(dante.matchingDanteIndex[a]).Theta(), true);
                // try{
                // auto ex = reactionsIt->second.Set_Theta_Phi_Theta( dante.Pos.at(a).Theta(),
                //																	dante.Pos.at(a).Phi(),
                //                                                    dante.Pos.at(dante.matchingDanteIndex[a]).Theta(),
                //                                                    true);
                dante.Qval.at(a)                           = Qval;
                dante.Qval.at(dante.matchingDanteIndex[a]) = Qval;
                //}catch(...){
                reactionsIt->second.Set_Theta_Phi(dante.Pos.at(a).Theta(), dante.Pos.at(a).Phi(),
                                                  true);
                //};
                // std::cout << "Ex found is " << ex << std::endl;
                {
                    TLorentzVector p = reactionsIt->second.GetFixedFragment().Get_P();
                    EvalInTarget(p, detectedIon);
                    dante.p4.at(a) = p;
                }
                {
                    TLorentzVector p = reactionsIt->second.GetFreeFragment().Get_P();
                    EvalInTarget(p, dante.ions.at(dante.matchingDanteIndex.at(a)));
                    dante.p3.at(a) = p;
                }
            }
        }
    }
}

bool DanteAnalysis::FillHistos()
{
    if (!DetectorAnalysis::FillHistos())
        return false;

    // Filling histograms here
    DanteHistograms &h = danteHistograms;
    for (unsigned int i{0}; i < dante.Pos.size(); ++i)
    {
        Fill(h.h_danteID, dante.Id.at(i));

        if (conf.dante.hasTof(dante.Id.at(i)) && (dante.TOF.at(i).second == Dante::TofType::DANTE ||
                                                  dante.TOF.at(i).second == Dante::TofType::PRISMA))
        {
            Fill(h.h_TOF.at(dante.Id.at(i)), dante.TOF.at(i).first / UNITS::ns);
            Fill(h.h_TOF_Theta.at(dante.Id.at(i)), dante.TOF.at(i).first / UNITS::ns,
                 dante.Pos.at(i).Theta() / UNITS::deg);
        }

        Fill(h.m_layout_front, -dante.Pos.at(i).x() / UNITS::mm, dante.Pos.at(i).y() / UNITS::mm);
        Fill(h.m_layout_top, -dante.Pos.at(i).x() / UNITS::mm, dante.Pos.at(i).z() / UNITS::mm);
        Fill(h.m_layout_side, -dante.Pos.at(i).z() / UNITS::mm, dante.Pos.at(i).y() / UNITS::mm);
        Fill(h.m_layout_XYZ, dante.Pos.at(i).x() / UNITS::mm, dante.Pos.at(i).y() / UNITS::mm,
             dante.Pos.at(i).z() / UNITS::mm);

        // dante-dante coincidence
        for (unsigned int j{0}; j < i; ++j)
        {
            Fill(h.m_dante_dante_theta, dante.Pos.at(i).Theta() / UNITS::deg,
                 dante.Pos.at(j).Theta() / UNITS::deg);
            Fill(h.m_dante_dante_phi, dante.Pos.at(i).Phi() / UNITS::deg,
                 dante.Pos.at(j).Phi() / UNITS::deg);
        }

        if (dante.matchingDanteIndex.at(i) != -1)
        {
            if (dante.ions.at(i) == conf.dante.ion)
            {
                Fill(h.m_dante_dante_theta_tof, dante.Pos.at(i).Theta() / UNITS::deg,
                     dante.Pos.at(dante.matchingDanteIndex[i]).Theta() / UNITS::deg);
            }
            else
            {
                Fill(h.m_dante_dante_theta_tof,
                     dante.Pos.at(dante.matchingDanteIndex[i]).Theta() / UNITS::deg,
                     dante.Pos.at(i).Theta() / UNITS::deg);
            }
        }
    }

    return true;
}

void DanteAnalysis::Instantiate()
{
    DetectorAnalysis::Instantiate();
    TTreeReader &fReader{input.fReader};

    if (conf.detectorsPresent.Dante)
    {
        if (input.treeToRead->FindLeaf("nbDANTE") == nullptr)
            throw std::runtime_error("DANTE enabled in the config but not found in the tree");

        danteReader.nbDANTE     = new TTreeReaderValue<Int_t>(fReader, "nbDANTE");
        danteReader.TSDANTE     = new TTreeReaderValue<ULong64_t>(fReader, "TSDANTE");
        danteReader.TSDANTE_Hit = new TTreeReaderArray<ULong64_t>(fReader, "TSDANTE_Hit");

        if (input.treeToRead->FindLeaf("DANTEChannelId") != nullptr)
            danteReader.DANTEChannel =
                new TTreeReaderArray<UShort_t>(fReader, "DANTEChannel.DANTEChannelId");
        else
            danteReader.DANTEChannel = new TTreeReaderArray<UShort_t>(fReader, "DANTEChannel");

        if (input.treeToRead->FindLeaf("DANTEBoardId") != nullptr)
            danteReader.DANTEBoard =
                new TTreeReaderArray<UShort_t>(fReader, "DANTEBoard.DANTEBoardId");
        else
            danteReader.DANTEBoard = new TTreeReaderArray<UShort_t>(fReader, "DANTEBoard");

        danteReader.DANTEEnergy = new TTreeReaderArray<Float_t>(fReader, "DANTEEnergy");
        danteReader.DANTETime   = new TTreeReaderArray<Float_t>(fReader, "DANTETime");
    }
}

void DanteAnalysis::Finalize()
{
    DetectorAnalysis::Finalize();
    NormalizeHistos<DetectorConf>(conf.dante);
}
