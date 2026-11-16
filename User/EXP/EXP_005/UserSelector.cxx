#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    UserHistograms& h = userHistograms;

    h.dir             = outFile.mkdir("User", "User histos");
    h.dir->cd();

    h.ptrs.push_back(h.Egamma_Ep_h59
                     = new TH2D("Egamma_Ep_h59", "Egamma_Ep_h59", 4000, 0,
                                4000, 450, 0, 22));
    h.ptrs.push_back(h.Egamma_Eeu = new TH2D("Egamma_Eeu", "Egamma_Eeu", 4000,
                                             0, 4000, 1000, 0, 40));
    h.ptrs.push_back(h.Egamma_h59
                     = new TH1D("Egamma_h59", "Egamma_h59", 4000, 0, 4000));
    h.ptrs.push_back(h.monitor = new TH1D("monitor", "monitor", 2000, 0, 40));
    h.ptrs.push_back(h.monitor2
                     = new TH1D("monitor2", "monitor2", 2000, -1, 40));
    h.ptrs.push_back(h.fondo = new TH1D("fondo", "fondo", 4000, 0, 4000));
    h.ptrs.push_back(h.fondop = new TH1D("fondop", "fondop", 4000, 0, 4000));
    h.ptrs.push_back(h.fondoa = new TH1D("fondoa", "fondoa", 4000, 0, 4000));
    h.ptrs.push_back(h.picco_c = new TH1D(
                         "picco_c", "picco_c; TS difference [10 ns]; Counts",
                         (conf.agataEuclides.windowRight
                          - conf.agataEuclides.windowLeft + 800)
                             * 10,
                         conf.agataEuclides.windowLeft - 400,
                         conf.agataEuclides.windowRight + 400));
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);
    UserHistograms&     h = userHistograms;

    std::vector<double> gammaDC;
    for(int a{0}; a < static_cast<int>(agataOutput.E.size()); ++a) {
        TLorentzVector gamma = agataOutput.P.at(a);
        gamma.Boost(-euclidesOutput.p3.BoostVector());
        gammaDC.emplace_back(gamma.E());
    }
    for(unsigned int i{0}; i < gammaDC.size(); ++i) {
        for(unsigned int j{0}; j < euclidesOutput.E.size(); ++j) {
            if(euclidesOutput.Id.at(j) == 2300) {
                h.Egamma_Ep_h59->Fill(gammaDC.at(i), euclidesOutput.Ex);
                h.Egamma_h59->Fill(gammaDC.at(i));
            }
        }
    }
    if(conf.detectorsPresent.Agata && conf.detectorsPresent.Euclides
       && euclidesOutput.TS != 0 && agataOutput.TS != 0) {
        if(settings.verbosity_level > 1) std::cout << "EuclidesAgata\n";
        for(size_t aid{0}; aid < agataOutput.Time.size(); ++aid) {
            auto atime{agataOutput.Time.at(aid)};
            for(size_t eid{0}; eid < euclidesOutput.Time.size(); ++eid) {
                auto   etime{euclidesOutput.Time.at(eid)};

                double diff = unsignedDiff<double, unsigned long long>(
                                  euclidesOutput.TS, agataOutput.TS)
                            + (etime - atime) / (10. * UNITS::ns);

                h.picco_c->Fill(diff);

                if((diff > -31 && diff < -16) || (diff > 16 && diff < 31)) {
                    for(unsigned int i{0}; i < gammaDC.size(); ++i) {
                        for(unsigned int j{0}; j < euclidesOutput.E.size();
                            ++j) {
                            if(euclidesOutput.Z.at(j) == 2
                               && euclidesOutput.A.at(j) == 4) {
                                h.fondoa->Fill(gammaDC.at(i));
                            }
                            if(euclidesOutput.Z.at(j) == 1
                               && euclidesOutput.A.at(j) == 1) {
                                h.fondop->Fill(gammaDC.at(i));
                            }
                        }

                        h.fondo->Fill(gammaDC.at(i));
                    }
                }
            }
        }
    }
    for(unsigned int i{0};
        i < euclidesAnalysis->euclidesReader.EUCLIDESEnergy->GetSize(); ++i) {
        unsigned int brd
            = euclidesAnalysis->euclidesReader.EUCLIDESBoard->At(i);
        unsigned int chan
            = euclidesAnalysis->euclidesReader.EUCLIDESChannel->At(i);
        double energy{0};

        energy = conf.euclides.getEnergy(
            euclidesAnalysis->euclidesReader.EUCLIDESEnergy->At(i),
            euclidesAnalysis->euclidesReader.EUCLIDESBoard->At(i),
            euclidesAnalysis->euclidesReader.EUCLIDESChannel->At(i));
        for(unsigned int j{0}; j < gammaDC.size(); ++j) {
            h.Egamma_Eeu->Fill(gammaDC.at(j), energy);
        }
        if(brd == 5 && chan == 0) { h.monitor->Fill(energy); }
        if(brd == 5 && chan == 1) { h.monitor2->Fill(energy); }
    }

    // User process
    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
}
