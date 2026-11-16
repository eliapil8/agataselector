#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User begin

    initTS            = AgataSelector::GetFirstTS();

    UserHistograms& h = userHistograms;
    h.dir             = outFile.mkdir("User", "User histos");
    h.dir->cd();

    h.coincs.push_back(UserHistograms::Coincidence(
        conf.user.IsotopeName, conf.user.minE1 / UNITS::keV,
        conf.user.maxE1 / UNITS::keV, conf.user.minE2 / UNITS::keV,
        conf.user.maxE2 / UNITS::keV, 0.0, 0.0, 0.0, 0.0));

    // h.coincs.push_back(UserHistograms::Coincidence("60Co",1166,1178,1325,1337));
    // h.coincs.push_back(UserHistograms::Coincidence("152Eu",1401,1412,118,125));
    // h.coincs.push_back(UserHistograms::Coincidence("152Eu",241,248,118,125));
    // h.coincs.push_back(UserHistograms::Coincidence("152Eu",775,782,341,347));

    for(unsigned int k = 0; k < h.coincs.size(); ++k) {
        h.coincs[k].dir = h.dir->mkdir(
            Form("Coinc_%s_%3.0f_%3.0f", h.coincs[k].Name.c_str(),
                 h.coincs[k].Egamma1, h.coincs[k].Egamma2));

        h.coincs[k].dir->cd();

        h.coincs[k].Initialize();
        h.coincs[k].SetInitTS(initTS);
        h.coincs[k].SetCrysSpectraOn(conf.user.crysSpectra);
    }
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    UserHistograms& h = userHistograms;

    if(**agataAnalysis->agataReader.nbHits == 0) return kTRUE;

    if(**agataAnalysis->agataReader.nbCores > 0) {
        for(int i = 0; i < **agataAnalysis->agataReader.nbCores; ++i) {
            double timeCore = unsignedDiff<double, unsigned long long>(
                                  agataAnalysis->agataReader.coreTS->At(i),
                                  h.coincs.back().GetInitTS())
                            / 1e8;
            h.coincs.back().CoreHisto.h_id_vs_time_total->Fill(
                timeCore, agataAnalysis->agataReader.coreId->At(i));
        }
    }

    if(**agataAnalysis->agataReader.nbTrack > 0) {
        for(int i = 0; i < **agataAnalysis->agataReader.nbTrack; ++i) {
            double timeTrack = unsignedDiff<double, unsigned long long>(
                                   **agataAnalysis->agataReader.TStrack,
                                   h.coincs.back().GetInitTS())
                             / 1e8;
            h.coincs.back().TrackHisto.h_id_vs_time_total->Fill(
                timeTrack, agataAnalysis->agataReader.trackCrystalID->At(i));
        }
    }

    if(**agataAnalysis->agataReader.nbAdd > 0) {
        for(int i = 0; i < **agataAnalysis->agataReader.nbAdd; ++i) {
            double timeAdd = unsignedDiff<double, unsigned long long>(
                                 agataAnalysis->agataReader.AddTS->At(i),
                                 h.coincs.back().GetInitTS())
                           / 1e8;
            h.coincs.back().AddHisto.h_id_vs_time_total->Fill(
                timeAdd, agataAnalysis->agataReader.AddId->At(i));
        }
    }

    double EventTime = unsignedDiff<double, unsigned long long>(
                           agataAnalysis->agataReader.coreTS->At(0),
                           h.coincs.back().GetInitTS())
                     / 1e8;
    if(EventTime < conf.user.minT || EventTime > conf.user.maxT) return kTRUE;

    for(unsigned int k = 0; k < h.coincs.size(); ++k) {
        h.coincs[k].Compute(agataAnalysis->agataReader);
    }

    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate

    UserHistograms& h = userHistograms;

    for(auto it : h.coincs) {
        it.UncorrProcess();
        // it.FitDistribution();
    }
}

UserSelector::UserHistograms::Coincidence::Coincidence(
    std::string& name, double elow1, double ehigh1, double elow2,
    double ehigh2, double leftshift1, double rightshift1, double leftshift2,
    double rightshift2) {
    Name        = name;
    Elow1       = elow1;
    Ehigh1      = ehigh1;
    Elow2       = elow2;
    Ehigh2      = ehigh2;

    LeftShift1  = leftshift1; // Shifts from the edge of the gate to calculate
                              // background subtraction
    RightShift1 = rightshift1;

    LeftShift2  = leftshift2;
    RightShift2 = rightshift2;

    Egamma1     = 0.5 * (Elow1 + Ehigh1);
    Egamma2     = 0.5 * (Elow2 + Ehigh2);
    GateWidth1  = Ehigh1 - Elow1;
    GateWidth2  = Ehigh2 - Elow2;

    // std::cout<< "Eg1  "<<Egamma1 << std::endl;
    // std::cout<< "Eg2  "<<Egamma2 << std::endl;

    // std::cout<< "G1  "<<GateWidth1 << std::endl;
    // std::cout<< "G2  "<<GateWidth2 << std::endl;
};

UserSelector::UserHistograms::Coincidence::~Coincidence() {}

void UserSelector::UserHistograms::Coincidence::Initialize() {
    CoreHisto.dir = dir->mkdir("Core");
    CoreHisto.dir->cd();

    CoreHisto.ptrs.push_back(
        CoreHisto.h_singles = new TH1D(
            "h_singles", Form("Energy spectrum in singles (%s)", Name.c_str()),
            4000, 0, 4000));
    CoreHisto.ptrs.push_back(
        CoreHisto.h_gamma_gamma
        = new TH2D("h_gamma_gamma",
                   Form("Energy spectrum gamma-gamma (%s)", Name.c_str()),
                   4000, 0, 4000, 4000, 0, 4000));

    CoreHisto.ptrs.push_back(
        CoreHisto.h_id_vs_time_gated
        = new TH2D("h_id_vs_time_gated",
                   Form("Counting rate per detector vs time (%s) ; Time [s]",
                        Name.c_str()),
                   12 * 360, 0, 12 * 3600, 45, -0.5, 44.5));
    CoreHisto.ptrs.push_back(
        CoreHisto.h_id_vs_time_total
        = new TH2D("h_id_vs_time_total",
                   Form("Counting rate per detector vs time (%s) ; Time [s]",
                        Name.c_str()),
                   12 * 360, 0, 12 * 3600, 45, -0.5, 44.5));

    CoreHisto.ptrs.push_back(CoreHisto.h_gg_Theta = new TH1D(
                                 "h_gg_Theta",
                                 Form("Angular separation distribution "
                                      "between %3.0f keV and %3.0f keV (%s)",
                                      Egamma1, Egamma2, Name.c_str()),
                                 180, 0, 180));
    CoreHisto.ptrs.push_back(
        CoreHisto.h_gg_ang_corr = new TH1D(
            "h_gg_ang_corr",
            Form("Angular correlation in %s between %3.0f keV and %3.0f keV ",
                 Name.c_str(), Egamma1, Egamma2),
            180, 0, 180));
    CoreHisto.ptrs.push_back(
        CoreHisto.h_gg_Theta_uncorrelated = new TH1D(
            "h_gg_Theta_uncorrelated",
            "Angular correlation between uncorrelated gammas", 180, 0, 180));
    CoreHisto.ptrs.push_back(
        CoreHisto.h_gg_Theta_bg = new TH1D(
            "h_gg_Theta_bg",
            Form(
                "BG random coincidence in %s between %3.0f keV and %3.0f keV ",
                Name.c_str(), Egamma1, Egamma2),
            180, 0, 180));
    CoreHisto.ptrs.push_back(CoreHisto.h_gg_Theta_bg_uncorrelated = new TH1D(
                                 "h_gg_Theta_bg_uncorrelated",
                                 Form("BG uncorrelated angular correlation in "
                                      "%s between %3.0f keV and %3.0f keV ",
                                      Name.c_str(), Egamma1, Egamma2),
                                 180, 0, 180));
    CoreHisto.ptrs.push_back(
        CoreHisto.h_gg_Theta_random = new TH1D(
            "h_gg_Theta_random",
            Form("Random coincidence in %s between %3.0f keV and %3.0f keV ",
                 Name.c_str(), Egamma1, Egamma2),
            180, 0, 180));

    TrackHisto.dir = dir->mkdir("Tracked");
    TrackHisto.dir->cd();

    TrackHisto.ptrs.push_back(
        TrackHisto.h_singles = new TH1D(
            "h_singles", Form("Energy spectrum in singles (%s)", Name.c_str()),
            4000, 0, 4000));
    TrackHisto.ptrs.push_back(
        TrackHisto.h_gamma_gamma
        = new TH2D("h_gamma_gamma",
                   Form("Energy spectrum gamma-gamma (%s)", Name.c_str()),
                   4000, 0, 4000, 4000, 0, 4000));
    TrackHisto.ptrs.push_back(
        TrackHisto.h_id_vs_time_gated
        = new TH2D("h_id_vs_time_gated",
                   Form("Counting rate per detector vs time (%s) ; Time [s]",
                        Name.c_str()),
                   12 * 360, 0, 12 * 3600, 45, -0.5, 44.5));
    TrackHisto.ptrs.push_back(
        TrackHisto.h_id_vs_time_total
        = new TH2D("h_id_vs_time_total",
                   Form("Counting rate per detector vs time (%s) ; Time [s]",
                        Name.c_str()),
                   12 * 360, 0, 12 * 3600, 45, -0.5, 44.5));

    TrackHisto.ptrs.push_back(TrackHisto.h_gg_Theta = new TH1D(
                                  "h_gg_Theta",
                                  Form("Angular separation distribution "
                                       "between %3.0f keV and %3.0f keV (%s)",
                                       Egamma1, Egamma2, Name.c_str()),
                                  180, 0, 180));
    TrackHisto.ptrs.push_back(
        TrackHisto.h_gg_ang_corr = new TH1D(
            "h_gg_ang_corr",
            Form("Angular correlation in %s between %3.0f keV and %3.0f keV ",
                 Name.c_str(), Egamma1, Egamma2),
            180, 0, 180));
    TrackHisto.ptrs.push_back(
        TrackHisto.h_gg_Theta_uncorrelated = new TH1D(
            "h_gg_Theta_uncorrelated",
            "Angular correlation between uncorrelated gammas", 180, 0, 180));
    TrackHisto.ptrs.push_back(
        TrackHisto.h_gg_Theta_bg = new TH1D(
            "h_gg_Theta_bg",
            Form(
                "BG random coincidence in %s between %3.0f keV and %3.0f keV ",
                Name.c_str(), Egamma1, Egamma2),
            180, 0, 180));
    TrackHisto.ptrs.push_back(
        TrackHisto.h_gg_Theta_bg_uncorrelated
        = new TH1D("h_gg_Theta_bg_uncorrelated",
                   Form("BG uncorrelated angular correlation in %s between "
                        "%3.0f keV and %3.0f keV ",
                        Name.c_str(), Egamma1, Egamma2),
                   180, 0, 180));
    TrackHisto.ptrs.push_back(
        TrackHisto.h_gg_Theta_random = new TH1D(
            "h_gg_Theta_random",
            Form("Random coincidence in %s between %3.0f keV and %3.0f keV ",
                 Name.c_str(), Egamma1, Egamma2),
            180, 0, 180));

    AddHisto.dir = dir->mkdir("AddBack");
    AddHisto.dir->cd();

    AddHisto.ptrs.push_back(
        AddHisto.h_singles = new TH1D(
            "h_singles", Form("Energy spectrum in singles (%s)", Name.c_str()),
            4000, 0, 4000));
    AddHisto.ptrs.push_back(
        AddHisto.h_gamma_gamma
        = new TH2D("h_gamma_gamma",
                   Form("Energy spectrum gamma-gamma (%s)", Name.c_str()),
                   4000, 0, 4000, 4000, 0, 4000));
    AddHisto.ptrs.push_back(
        AddHisto.h_id_vs_time_gated
        = new TH2D("h_id_vs_time_gated",
                   Form("Counting rate per detector vs time (%s) ; Time [s]",
                        Name.c_str()),
                   12 * 360, 0, 12 * 3600, 45, -0.5, 44.5));
    AddHisto.ptrs.push_back(
        AddHisto.h_id_vs_time_total
        = new TH2D("h_id_vs_time_total",
                   Form("Counting rate per detector vs time (%s) ; Time [s]",
                        Name.c_str()),
                   12 * 360, 0, 12 * 3600, 45, -0.5, 44.5));

    AddHisto.ptrs.push_back(AddHisto.h_gg_Theta = new TH1D(
                                "h_gg_Theta",
                                Form("Angular separation distribution between "
                                     "%3.0f keV and %3.0f keV (%s)",
                                     Egamma1, Egamma2, Name.c_str()),
                                180, 0, 180));
    AddHisto.ptrs.push_back(
        AddHisto.h_gg_ang_corr = new TH1D(
            "h_gg_ang_corr",
            Form("Angular correlation in %s between %3.0f keV and %3.0f keV ",
                 Name.c_str(), Egamma1, Egamma2),
            180, 0, 180));
    AddHisto.ptrs.push_back(
        AddHisto.h_gg_Theta_uncorrelated = new TH1D(
            "h_gg_Theta_uncorrelated",
            "Angular correlation between uncorrelated gammas", 180, 0, 180));
    AddHisto.ptrs.push_back(
        AddHisto.h_gg_Theta_bg = new TH1D(
            "h_gg_Theta_bg",
            Form(
                "BG random coincidence in %s between %3.0f keV and %3.0f keV ",
                Name.c_str(), Egamma1, Egamma2),
            180, 0, 180));
    AddHisto.ptrs.push_back(AddHisto.h_gg_Theta_bg_uncorrelated = new TH1D(
                                "h_gg_Theta_bg_uncorrelated",
                                Form("BG uncorrelated angular correlation in "
                                     "%s between %3.0f keV and %3.0f keV ",
                                     Name.c_str(), Egamma1, Egamma2),
                                180, 0, 180));
    AddHisto.ptrs.push_back(
        AddHisto.h_gg_Theta_random = new TH1D(
            "h_gg_Theta_random",
            Form("Random coincidence in %s between %3.0f keV and %3.0f keV ",
                 Name.c_str(), Egamma1, Egamma2),
            180, 0, 180));

    // if(CrysSpectra){
    // std::cout<<"Here 1 "<<CrysSpectra<<std::endl;

    crysHistos.dir = dir->mkdir("Crystals");
    crysHistos.dir->cd();

    // std::cout<<"Here 2 "<<CrysSpectra<<std::endl;

    crysHistos.ptrs.push_back(
        crysHistos.h_core_0
        = new TH1D(Form("h_core_0"), Form("h_core_0"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_1
        = new TH1D(Form("h_core_1"), Form("h_core_1"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_2
        = new TH1D(Form("h_core_2"), Form("h_core_2"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_3
        = new TH1D(Form("h_core_3"), Form("h_core_3"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_4
        = new TH1D(Form("h_core_4"), Form("h_core_4"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_5
        = new TH1D(Form("h_core_5"), Form("h_core_5"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_6
        = new TH1D(Form("h_core_6"), Form("h_core_6"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_7
        = new TH1D(Form("h_core_7"), Form("h_core_7"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_8
        = new TH1D(Form("h_core_8"), Form("h_core_8"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_9
        = new TH1D(Form("h_core_9"), Form("h_core_9"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_10
        = new TH1D(Form("h_core_10"), Form("h_core_10"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_11
        = new TH1D(Form("h_core_11"), Form("h_core_11"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_12
        = new TH1D(Form("h_core_12"), Form("h_core_12"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_13
        = new TH1D(Form("h_core_13"), Form("h_core_13"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_14
        = new TH1D(Form("h_core_14"), Form("h_core_14"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_15
        = new TH1D(Form("h_core_15"), Form("h_core_15"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_16
        = new TH1D(Form("h_core_16"), Form("h_core_16"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_17
        = new TH1D(Form("h_core_17"), Form("h_core_17"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_18
        = new TH1D(Form("h_core_18"), Form("h_core_18"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_19
        = new TH1D(Form("h_core_19"), Form("h_core_19"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_20
        = new TH1D(Form("h_core_20"), Form("h_core_20"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_21
        = new TH1D(Form("h_core_21"), Form("h_core_21"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_22
        = new TH1D(Form("h_core_22"), Form("h_core_22"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_23
        = new TH1D(Form("h_core_23"), Form("h_core_23"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_24
        = new TH1D(Form("h_core_24"), Form("h_core_24"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_25
        = new TH1D(Form("h_core_25"), Form("h_core_25"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_26
        = new TH1D(Form("h_core_26"), Form("h_core_26"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_27
        = new TH1D(Form("h_core_27"), Form("h_core_27"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_28
        = new TH1D(Form("h_core_28"), Form("h_core_28"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_29
        = new TH1D(Form("h_core_29"), Form("h_core_29"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_30
        = new TH1D(Form("h_core_30"), Form("h_core_30"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_31
        = new TH1D(Form("h_core_31"), Form("h_core_31"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_32
        = new TH1D(Form("h_core_32"), Form("h_core_32"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_33
        = new TH1D(Form("h_core_33"), Form("h_core_33"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_34
        = new TH1D(Form("h_core_34"), Form("h_core_34"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_35
        = new TH1D(Form("h_core_35"), Form("h_core_35"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_36
        = new TH1D(Form("h_core_36"), Form("h_core_36"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_37
        = new TH1D(Form("h_core_37"), Form("h_core_37"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_38
        = new TH1D(Form("h_core_38"), Form("h_core_38"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_39
        = new TH1D(Form("h_core_39"), Form("h_core_39"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_40
        = new TH1D(Form("h_core_40"), Form("h_core_40"), 4000, 0, 4000));
    crysHistos.ptrs.push_back(
        crysHistos.h_core_41
        = new TH1D(Form("h_core_41"), Form("h_core_41"), 4000, 0, 4000));

    //}
    //  std::cout<<"CrysSpectra "<<CrysSpectra<<std::endl;

    return;
};
void UserSelector::UserHistograms::Coincidence::Compute(
    AgataAnalysis::AgataReader& agataReader) {
    int verb = 0;

    if(verb > 1) std::cout << "Coincidence Compute" << std::endl;

    if(**agataReader.nbHits == 0) return;

    if(**agataReader.nbCores > 0) {
        for(int i = 0; i < **agataReader.nbCores; ++i) {
            CoreHisto.h_singles->Fill(agataReader.coreE0->At(i));

            double timeCore = unsignedDiff<double, unsigned long long>(
                                  agataReader.coreTS->At(i), InitTS)
                            / 1e8;
            CoreHisto.h_id_vs_time_gated->Fill(timeCore,
                                               agataReader.coreId->At(i));

            // std::cout<<"CrysSpectra 2 "<<CrysSpectra<<std::endl;

            if(CrysSpectra) {
                // std::cout<<"Here"<<std::endl;
                if(agataReader.coreId->At(i) == 0)
                    crysHistos.h_core_0->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 1)
                    crysHistos.h_core_1->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 2)
                    crysHistos.h_core_2->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 3)
                    crysHistos.h_core_3->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 4)
                    crysHistos.h_core_4->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 5)
                    crysHistos.h_core_5->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 6)
                    crysHistos.h_core_6->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 7)
                    crysHistos.h_core_7->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 8)
                    crysHistos.h_core_8->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 9)
                    crysHistos.h_core_9->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 10)
                    crysHistos.h_core_10->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 11)
                    crysHistos.h_core_11->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 12)
                    crysHistos.h_core_12->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 13)
                    crysHistos.h_core_13->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 14)
                    crysHistos.h_core_14->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 15)
                    crysHistos.h_core_15->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 16)
                    crysHistos.h_core_16->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 17)
                    crysHistos.h_core_17->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 18)
                    crysHistos.h_core_18->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 19)
                    crysHistos.h_core_19->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 20)
                    crysHistos.h_core_20->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 21)
                    crysHistos.h_core_21->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 22)
                    crysHistos.h_core_22->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 23)
                    crysHistos.h_core_23->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 24)
                    crysHistos.h_core_24->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 25)
                    crysHistos.h_core_25->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 26)
                    crysHistos.h_core_26->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 27)
                    crysHistos.h_core_27->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 28)
                    crysHistos.h_core_28->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 29)
                    crysHistos.h_core_29->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 30)
                    crysHistos.h_core_30->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 31)
                    crysHistos.h_core_31->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 32)
                    crysHistos.h_core_32->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 33)
                    crysHistos.h_core_33->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 34)
                    crysHistos.h_core_34->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 35)
                    crysHistos.h_core_35->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 36)
                    crysHistos.h_core_36->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 37)
                    crysHistos.h_core_37->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 38)
                    crysHistos.h_core_38->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 39)
                    crysHistos.h_core_39->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 40)
                    crysHistos.h_core_40->Fill(agataReader.coreE0->At(i));
                if(agataReader.coreId->At(i) == 41)
                    crysHistos.h_core_41->Fill(agataReader.coreE0->At(i));
            }
        }
    }

    if(**agataReader.nbTrack > 0) {
        for(int i = 0; i < **agataReader.nbTrack; ++i) {
            TrackHisto.h_singles->Fill(agataReader.trackE->At(i));

            double timeTrack = unsignedDiff<double, unsigned long long>(
                                   **agataReader.TStrack, InitTS)
                             / 1e8;
            TrackHisto.h_id_vs_time_gated->Fill(
                timeTrack, agataReader.trackCrystalID->At(i));
        }
    }

    if(**agataReader.nbAdd > 0) {
        for(int i = 0; i < **agataReader.nbAdd; ++i) {
            AddHisto.h_singles->Fill(agataReader.AddE->At(i));

            double timeAdd = unsignedDiff<double, unsigned long long>(
                                 agataReader.AddTS->At(i), InitTS)
                           / 1e8;
            AddHisto.h_id_vs_time_gated->Fill(timeAdd,
                                              agataReader.AddId->At(i));
        }
    }

    // if(Egamma1 < 0 || Egamma2 < 0 ) return;

    //=================================================================================================================================
    //============================================================= CORES
    //=============================================================

    unsigned long long lowestCoreTS{
        std::numeric_limits<unsigned long long>::max()};
    double                    lowestCoreT{0};
    int                       lowestCoreIdx{0};
    int                       mult_cores = **agataReader.nbCores;
    std::vector<unsigned int> idx_cores;
    std::vector<unsigned int> idx_cores_random;

    // Get lowest TS cores

    for(int i{0}; i < mult_cores; ++i) {
        if(lowestCoreTS > agataReader.coreTS->At(i)) {
            lowestCoreTS  = agataReader.coreTS->At(i);
            lowestCoreIdx = i;
            lowestCoreT   = agataReader.coreT0->At(i) - 39.5;
        }
    }

    if(verb > 1) std::cout << "Coinc cores lowestTS" << std::endl;

    // Add indices for coincidence peak and random coincidence

    idx_cores.push_back(lowestCoreIdx);
    idx_cores_random.push_back(lowestCoreIdx);
    for(int i{0}; i < mult_cores; ++i) {
        if(agataReader.coreTS->At(i) != 0 && i != lowestCoreIdx
           && unsignedDiff<double, unsigned long long>(
                  agataReader.coreTS->At(i), lowestCoreTS)
                      + agataReader.coreT0->At(i) - 39.5 - lowestCoreT
                  >= 0
           && unsignedDiff<double, unsigned long long>(
                  agataReader.coreTS->At(i), lowestCoreTS)
                      + agataReader.coreT0->At(i) - 39.5 - lowestCoreT
                  < 10)
            idx_cores.push_back(i);
        if(agataReader.coreTS->At(i) != 0 && i != lowestCoreIdx
           && unsignedDiff<double, unsigned long long>(
                  agataReader.coreTS->At(i), lowestCoreTS)
                      + agataReader.coreT0->At(i) - 39.5 - lowestCoreT
                  >= 10
           && unsignedDiff<double, unsigned long long>(
                  agataReader.coreTS->At(i), lowestCoreTS)
                      + agataReader.coreT0->At(i) - 39.5 - lowestCoreT
                  < 20)
            idx_cores_random.push_back(i);
    }

    if(verb > 1) std::cout << "Coinc cores time coincidence" << std::endl;

    if(core_vec_pos_1.size() < 1000) {
        // Add core positions to uncorrelated vectors
        for(unsigned int i{0}; i < mult_cores; ++i) {
            if(std::count(idx_cores.begin(), idx_cores.end(), i)) {
                double x_i, y_i, z_i;
                float  maxEneHit_i{0};
                // Calculate core positions from hits
                for(int k{0}; k < **agataReader.nbHits; ++k) {
                    if(agataReader.hitId->At(k) == agataReader.coreId->At(i)
                       && agataReader.hitE->At(k) > maxEneHit_i) {
                        maxEneHit_i = agataReader.hitE->At(k);
                        x_i         = agataReader.hitGX->At(k);
                        y_i         = agataReader.hitGY->At(k);
                        z_i         = agataReader.hitGZ->At(k);
                    }
                }

                double e_i       = agataReader.coreE0->At(i);

                auto   e1_out_sx = Elow1 - LeftShift1;
                auto   e1_out_dx = Ehigh1 + RightShift1;
                auto   e2_out_sx = Elow2 - LeftShift2;
                auto   e2_out_dx = Ehigh2 + RightShift2;

                if(e_i > Elow1 && e_i < Ehigh1) {
                    core_vec_pos_1.push_back(TVector3(
                        y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm));
                    break;
                }
                if(e_i > Elow2 && e_i < Ehigh2) {
                    core_vec_pos_2.push_back(TVector3(
                        y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm));
                    break;
                }
                if((e_i > e1_out_sx - GateWidth1 / 2 && e_i < e1_out_sx)
                   || (e_i > e1_out_dx && e_i < e1_out_dx + GateWidth1 / 2)) {
                    bg_core_vec_pos_1.push_back(TVector3(
                        y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm));
                    break;
                }
                if((e_i > e2_out_sx - GateWidth2 / 2 && e_i < e2_out_sx)
                   || (e_i > e2_out_dx && e_i < e2_out_dx + GateWidth2 / 2)) {
                    bg_core_vec_pos_2.push_back(TVector3(
                        y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm));
                    break;
                }
            }
            if(std::count(idx_cores_random.begin(), idx_cores_random.end(),
                          i)) {
                double x_i, y_i, z_i;
                float  maxEneHit_i{0};
                // Calculate core positions from hits
                for(int k{0}; k < **agataReader.nbHits; ++k) {
                    if(agataReader.hitId->At(k) == agataReader.coreId->At(i)
                       && agataReader.hitE->At(k) > maxEneHit_i) {
                        maxEneHit_i = agataReader.hitE->At(k);
                        x_i         = agataReader.hitGX->At(k);
                        y_i         = agataReader.hitGY->At(k);
                        z_i         = agataReader.hitGZ->At(k);
                    }
                }

                double e_i       = agataReader.coreE0->At(i);

                auto   e1_out_sx = Elow1 - LeftShift1;
                auto   e1_out_dx = Ehigh1 + RightShift1;
                auto   e2_out_sx = Elow2 - LeftShift2;
                auto   e2_out_dx = Ehigh2 + RightShift2;

                if(e_i > Elow1 && e_i < Ehigh1) {
                    random_core_vec_pos_1.push_back(TVector3(
                        y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm));
                    break;
                }
                if(e_i > Elow2 && e_i < Ehigh2) {
                    random_core_vec_pos_2.push_back(TVector3(
                        y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm));
                    break;
                }
            }
        }
    }

    if(verb > 1) std::cout << "Coinc cores position vectors" << std::endl;

    // Loop on coincident gammas and fill angular distribution histograms

    for(unsigned int i{0}; i < idx_cores.size(); ++i) {
        for(unsigned int j{0}; j < i; ++j) {
            double x_i, y_i, z_i;
            double x_j, y_j, z_j;
            float  maxEneHit_i{0};
            float  maxEneHit_j{0};

            // Calculate core positions from hits for core of index i and core
            // of index j
            for(int k{0}; k < **agataReader.nbHits; ++k) {
                if(agataReader.hitId->At(k)
                       == agataReader.coreId->At(idx_cores[i])
                   && agataReader.hitE->At(k) > maxEneHit_i) {
                    maxEneHit_i = agataReader.hitE->At(k);
                    x_i         = agataReader.hitGX->At(k);
                    y_i         = agataReader.hitGY->At(k);
                    z_i         = agataReader.hitGZ->At(k);
                }
                if(agataReader.hitId->At(k)
                       == agataReader.coreId->At(idx_cores[j])
                   && agataReader.hitE->At(k) > maxEneHit_j) {
                    maxEneHit_j = agataReader.hitE->At(k);
                    x_j         = agataReader.hitGX->At(k);
                    y_j         = agataReader.hitGY->At(k);
                    z_j         = agataReader.hitGZ->At(k);
                }
            }

            TVector3 pos_i
                = TVector3(y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm);
            TVector3 pos_j
                = TVector3(y_j * UNITS::mm, -x_j * UNITS::mm, z_j * UNITS::mm);
            double theta = pos_i.Angle(pos_j) * 180 / TMath::Pi();

            auto   e_i   = agataReader.coreE0->At(idx_cores[i]);
            auto   e_j   = agataReader.coreE0->At(idx_cores[j]);

            CoreHisto.h_gamma_gamma->Fill(e_i, e_j);
            CoreHisto.h_gamma_gamma->Fill(e_j, e_i);

            auto e1_out_sx = Elow1 - LeftShift1;
            auto e1_out_dx = Ehigh1 + RightShift1;

            auto e2_out_sx = Elow2 - LeftShift2;
            auto e2_out_dx = Ehigh2 + RightShift2;

            if(((e_i > Elow1 && e_i < Ehigh1 && e_j > Elow2 && e_j < Ehigh2)
                || (e_j > Elow1 && e_j < Ehigh1 && e_i > Elow2
                    && e_i < Ehigh2))) {
                CoreHisto.h_gg_Theta->Fill(theta);
                CoreHisto.h_gg_ang_corr->Fill(theta);
            }

            if(((e_i > e1_out_sx - GateWidth1 / 2 && e_i < e1_out_sx
                 && e_j > Elow2 && e_j < Ehigh2)
                || (e_j > e1_out_sx - GateWidth1 / 2 && e_j < e1_out_sx
                    && e_i > Elow2 && e_i < Ehigh2)
                || (e_i > e1_out_dx && e_i < e1_out_dx + GateWidth1 / 2
                    && e_j > Elow2 && e_j < Ehigh2)
                || (e_j > e1_out_dx && e_j < e1_out_dx + GateWidth1 / 2
                    && e_i > Elow2 && e_i < Ehigh2)
                ||

                (e_i > e2_out_sx - GateWidth2 / 2 && e_i < e2_out_sx
                 && e_j > Elow1 && e_j < Ehigh1)
                || (e_j > e2_out_sx - GateWidth2 / 2 && e_j < e2_out_sx
                    && e_i > Elow1 && e_i < Ehigh1)
                || (e_i > e2_out_dx && e_i < e2_out_dx + GateWidth2 / 2
                    && e_j > Elow1 && e_j < Ehigh1)
                || (e_j > e2_out_dx && e_j < e2_out_dx + GateWidth2 / 2
                    && e_i > Elow1 && e_i < Ehigh1)
                ||

                (e_i > e2_out_sx - GateWidth2 / 2 && e_i < e2_out_sx
                 && e_j > e1_out_sx - GateWidth1 / 2 && e_j < e1_out_sx)
                || (e_j > e2_out_sx - GateWidth2 / 2 && e_j < e2_out_sx
                    && e_i > e1_out_sx - GateWidth1 / 2 && e_i < e1_out_sx)
                || (e_i > e2_out_dx && e_i < e2_out_dx + GateWidth2 / 2
                    && e_j > e1_out_dx && e_j < e1_out_dx + GateWidth1 / 2)
                || (e_j > e2_out_dx && e_j < e2_out_dx + GateWidth2 / 2
                    && e_i > e1_out_dx && e_i < e1_out_dx + GateWidth1 / 2)
                || (e_i > e2_out_sx - GateWidth2 / 2 && e_i < e2_out_sx
                    && e_j > e1_out_dx && e_j < e1_out_dx + GateWidth1 / 2)
                || (e_j > e2_out_sx - GateWidth2 / 2 && e_j < e2_out_sx
                    && e_i > e1_out_dx && e_i < e1_out_dx + GateWidth1 / 2)
                || (e_i > e2_out_dx && e_i < e2_out_dx + GateWidth2 / 2
                    && e_j > e1_out_sx - GateWidth1 / 2 && e_j < e1_out_sx)
                || (e_j > e2_out_dx && e_j < e2_out_dx + GateWidth2 / 2
                    && e_i > e1_out_sx - GateWidth1 / 2 && e_i < e1_out_sx))) {
                CoreHisto.h_gg_Theta_bg->Fill(theta);
            }
        }
    }

    if(verb > 1)
        std::cout << "Coinc cores filled angular distributions" << std::endl;

    // Loop on random coincidences

    if(idx_cores_random.size() > 1) {
        for(unsigned int i{0}; i < idx_cores_random.size(); ++i) {
            for(unsigned int j{0}; j < i; ++j) {
                double x_i, y_i, z_i;
                double x_j, y_j, z_j;
                float  maxEneHit_i{0};
                float  maxEneHit_j{0};
                if(verb > 1)
                    std::cout << "Loop cores random angular distributions"
                              << std::endl;
                for(int k{0}; k < **agataReader.nbHits; ++k) {
                    if(agataReader.hitId->At(k)
                           == agataReader.coreId->At(idx_cores_random[i])
                       && agataReader.hitE->At(k) > maxEneHit_i) {
                        maxEneHit_i = agataReader.hitE->At(k);
                        x_i         = agataReader.hitGX->At(k);
                        y_i         = agataReader.hitGY->At(k);
                        z_i         = agataReader.hitGZ->At(k);
                        if(verb > 1)
                            std::cout
                                << " Cores random i "
                                << agataReader.hitId->At(k) << "  "
                                << agataReader.coreId->At(idx_cores_random[i])
                                << std::endl;
                    }

                    if(agataReader.hitId->At(k)
                           == agataReader.coreId->At(idx_cores_random[j])
                       && agataReader.hitE->At(k) > maxEneHit_j) {
                        maxEneHit_j = agataReader.hitE->At(k);
                        x_j         = agataReader.hitGX->At(k);
                        y_j         = agataReader.hitGY->At(k);
                        z_j         = agataReader.hitGZ->At(k);
                        if(verb > 1)
                            std::cout
                                << " Cores random j "
                                << agataReader.hitId->At(k) << "  "
                                << agataReader.coreId->At(idx_cores_random[j])
                                << std::endl;
                    }
                }

                TVector3 pos_i = TVector3(y_i * UNITS::mm, -x_i * UNITS::mm,
                                          z_i * UNITS::mm);
                TVector3 pos_j = TVector3(y_j * UNITS::mm, -x_j * UNITS::mm,
                                          z_j * UNITS::mm);
                double   theta = pos_i.Angle(pos_j) * 180 / TMath::Pi();
                if(verb > 1) std::cout << " Cores random angle" << std::endl;
                auto e_i = agataReader.coreE0->At(idx_cores_random[i]);
                auto e_j = agataReader.coreE0->At(idx_cores_random[j]);

                if((e_i > Elow1 && e_i < Ehigh1 && e_j > Elow2 && e_j < Ehigh2)
                   || (e_j > Elow1 && e_j < Ehigh1 && e_i > Elow2
                       && e_i < Ehigh2)) {
                    CoreHisto.h_gg_Theta_random->Fill(theta);
                }
            }
        }
    }

    if(verb > 1)
        std::cout << "Coinc cores filled random angular distributions"
                  << std::endl;

    //===================================================================================================================================
    //============================================================= TRACKED
    //=============================================================

    unsigned long long lowestTrackT{
        std::numeric_limits<unsigned long long>::max()};
    int                       lowestTrackIdx{0};
    int                       mult_tracks = **agataReader.nbTrack;
    std::vector<unsigned int> idx_tracks;
    std::vector<unsigned int> idx_tracks_random;

    // Get lowest TS tracked
    for(int i{0}; i < mult_tracks; ++i) {
        if(lowestTrackT > agataReader.trackT->At(i)) {
            lowestTrackT   = agataReader.trackT->At(i) - 39.5;
            lowestTrackIdx = i;
        }
    }
    if(verb > 1) std::cout << "Coinc tracked lowestTS" << std::endl;

    // Add indices for coincidence peak and random coincidence
    idx_tracks.push_back(lowestTrackIdx);
    idx_tracks_random.push_back(lowestTrackIdx);
    for(int i{0}; i < mult_tracks; ++i) {
        if(**agataReader.TStrack != 0 && i != lowestTrackIdx
           && agataReader.trackT->At(i) - 39.5 - lowestTrackT >= 0
           && agataReader.trackT->At(i) - 39.5 - lowestTrackT < 10)
            idx_tracks.push_back(i);
        if(**agataReader.TStrack != 0 && i != lowestTrackIdx
           && agataReader.trackT->At(i) - 39.5 - lowestTrackT >= 10
           && agataReader.trackT->At(i) - 39.5 - lowestTrackT < 20)
            idx_tracks_random.push_back(i);
    }

    if(verb > 1) std::cout << "Coinc tracked time coincidence" << std::endl;

    // Add core positions to uncorrelated vectors
    for(unsigned int i{0}; i < idx_tracks.size(); ++i) {
        double e_i       = agataReader.trackE->At(idx_tracks[i]);

        auto   e1_out_sx = Elow1 - LeftShift1;
        auto   e1_out_dx = Ehigh1 + RightShift1;
        auto   e2_out_sx = Elow2 - LeftShift2;
        auto   e2_out_dx = Ehigh2 + RightShift2;

        double x_i       = agataReader.trackX1->At(i);
        double y_i       = agataReader.trackY1->At(i);
        double z_i       = agataReader.trackZ1->At(i);

        if(tracked_vec_pos_1.size() < 1000) {
            if(e_i > Elow1 && e_i < Ehigh1) {
                tracked_vec_pos_1.push_back(TVector3(
                    y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm));
                break;
            }
            if(e_i > Elow2 && e_i < Ehigh2) {
                tracked_vec_pos_2.push_back(TVector3(
                    y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm));
                break;
            }
            if((e_i > e1_out_sx - GateWidth1 / 2 && e_i < e1_out_sx)
               || (e_i > e1_out_dx && e_i < e1_out_dx + GateWidth1 / 2)) {
                bg_tracked_vec_pos_1.push_back(TVector3(
                    y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm));
                break;
            }
            if((e_i > e2_out_sx - GateWidth2 / 2 && e_i < e2_out_sx)
               || (e_i > e2_out_dx && e_i < e2_out_dx + GateWidth2 / 2)) {
                bg_tracked_vec_pos_2.push_back(TVector3(
                    y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm));
                break;
            }
        }
    }

    if(verb > 1) std::cout << "Coinc tracked position vectors" << std::endl;

    // Loop on coincident gammas and fill angular distribution histograms

    for(unsigned int i{0}; i < idx_tracks.size(); ++i) {
        for(unsigned int j{0}; j < i; ++j) {
            double   x_i = agataReader.trackX1->At(i);
            double   y_i = agataReader.trackY1->At(i);
            double   z_i = agataReader.trackZ1->At(i);

            double   x_j = agataReader.trackX1->At(j);
            double   y_j = agataReader.trackY1->At(j);
            double   z_j = agataReader.trackZ1->At(j);

            TVector3 pos_i
                = TVector3(y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm);
            TVector3 pos_j
                = TVector3(y_j * UNITS::mm, -x_j * UNITS::mm, z_j * UNITS::mm);

            double theta = pos_i.Angle(pos_j) * 180 / TMath::Pi();

            auto   e_i   = agataReader.trackE->At(idx_tracks[i]);
            auto   e_j   = agataReader.trackE->At(idx_tracks[j]);

            TrackHisto.h_gamma_gamma->Fill(e_i, e_j);
            TrackHisto.h_gamma_gamma->Fill(e_j, e_i);

            auto e1_out_sx = Elow1 - LeftShift1;
            auto e1_out_dx = Ehigh1 + RightShift1;

            auto e2_out_sx = Elow2 - LeftShift2;
            auto e2_out_dx = Ehigh2 + RightShift2;

            if((e_i > Elow1 && e_i < Ehigh1 && e_j > Elow2 && e_j < Ehigh2)
               || (e_j > Elow1 && e_j < Ehigh1 && e_i > Elow2
                   && e_i < Ehigh2)) {
                TrackHisto.h_gg_Theta->Fill(theta);
                TrackHisto.h_gg_ang_corr->Fill(theta);
            }
            if((e_i > e1_out_sx - GateWidth1 / 2 && e_i < e1_out_sx
                && e_j > Elow2 && e_j < Ehigh2)
               || (e_j > e1_out_sx - GateWidth1 / 2 && e_j < e1_out_sx
                   && e_i > Elow2 && e_i < Ehigh2)
               || (e_i > e1_out_dx && e_i < e1_out_dx + GateWidth1 / 2
                   && e_j > Elow2 && e_j < Ehigh2)
               || (e_j > e1_out_dx && e_j < e1_out_dx + GateWidth1 / 2
                   && e_i > Elow2 && e_i < Ehigh2)
               ||

               (e_i > e2_out_sx - GateWidth2 / 2 && e_i < e2_out_sx
                && e_j > Elow1 && e_j < Ehigh1)
               || (e_j > e2_out_sx - GateWidth2 / 2 && e_j < e2_out_sx
                   && e_i > Elow1 && e_i < Ehigh1)
               || (e_i > e2_out_dx && e_i < e2_out_dx + GateWidth2 / 2
                   && e_j > Elow1 && e_j < Ehigh1)
               || (e_j > e2_out_dx && e_j < e2_out_dx + GateWidth2 / 2
                   && e_i > Elow1 && e_i < Ehigh1)) {
                TrackHisto.h_gg_Theta_bg->Fill(theta);
            }
        }
    }

    if(verb > 1)
        std::cout << "Coinc tracked filled angular distributions" << std::endl;

    // Loop on random coincidences

    for(unsigned int i{0}; i < idx_tracks_random.size(); ++i) {
        for(unsigned int j{0}; j < i; ++j) {
            double   x_i = agataReader.trackX1->At(i);
            double   y_i = agataReader.trackY1->At(i);
            double   z_i = agataReader.trackZ1->At(i);

            double   x_j = agataReader.trackX1->At(j);
            double   y_j = agataReader.trackY1->At(j);
            double   z_j = agataReader.trackZ1->At(j);

            TVector3 pos_i
                = TVector3(y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm);
            TVector3 pos_j
                = TVector3(y_j * UNITS::mm, -x_j * UNITS::mm, z_j * UNITS::mm);

            double theta = pos_i.Angle(pos_j) * 180 / TMath::Pi();

            auto   e_i   = agataReader.trackE->At(idx_tracks_random[i]);
            auto   e_j   = agataReader.trackE->At(idx_tracks_random[j]);

            if((e_i > Elow1 && e_i < Ehigh1 && e_j > Elow2 && e_j < Ehigh2)
               || (e_j > Elow1 && e_j < Ehigh1 && e_i > Elow2
                   && e_i < Ehigh2)) {
                TrackHisto.h_gg_Theta_random->Fill(theta);
            }
        }
    }

    if(verb > 1)
        std::cout << "Coinc tracked filled random angular distributions"
                  << std::endl;

    //===================================================================================================================================
    //============================================================= ADDBACK
    //=============================================================

    unsigned long long lowestAddTS{
        std::numeric_limits<unsigned long long>::max()};
    double                    lowestAddT{0};
    int                       lowestAddIdx{0};
    int                       mult_add = **agataReader.nbAdd;
    std::vector<unsigned int> idx_add;
    std::vector<unsigned int> idx_add_random;

    // Get lowest TS addback

    for(int i{0}; i < mult_add; ++i) {
        if(lowestAddTS > agataReader.AddTS->At(i)) {
            lowestAddTS  = agataReader.AddTS->At(i);
            lowestAddIdx = i;
            for(int j{0}; j < **agataReader.nbCores; ++j) {
                if(agataReader.coreId->At(j) == agataReader.AddId->At(i))
                    lowestAddT = agataReader.coreT0->At(j) - 39.5;
            }
        }
    }

    if(verb > 1) std::cout << "Coinc addback lowestTS" << std::endl;

    // Add indices for coincidence peak and random coincidence

    idx_add.push_back(lowestCoreIdx);
    idx_add_random.push_back(lowestCoreIdx);
    for(int i{0}; i < mult_add; ++i) {
        int coreIndex;
        for(int j{0}; j < **agataReader.nbCores; ++j) {
            if(agataReader.coreId->At(j) == agataReader.AddId->At(i)) {
                coreIndex = j;
                break;
            }
        }

        if(agataReader.AddTS->At(i) != 0 && i != lowestAddIdx
           && unsignedDiff<double, unsigned long long>(
                  agataReader.AddTS->At(i), lowestAddTS)
                      + agataReader.coreT0->At(coreIndex) - 39.5 - lowestAddT
                  >= 0
           && agataReader.AddTS->At(i) != 0 && i != lowestAddIdx
           && unsignedDiff<double, unsigned long long>(
                  agataReader.AddTS->At(i), lowestAddTS)
                      + agataReader.coreT0->At(coreIndex) - 39.5 - lowestAddT
                  < 10)
            idx_add.push_back(i);
        if(agataReader.AddTS->At(i) != 0 && i != lowestAddIdx
           && unsignedDiff<double, unsigned long long>(
                  agataReader.AddTS->At(i), lowestAddTS)
                      + agataReader.coreT0->At(coreIndex) - 39.5 - lowestAddT
                  >= 10
           && agataReader.AddTS->At(i) != 0 && i != lowestAddIdx
           && unsignedDiff<double, unsigned long long>(
                  agataReader.AddTS->At(i), lowestAddTS)
                      + agataReader.coreT0->At(coreIndex) - 39.5 - lowestAddT
                  < 20)
            idx_add_random.push_back(i);
    }

    if(verb > 1) std::cout << "Coinc addback time coincidence" << std::endl;

    // Add addback positions to uncorrelated vectors
    for(unsigned int i{0}; i < idx_add.size(); ++i) {
        double x_i       = agataReader.AddX->At(i);
        double y_i       = agataReader.AddY->At(i);
        double z_i       = agataReader.AddZ->At(i);

        double e_i       = agataReader.AddE->At(idx_add[i]);

        auto   e1_out_sx = Elow1 - LeftShift1;
        auto   e1_out_dx = Ehigh1 + RightShift1;
        auto   e2_out_sx = Elow2 - LeftShift2;
        auto   e2_out_dx = Ehigh2 + RightShift2;

        if(core_vec_pos_1.size() < 1000) {
            if(e_i > Elow1 && e_i < Ehigh1) {
                addback_vec_pos_1.push_back(TVector3(
                    y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm));
                break;
            }
            if(e_i > Elow2 && e_i < Ehigh2) {
                addback_vec_pos_2.push_back(TVector3(
                    y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm));
                break;
            }
            if((e_i > e1_out_sx - GateWidth1 / 2 && e_i < e1_out_sx)
               || (e_i > e1_out_dx && e_i < e1_out_dx + GateWidth1 / 2)) {
                bg_addback_vec_pos_1.push_back(TVector3(
                    y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm));
                break;
            }
            if((e_i > e2_out_sx - GateWidth2 / 2 && e_i < e2_out_sx)
               || (e_i > e2_out_dx && e_i < e2_out_dx + GateWidth2 / 2)) {
                bg_addback_vec_pos_2.push_back(TVector3(
                    y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm));
                break;
            }
        }
    }

    if(verb > 1) std::cout << "Coinc addback position vectors" << std::endl;

    // Loop on coincident gammas and fill angular distribution histograms

    for(unsigned int i{0}; i < idx_add.size(); ++i) {
        for(unsigned int j{0}; j < i; ++j) {
            double   x_i = agataReader.AddX->At(i);
            double   y_i = agataReader.AddY->At(i);
            double   z_i = agataReader.AddZ->At(i);

            double   x_j = agataReader.AddX->At(j);
            double   y_j = agataReader.AddY->At(j);
            double   z_j = agataReader.AddZ->At(j);

            TVector3 pos_i
                = TVector3(y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm);
            TVector3 pos_j
                = TVector3(y_j * UNITS::mm, -x_j * UNITS::mm, z_j * UNITS::mm);

            double theta = pos_i.Angle(pos_j) * 180 / TMath::Pi();

            auto   e_i   = agataReader.AddE->At(idx_add[i]);
            auto   e_j   = agataReader.AddE->At(idx_add[j]);

            AddHisto.h_gamma_gamma->Fill(e_i, e_j);
            AddHisto.h_gamma_gamma->Fill(e_j, e_i);

            auto e1_out_sx = Elow1 - LeftShift1;
            auto e1_out_dx = Ehigh1 + RightShift1;

            auto e2_out_sx = Elow2 - LeftShift2;
            auto e2_out_dx = Ehigh2 + RightShift2;

            if((e_i > Elow1 && e_i < Ehigh1 && e_j > Elow2 && e_j < Ehigh2)
               || (e_j > Elow1 && e_j < Ehigh1 && e_i > Elow2
                   && e_i < Ehigh2)) {
                AddHisto.h_gg_Theta->Fill(theta);
                AddHisto.h_gg_ang_corr->Fill(theta);
            }
            if((e_i > e1_out_sx - GateWidth1 / 2 && e_i < e1_out_sx
                && e_j > Elow2 && e_j < Ehigh2)
               || (e_j > e1_out_sx - GateWidth1 / 2 && e_j < e1_out_sx
                   && e_i > Elow2 && e_i < Ehigh2)
               || (e_i > e1_out_dx && e_i < e1_out_dx + GateWidth1 / 2
                   && e_j > Elow2 && e_j < Ehigh2)
               || (e_j > e1_out_dx && e_j < e1_out_dx + GateWidth1 / 2
                   && e_i > Elow2 && e_i < Ehigh2)
               ||

               (e_i > e2_out_sx - GateWidth2 / 2 && e_i < e2_out_sx
                && e_j > Elow1 && e_j < Ehigh1)
               || (e_j > e2_out_sx - GateWidth2 / 2 && e_j < e2_out_sx
                   && e_i > Elow1 && e_i < Ehigh1)
               || (e_i > e2_out_dx && e_i < e2_out_dx + GateWidth2 / 2
                   && e_j > Elow1 && e_j < Ehigh1)
               || (e_j > e2_out_dx && e_j < e2_out_dx + GateWidth2 / 2
                   && e_i > Elow1 && e_i < Ehigh1)) {
                AddHisto.h_gg_Theta_bg->Fill(theta);
            }
        }
    }

    if(verb > 1)
        std::cout << "Coinc addback filled angular distributions" << std::endl;

    // Loop on random coincidences

    for(unsigned int i{0}; i < idx_add_random.size(); ++i) {
        for(unsigned int j{0}; j < i; ++j) {
            double   x_i = agataReader.AddX->At(i);
            double   y_i = agataReader.AddY->At(i);
            double   z_i = agataReader.AddZ->At(i);

            double   x_j = agataReader.AddX->At(j);
            double   y_j = agataReader.AddY->At(j);
            double   z_j = agataReader.AddZ->At(j);

            TVector3 pos_i
                = TVector3(y_i * UNITS::mm, -x_i * UNITS::mm, z_i * UNITS::mm);
            TVector3 pos_j
                = TVector3(y_j * UNITS::mm, -x_j * UNITS::mm, z_j * UNITS::mm);

            double theta = pos_i.Angle(pos_j) * 180 / TMath::Pi();

            auto   e_i   = agataReader.AddE->At(idx_add_random[i]);
            auto   e_j   = agataReader.AddE->At(idx_add_random[j]);

            if((e_i > Elow1 && e_i < Ehigh1 && e_j > Elow2 && e_j < Ehigh2)
               || (e_j > Elow1 && e_j < Ehigh1 && e_i > Elow2
                   && e_i < Ehigh2)) {
                AddHisto.h_gg_Theta_random->Fill(theta);
            }
        }
    }

    if(verb > 1)
        std::cout << "Coinc addback filled random angular distributions"
                  << std::endl;

    return;
};

void UserSelector::UserHistograms::Coincidence::UncorrProcess() {
    // if(Egamma1 < 0 || Egamma2 < 0 ) return;

    int verb = 0;

    if(verb > 1) std::cout << "Start UncorrProcess" << std::endl;

    for(auto p1 : core_vec_pos_1) {
        for(auto p2 : core_vec_pos_2) {
            CoreHisto.h_gg_Theta_uncorrelated->Fill(p1.Angle(p2) * 180
                                                    / TMath::Pi());
        }
        for(auto bg2 : bg_core_vec_pos_2) {
            CoreHisto.h_gg_Theta_bg_uncorrelated->Fill(p1.Angle(bg2) * 180
                                                       / TMath::Pi());
        }
    }
    for(auto bg1 : bg_core_vec_pos_1) {
        for(auto p2 : core_vec_pos_2) {
            CoreHisto.h_gg_Theta_bg_uncorrelated->Fill(bg1.Angle(p2) * 180
                                                       / TMath::Pi());
        }
        for(auto bg2 : bg_core_vec_pos_2) {
            CoreHisto.h_gg_Theta_bg_uncorrelated->Fill(bg1.Angle(bg2) * 180
                                                       / TMath::Pi());
        }
    }

    for(auto rand1 : random_core_vec_pos_1) {
        for(auto rand2 : random_core_vec_pos_2) {
            CoreHisto.h_gg_Theta_bg_uncorrelated->Fill(rand1.Angle(rand2) * 180
                                                       / TMath::Pi());
        }
    }

    if(verb > 1) std::cout << "Start UncorrProcess" << std::endl;

    for(auto p1 : tracked_vec_pos_1) {
        for(auto p2 : tracked_vec_pos_2) {
            TrackHisto.h_gg_Theta_uncorrelated->Fill(p1.Angle(p2) * 180
                                                     / TMath::Pi());
        }
        for(auto bg2 : bg_tracked_vec_pos_2) {
            TrackHisto.h_gg_Theta_bg_uncorrelated->Fill(p1.Angle(bg2) * 180
                                                        / TMath::Pi());
        }
    }
    for(auto bg1 : bg_tracked_vec_pos_1) {
        for(auto p2 : tracked_vec_pos_2) {
            TrackHisto.h_gg_Theta_bg_uncorrelated->Fill(bg1.Angle(p2) * 180
                                                        / TMath::Pi());
        }
        // for(auto bg2:bg_track_vec_pos_2){
        //     TrackHisto.h_gg_Theta_bg_uncorrelated->Fill(bg1.Angle(bg2)*180/TMath::Pi());
        // }
    }
    if(verb > 1) std::cout << "Start UncorrProcess" << std::endl;

    for(auto p1 : addback_vec_pos_1) {
        for(auto p2 : addback_vec_pos_2) {
            AddHisto.h_gg_Theta_uncorrelated->Fill(p1.Angle(p2) * 180
                                                   / TMath::Pi());
        }
        for(auto bg2 : bg_addback_vec_pos_2) {
            AddHisto.h_gg_Theta_bg_uncorrelated->Fill(p1.Angle(bg2) * 180
                                                      / TMath::Pi());
        }
    }
    for(auto bg1 : bg_addback_vec_pos_1) {
        for(auto p2 : addback_vec_pos_2) {
            AddHisto.h_gg_Theta_bg_uncorrelated->Fill(bg1.Angle(p2) * 180
                                                      / TMath::Pi());
        }
        // for(auto bg2:bg_track_vec_pos_2){
        //     TrackHisto.h_gg_Theta_bg_uncorrelated->Fill(bg1.Angle(bg2)*180/TMath::Pi());
        // }
    }

    CoreHisto.h_gg_Theta->Sumw2(kTRUE);
    CoreHisto.h_gg_ang_corr->Sumw2(kTRUE);
    CoreHisto.h_gg_Theta_uncorrelated->Sumw2(kTRUE);

    CoreHisto.h_gg_ang_corr->Add(CoreHisto.h_gg_Theta, CoreHisto.h_gg_Theta_bg,
                                 1.0, -1.0);
    CoreHisto.h_gg_ang_corr->Add(CoreHisto.h_gg_ang_corr,
                                 CoreHisto.h_gg_Theta_random, 1.0, -1.0);

    CoreHisto.h_gg_Theta_uncorrelated->Add(
        CoreHisto.h_gg_Theta_uncorrelated,
        CoreHisto.h_gg_Theta_bg_uncorrelated, 1.0, -1.0);

    CoreHisto.h_gg_ang_corr->Divide(CoreHisto.h_gg_ang_corr,
                                    CoreHisto.h_gg_Theta_uncorrelated);

    // CoreHisto.h_gg_ang_corr->Add(CoreHisto.h_gg_Theta_bg,-1);
    // CoreHisto.h_gg_ang_corr->Add(CoreHisto.h_gg_Theta_random,-1);
    // CoreHisto.h_gg_Theta_uncorrelated->Add(CoreHisto.h_gg_Theta_bg_uncorrelated,-1);
    //
    // CoreHisto.h_gg_ang_corr->Divide(CoreHisto.h_gg_Theta_uncorrelated);

    TrackHisto.h_gg_ang_corr->Sumw2(kTRUE);
    TrackHisto.h_gg_Theta_uncorrelated->Sumw2(kTRUE);

    TrackHisto.h_gg_ang_corr->Add(TrackHisto.h_gg_Theta_bg, -1);
    TrackHisto.h_gg_ang_corr->Add(TrackHisto.h_gg_Theta_random, -1);
    TrackHisto.h_gg_Theta_uncorrelated->Add(
        TrackHisto.h_gg_Theta_bg_uncorrelated, -1);

    TrackHisto.h_gg_ang_corr->Divide(TrackHisto.h_gg_Theta_uncorrelated);

    AddHisto.h_gg_ang_corr->Sumw2(kTRUE);
    AddHisto.h_gg_Theta_uncorrelated->Sumw2(kTRUE);

    AddHisto.h_gg_ang_corr->Add(AddHisto.h_gg_Theta_bg, -1);
    AddHisto.h_gg_ang_corr->Add(AddHisto.h_gg_Theta_random, -1);
    AddHisto.h_gg_Theta_uncorrelated->Add(AddHisto.h_gg_Theta_bg_uncorrelated,
                                          -1);

    AddHisto.h_gg_ang_corr->Divide(AddHisto.h_gg_Theta_uncorrelated);

    // if(CrysSpectra){
    //     dir->mkdir("CrysSpectra")->cd();
    //     //dir->cd("CrysSpectra");
    //     for(int i = 0; i<42; ++i){
    //         if(h_crystals[i]!=nullptr)      h_crystals[i]->Write();
    //     }
    // }
};