#pragma once
#include "AgataSelector.h"

class UserSelector : public AgataSelector {
  public:
    UserSelector() = default;
    virtual Bool_t Process(Long64_t entry) override;
    virtual void   SlaveBegin(TTree* tree) override;
    virtual void   SlaveTerminate() override;

  private:
    // USER//////////////////////////////////////////////////////

    struct Histograms {
        std::vector<TObject*> ptrs;
        TDirectory*           dir{nullptr};
    };

    struct UserHistograms : public Histograms {
        class Coincidence {
          public:
            TDirectory* dir{nullptr};
            std::string Name;
            double      Elow1;
            double      Ehigh1;
            double      Egamma1;
            double      Elow2;
            double      Ehigh2;
            double      Egamma2;
            double      GateWidth1; // Gating interval on gamma above
            double      GateWidth2;
            double      LeftShift1{0};
            double      RightShift1{0};
            double      LeftShift2{0};
            double      RightShift2{0};

            struct AngCorrHistograms : public Histograms {
                TH1D* h_gg_Theta{nullptr};
                TH1D* h_gg_ang_corr{nullptr};
                TH1D* h_gg_Theta_uncorrelated{nullptr};
                TH1D* h_gg_Theta_bg{nullptr};
                TH1D* h_gg_Theta_bg_uncorrelated{nullptr};
            };

            AngCorrHistograms     CoreHisto;
            AngCorrHistograms     TrackHisto;
            std::vector<TVector3> vec_pos_1;
            std::vector<TVector3> vec_pos_2;
            std::vector<TVector3> vec_pos_bg_1;
            std::vector<TVector3> vec_pos_bg_2;

            Coincidence(std::string name, double elow1, double ehigh1,
                        double elow2, double ehigh2, double leftshift1 = 0,
                        double rightshift1 = 0, double leftshift2 = 0,
                        double rightshift2 = 0) {
                Name       = name;
                Elow1      = elow1;
                Ehigh1     = ehigh1;
                Elow2      = elow2;
                Ehigh2     = ehigh2;

                LeftShift1 = leftshift1; // Shifts from the edge of the gate to
                                         // calculate background subtraction
                RightShift1 = rightshift1;

                LeftShift2  = leftshift2;
                RightShift2 = rightshift2;

                Egamma1     = 0.5 * (Elow1 + Ehigh1);
                Egamma2     = 0.5 * (Elow2 + Ehigh2);
                GateWidth1  = Ehigh1 - Elow1;
                GateWidth2  = Ehigh2 - Elow2;
            };

            void Initialize() {
                CoreHisto.dir = dir->mkdir("Core");
                CoreHisto.dir->cd();

                CoreHisto.ptrs.push_back(
                    CoreHisto.h_gg_Theta
                    = new TH1D("h_gg_Theta",
                               Form("Angular separation distribution between "
                                    "%3.0f keV and %3.0f keV (%s)",
                                    Egamma1, Egamma2, Name.c_str()),
                               180, 0, 180));
                CoreHisto.ptrs.push_back(
                    CoreHisto.h_gg_ang_corr
                    = new TH1D("h_gg_ang_corr",
                               Form("Angular correlation in %s between %3.0f "
                                    "keV and %3.0f keV ",
                                    Name.c_str(), Egamma1, Egamma2),
                               180, 0, 180));
                CoreHisto.ptrs.push_back(
                    CoreHisto.h_gg_Theta_uncorrelated = new TH1D(
                        "h_gg_Theta_uncorrelated",
                        "Angular correlation between uncorrelated gammas", 180,
                        0, 180));
                CoreHisto.ptrs.push_back(
                    CoreHisto.h_gg_Theta_bg
                    = new TH1D("h_gg_Theta_bg",
                               Form("BG random coincidence in %s between "
                                    "%3.0f keV and %3.0f keV ",
                                    Name.c_str(), Egamma1, Egamma2),
                               180, 0, 180));

                TrackHisto.dir = dir->mkdir("Tracked");
                TrackHisto.dir->cd();

                TrackHisto.ptrs.push_back(
                    TrackHisto.h_gg_Theta
                    = new TH1D("h_gg_Theta",
                               Form("Angular separation distribution between "
                                    "%3.0f keV and %3.0f keV (%s)",
                                    Egamma1, Egamma2, Name.c_str()),
                               180, 0, 180));
                TrackHisto.ptrs.push_back(
                    TrackHisto.h_gg_ang_corr
                    = new TH1D("h_gg_ang_corr",
                               Form("Angular correlation in %s between %3.0f "
                                    "keV and %3.0f keV ",
                                    Name.c_str(), Egamma1, Egamma2),
                               180, 0, 180));
                TrackHisto.ptrs.push_back(
                    TrackHisto.h_gg_Theta_uncorrelated = new TH1D(
                        "h_gg_Theta_uncorrelated",
                        "Angular correlation between uncorrelated gammas", 180,
                        0, 180));
                TrackHisto.ptrs.push_back(
                    TrackHisto.h_gg_Theta_bg
                    = new TH1D("h_gg_Theta_bg",
                               Form("BG random coincidence in %s between "
                                    "%3.0f keV and %3.0f keV ",
                                    Name.c_str(), Egamma1, Egamma2),
                               180, 0, 180));
                TrackHisto.ptrs.push_back(
                    TrackHisto.h_gg_Theta_bg_uncorrelated
                    = new TH1D("h_gg_Theta_bg_uncorrelated",
                               Form("BG uncorrelated angular correlation in "
                                    "%s between %3.0f keV and %3.0f keV ",
                                    Name.c_str(), Egamma1, Egamma2),
                               180, 0, 180));

                return;
            };

            void Compute(AgataReader& agataReader, const ConfManager* conf) {
                double lowestCoreTS{std::numeric_limits<double>::max()};
                int    lowestCoreIdx{0};
                int    mult_cores = **agataReader.nbCores;
                std::vector<unsigned int> idx_cores;

                for(int i{0}; i < mult_cores; ++i) {
                    if(lowestCoreTS > agataReader.coreTS->At(i)) {
                        lowestCoreTS  = agataReader.coreTS->At(i);
                        lowestCoreIdx = i;
                    }
                }

                idx_cores.push_back(lowestCoreIdx);
                for(int i{0}; i < mult_cores; ++i) {
                    if(agataReader.coreTS->At(i) != 0 && i != lowestCoreIdx
                       && agataReader.coreTS->At(i) - lowestCoreTS
                              >= conf->agata.windowLeft
                       && agataReader.coreTS->At(i) - lowestCoreTS
                              < conf->agata.windowRight)
                        idx_cores.push_back(i);
                }

                for(unsigned int i{0}; i < idx_cores.size(); ++i) {
                    for(unsigned int j{0}; j < i; ++j) {
                        TVector3 pos_i = TVector3(
                            agataReader.hitGY->At(idx_cores[i]) * UNITS::mm,
                            -agataReader.hitGX->At(idx_cores[i]) * UNITS::mm,
                            agataReader.hitGZ->At(idx_cores[i]) * UNITS::mm);
                        TVector3 pos_j = TVector3(
                            agataReader.hitGY->At(idx_cores[j]) * UNITS::mm,
                            -agataReader.hitGX->At(idx_cores[j]) * UNITS::mm,
                            agataReader.hitGZ->At(idx_cores[j]) * UNITS::mm);
                        double theta = pos_i.Angle(pos_j) * 180 / TMath::Pi();

                        if((agataReader.coreE0->At(idx_cores[i]) > Elow1
                            && agataReader.coreE0->At(idx_cores[i]) < Ehigh1
                            && agataReader.coreE0->At(idx_cores[j]) > Elow2
                            && agataReader.coreE0->At(idx_cores[j]) < Ehigh2)
                           || (agataReader.coreE0->At(idx_cores[j]) > Elow1
                               && agataReader.coreE0->At(idx_cores[j]) < Ehigh1
                               && agataReader.coreE0->At(idx_cores[i]) > Elow2
                               && agataReader.coreE0->At(idx_cores[i])
                                      < Ehigh2)) {
                            Fill(CoreHisto.h_gg_Theta, theta);
                            Fill(CoreHisto.h_gg_ang_corr, theta);
                        }

                        if((agataReader.coreE0->At(idx_cores[i])
                                > Elow1 - LeftShift1 - GateWidth1 / 2
                            && agataReader.coreE0->At(idx_cores[i])
                                   < Elow1 - LeftShift1
                            && agataReader.coreE0->At(idx_cores[j]) > Elow2
                            && agataReader.coreE0->At(idx_cores[j]) < Ehigh2)
                           || (agataReader.coreE0->At(idx_cores[j])
                                   > Elow1 - LeftShift1 - GateWidth1 / 2
                               && agataReader.coreE0->At(idx_cores[j])
                                      < Elow1 - LeftShift1
                               && agataReader.coreE0->At(idx_cores[i]) > Elow2
                               && agataReader.coreE0->At(idx_cores[i])
                                      < Ehigh2)
                           || (agataReader.coreE0->At(idx_cores[i])
                                   > Ehigh1 + RightShift1
                               && agataReader.coreE0->At(idx_cores[i])
                                      < Ehigh1 + RightShift1 + GateWidth1 / 2
                               && agataReader.coreE0->At(idx_cores[j]) > Elow2
                               && agataReader.coreE0->At(idx_cores[j])
                                      < Ehigh2)
                           || (agataReader.coreE0->At(idx_cores[j])
                                   > Ehigh1 + RightShift1
                               && agataReader.coreE0->At(idx_cores[j])
                                      < Ehigh1 + RightShift1 + GateWidth1 / 2
                               && agataReader.coreE0->At(idx_cores[i]) > Elow2
                               && agataReader.coreE0->At(idx_cores[i])
                                      < Ehigh2)
                           ||

                           (agataReader.coreE0->At(idx_cores[i])
                                > Elow2 - LeftShift2 - GateWidth2 / 2
                            && agataReader.coreE0->At(idx_cores[i])
                                   < Elow2 - LeftShift2
                            && agataReader.coreE0->At(idx_cores[j]) > Elow1
                            && agataReader.coreE0->At(idx_cores[j]) < Ehigh1)
                           || (agataReader.coreE0->At(idx_cores[j])
                                   > Elow2 - LeftShift2 - GateWidth2 / 2
                               && agataReader.coreE0->At(idx_cores[j])
                                      < Elow2 - LeftShift2
                               && agataReader.coreE0->At(idx_cores[i]) > Elow1
                               && agataReader.coreE0->At(idx_cores[i])
                                      < Ehigh1)
                           || (agataReader.coreE0->At(idx_cores[i])
                                   > Ehigh2 + RightShift2
                               && agataReader.coreE0->At(idx_cores[i])
                                      < Ehigh2 + RightShift2 + GateWidth2 / 2
                               && agataReader.coreE0->At(idx_cores[j]) > Elow1
                               && agataReader.coreE0->At(idx_cores[j])
                                      < Ehigh1)
                           || (agataReader.coreE0->At(idx_cores[j])
                                   > Ehigh2 + RightShift2
                               && agataReader.coreE0->At(idx_cores[j])
                                      < Ehigh2 + RightShift2 + GateWidth2 / 2
                               && agataReader.coreE0->At(idx_cores[i]) > Elow1
                               && agataReader.coreE0->At(idx_cores[i])
                                      < Ehigh1)
                           ||

                           (agataReader.coreE0->At(idx_cores[i])
                                > Elow2 - LeftShift2 - GateWidth2 / 2
                            && agataReader.coreE0->At(idx_cores[i])
                                   < Elow2 - LeftShift2
                            && agataReader.coreE0->At(idx_cores[j])
                                   > Elow1 - LeftShift1 - GateWidth1 / 2
                            && agataReader.coreE0->At(idx_cores[j])
                                   < Elow1 - LeftShift1)
                           || (agataReader.coreE0->At(idx_cores[j])
                                   > Elow2 - LeftShift2 - GateWidth2 / 2
                               && agataReader.coreE0->At(idx_cores[j])
                                      < Elow2 - LeftShift2
                               && agataReader.coreE0->At(idx_cores[i])
                                      > Elow1 - LeftShift1 - GateWidth1 / 2
                               && agataReader.coreE0->At(idx_cores[i])
                                      < Elow1 - LeftShift1)
                           || (agataReader.coreE0->At(idx_cores[i])
                                   > Ehigh2 + RightShift2
                               && agataReader.coreE0->At(idx_cores[i])
                                      < Ehigh2 + RightShift2 + GateWidth2 / 2
                               && agataReader.coreE0->At(idx_cores[j])
                                      > Ehigh1 + RightShift1
                               && agataReader.coreE0->At(idx_cores[j])
                                      < Ehigh1 + RightShift1 + GateWidth1 / 2)
                           || (agataReader.coreE0->At(idx_cores[j])
                                   > Ehigh2 + RightShift2
                               && agataReader.coreE0->At(idx_cores[j])
                                      < Ehigh2 + RightShift2 + GateWidth2 / 2
                               && agataReader.coreE0->At(idx_cores[i])
                                      > Ehigh1 + RightShift1
                               && agataReader.coreE0->At(idx_cores[i])
                                      < Ehigh1 + RightShift1 + GateWidth1 / 2)
                           || (agataReader.coreE0->At(idx_cores[i])
                                   > Elow2 - LeftShift2 - GateWidth2 / 2
                               && agataReader.coreE0->At(idx_cores[i])
                                      < Elow2 - LeftShift2
                               && agataReader.coreE0->At(idx_cores[j])
                                      > Ehigh1 + RightShift1
                               && agataReader.coreE0->At(idx_cores[j])
                                      < Ehigh1 + RightShift1 + GateWidth1 / 2)
                           || (agataReader.coreE0->At(idx_cores[j])
                                   > Elow2 - LeftShift2 - GateWidth2 / 2
                               && agataReader.coreE0->At(idx_cores[j])
                                      < Elow2 - LeftShift2
                               && agataReader.coreE0->At(idx_cores[i])
                                      > Ehigh1 + RightShift1
                               && agataReader.coreE0->At(idx_cores[i])
                                      < Ehigh1 + RightShift1 + GateWidth1 / 2)
                           || (agataReader.coreE0->At(idx_cores[i])
                                   > Ehigh2 + RightShift2
                               && agataReader.coreE0->At(idx_cores[i])
                                      < Ehigh2 + RightShift2 + GateWidth2 / 2
                               && agataReader.coreE0->At(idx_cores[j])
                                      > Elow1 - LeftShift1 - GateWidth1 / 2
                               && agataReader.coreE0->At(idx_cores[j])
                                      < Elow1 - LeftShift1)
                           || (agataReader.coreE0->At(idx_cores[j])
                                   > Ehigh2 + RightShift2
                               && agataReader.coreE0->At(idx_cores[j])
                                      < Ehigh2 + RightShift2 + GateWidth2 / 2
                               && agataReader.coreE0->At(idx_cores[i])
                                      > Elow1 - LeftShift1 - GateWidth1 / 2
                               && agataReader.coreE0->At(idx_cores[i])
                                      < Elow1 - LeftShift1)) {
                            Fill(CoreHisto.h_gg_Theta_bg, theta);
                        }
                    }
                }

                double lowestTrackT{std::numeric_limits<double>::max()};
                int    lowestTrackIdx{0};
                int    mult_tracks = **agataReader.nbTrack;
                std::vector<unsigned int> idx_tracks;

                for(int i{0}; i < mult_tracks; ++i) {
                    if(lowestTrackT > agataReader.trackT->At(i)) {
                        lowestTrackT   = agataReader.trackT->At(i);
                        lowestTrackIdx = i;
                    }
                }

                idx_tracks.push_back(lowestTrackIdx);
                for(int i{0}; i < mult_tracks; ++i) {
                    if(**agataReader.TStrack != 0 && i != lowestTrackIdx
                       && agataReader.trackT->At(i) - lowestTrackT
                              >= conf->agata.windowLeft
                       && agataReader.trackT->At(i) - lowestTrackT
                              < conf->agata.windowRight)
                        idx_tracks.push_back(i);
                }

                // Tracked loop
                for(unsigned int i{0}; i < idx_tracks.size(); ++i) {
                    if(vec_pos_1.size() < 1000) {
                        if(agataReader.trackE->At(idx_tracks[i]) > Elow1
                           && agataReader.trackE->At(idx_tracks[i]) < Ehigh1) {
                            vec_pos_1.push_back(TVector3(
                                agataReader.trackY1->At(idx_tracks[i])
                                    * UNITS::mm,
                                -agataReader.trackX1->At(idx_tracks[i])
                                    * UNITS::mm,
                                agataReader.trackZ1->At(idx_tracks[i])
                                    * UNITS::mm));
                            break;
                        }
                        if(agataReader.trackE->At(idx_tracks[i]) > Elow2
                           && agataReader.trackE->At(idx_tracks[i]) < Ehigh2) {
                            vec_pos_2.push_back(TVector3(
                                agataReader.trackY1->At(idx_tracks[i])
                                    * UNITS::mm,
                                -agataReader.trackX1->At(idx_tracks[i])
                                    * UNITS::mm,
                                agataReader.trackZ1->At(idx_tracks[i])
                                    * UNITS::mm));
                            break;
                        }
                        if((agataReader.trackE->At(idx_tracks[i])
                                > Elow1 - LeftShift1 - GateWidth1 / 2
                            && agataReader.trackE->At(idx_tracks[i])
                                   < Elow1 - LeftShift1)
                           || (agataReader.trackE->At(idx_tracks[i])
                                   > Ehigh1 + RightShift1
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Ehigh1 + RightShift1
                                            + GateWidth1 / 2)) {
                            vec_pos_bg_1.push_back(TVector3(
                                agataReader.trackY1->At(idx_tracks[i])
                                    * UNITS::mm,
                                -agataReader.trackX1->At(idx_tracks[i])
                                    * UNITS::mm,
                                agataReader.trackZ1->At(idx_tracks[i])
                                    * UNITS::mm));
                        }
                        if((agataReader.trackE->At(idx_tracks[i])
                                > Elow2 - LeftShift2 - GateWidth2 / 2
                            && agataReader.trackE->At(idx_tracks[i])
                                   < Elow2 - LeftShift2)
                           || (agataReader.trackE->At(idx_tracks[i])
                                   > Ehigh2 + RightShift2
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Ehigh2 + RightShift2
                                            + GateWidth2 / 2)) {
                            vec_pos_bg_2.push_back(TVector3(
                                agataReader.trackY1->At(idx_tracks[i])
                                    * UNITS::mm,
                                -agataReader.trackX1->At(idx_tracks[i])
                                    * UNITS::mm,
                                agataReader.trackZ1->At(idx_tracks[i])
                                    * UNITS::mm));
                        }
                    }
                }

                for(unsigned int i{0}; i < idx_tracks.size(); ++i) {
                    for(unsigned int j{0}; j < i; ++j) {
                        TVector3 pos_i = TVector3(
                            agataReader.trackY1->At(idx_tracks[i]) * UNITS::mm,
                            -agataReader.trackX1->At(idx_tracks[i])
                                * UNITS::mm,
                            agataReader.trackZ1->At(idx_tracks[i])
                                * UNITS::mm);
                        TVector3 pos_j = TVector3(
                            agataReader.trackY1->At(idx_tracks[j]) * UNITS::mm,
                            -agataReader.trackX1->At(idx_tracks[j])
                                * UNITS::mm,
                            agataReader.trackZ1->At(idx_tracks[j])
                                * UNITS::mm);
                        double theta = pos_i.Angle(pos_j) * 180 / TMath::Pi();

                        if((agataReader.trackE->At(idx_tracks[i]) > Elow1
                            && agataReader.trackE->At(idx_tracks[i]) < Ehigh1
                            && agataReader.trackE->At(idx_tracks[j]) > Elow2
                            && agataReader.trackE->At(idx_tracks[j]) < Ehigh2)
                           || (agataReader.trackE->At(idx_tracks[j]) > Elow1
                               && agataReader.trackE->At(idx_tracks[j])
                                      < Ehigh1
                               && agataReader.trackE->At(idx_tracks[i]) > Elow2
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Ehigh2)) {
                            Fill(TrackHisto.h_gg_Theta, theta);
                            Fill(TrackHisto.h_gg_ang_corr, theta);
                        }
                        if((agataReader.trackE->At(idx_tracks[i])
                                > Elow1 - LeftShift1 - GateWidth1 / 2
                            && agataReader.trackE->At(idx_tracks[i])
                                   < Elow1 - LeftShift1
                            && agataReader.trackE->At(idx_tracks[j]) > Elow2
                            && agataReader.trackE->At(idx_tracks[j]) < Ehigh2)
                           || (agataReader.trackE->At(idx_tracks[j])
                                   > Elow1 - LeftShift1 - GateWidth1 / 2
                               && agataReader.trackE->At(idx_tracks[j])
                                      < Elow1 - LeftShift1
                               && agataReader.trackE->At(idx_tracks[i]) > Elow2
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Ehigh2)
                           || (agataReader.trackE->At(idx_tracks[i])
                                   > Ehigh1 + RightShift1
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Ehigh1 + RightShift1 + GateWidth1 / 2
                               && agataReader.trackE->At(idx_tracks[j]) > Elow2
                               && agataReader.trackE->At(idx_tracks[j])
                                      < Ehigh2)
                           || (agataReader.trackE->At(idx_tracks[j])
                                   > Ehigh1 + RightShift1
                               && agataReader.trackE->At(idx_tracks[j])
                                      < Ehigh1 + RightShift1 + GateWidth1 / 2
                               && agataReader.trackE->At(idx_tracks[i]) > Elow2
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Ehigh2)
                           ||

                           (agataReader.trackE->At(idx_tracks[i])
                                > Elow2 - LeftShift2 - GateWidth2 / 2
                            && agataReader.trackE->At(idx_tracks[i])
                                   < Elow2 - LeftShift2
                            && agataReader.trackE->At(idx_tracks[j]) > Elow1
                            && agataReader.trackE->At(idx_tracks[j]) < Ehigh1)
                           || (agataReader.trackE->At(idx_tracks[j])
                                   > Elow2 - LeftShift2 - GateWidth2 / 2
                               && agataReader.trackE->At(idx_tracks[j])
                                      < Elow2 - LeftShift2
                               && agataReader.trackE->At(idx_tracks[i]) > Elow1
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Ehigh1)
                           || (agataReader.trackE->At(idx_tracks[i])
                                   > Ehigh2 + RightShift2
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Ehigh2 + RightShift2 + GateWidth2 / 2
                               && agataReader.trackE->At(idx_tracks[j]) > Elow1
                               && agataReader.trackE->At(idx_tracks[j])
                                      < Ehigh1)
                           || (agataReader.trackE->At(idx_tracks[j])
                                   > Ehigh2 + RightShift2
                               && agataReader.trackE->At(idx_tracks[j])
                                      < Ehigh2 + RightShift2 + GateWidth2 / 2
                               && agataReader.trackE->At(idx_tracks[i]) > Elow1
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Ehigh1)
                           ||

                           (agataReader.trackE->At(idx_tracks[i])
                                > Elow2 - LeftShift2 - GateWidth2 / 2
                            && agataReader.trackE->At(idx_tracks[i])
                                   < Elow2 - LeftShift2
                            && agataReader.trackE->At(idx_tracks[j])
                                   > Elow1 - LeftShift1 - GateWidth1 / 2
                            && agataReader.trackE->At(idx_tracks[j])
                                   < Elow1 - LeftShift1)
                           || (agataReader.trackE->At(idx_tracks[j])
                                   > Elow2 - LeftShift2 - GateWidth2 / 2
                               && agataReader.trackE->At(idx_tracks[j])
                                      < Elow2 - LeftShift2
                               && agataReader.trackE->At(idx_tracks[i])
                                      > Elow1 - LeftShift1 - GateWidth1 / 2
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Elow1 - LeftShift1)
                           || (agataReader.trackE->At(idx_tracks[i])
                                   > Ehigh2 + RightShift2
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Ehigh2 + RightShift2 + GateWidth2 / 2
                               && agataReader.trackE->At(idx_tracks[j])
                                      > Ehigh1 + RightShift1
                               && agataReader.trackE->At(idx_tracks[j])
                                      < Ehigh1 + RightShift1 + GateWidth1 / 2)
                           || (agataReader.trackE->At(idx_tracks[j])
                                   > Ehigh2 + RightShift2
                               && agataReader.trackE->At(idx_tracks[j])
                                      < Ehigh2 + RightShift2 + GateWidth2 / 2
                               && agataReader.trackE->At(idx_tracks[i])
                                      > Ehigh1 + RightShift1
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Ehigh1 + RightShift1 + GateWidth1 / 2)
                           || (agataReader.trackE->At(idx_tracks[i])
                                   > Elow2 - LeftShift2 - GateWidth2 / 2
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Elow2 - LeftShift2
                               && agataReader.trackE->At(idx_tracks[j])
                                      > Ehigh1 + RightShift1
                               && agataReader.trackE->At(idx_tracks[j])
                                      < Ehigh1 + RightShift1 + GateWidth1 / 2)
                           || (agataReader.trackE->At(idx_tracks[j])
                                   > Elow2 - LeftShift2 - GateWidth2 / 2
                               && agataReader.trackE->At(idx_tracks[j])
                                      < Elow2 - LeftShift2
                               && agataReader.trackE->At(idx_tracks[i])
                                      > Ehigh1 + RightShift1
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Ehigh1 + RightShift1 + GateWidth1 / 2)
                           || (agataReader.trackE->At(idx_tracks[i])
                                   > Ehigh2 + RightShift2
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Ehigh2 + RightShift2 + GateWidth2 / 2
                               && agataReader.trackE->At(idx_tracks[j])
                                      > Elow1 - LeftShift1 - GateWidth1 / 2
                               && agataReader.trackE->At(idx_tracks[j])
                                      < Elow1 - LeftShift1)
                           || (agataReader.trackE->At(idx_tracks[j])
                                   > Ehigh2 + RightShift2
                               && agataReader.trackE->At(idx_tracks[j])
                                      < Ehigh2 + RightShift2 + GateWidth2 / 2
                               && agataReader.trackE->At(idx_tracks[i])
                                      > Elow1 - LeftShift1 - GateWidth1 / 2
                               && agataReader.trackE->At(idx_tracks[i])
                                      < Elow1 - LeftShift1)) {
                            Fill(TrackHisto.h_gg_Theta_bg, theta);
                        }
                    }
                }
                return;
            };

            void UncorrProcess() {
                for(auto p1 : vec_pos_1) {
                    for(auto p2 : vec_pos_2) {
                        Fill(TrackHisto.h_gg_Theta_uncorrelated,
                             p1.Angle(p2) * 180 / TMath::Pi());
                    }
                    for(auto bg2 : vec_pos_bg_2) {
                        Fill(TrackHisto.h_gg_Theta_bg_uncorrelated,
                             p1.Angle(bg2) * 180 / TMath::Pi());
                    }
                }

                for(auto bg1 : vec_pos_bg_1) {
                    for(auto p2 : vec_pos_2) {
                        Fill(TrackHisto.h_gg_Theta_bg_uncorrelated,
                             bg1.Angle(p2) * 180 / TMath::Pi());
                    }
                    for(auto bg2 : vec_pos_bg_2) {
                        Fill(TrackHisto.h_gg_Theta_bg_uncorrelated,
                             bg1.Angle(bg2) * 180 / TMath::Pi());
                    }
                }

                CoreHisto.h_gg_ang_corr->Sumw2(kTRUE);
                TrackHisto.h_gg_ang_corr->Sumw2(kTRUE);
                TrackHisto.h_gg_Theta_uncorrelated->Sumw2(kTRUE);

                // TrackHisto.h_gg_ang_corr->Add(TrackHisto.h_gg_Theta_bg,-1);
                // TrackHisto.h_gg_Theta_uncorrelated->Add(TrackHisto.h_gg_Theta_bg_uncorrelated,-1);

                // TrackHisto.h_gg_ang_corr->Divide(TrackHisto.h_gg_Theta_uncorrelated);
            };

          private:
        };

        std::vector<Coincidence> coincs;

    } userHistograms;
};
