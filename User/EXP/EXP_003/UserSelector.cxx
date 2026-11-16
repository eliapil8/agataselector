#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    // User begin
    // std::cout << "Example value retrieved from conf file: "<<
    // conf.user.example/UNITS::mm << " mm" << std::endl;
    UserHistograms& h   = userHistograms;
    TDirectory*     dir = outFile.mkdir("User", "User selector");
    dir->cd();
    h.m_spider_vs_agata = new TH2D("m_spider_vs_agata", "m_spider_vs_agata",
                                   2050, -5, 200, 5000, 0, 5000);
    h.m_spider_vs_agata_multip2
        = new TH2D("m_spider_vs_agata_multip2", "m_spider_vs_agata_multip2",
                   2050, -5, 200, 5000, 0, 5000);
}

Bool_t UserSelector::Process(Long64_t entry) {
    AgataSelector::Process(entry);
    UserHistograms&                          h = userHistograms;

    std::vector<std::vector<TLorentzVector>> gammaDC;
    std::vector<std::vector<TLorentzVector>> gammaDCBP;

    for(int s{0}; s < static_cast<int>(
                      spiderAnalysis->spiderReader.SPIDEREnergy->GetSize());
        ++s) {
        gammaDC.emplace_back();
        gammaDCBP.emplace_back();
        unsigned int brd  = spiderAnalysis->spiderReader.SPIDERBoard->At(s);
        unsigned int chan = spiderAnalysis->spiderReader.SPIDERChannel->At(s);
        double       energy{conf.spider.getEnergy(
            spiderAnalysis->spiderReader.SPIDEREnergy->At(s), brd, chan)};

        long long    diff;
        unsigned long long val1 = conf.spider.getTime(
            spiderAnalysis->spiderReader.TSSPIDER_Hit->At(s), brd, chan);
        unsigned long long val2 = agataOutput.TS;
        if(val1 > val2)
            diff = val1 - val2;
        else {
            diff = val2 - val1;
            diff = -diff;
        }
        if(diff > conf.spider.windowLeft && diff < conf.spider.windowRight) {
            for(int a{0}; a < static_cast<int>(agataOutput.E.size()); ++a) {
                // TLorentzVector p3 = SlowInTarget(spiderOutput.p3.at(s),
                // conf.agataSpider.dcPos, conf.spider.ion); TLorentzVector p4
                // = SlowInTarget(spiderOutput.p4.at(s),
                // conf.agataSpider.bpDcPos, conf.spider.ion);

                // gammaDC.back().push_back(agataOutput.P.at(a));
                // gammaDCBP.back().push_back(agataOutput.P.at(a));
                // gammaDC.back().back().Boost(-p3.BoostVector());
                // gammaDCBP.back().back().Boost(-p4.BoostVector());

                h.m_spider_vs_agata->Fill(energy, agataOutput.E.at(a));
                if(agataOutput.E.size() == 2)
                    h.m_spider_vs_agata_multip2->Fill(energy,
                                                      agataOutput.E.at(a));
            }
        }
    }

    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
}
