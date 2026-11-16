#include "GammadetectorEuclidesAnalysis.h"

#include "Lut.h"

GammadetectorEuclidesAnalysis::GammadetectorEuclidesAnalysis(
    Inputs& in, GammadetectorEuclidesConf& gpConf, Gammadetector& g,
    GammadetectorConf& gConf, Euclides& e, Readers& rg,
    EuclidesAnalysis::EuclidesReader& re, GammaDC& gdc,
    GammadetectorEuclidesHistograms& geh)
    : CoincidenceAnalysis(in, g, gConf, e, ConfManager::getInstance().euclides,
                          gpConf, rg, re, geh),
      gammadetector(g), euclides(e),
      euclidesConf(ConfManager::getInstance().euclides),
      gammadetectorEuclidesConf(gpConf), gammadetectorConf(gConf),
      gammaDC(gdc), gammadetectorEuclidesHistograms(geh), euclidesReader(re) {}

bool GammadetectorEuclidesAnalysis::AllocateHistos() {
    if(!CoincidenceAnalysis::AllocateHistos()) return false;

    GammadetectorEuclidesHistograms& h = gammadetectorEuclidesHistograms;
    h.dir->cd();

    Initialize<TH2D>(h.m_TSdiff_TS,
                     new TH2D("m_TSdiff_TS", "m_TSdiff_TS", 12 * 360, 0,
                              12 * 3600,
                              (gammadetectorEuclidesConf.windowRight
                               - gammadetectorEuclidesConf.windowLeft + 200),
                              gammadetectorEuclidesConf.windowLeft - 100,
                              gammadetectorEuclidesConf.windowRight + 100),
                     h.ptrs);

    Initialize<TH2D>(h.m_TSeuclidesmTSgammadetector_euclidesID,
                     new TH2D("m_TSeuclidesmTSgammadetector_euclidesID",
                              "EuclidesTS - GammadetectorTS vs EUCLIDES "
                              "segment; EUCLIDES segment;TS "
                              "difference [10 ns]",
                              6 * Lut::MAXCH, 0, 6 * Lut::MAXCH,
                              (gammadetectorEuclidesConf.windowRight
                               - gammadetectorEuclidesConf.windowLeft + 200)
                                  * 10,
                              gammadetectorEuclidesConf.windowLeft - 500,
                              gammadetectorEuclidesConf.windowRight + 500),
                     h.ptrs);
    SetupLabelsForChannelID<EuclidesConf>(
        conf.euclides, h.m_TSeuclidesmTSgammadetector_euclidesID);

    Initialize<TH1D>(
        h.h_E,
        new TH1D("h_E",
                 Form("E spectrum in coincidence with Euclides;Energy "
                      "[keV];Counts/%d keV",
                      gammadetectorConf.binWidth),
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    Initialize<TH1D>(
        h.h_EDC,
        new TH1D("h_EDCBP",
                 Form("E Doppler corrected spectrum in coincidence with "
                      "Euclides;Energy [keV];Counts/%d keV",
                      gammadetectorConf.binWidth),
                 static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                  / gammadetectorConf.binWidth),
                 0, gammadetectorConf.gammaEmax / UNITS::keV),
        h.ptrs);

    for(const auto& ion : conf.euclides.getListOfParticles()) {
        GammadetectorEuclidesHistograms::IonHistograms ion_histos;
        ion_histos.dir = h.dir->mkdir(
            Form("Ion_gate_Z%d_A%d", ion.first.Z, ion.first.A),
            Form("Gamma spectra with cuts on the identified ion: Z=%d, A=%d",
                 ion.first.Z, ion.first.A));
        ion_histos.dir->cd();

        Initialize<TH1D>(
            ion_histos.h_E,
            new TH1D(
                Form("h_E_ion_%i_%i", ion.first.A, ion.first.Z),
                Form("E spectrum for A,%i and Z,%i;Energy [keV];Counts/%d keV",
                     ion.first.A, ion.first.Z, gammadetectorConf.binWidth),
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);

        Initialize<TH1D>(
            ion_histos.h_EDCBP,
            new TH1D(
                Form("h_DCE_ion_%i_%i", ion.first.A, ion.first.Z),
                Form(
                    "DC spectrum for A,%i and Z,%i;Energy [keV];Counts/%d keV",
                    ion.first.A, ion.first.Z, gammadetectorConf.binWidth),
                static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                 / gammadetectorConf.binWidth),
                0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);

        Initialize<TH2D>(
            ion_histos.m_E_E,
            new TH2D(Form("m_E_E_ion_%i_%i", ion.first.A, ion.first.Z),
                     Form("Gamma-gamma DC spectrum for A,%i and Z,%i;Energy "
                          "[keV];Energy [keV];Counts/%d keV",
                          ion.first.A, ion.first.Z,
                          gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);

        Initialize<TH2D>(
            ion_histos.m_EDCBP_EDCBP,
            new TH2D(Form("m_EDCBP_EDCBP_ion_%i_%i", ion.first.A, ion.first.Z),
                     Form("Gamma-gamma DC spectrum for A,%i and Z,%i;Energy "
                          "[keV];Energy [keV];Counts/%d keV",
                          ion.first.A, ion.first.Z,
                          gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            ion_histos.ptrs);

        h.ionHistos[ion.first] = ion_histos;
    }

    for(const auto& reaction : input.nBodyReactions) {
        GammadetectorEuclidesHistograms::ReactionHistograms& reactionHistos
            = h.reactionHistos[reaction.first];
        std::string histoName;
        std::string histoTitle;

        for(const auto& it : reaction.first) {
            histoName += "_";
            histoName += "A" + std::to_string(it.A);
            histoName += "Z" + std::to_string(it.Z);

            histoTitle += " A=" + std::to_string(it.A);
            histoTitle += " Z=" + std::to_string(it.Z);
        }

        reactionHistos.dir = h.dir->mkdir(
            Form("Reaction%s", histoName.c_str()),
            Form("Gamma spectra with cuts on the identified ion: %s",
                 histoTitle.c_str()));
        reactionHistos.dir->cd();

        Initialize<TH1D>(
            reactionHistos.h_E,
            new TH1D(Form("h_E_ion%s", histoName.c_str()),
                     Form("E spectrum for %s;Energy [keV];Counts/%d keV",
                          histoTitle.c_str(), gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            reactionHistos.ptrs);

        Initialize<TH1D>(
            reactionHistos.h_EDCBP,
            new TH1D(Form("h_EDCBP_ion%s", histoName.c_str()),
                     Form("DC spectrum for %s;Energy DC [keV];Counts/%d keV",
                          histoTitle.c_str(), gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_Ex_E,
            new TH2D(Form("m_Ex_E_ion%s", histoName.c_str()),
                     Form("Excitation energy spectrum vs gamma energy for "
                          "%s;Energy [keV];Excitation energy [MeV]",
                          histoTitle.c_str()),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV, 1000, -10,
                     30),
            reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_Ex_EDCBP,
            new TH2D(Form("m_Ex_DCEBP_ion%s", histoName.c_str()),
                     Form("Excitation energy spectrum vs DC gamma energy for "
                          "%s;Energy DC [keV];Excitation energy [MeV]",
                          histoTitle.c_str()),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV, 1000, -10,
                     30),
            reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_E_E,
            new TH2D(Form("m_E_E_ion%s", histoName.c_str()),
                     Form("Gamma-gamma spectrum for "
                          "%s;Energy [keV];Energy [keV];Counts/%d keV",
                          histoTitle.c_str(), gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_EDCBP_EDCBP,
            new TH2D(Form("m_EDCBP_EDCBP_ion%s", histoName.c_str()),
                     Form("Gamma-gamma DC spectrum for "
                          "%s;DC Energy [keV];DC Energy [keV];Counts/%d keV",
                          histoTitle.c_str(), gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            reactionHistos.ptrs);
    }

    for(const auto& reaction : input.reactions) {
        GammadetectorEuclidesHistograms::ReactionHistograms& reactionHistos
            = h.reactionHistos[{reaction.first}];
        std::string histoName = "_A" + std::to_string(reaction.first.A) + "Z"
                              + std::to_string(reaction.first.Z);
        std::string histoTitle = " A=" + std::to_string(reaction.first.A)
                               + " Z=" + std::to_string(reaction.first.Z);

        reactionHistos.dir = h.dir->mkdir(
            Form("Reaction%s", histoName.c_str()),
            Form("Gamma spectra with cuts on the identified ion: %s",
                 histoTitle.c_str()));
        reactionHistos.dir->cd();

        Initialize<TH1D>(
            reactionHistos.h_E,
            new TH1D(Form("h_E_ionr%s", histoName.c_str()),
                     Form("E spectrum for %s;Energy [keV];Counts/%d keV",
                          histoTitle.c_str(), gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            reactionHistos.ptrs);

        Initialize<TH1D>(
            reactionHistos.h_EDCBP,
            new TH1D(Form("h_EDCBP_ionr%s", histoName.c_str()),
                     Form("DC spectrum for %s;Energy DC [keV];Counts/%d keV",
                          histoTitle.c_str(), gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_Ex_E,
            new TH2D(Form("m_Ex_E_ionr%s", histoName.c_str()),
                     Form("Excitation energy spectrum vs gamma energy for "
                          "%s;Energy [keV];Excitation energy [MeV]",
                          histoTitle.c_str()),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV, 1000, -10,
                     30),
            reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_Ex_EDCBP,
            new TH2D(Form("m_Ex_DCEBP_ionr%s", histoName.c_str()),
                     Form("Excitation energy spectrum vs DC gamma energy for "
                          "%s;Energy DC [keV];Excitation energy [MeV]",
                          histoTitle.c_str()),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV, 1000, -10,
                     30),
            reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_E_E,
            new TH2D(Form("m_E_E_ionr%s", histoName.c_str()),
                     Form("Gamma-gamma DC spectrum for %s;Energy [keV];Energy "
                          "[keV];Counts/%d keV",
                          histoTitle.c_str(), gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            reactionHistos.ptrs);

        Initialize<TH2D>(
            reactionHistos.m_EDCBP_EDCBP,
            new TH2D(Form("m_EDCBP_EDCBP_ionr%s", histoName.c_str()),
                     Form("Gamma-gamma DC spectrum for %s;Energy [keV];Energy "
                          "[keV];Counts/%d keV",
                          histoName.c_str(), gammadetectorConf.binWidth),
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV,
                     static_cast<int>(gammadetectorConf.gammaEmax / UNITS::keV
                                      / gammadetectorConf.binWidth),
                     0, gammadetectorConf.gammaEmax / UNITS::keV),
            reactionHistos.ptrs);
    }
    return true;
}

void GammadetectorEuclidesAnalysis::Analyze() {
    CoincidenceAnalysis::Analyze();
    gammaDC.clear();
    //if(!in_coinc) return;

    gammaDC.resize(gammadetector.E.size());
    for(int a{0}; a < static_cast<int>(gammadetector.E.size()); ++a) {
        // Find the final ion
        ReactionFragment::Ion p3Ion{
            conf.reaction.beam.A + conf.reaction.target.A,
            conf.reaction.beam.Z + conf.reaction.target.Z};
        for(int e{0}; e < static_cast<int>(euclides.E.size()); ++e) {
            if(!euclides.identified.at(e)) { continue; }
            p3Ion.A -= euclides.A.at(e);
            p3Ion.Z -= euclides.Z.at(e);
        }

        gammaDC.p3 = SlowInTarget(euclides.p3, gammadetectorEuclidesConf.dcPos,
                                  p3Ion);
        gammaDC.EDCBP.at(a) = gammadetector.P.at(a);
        gammaDC.EDCBP.at(a).Boost(-euclides.p3.BoostVector());
    }
}

bool GammadetectorEuclidesAnalysis::FillHistos() {
    if(!CoincidenceAnalysis::FillHistos()) return false;

    GammadetectorEuclidesHistograms& h = gammadetectorEuclidesHistograms;

    for(unsigned int e{0}; e<euclidesReader.TSEUCLIDES_Hit->GetSize(); ++e) {
        // cms change 08/02/2024 - get EUCLIDES board and channel for use below
        unsigned int brd  = euclidesReader.EUCLIDESBoard->At(e);
        unsigned int chan = euclidesReader.EUCLIDESChannel->At(e);

        for(int a{0}; a < static_cast<int>(gammadetector.E.size()); ++a) {
            auto diff = unsignedDiff<double, unsigned long long>(gammadetector.TS,euclidesReader.TSEUCLIDES_Hit->At(e));

            // cms change 08/02/2024 - get EUCLIDES ID from LUT instead of conf data structure, preventing error messages in selector
            // Fill(h.m_TSeuclidesmTSgammadetector_euclidesID,
            //      euclidesConf
            //          .getName(euclidesReader.EUCLIDESBoard->At(e),
            //                   euclidesReader.EUCLIDESChannel->At(e))
            //          .c_str(),
            //      diff);
            Fill(h.m_TSeuclidesmTSgammadetector_euclidesID, brd, Lut::getID(brd, chan), diff);
        }
    }

    //if(!in_coinc) return true;

    for(unsigned int a{0}; a < gammadetector.E.size(); ++a) {
        Fill(h.h_E, gammadetector.E.at(a));
        Fill(h.h_EDC, gammaDC.EDCBP.at(a).E());

        for(unsigned int e{0}; e < euclides.E.size(); ++e) {
            if(!euclides.identified.at(e)) { continue; }

            ReactionFragment::Ion idIon{euclides.A.at(e), euclides.Z.at(e)};
            auto& hi      = gammadetectorEuclidesHistograms.ionHistos;
            auto  histoIt = hi.find(idIon);
            if(histoIt != hi.end()) {
                Fill(histoIt->second.h_E, gammadetector.E.at(a));
                Fill(histoIt->second.h_EDCBP, gammaDC.EDCBP.at(a).E());
                for(unsigned int a2{0}; a2 < a; ++a2) {
                    Fill(histoIt->second.m_E_E, gammadetector.E.at(a),
                         gammadetector.E.at(a2));
                    Fill(histoIt->second.m_E_E, gammadetector.E.at(a2),
                         gammadetector.E.at(a));

                    Fill(histoIt->second.m_EDCBP_EDCBP,
                         gammaDC.EDCBP.at(a).E(), gammaDC.EDCBP.at(a2).E());
                    Fill(histoIt->second.m_EDCBP_EDCBP,
                         gammaDC.EDCBP.at(a2).E(), gammaDC.EDCBP.at(a).E());
                }
            } else {
                throw std::runtime_error(
                    std::string("Particle identified in EUCLIDES but "
                                "histogram is not present for ion: A")
                    + std::to_string(euclides.A.at(e)) + " Z"
                    + std::to_string(euclides.A.at(e)));
            }
        }
    }

    // Now analyze the complete reaction
    std::vector<ReactionFragment::Ion> reaction;
    for(unsigned int e{0}; e < euclides.E.size(); ++e) {
        if(!euclides.identified.at(e)) { continue; }

        ReactionFragment::Ion idIon{euclides.A.at(e), euclides.Z.at(e)};
        reaction.emplace_back(idIon);
    }
    std::sort(reaction.begin(), reaction.end());

    auto reactionHist = h.reactionHistos.find(reaction);
    if(reactionHist != h.reactionHistos.end()) {
        for(unsigned int a{0}; a < gammaDC.E.size(); ++a) {
            Fill(reactionHist->second.h_E, gammadetector.E.at(a));
            Fill(reactionHist->second.h_EDCBP, gammaDC.EDCBP.at(a).E());
            Fill(reactionHist->second.m_Ex_E, gammadetector.E.at(a),
                 euclides.Ex);
            Fill(reactionHist->second.m_Ex_EDCBP, gammaDC.EDCBP.at(a).E(),
                 euclides.Ex);
            for(unsigned int a2{0}; a2 < a; ++a2) {
                Fill(reactionHist->second.m_E_E, gammadetector.E.at(a),
                     gammadetector.E.at(a2));
                Fill(reactionHist->second.m_E_E, gammadetector.E.at(a2),
                     gammadetector.E.at(a));

                Fill(reactionHist->second.m_EDCBP_EDCBP,
                     gammaDC.EDCBP.at(a).E(), gammaDC.EDCBP.at(a2).E());
                Fill(reactionHist->second.m_EDCBP_EDCBP,
                     gammaDC.EDCBP.at(a2).E(), gammaDC.EDCBP.at(a).E());
            }
        }
    }

    return true;
}

void GammadetectorEuclidesAnalysis::Finalize() {
    CoincidenceAnalysis::Finalize();
}
void GammadetectorEuclidesAnalysis::Clear() { gammaDC.clear(); }
