#include "UserSelector.h"

#include "CoincidenceAnalysis/CoincidenceAnalysis.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User selector intitialisation - done just once per file

    // std::cout << "Example value retrieved from conf file: "<<
    // conf.user.example/UNITS::mm << " mm" << std::endl;

    // outFile.mkdir("User","User's custom analysis");
    // outFile.cd("User");

    // UserHistograms& hu = userHistograms;
    // hu.h_agataCR = new TH1D("h_agataCR","Count Rate per minute of
    // AGATA;Time[min];Counts per min",10080,0,10080);
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);

    // User process - done for each entry

    // AgataHistograms& ha = agataHistograms;
    // AgataPrismaHistograms& hap = agataPrismaHistograms;
    // UserHistograms& hu = userHistograms;

    // double timeDiff = 0;
    // if(agataOutput.TS >= timestampRangeAgata.first){
    //     timeDiff = (double)(agataOutput.TS - timestampRangeAgata.first)
    //     /1.e8/60.; for(unsigned int i{0}; i<agataOutput.E.size(); i++){
    //        Fill(hu.h_agataCR, timeDiff);
    //     }
    // }
    // else {
    //     std::cout << "Something wrong with time stamps; lower time than
    //     reference found " << std::endl;
    // }

    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
    CoincidenceAnalysis* coincAnalysis{nullptr};
    for (const auto& it: detectorAnaList){
        if(conf.user.detector == "SPIDER"){
            coincAnalysis = dynamic_cast<AgataSpiderAnalysis*>(it);
        } else if (conf.user.detector == "EUCLIDES") {
            coincAnalysis = dynamic_cast<AgataEuclidesAnalysis*>(it);
        }
        if(coincAnalysis != nullptr) break;
    }

    std::vector<TH2D const*> histos;
    for(const auto& it: conf.user.histoList){
        auto h = coincAnalysis->GetHisto<TH2D>(it);
        if (h == nullptr)
            throw std::runtime_error("The provided histogram "+it+" does not exist or is not a TH2D\n");
        histos.push_back(h);
    }

    outFile.mkdir("User","User's custom analysis");
    outFile.cd("User");
    UserHistograms& hu = userHistograms;

    auto convertTH2DtoTH1Ds = [](TH2D const* in, std::vector<TH1D*>& out){
        for(int x{0}; x<in->GetNbinsX(); ++x){
            out.emplace_back(new TH1D{ Form("h_EDC_%i", x),
                                            Form("Energy doppler corrected of Spider with ID %i", x),
                                            in->GetNbinsY(),
                                            in->GetYaxis()->GetXmin(),
                                            in->GetYaxis()->GetXmax()});

            for(int y{0}; y<in->GetNbinsY(); ++y){
                out.back()->SetBinContent(y, in->GetBinContent(x,y));
            }
        }
    };

    for(const auto& it: histos){
        convertTH2DtoTH1Ds(it, hu.histos[it->GetName()]);
    }


    //Write histograms
    for(const auto& it: hu.histos){
        for(const auto& it2: it.second){
            it2->Write();
        }
    }
}
