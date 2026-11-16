#include "OscarAnalysis.h"
#include "Oscar.h"
#include "ReactionFragment.h"

#include <cmath>

OscarAnalysis::OscarAnalysis(DetectorAnalysis::Inputs in, Oscar &o, OscarConf &c)
    : DetectorAnalysis(in, o, c, oscarHistograms), oscar(o), oscarConf(c)
{
    for (const auto &gate : oscarConf.excitationEnergyGates)
    {
        if (!std::isfinite(gate.first) || !std::isfinite(gate.second) ||
            gate.first >= gate.second)
            throw std::runtime_error("Invalid OSCAR EX_GATE: lower bound must be smaller than "
                                     "upper bound");
    }
    if (oscarConf.rejectOutsideDEEnergy && !oscarConf.hasDEEnergyCuts())
        throw std::runtime_error(
            "OSCAR REJECT_OUTSIDE_DE_E is YES, but no E-dE cuts were loaded");
    if (oscarConf.rejectOutsideEnergyTheta && oscarConf.energyThetaCuts.empty())
        throw std::runtime_error(
            "OSCAR REJECT_OUTSIDE_E_THETA is YES, but no E-theta cuts were loaded from " +
            oscarConf.energyThetaCutPath);
    if (oscarConf.enable_treeFill && input.type == Inputs::Type::SIGNAL)
        input.tree->Branch("Oscar", &oscar);
}

OscarUpAnalysis::OscarUpAnalysis(DetectorAnalysis::Inputs in, Oscar &s)
    : OscarAnalysis(in, s, ConfManager::getInstance().oscarUp)
{
    if (oscarConf.enable_treeFill)
        input.tree->Branch("OscarUp", &oscar);
}

OscarDownAnalysis::OscarDownAnalysis(DetectorAnalysis::Inputs in, Oscar &s)
    : OscarAnalysis(in, s, ConfManager::getInstance().oscarDown)
{
    if (oscarConf.enable_treeFill)
        input.tree->Branch("OscarDown", &oscar);
}

bool OscarUpAnalysis::AllocateHistos()
{
    OscarHistograms &h = oscarHistograms;
    h.dir              = input.outFile.mkdir("OscarUp", "Oscar Up angles histograms");
    h.dir->cd();
    if (!OscarAnalysis::AllocateHistos())
        return false;
    return true;
}

bool OscarDownAnalysis::AllocateHistos()
{
    OscarHistograms &h = oscarHistograms;
    h.dir              = input.outFile.mkdir("OscarDown", "Oscar Down angles histograms");
    h.dir->cd();
    if (!OscarAnalysis::AllocateHistos())
        return false;
    return true;
}

void OscarAnalysis::BuildEvents()
{
    fOscarEvents.clear();
    for (unsigned int i = 0; i < fOscarHits.size(); i++)
    {
        for (unsigned int j = i + 1; j < fOscarHits.size(); j++)
        {
            if (fOscarHits[i].layer->type == fOscarHits[j].layer->type)
            {
                continue; // if same type, skip, we want dE-E pairs
            }
            OscarHit &dEHit = fOscarHits[i].layer->type == OscarLut::OscarLayer::DELTA_E
                                  ? fOscarHits[i]
                                  : fOscarHits[j];
            OscarHit &EHit  = fOscarHits[i].layer->type == OscarLut::OscarLayer::E_RES
                                  ? fOscarHits[i]
                                  : fOscarHits[j];

            auto *t = oscarConf.getOscarTelescope(dEHit.layer->layerID, EHit.layer->layerID);
            if (!t)
            {
                // std::cerr << "Warning: No telescope defined for dE layerID: "
                //           << dEHit.layer->layerID << " and E layerID: " << EHit.layer->layerID
                //           << std::endl;
                continue; // if no telescope is defined for this combination, skip
            }
            fOscarEvents.emplace_back(&EHit, &dEHit, t);

            fOscarEvents.back().time =
                unsignedDiff<double, ULong64_t>(EHit.time, fFirstTS) * oscarConf.time_unit +
                EHit.cfd_time * oscarConf.cfd_unit + t->timeOffset * oscarConf.time_unit;

            auto dt = unsignedDiff<double, ULong64_t>(EHit.time, dEHit.time) * oscarConf.time_unit +
                      (EHit.cfd_time - dEHit.cfd_time) * oscarConf.cfd_unit;
            fOscarEvents.back().dT = dt;
        }
    }

    for (auto &ev : fOscarEvents)
    {
        if (oscarConf.global_dE_E_Cuts)
        {
            // oscarConf.idParticleFrom_dE_E(hitTmp.ESi + hitTmp.dESi, hitTmp.dESi);
            ev.ion = oscarConf.idParticleFrom_dE_E(ev.hit_E->e + ev.hit_dE->e, ev.hit_dE->e);
            throw std::runtime_error(
                "OscarAnalysis::BuildEvents: global_dE_E_Cuts is not implemented yet, gates should "
                "be set per telescope, not globally");
        }
        else
        {
            // ev.ion =
            // oscarConf.idParticleFrom_dE_E(ev.hit_E->e, ev.hit_dE->e,
            // ev.telescope->telescopeID);
            // we are using uncalibrated energy for particle ID in case of later change in
            // calibration
            ev.ion = oscarConf.idParticleFrom_dE_E(ev.hit_E->e_raw, ev.hit_dE->e_raw,
                                                   ev.telescope->telescopeID);
            // oscarReader.OSCAREnergy->At(id_E), oscarReader.OSCAREnergy->At(id_dE), pad, strip);
        }
        // set position with selector corrections
        ev.pos.SetXYZ(ev.telescope->pos_x + oscarConf.xShift,
                      ev.telescope->pos_y + oscarConf.yShift,
                      ev.telescope->pos_z + oscarConf.zShift);
        // another corrections?
        ev.pos.RotateZ(oscarConf.phi);
        ev.pos.RotateX(oscarConf.thetaX);
        ev.pos.RotateY(oscarConf.thetaY);

        // ????
        ev.pos.RotateY(oscarConf.angle);
    }
}

void OscarAnalysis::VetoHits()
{

    bool move_pop;
    for (int i = 0; i < static_cast<int>(fOscarHits.size()); i++)
    {
        move_pop  = false;
        auto &hit = fOscarHits[i];
        for (const auto &it : oscarConf.brokenChannels)
        {
            if (static_cast<int>(hit.layer->board) == it.first &&
                static_cast<int>(hit.layer->channel) == it.second)
            {
                move_pop = true;
                break;
            }
        }
        // check threshold
        if (move_pop || !(hit.e > hit.layer->thr_low && hit.e < hit.layer->thr_high))
        {
            if (i + 1 != static_cast<int>(fOscarHits.size()))
            {
                fOscarHits[i] = std::move(fOscarHits.back());
            }
            fOscarHits.pop_back();
            // index i now has a new element, needs to be re-checked
            --i;
            continue;
        }
    }
}

bool OscarAnalysis::AllocateHistos()
{
    OscarHistograms &h = oscarHistograms;
    if (!DetectorAnalysis::AllocateHistos())
        return false;

    // Setup deadlayer energy loss
    std::string sTemp = conf.replay.conf_path + "/EnergyLoss/Target-" +
                        oscarConf.deadlayerMaterial + "_Ion-" + std::to_string(oscarConf.ion.A) +
                        "-" + std::to_string(oscarConf.ion.Z) + ".txt";
    if (settings.verbosity_level > 0)
    {
        std::cout << "Oscar dead layer energy loss: " << sTemp
                  << " - is present: " << FileIsPresent(sTemp) << std::endl;
    }
    if (FileIsPresent(sTemp))
    {
        oscarDeadlayerELoss = {true, EnergyLoss(sTemp, "SRIM", 100)};
    }
    else
    {
        std::runtime_error("Oscar dead layer energy loss file not found: " + sTemp);
    }

    // Find Oscar normal direction
    oscarNormalDir = oscarConf.getNormalDir();

    // Small variations from lookup table
    oscarNormalDir.RotateZ(oscarConf.phi);
    oscarNormalDir.RotateX(oscarConf.thetaX);
    oscarNormalDir.RotateY(oscarConf.thetaY);

    // Major variation
    // ????
    oscarNormalDir.RotateY(oscarConf.angle);

    // Initialize all histos
    Initialize<TH1F>(
        h.h_mult, new TH1F("h_mult", "OSCAR multiplicity;Multiplicity;Counts", 10, 0, 10), h.ptrs);

    Initialize<TH2F>(h.m_Energy_TS,
                     new TH2F("m_Energy_TS", "OSCAR TimeStamp vs energy;Time [s];Energy [MeV]",
                              4320, 0, 12 * 3600, 2000, 0, 200),
                     h.ptrs);

    /*  Initialize(h.m_Energy_Time , new TH2F("m_Energy_Time",
                    "OSCAR time vs energy;Time [s];Energy [MeV]",
                    4320, 0, 12*3600,
                    2000, 0, 200), h.ptrs);
    */
    Initialize<TH2F>(h.m_Theta_Phi,
                     new TH2F("m_Theta_Phi", "OSCAR Theta vs Phi;Theta [deg];Phi [deg]", 90, 0, 90,
                              120, -60, 60),
                     h.ptrs);

    Initialize<TH2F>(
        h.m_layout_XY,
        new TH2F("m_layout_XY", "OSCAR X vs Y;X[mm];Y[mm]", 400, -200, 200, 400, -200, 200),
        h.ptrs);

    Initialize<TH2F>(
        h.m_layout_XZ,
        new TH2F("m_layout_XZ", "OSCAR X vs Z;X[mm];Z[mm]", 400, -200, 200, 400, -200, 200),
        h.ptrs);

    Initialize<TH2F>(
        h.m_layout_YZ,
        new TH2F("m_layout_YZ", "OSCAR Y vs Z;Y[mm];Z[mm]", 400, -200, 200, 400, -200, 200),
        h.ptrs);

    Initialize<TH2F>(
        h.m_Energy_Theta,
        new TH2F("m_Energy_Theta", "OSCAR theta vs energy;Theta [deg]; Energy [MeV]", 90, 0, 90,
                 static_cast<int>(oscarConf.oscarEmax / UNITS::MeV / oscarConf.oscarbinWidth), 0,
                 oscarConf.oscarEmax / UNITS::MeV),
        h.ptrs);

    Initialize<TH2F>(
        h.m_Energy_oscarID,
        new TH2F("m_Energy_oscarID", "OSCAR energy vs oscarID;OSCAR segment; Energy [MeV]", 100, 0,
                 100,
                 static_cast<int>(oscarConf.oscarEmax / UNITS::MeV / oscarConf.oscarbinWidth) +
                     static_cast<int>(10 / oscarConf.oscarbinWidth),
                 -10, oscarConf.oscarEmax / UNITS::MeV),
        h.ptrs);
    SetupLabelsForChannelID(oscarConf, h.m_Energy_oscarID);

    Initialize<TH2F>(h.m_Energy_oscarID_uncal,
                     new TH2F("m_Energy_oscarID_uncal",
                              "OSCAR energy vs oscarID not "
                              "calibrated;OSCAR segment; Energy channel;",
                              100, 0, 100, 2000, 0, 20000),
                     h.ptrs);
    SetupLabelsForChannelID(oscarConf, h.m_Energy_oscarID_uncal);

    Initialize<TH2F>(h.m_oscarBrdCh_oscarBrdCh,
                     new TH2F("m_oscarBrdCh_oscarBrdCh",
                              "OSCAR channel correlation matrix "
                              "OSCAR id;OSCAR id; Counts;",
                              100, 0, 100, 100, 0, 100),
                     h.ptrs);
    SetupLabelsForChannelID(oscarConf, h.m_oscarBrdCh_oscarBrdCh, true);

    Initialize<TH2F>(h.m_oscarBrdCh_oscarBrdCh_withcond,
                     new TH2F("m_oscarBrdCh_oscarBrdCh_withcond",
                              "OSCAR channel correlation matrix with front+back condition"
                              "OSCAR id;OSCAR id; Counts;",
                              100, 0, 100, 100, 0, 100),
                     h.ptrs);
    SetupLabelsForChannelID(oscarConf, h.m_oscarBrdCh_oscarBrdCh_withcond, true);

    Initialize<TH1F>(
        h.h_TimeDiff,
        new TH1F("h_TimeDiff", "OSCAR time difference;Time [10 ns];Counts/1 ns", 3300, -30, 300),
        h.ptrs);

    Initialize<TH1F>(
        h.h_Ex,
        new TH1F("h_Ex", "OSCAR excitation energy;Excitation Energy [MeV];Counts/0.1 MeV",
                 static_cast<int>(oscarConf.oscarExmax / UNITS::MeV / oscarConf.oscarExbinWidth) +
                     static_cast<int>(10 / oscarConf.oscarExbinWidth),
                 -10, oscarConf.oscarExmax / UNITS::MeV),
        h.ptrs);

    Initialize<TH2F>(
        h.m_dE_E,
        new TH2F("m_dE_E", "OSCAR dE vs (E+dE);E+dE [MeV];Delta E [MeV]",
                 static_cast<int>(oscarConf.oscarEmax / UNITS::MeV / oscarConf.oscarbinWidth) +
                     static_cast<int>(10 / oscarConf.oscarbinWidth),
                 0, oscarConf.oscarEmax / UNITS::MeV,
                 static_cast<int>(oscarConf.oscarEmax / UNITS::MeV / oscarConf.oscarbinWidth) +
                     static_cast<int>(10 / oscarConf.oscarbinWidth),
                 0, oscarConf.oscarEmax / UNITS::MeV),
        h.ptrs);

    if (oscarConf.dE_E_Histos)
    {
        h.EplusdEHistograms.dir =
            h.dir->mkdir("Etot_dE", "Corrected total particle energy vs dE histograms");
        h.EplusdEHistograms.dir->cd();

        for (const auto *telescope : oscarConf.getOscarTelescopes())
        {
            const auto        telescopeID   = telescope->telescopeID;
            const std::string histogramName = Form("m_Etot_dE_tel_%i", telescopeID);
            InitializeLazy<TH2F>(h.EplusdEHistograms.m_EdE_telescope[telescopeID], histogramName,
                                 [histogramName, telescopeID]() {
                                     return new TH2F(
                                         histogramName.c_str(),
                                         Form("OSCAR dE vs corrected (E+dE) - telescope "
                                              "%i;Corrected particle energy [MeV];Delta E [MeV]",
                                              telescopeID),
                                         1600, 0, 80, 1600, 0, 80);
                                 },
                                 h.ptrs, {"m_Etot_dE"});
        }

        h.EdEHistograms.dir = h.dir->mkdir("EdE", "Residual-E versus dE histograms");
        h.EdEHistograms.dir->cd();

        for (const auto *telescope : oscarConf.getOscarTelescopes())
        {
            const auto        telescopeID   = telescope->telescopeID;
            const std::string histogramName = Form("m_EdE_tel_%i", telescopeID);
            InitializeLazy<TH2F>(h.EdEHistograms.m_EdE_telescope[telescopeID], histogramName,
                                 [histogramName, telescopeID]() {
                                     return new TH2F(histogramName.c_str(),
                                                     Form("OSCAR residual E vs dE - telescope "
                                                          "%i;Residual E [MeV];Delta E [MeV]",
                                                          telescopeID),
                                                     1600, 0, 80, 1600, 0, 80);
                                 },
                                 h.ptrs, {"m_dE_E", "m_dE_E_residual"});
        }

        h.EdEHistograms_raw.dir = h.dir->mkdir("EdE_raw", "EdE histograms");
        h.EdEHistograms_raw.dir->cd();

        for (const auto *telescope : oscarConf.getOscarTelescopes())
        {
            const auto        telescopeID   = telescope->telescopeID;
            const std::string histogramName = Form("EdE_raw_tel_%i", telescopeID);
            InitializeLazy<TH2F>(h.EdEHistograms_raw.m_EdE_telescope[telescopeID], histogramName,
                                 [histogramName, telescopeID]() {
                                     return new TH2F(
                                         histogramName.c_str(),
                                         Form("OSCAR dE vs E - telescope %i raw E;Energy "
                                              "[channels];Delta E [channels]",
                                              telescopeID),
                                         1600, 0, 16000, 1600, 0, 16000);
                                 },
                                 h.ptrs, {"m_dE_E_raw"});
        }

        // parent_dir->cd();

        h.ExHistograms.dir = h.dir->mkdir("Ex", "Ex histograms");
        h.ExHistograms.dir->cd();

        for (const auto *telescope : oscarConf.getOscarTelescopes())
        {
            const auto        telescopeID   = telescope->telescopeID;
            const std::string histogramName = Form("h_Ex_tel_%i", telescopeID);
            InitializeLazy<TH1F>(h.ExHistograms.h_Ex_telescope[telescopeID], histogramName,
                                 [histogramName, telescopeID]() {
                                     return new TH1F(
                                         histogramName.c_str(),
                                         Form("OSCAR excitation energy - telescope "
                                              "%i;Excitation Energy [MeV];Counts/0.05 MeV",
                                              telescopeID),
                                         1800, -10, 80);
                                 },
                                 h.ptrs, {"h_tEx"});
        }

        h.timeEvodTHistograms_tel.dir =
            h.dir->mkdir("dT_timeEvo", "dT time evo histograms for telescopes");
        h.timeEvodTHistograms_tel.dir->cd();

        for (const auto *telescope : oscarConf.getOscarTelescopes())
        {
            const auto        telescopeID   = telescope->telescopeID;
            const std::string histogramName = Form("h_timeEvo_dT_tel_%i", telescopeID);
            InitializeLazy<TH2F>(
                h.timeEvodTHistograms_tel.h_timeEvodT_telescope[telescopeID], histogramName,
                [histogramName, telescopeID]() {
                    return new TH2F(histogramName.c_str(),
                                    Form("OSCAR time evo of dT between E and dE - telescope "
                                         "%i; TS [s]; dT [ns]",
                                         telescopeID),
                                    500, 0, 15000, 100, -50, 50);
                },
                h.ptrs, {"h_timeEvo_dT"});
        }

        h.dTHistograms_tel.dir = h.dir->mkdir("dT_tel", "dT histograms for telescopes");
        h.dTHistograms_tel.dir->cd();

        for (const auto *telescope : oscarConf.getOscarTelescopes())
        {
            const auto        telescopeID   = telescope->telescopeID;
            const std::string histogramName = Form("h_dT_tel_%i", telescopeID);
            InitializeLazy<TH1F>(h.dTHistograms_tel.h_dT_telescope[telescopeID], histogramName,
                                 [histogramName, telescopeID]() {
                                     return new TH1F(histogramName.c_str(),
                                                     Form("OSCAR dT between E and dE - telescope "
                                                          "%i; dT [ns];Counts/10 ns",
                                                          telescopeID),
                                                     400, -200, 200);
                                 },
                                 h.ptrs, {"h_tdT"});
        }

        h.dTHistograms_strip.dir = h.dir->mkdir("dT_strip", "dT histograms for strips");
        h.dTHistograms_strip.dir->cd();

        for (const auto *strip : oscarConf.getOscarStrips())
        {
            const auto        stripID       = strip->layerID;
            const std::string histogramName = Form("h_dT_strip_%i", stripID);
            InitializeLazy<TH1F>(h.dTHistograms_strip.h_dT_telescope[stripID], histogramName,
                                 [histogramName, stripID]() {
                                     return new TH1F(histogramName.c_str(),
                                                     Form("OSCAR dT between dE and firstTS "
                                                          "%i; dT [ns];Counts/10 ns",
                                                          stripID),
                                                     400, -200, 200);
                                 },
                                 h.ptrs, {"h_sdT"});
        }

        h.dTHistograms_pad.dir = h.dir->mkdir("dT_pad", "dT histograms for pads");
        h.dTHistograms_pad.dir->cd();

        for (const auto *pad : oscarConf.getOscarPads())
        {
            const auto        padID         = pad->layerID;
            const std::string histogramName = Form("h_dT_pad_%i", padID);
            InitializeLazy<TH1F>(h.dTHistograms_pad.h_dT_telescope[padID], histogramName,
                                 [histogramName, padID]() {
                                     return new TH1F(histogramName.c_str(),
                                                     Form("OSCAR dT between E and first TS"
                                                          "%i; dT [ns];Counts/10 ns",
                                                          padID),
                                                     400, -200, 200);
                                 },
                                 h.ptrs, {"h_pdT"});
        }
    }

    for (const auto &reaction : input.reactions)
    {
        OscarHistograms::ReactionHistograms &reactionHistos = h.reactionHistos[{reaction.first}];
        std::string                          histoName =
            "_A" + std::to_string(reaction.first.A) + "Z" + std::to_string(reaction.first.Z);
        std::string histoTitle =
            " A=" + std::to_string(reaction.first.A) + " Z=" + std::to_string(reaction.first.Z);

        reactionHistos.dir = h.dir->mkdir(
            Form("Reaction%s", histoName.c_str()),
            Form("Gamma spectra with cuts on the identified ion: %s", histoTitle.c_str()));
        reactionHistos.dir->cd();

        Initialize<TH1F>(reactionHistos.h_Ex,
                         new TH1F("h_Ex",
                                  "OSCAR excitation energy;Excitation Energy "
                                  "[MeV];Counts/0.1 MeV",
                                  static_cast<int>(oscarConf.oscarExmax / UNITS::MeV /
                                                   oscarConf.oscarExbinWidth) +
                                      static_cast<int>(10 / oscarConf.oscarExbinWidth),
                                  -10, oscarConf.oscarExmax / UNITS::MeV),
                         reactionHistos.ptrs);

        Initialize<TH2F>(
            reactionHistos.m_Energy_Theta,
            new TH2F("m_Energy_Theta", "OSCAR theta vs energy;Theta [deg]; Energy [MeV]", 180, 0,
                     180,
                     static_cast<int>(oscarConf.oscarEmax / UNITS::MeV / oscarConf.oscarbinWidth),
                     0, oscarConf.oscarEmax / UNITS::MeV),
            reactionHistos.ptrs);
    }

    for (const auto &reaction : input.nBodyReactions)
    {
        OscarHistograms::ReactionHistograms &reactionHistos = h.reactionHistos[reaction.first];
        std::string                          histoName;
        std::string                          histoTitle;

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

        Initialize<TH1F>(reactionHistos.h_Ex,
                         new TH1F("h_Ex",
                                  "OSCAR excitation energy;Excitation Energy "
                                  "[MeV];Counts/0.1 MeV",
                                  static_cast<int>(oscarConf.oscarExmax / UNITS::MeV /
                                                   oscarConf.oscarExbinWidth) +
                                      static_cast<int>(10 / oscarConf.oscarExbinWidth),
                                  -10, oscarConf.oscarExmax / UNITS::MeV),
                         reactionHistos.ptrs);

        Initialize<TH2F>(
            reactionHistos.m_Energy_Theta,
            new TH2F("m_Energy_Theta", "OSCAR theta vs energy;Theta [deg]; Energy [MeV]", 180, 0,
                     180,
                     static_cast<int>(oscarConf.oscarEmax / UNITS::MeV / oscarConf.oscarbinWidth),
                     0, oscarConf.oscarEmax / UNITS::MeV),
            reactionHistos.ptrs);
    }

    return true;
}

void OscarAnalysis::Analyze()
{
    DetectorAnalysis::Analyze();
    int nOscar = **oscarReader.nbOSCAR;
    if (nOscar <= 1)
    {
        return;
    }

    const auto currentEntry = input.fReader.GetCurrentEntry();
    for (TLeaf *leaf : {oscarReader.TSOSCAR_Hit, oscarReader.OSCARChannel, oscarReader.OSCARBoard,
                        oscarReader.OSCAREnergy, oscarReader.OSCARTime})
    {
        if (leaf->GetBranch()->GetEntry(currentEntry) < 0)
            throw std::runtime_error("Failed to load an OSCAR array branch");
    }

    if (settings.verbosity_level >= 8)
    {
        std::cout << "\n------------------------------------------------------"
                     "-------------------------------\n";
        std::cout << "NEW OSCAR EVENT\n " << std::endl;
    }

    if (settings.verbosity_level >= 10)
    {
        std::cout << "INPUT:\n";
        std::cout << "nbOSCAR " << nOscar << "\n";
        std::cout << "TSOSCAR " << (**oscarReader.TSOSCAR) << "\n";
        std::cout << std::endl;
    }

    fOscarHits.clear();
    // create and fill oscar hit vector;
    for (unsigned int i{0}; i < static_cast<unsigned int>(nOscar); ++i)
    {
        unsigned short b  = static_cast<unsigned short>(oscarReader.OSCARBoard->GetValue(i));
        unsigned short ch = static_cast<unsigned short>(oscarReader.OSCARChannel->GetValue(i));
        auto          *l  = oscarConf.getOscarLayer(b, ch);
        if (!l)
        {
            continue;
        }
        fOscarHits.emplace_back(l);

        // check if layer is present in LUT, if not skip (its another detector)
        // calibrate energy
        auto &this_hit = fOscarHits.back();
        this_hit.e_raw = oscarReader.OSCAREnergy->GetValue(i);
        this_hit.e     = oscarConf.getEnergy(this_hit.layer, this_hit.e_raw);
        auto ts_offset = oscarConf.getTSOffset(this_hit.layer);

        if (ts_offset >= 0)
        {
            this_hit.time = oscarReader.TSOSCAR_Hit->GetTypedValue<ULong64_t>(i) +
                            static_cast<ULong64_t>(ts_offset);
        }
        else
        {
            this_hit.time = oscarReader.TSOSCAR_Hit->GetTypedValue<ULong64_t>(i) -
                            static_cast<ULong64_t>(-ts_offset);
        }
        this_hit.cfd_time = oscarReader.OSCARTime->GetValue(i);
    }

    // Fill hit-correlation matrix
    for (unsigned int i{0}; i < fOscarHits.size(); i++)
    {
        for (unsigned int j{i + 1}; j < fOscarHits.size(); j++)
        {
            Fill(oscarHistograms.m_oscarBrdCh_oscarBrdCh, fOscarHits[i].layer->selector_id,
                 fOscarHits[j].layer->selector_id);
            Fill(oscarHistograms.m_oscarBrdCh_oscarBrdCh, fOscarHits[j].layer->selector_id,
                 fOscarHits[i].layer->selector_id);
        }
    }

    // Fill raw histograms before any condition
    for (auto &hit : fOscarHits)
    {
        Fill(oscarHistograms.m_Energy_oscarID, hit.layer->selector_id, hit.e);
        Fill(oscarHistograms.m_Energy_oscarID_uncal, hit.layer->selector_id, hit.e_raw);
    }

    // apply energy thresholds and broken channel conditions - remove hits that do not satisfy these
    // conditions from the analysis

    this->VetoHits();
    if (fOscarHits.size() < 2)
    {
        return;
    }
    std::sort(fOscarHits.begin(), fOscarHits.end(),
              [](const OscarHit &a, const OscarHit &b) { return a.time < b.time; });

    if (settings.verbosity_level >= 8)
    {
        std::cout << "nbOSCAR after veto: " << fOscarHits.size() << std::endl;
    }

    fFirstTS = fOscarHits.front().time;
    // remove hits outside of time gate
    //  time is counted from the first event - to make sure we are in prompt coincidence
    for (unsigned int i = 1; i < fOscarHits.size(); i++)
    {
        auto time =
            unsignedDiff<double, ULong64_t>(fOscarHits[i].time, fFirstTS) * oscarConf.time_unit +
            (fOscarHits[i].cfd_time - fOscarHits.front().cfd_time) * oscarConf.cfd_unit;
        // fOscarHits[i].time = time;
        Fill(oscarHistograms.h_TimeDiff, time / (10 * UNITS::ns));
        if (!(time / (10 * UNITS::ns) >= windowLeft && time / (10 * UNITS::ns) < windowRight))
        {
            fOscarHits.erase(fOscarHits.begin() + i);
            i--;
            // in theory, we can remove all events until the end once this condtion is met, no?
            // fOscarHits.erase(fOscarHits.begin() + i, fOscarHits.end());
            // break;
        }
    }

    if (settings.verbosity_level >= 8)
    {
        std::cout << "Number of Oscar hits after time condition " << fOscarHits.size() << std::endl;
    }

    // for (const auto &hit : fOscarHits)
    // {
    //     if (hit.layer->type == OscarLut::OscarLayer::DELTA_E)
    //     {
    //         // if (hit.time == fFirstTS)
    //         // {
    //         //     continue;
    //         // }
    //         auto stripHisto_dT =
    //             oscarHistograms.dTHistograms_strip.h_dT_telescope.find(hit.layer->layerID);
    //         if (stripHisto_dT != oscarHistograms.dTHistograms_strip.h_dT_telescope.end())
    //         {
    //             Fill(stripHisto_dT->second,
    //                  (unsignedDiff<double, ULong64_t>(hit.time, fFirstTS) * oscarConf.time_unit +
    //                   hit.cfd_time * oscarConf.cfd_unit) /
    //                      (UNITS::ns));
    //         }
    //     }
    //     else
    //     {
    //         auto padHisto_dT =
    //             oscarHistograms.dTHistograms_pad.h_dT_telescope.find(hit.layer->layerID);
    //         if (padHisto_dT != oscarHistograms.dTHistograms_pad.h_dT_telescope.end())
    //         {
    //             Fill(padHisto_dT->second,
    //                  (unsignedDiff<double, ULong64_t>(hit.time, fFirstTS) * oscarConf.time_unit +
    //                   hit.cfd_time * oscarConf.cfd_unit) /
    //                      (UNITS::ns));
    //         }
    //     }
    // }

    if (fOscarHits.size() < 2)
    {
        return;
    }

    // oscar time gate
    if (settings.verbosity_level >= 8)
    {
        std::cout << "Number of Oscar hits after time condition " << fOscarHits.size() << std::endl;
    }

    // build oscar events based on time condition
    this->BuildEvents();

    if (settings.verbosity_level >= 8)
    {
        std::cout << "OscarEvent list size: " << fOscarEvents.size() << std::endl;
        if (settings.verbosity_level >= 10)
        {
            for (const auto &event : fOscarEvents)
            {
                std::cout << "id telescope: " << event.telescope->telescopeID << "\n";
                std::cout << "id dE: " << event.hit_dE->layer->selector_id << "\n";
                std::cout << "Id  E: " << event.hit_E->layer->selector_id << "\n";

                std::cout << "\nTime: " << event.hit_E->time << "\n";
                std::cout << "dE : " << event.hit_dE->e << "\n";
                std::cout << "E  : " << event.hit_E->e << "\n";
                std::cout << "Pos:   " << event.pos.X() << " " << event.pos.Y() << " "
                          << event.pos.Z() << "\n";
            }
            std::cout << std::endl;
        }
    }

    if (fOscarEvents.size() != 1)
    {
        return;
    }

    {
        static long bad_counter  = 0;
        static long good_counter = 0;
        if (fOscarEvents.front().hit_E->e + fOscarEvents.front().hit_dE->e > 100)
        {
            bad_counter++;
            // fOscarEvents.clear();
            if (settings.verbosity_level >= 8)
            {
                std::cout << "Bad event detected, clearing events. Bad counter: " << bad_counter
                          << "/" << (bad_counter + good_counter) << " ratio: "
                          << (double)bad_counter / (double)(bad_counter + good_counter)
                          << std::endl;
            }
            return;
        }
        else
        {
            good_counter++;
        }
    }

    oscar.resize(fOscarEvents.size());
    oscar.TS = fFirstTS * (oscarConf.time_unit / (10 * UNITS::ns));

    for (const auto &ev : fOscarEvents)
    {
        // time evo
        {
            static ULong64_t timeEvoFirstTS = fFirstTS;
            auto             telescope_timeevoDT =
                oscarHistograms.timeEvodTHistograms_tel.h_timeEvodT_telescope.find(
                    ev.telescope->telescopeID);
            if (telescope_timeevoDT !=
                oscarHistograms.timeEvodTHistograms_tel.h_timeEvodT_telescope.end())
            {
                Fill(telescope_timeevoDT->second,
                     (unsignedDiff<double, ULong64_t>(ev.hit_E->time, timeEvoFirstTS) *
                          oscarConf.time_unit +
                      ev.hit_E->cfd_time * oscarConf.cfd_unit) /
                         (UNITS::s),
                     (unsignedDiff<double, ULong64_t>(ev.hit_E->time, ev.hit_dE->time) *
                          oscarConf.time_unit +
                      (ev.hit_E->cfd_time - ev.hit_dE->cfd_time) * oscarConf.cfd_unit) /
                         (UNITS::ns));
            }
        }

        Fill(oscarHistograms.m_oscarBrdCh_oscarBrdCh_withcond, ev.hit_dE->layer->selector_id,
             ev.hit_E->layer->selector_id);
        Fill(oscarHistograms.m_oscarBrdCh_oscarBrdCh_withcond, ev.hit_E->layer->selector_id,
             ev.hit_dE->layer->selector_id);

        auto telescopeHisto_raw =
            oscarHistograms.EdEHistograms_raw.m_EdE_telescope.find(ev.telescope->telescopeID);
        if (telescopeHisto_raw != oscarHistograms.EdEHistograms_raw.m_EdE_telescope.end())
        {
            Fill(telescopeHisto_raw->second, ev.hit_E->e_raw, ev.hit_dE->e_raw);
        }

        auto telescopeHisto_dT =
            oscarHistograms.dTHistograms_tel.h_dT_telescope.find(ev.telescope->telescopeID);
        if (telescopeHisto_dT != oscarHistograms.dTHistograms_tel.h_dT_telescope.end())
        {
            Fill(telescopeHisto_dT->second, ev.dT / (UNITS::ns));
        }

        auto stripHisto_dT =
            oscarHistograms.dTHistograms_strip.h_dT_telescope.find(ev.telescope->layer_dE->layerID);
        if (stripHisto_dT != oscarHistograms.dTHistograms_strip.h_dT_telescope.end())
        {
            Fill(stripHisto_dT->second,
                 (unsignedDiff<double, ULong64_t>(ev.hit_dE->time, fFirstTS) * oscarConf.time_unit +
                  ev.hit_dE->cfd_time * oscarConf.cfd_unit) /
                     (UNITS::ns));
        }

        auto padHisto_dT =
            oscarHistograms.dTHistograms_pad.h_dT_telescope.find(ev.telescope->layer_Eres->layerID);
        if (padHisto_dT != oscarHistograms.dTHistograms_pad.h_dT_telescope.end())
        {
            Fill(padHisto_dT->second,
                 (unsignedDiff<double, ULong64_t>(ev.hit_E->time, fFirstTS) * oscarConf.time_unit +
                  ev.hit_E->cfd_time * oscarConf.cfd_unit) /
                     (UNITS::ns));
        }
    }

    for (unsigned int i{0}; i < fOscarEvents.size(); i++)
    {
        oscar.IdTelescope[i] = fOscarEvents[i].telescope->telescopeID;
        oscar.IddE[i]        = fOscarEvents[i].hit_dE->layer->selector_id;
        oscar.IdE[i]         = fOscarEvents[i].hit_E->layer->selector_id;
        oscar.Time[i]        = fOscarEvents[i].time;
        oscar.dESi[i]        = fOscarEvents[i].hit_dE->e;
        oscar.ESi[i]         = fOscarEvents[i].hit_E->e;
        oscar.Pos[i]         = fOscarEvents[i].pos;

        oscar.A[i] = fOscarEvents[i].ion.A;
        oscar.Z[i] = fOscarEvents[i].ion.Z;
        if (oscar.A[i] != 0)
            oscar.identified[i] = true;

        // TODO: if E layer only, correct for energy loss in Delta E
        // if(oscarConf.useElayerOnly)
        //
        //

        ReactionFragment::Ion ion;
        if (oscar.identified[i])
            ion = ReactionFragment::Ion{oscar.A[i], oscar.Z[i]};
        else
            ion = oscarConf.ion;
        auto reactionsIt = input.reactions.find(ion);

        if (reactionsIt == input.reactions.end())
            throw std::runtime_error(Form("Ion %i %i in oscar not found in reactions of interest! "
                                          "Check the conf file\n",
                                          ion.A, ion.Z));

        if (oscarConf.EkFromBetaAvg)
        {
            oscar.E[i] = 0.5 * oscarConf.betaAvgMag * oscarConf.betaAvgMag;
            oscar.E[i] *= ion.A * UNITS::PHYSICS::amu_c2;
        }
        else
        {
            if (oscarDeadlayerELoss.first)
            {
                static const double silicon_density =
                    2321. /* mg/cm^3 */ / 10000. /* mg/cm^2/um */; // just multiply with thickness
                                                                   // in um to get desired mg/cm^2
                oscar.E[i] = oscarDeadlayerELoss.second.EvaluateInitialEnergyWithDensity(
                    oscar.ESi[i], fOscarEvents[i].telescope->thick_dead_Eres * silicon_density,
                    oscar.Pos[i].Angle(oscarNormalDir));

                oscar.E[i] = oscarDeadlayerELoss.second.EvaluateInitialEnergyWithDensity(
                    oscar.E[i] + oscar.dESi[i],
                    fOscarEvents[i].telescope->thick_dead_Eres * silicon_density,
                    oscar.Pos[i].Angle(oscarNormalDir));
            }
            else
                oscar.E[i] = oscar.ESi[i] + oscar.dESi[i];
        }

        reactionsIt->second.GetFixedFragment().Set_Ek(oscar.E[i]);
        reactionsIt->second.GetFixedFragment().Set_Dir(oscar.Pos[i]);

        TLorentzVector p = reactionsIt->second.GetFixedFragment().Get_P();
        EvalInTarget(p, ion);

        oscar.E[i] = p.E() - p.M();

        try
        {
            oscar.Ex[i] = static_cast<double>(
                reactionsIt->second.Set_Ek_Theta_Phi(oscar.E[i], p.Theta(), p.Phi()));
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what();
            return;
        }

        oscar.p3[i] = reactionsIt->second.GetFixedFragment().Get_P(); // Measured by oscar
        oscar.p4[i] = reactionsIt->second.GetFreeFragment().Get_P();  // Binary partner
    }

    for (std::size_t i = 0; i < oscar.E.size(); ++i)
    {
        auto telescopeHistoTotal =
            oscarHistograms.EplusdEHistograms.m_EdE_telescope.find(oscar.IdTelescope[i]);
        if (telescopeHistoTotal != oscarHistograms.EplusdEHistograms.m_EdE_telescope.end())
        {
            Fill(telescopeHistoTotal->second, oscar.E[i], oscar.dESi[i]);
        }

        auto telescopeHistoResidual =
            oscarHistograms.EdEHistograms.m_EdE_telescope.find(oscar.IdTelescope[i]);
        if (telescopeHistoResidual != oscarHistograms.EdEHistograms.m_EdE_telescope.end())
        {
            Fill(telescopeHistoResidual->second, oscar.ESi[i], oscar.dESi[i]);
        }

        auto telescopeHisto_Ex =
            oscarHistograms.ExHistograms.h_Ex_telescope.find(oscar.IdTelescope[i]);
        if (telescopeHisto_Ex != oscarHistograms.ExHistograms.h_Ex_telescope.end())
        {
            Fill(telescopeHisto_Ex->second, oscar.Ex[i]);
        }
    }

    // Optional hard event gates. When both are enabled, an event must pass
    // both independently configured cut families.
    if (oscarConf.rejectOutsideDEEnergy &&
        std::any_of(oscar.identified.begin(), oscar.identified.end(),
                    [](bool identified) { return !identified; }))
    {
        oscar.clear();
        return;
    }
    bool passesEnergyThetaCuts = true;
    if (oscarConf.rejectOutsideEnergyTheta)
    {
        for (std::size_t i = 0; i < oscar.E.size(); ++i)
        {
            if (!oscarConf.passesEnergyThetaCut(oscar.p3.at(i).Theta(), oscar.E.at(i)))
            {
                passesEnergyThetaCuts = false;
                break;
            }
        }
    }
    if (!passesEnergyThetaCuts)
    {
        oscar.clear();
        return;
    }

    // Retain the ungated per-telescope diagnostic matrices above, but expose
    // only accepted OSCAR events to standard histograms, trees, and subsequent
    // coincidence analyses.
    if (std::any_of(oscar.Ex.begin(), oscar.Ex.end(), [this](double excitationEnergy) {
            return !oscarConf.passesExcitationEnergyGate(excitationEnergy);
        }))
    {
        oscar.clear();
        return;
    }

    if (settings.verbosity_level >= 10)
    {
        for (unsigned int i{0}; i < oscar.E.size(); ++i)
        {
            std::cout << "\nTS\t" << oscar.TS << "\t" << input.firstTS << "\n";
            std::cout << "Time\t" << oscar.Time.at(i) << "\n";
            std::cout << "ID_dE " << oscar.IddE.at(i) << "\tID_E " << oscar.IdE.at(i) << "\n";
            std::cout << "E\t" << oscar.ESi.at(i) << "\t" << oscar.E.at(i) << "\n";
            std::cout << "Ex\t" << oscar.Ex.at(i) << "\n";
            std::cout << "Pos\t( " << oscar.Pos.at(i).X() / UNITS::mm << " "
                      << oscar.Pos.at(i).Y() / UNITS::mm << " " << oscar.Pos.at(i).Z() / UNITS::mm
                      << " )\t";
            std::cout << "( " << oscar.Pos.at(i).Mag() / UNITS::mm << " "
                      << oscar.Pos.at(i).Theta() / UNITS::deg << " "
                      << oscar.Pos.at(i).Phi() / UNITS::deg << " )\n";
            std::cout << "p3\t( " << oscar.p3.at(i).X() / UNITS::mm << " "
                      << oscar.p3.at(i).Y() / UNITS::mm << " " << oscar.p3.at(i).Z() / UNITS::mm
                      << " )\t";
            std::cout << "( " << oscar.p3.at(i).Mag() << " " << oscar.p3.at(i).Theta() / UNITS::deg
                      << " " << oscar.p3.at(i).Phi() / UNITS::deg << " )\n";
            std::cout << "p4\t( " << oscar.p4.at(i).X() / UNITS::mm << " "
                      << oscar.p4.at(i).Y() / UNITS::mm << " " << oscar.p4.at(i).Z() / UNITS::mm
                      << " )\t";
            std::cout << "( " << oscar.p4.at(i).Mag() << " " << oscar.p4.at(i).Theta() / UNITS::deg
                      << " " << oscar.p4.at(i).Phi() / UNITS::deg << " )\n";
            std::cout << "Mass g.s. " << oscar.p4.at(i).Mag() - oscar.Ex.at(i) << std::endl;
        }
    }
}

bool OscarAnalysis::FillHistos()
{
    if (!DetectorAnalysis::FillHistos())
        return false;

    // Filling histograms
    OscarHistograms &h = oscarHistograms;

    Fill(h.h_mult, static_cast<int>(oscar.E.size()));

    for (unsigned int i{0}; i < oscar.E.size(); ++i)
    {
        /*       Fill(h.m_Energy_Time,
                 unsignedDiff<double, unsigned long long>(oscar.Time[i],
           timestampRangeOscar.first)/1.E8, oscar.E[i]);
         */
        Fill(h.m_Energy_TS, unsignedDiff<double, ULong64_t>(oscar.TS, input.firstTS) / 1.e8,
             oscar.ESi[i]);

        Fill(h.m_Theta_Phi, oscar.p3[i].Theta() / UNITS::deg, oscar.p3[i].Phi() / UNITS::deg);

        Fill(h.m_Energy_Theta, oscar.p3[i].Theta() / UNITS::deg, oscar.E[i] / UNITS::MeV);

        Fill(h.m_layout_XY, oscar.Pos[i].X() / UNITS::mm, oscar.Pos[i].Y() / UNITS::mm);

        Fill(h.m_layout_XZ, oscar.Pos[i].X() / UNITS::mm, oscar.Pos[i].Z() / UNITS::mm);

        Fill(h.m_layout_YZ, oscar.Pos[i].Y() / UNITS::mm, oscar.Pos[i].Z() / UNITS::mm);

        Fill(h.h_Ex, oscar.Ex[i]);

        Fill(h.m_dE_E, oscar.ESi[i] + oscar.dESi[i], oscar.dESi[i]);

        // Particle identification histograms
        std::vector<ReactionFragment::Ion> reaction;
        for (unsigned int e{0}; e < oscar.E.size(); ++e)
        {
            if (!oscar.identified.at(e))
            {
                continue;
            }

            ReactionFragment::Ion idIon{oscar.A.at(e), oscar.Z.at(e)};
            reaction.emplace_back(idIon);
        }
        std::sort(reaction.begin(), reaction.end());

        auto reactionHist = h.reactionHistos.find(reaction);
        if (reactionHist != h.reactionHistos.end())
        {
            for (unsigned int s{0}; s < oscar.E.size(); ++s)
            {
                Fill(reactionHist->second.h_Ex, oscar.Ex.at(s));
                Fill(reactionHist->second.m_Energy_Theta, oscar.p3[s].Theta() / UNITS::deg,
                     oscar.E[s]);
            }
        }
    }
    return true;
}

void OscarUpAnalysis::Analyze()
{
    OscarAnalysis::Analyze();
}

void OscarDownAnalysis::Analyze()
{
    OscarAnalysis::Analyze();
}

void OscarAnalysis::Instantiate()
{
    DetectorAnalysis::Instantiate();
    TTreeReader &fReader{input.fReader};

    if (conf.detectorsPresent.OscarUp || conf.detectorsPresent.OscarDown)
    {
        if (input.treeToRead->FindLeaf("nbOSCAR") == nullptr)
            throw std::runtime_error("OSCAR enabled in the config but not found in the tree");

        oscarReader.nbOSCAR = new TTreeReaderValue<Int_t>(fReader, "nbOSCAR");
        oscarReader.TSOSCAR = new TTreeReaderValue<ULong64_t>(fReader, "TSOSCAR");

        oscarReader.TSOSCAR_Hit  = input.treeToRead->FindLeaf("TSOSCAR_Hit");
        oscarReader.OSCARChannel = input.treeToRead->FindLeaf("OSCARChannelId");
        if (oscarReader.OSCARChannel == nullptr)
            oscarReader.OSCARChannel = input.treeToRead->FindLeaf("OSCARChannel");
        oscarReader.OSCARBoard = input.treeToRead->FindLeaf("OSCARBoardId");
        if (oscarReader.OSCARBoard == nullptr)
            oscarReader.OSCARBoard = input.treeToRead->FindLeaf("OSCARBoard");
        oscarReader.OSCAREnergy = input.treeToRead->FindLeaf("OSCAREnergy");
        oscarReader.OSCARTime   = input.treeToRead->FindLeaf("OSCARTime");

        if (oscarReader.TSOSCAR_Hit == nullptr || oscarReader.OSCARChannel == nullptr ||
            oscarReader.OSCARBoard == nullptr || oscarReader.OSCAREnergy == nullptr ||
            oscarReader.OSCARTime == nullptr)
            throw std::runtime_error("One or more OSCAR array leaves were not found in the tree");
    }
}

void OscarAnalysis::Finalize()
{
    DetectorAnalysis::Finalize();
    NormalizeHistos<DetectorConf>(oscarConf);
}
