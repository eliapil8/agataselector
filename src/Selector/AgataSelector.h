#pragma once

// ROOT includes
#include <TChain.h>
#include <TCutG.h>
#include <TFile.h>
#include <TKey.h>
#include <TROOT.h>
#include <TSelector.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <TTreeReaderValue.h>
#include <TVector3.h>

#include <TGraph.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TObject.h>
#include <TRandom3.h>

// STL includes
#include <array>
#include <fstream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

// This project includes
#include "Agata.h"
#include "CoincidenceAnalysis/CoincidenceAnalysis.h"
#include "ConfManager.h"
#include "Dante.h"
#include "DetectorAnalysis/DetectorAnalysis.h"
#include "EnergyLoss.h"
#include "Euclides.h"
#include "GammaDC.h"
#include "Labr.h"
#include "Mcp.h"
#include "Oscar.h"
#include "Prisma.h"
#include "PrismaInput.h"
#include "ReactionFragment.h"
#include "ReactionReconstruction.h"
#include "Sauron.h"
#include "EXSSiDe.h"
#include "Settings.h"
#include "Spider.h"
#include "Monitor.h"
#include "SauronMcp.h"

#include "DetectorAnalysis/AgataAnalysis.h"
#include "DetectorAnalysis/DanteAnalysis.h"
#include "DetectorAnalysis/McpAnalysis.h"
#include "DetectorAnalysis/EuclidesAnalysis.h"
#include "DetectorAnalysis/LabrAnalysis.h"
#include "DetectorAnalysis/OscarAnalysis.h"
#include "DetectorAnalysis/PrismaAnalysis.h"
#include "DetectorAnalysis/PrismaInputAnalysis.h"
#include "DetectorAnalysis/SauronAnalysis.h"
#include "DetectorAnalysis/EXSSiDeAnalysis.h"
#include "DetectorAnalysis/SpiderAnalysis.h"
#include "DetectorAnalysis/MonitorAnalysis.h"

#include "CoincidenceAnalysis/AgataDanteAnalysis.h"
#include "CoincidenceAnalysis/AgataEuclidesAnalysis.h"
#include "CoincidenceAnalysis/AgataLabrAnalysis.h"
#include "CoincidenceAnalysis/AgataOscarAnalysis.h"
#include "CoincidenceAnalysis/AgataPrismaAnalysis.h"
#include "CoincidenceAnalysis/AgataPrismaDanteAnalysis.h"
#include "CoincidenceAnalysis/AgataSauronAnalysis.h"
#include "CoincidenceAnalysis/AgataMcpSauronAnalysis.h"
#include "CoincidenceAnalysis/AgataEXSSiDeAnalysis.h"
#include "CoincidenceAnalysis/AgataSpiderAnalysis.h"
#include "CoincidenceAnalysis/LabrDanteAnalysis.h"
#include "CoincidenceAnalysis/LabrEuclidesAnalysis.h"
#include "CoincidenceAnalysis/LabrOscarAnalysis.h"
#include "CoincidenceAnalysis/LabrPrismaAnalysis.h"
#include "CoincidenceAnalysis/LabrPrismaDanteAnalysis.h"
#include "CoincidenceAnalysis/LabrSauronAnalysis.h"
#include "CoincidenceAnalysis/LabrEXSSiDeAnalysis.h"
#include "CoincidenceAnalysis/LabrSpiderAnalysis.h"
#include "CoincidenceAnalysis/PrismaDanteAnalysis.h"
#include "CoincidenceAnalysis/McpEXSSiDeAnalysis.h"
#include "CoincidenceAnalysis/McpMonitorAnalysis.h"
#include "CoincidenceAnalysis/McpDanteAnalysis.h"
#include "CoincidenceAnalysis/McpSauronAnalysis.h"

class AgataSelector : public TSelector {
  public:
    TTreeReader fReader;         //! the tree reader
    TTree*      fChain{nullptr}; //! pointer to the analyzed TTree or TChain

    // Overridden functions
    AgataSelector(TTree* /*tree*/ = nullptr) = delete;
    explicit AgataSelector(TFile& oFile)
        : outFile(oFile),
          settings(Settings::getInstance()),
          conf(ConfManager::getInstance()){};
    ~AgataSelector() override {
        // delete outFile;
        // delete tree;
        for(auto& it : detectorAnaList) { delete it; }
        progress->store(201.); // Means the selector is destroyed
    }
    Int_t  Version() const override { return 2; }
    void   Begin(TTree* tree) override;
    void   SlaveBegin(TTree* tree) override;
    void   Init(TTree* tree) override;
    Bool_t Notify() override;
    Bool_t Process(Long64_t entry) override;
    Int_t  GetEntry(Long64_t entry, Int_t getall) override {
        return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0;
    }
    void   SetOption(const char* option) override { fOption = option; }
    void   SetObject(TObject* obj) override { fObject = obj; }
    void   SetInputList(TList* input) override { fInput = input; }
    TList* GetOutputList() const override { return fOutput; }
    void   SlaveTerminate() override;
    void   Terminate() override;

  protected:
    static bool FileIsPresent(const std::string&);

  protected:
    // Inputs needed for analysis classes
    //  Output tree
    TTree*                         tree{nullptr};
    // TTreeToRead is fChain
    // TTree* fChain
    // TTreeReader is fReader
    // TTreeReader fReader
    //  Output file
    TFile&                         outFile;
    // All histos handled by selector
    std::vector<TObject*>          allHistos;
    // All enabled histos
    std::vector<std::string>       enabledHistos;
    // Regular expressions used to enable histos
    std::vector<std::regex>        enabledHistoRegexes;
    // First TS of run
    unsigned long long             firstTS{0};
    // Mid target beam energy
    double                         midTargetBeamEnergy{0};
    DetectorAnalysis::Inputs::Type type{
        DetectorAnalysis::Inputs::Type::SIGNAL};
    // Reactions handled by selector
    std::unordered_map<ReactionFragment::Ion,
                       ReactionReconstruction2body<long double>,
                       ReactionFragment::Ion::hash_fn>
        reactions;
    // N-body reactions handled by selector
    std::unordered_map<std::vector<ReactionFragment::Ion>,
                       ReactionReconstructionNbody<long double>,
                       ReactionFragment::Ion::vec_hash_fn>
        nBodyReactions;
    // Energy loss of target
    std::unordered_map<ReactionFragment::Ion, EnergyLoss,
                       ReactionFragment::Ion::hash_fn>
        targetELoss;
    // Energy loss of degrader
    std::unordered_map<ReactionFragment::Ion, EnergyLoss,
                       ReactionFragment::Ion::hash_fn>
        degraderELoss;

  public:
    virtual void ConfProgress(std::atomic<double>* progr) {
        progress = progr;
    };
    void SetFirstTS(const unsigned long long& TS) { firstTS = TS; }
    void SetJobType(const DetectorAnalysis::Inputs::Type& t) { type = t; }
    unsigned long long GetFirstTS() const { return firstTS; }
    double GetMidTargetBeamEnergy() const { return midTargetBeamEnergy; }

  protected:
    std::atomic<double>* progress{nullptr};

    // Container classes
    Agata                agataOutput;
    Labr                 labrOutput;
    Prisma               prismaOutput;
    PrismaInput          prismaInputOutput;
    Spider               spiderOutput;
    Monitor              monitorOutput;
    Sauron               sauronFwOutput;
    Sauron               sauronBwOutput;
    EXSSiDe              exssideOutput;
    Oscar                oscarUpOutput;
    Oscar                oscarDownOutput;
    Dante                danteOutput;
    Mcp                  mcpOutput;
    Euclides             euclidesOutput;
    SauronMcp            sauronFwMcpOutput;
    SauronMcp            sauronBwMcpOutput;

    GammaDC              gammaAgataPrisma;
    GammaDC              gammaAgataPrismaDante;
    std::vector<GammaDC> gammaAgataSpider;
    std::vector<GammaDC> gammaAgataSauronFw;
    std::vector<GammaDC> gammaAgataSauronBw;
    std::vector<GammaDC> gammaAgataMcpSauronFw;
    std::vector<GammaDC> gammaAgataMcpSauronBw;
    std::vector<GammaDC> gammaAgataEXSSiDe;
    std::vector<GammaDC> gammaAgataOscarUp;
    std::vector<GammaDC> gammaAgataOscarDown;
    std::vector<GammaDC> gammaAgataDante;
    GammaDC              gammaAgataEuclides;
    GammaDC              gammaLabrPrisma;
    GammaDC              gammaLabrPrismaDante;
    std::vector<GammaDC> gammaLabrSpider;
    std::vector<GammaDC> gammaLabrSauronFw;
    std::vector<GammaDC> gammaLabrSauronBw;
    std::vector<GammaDC> gammaLabrEXSSiDe;
    std::vector<GammaDC> gammaLabrOscarUp;
    std::vector<GammaDC> gammaLabrOscarDown;
    std::vector<GammaDC> gammaLabrDante;
    GammaDC              gammaLabrEuclides;

  protected:
    const Settings& settings;
    ConfManager&    conf;

  protected:
    std::vector<DetectorAnalysis*> detectorAnaList;

  public:
    AgataAnalysis*     agataAnalysis{nullptr};
    LabrAnalysis*      labrAnalysis{nullptr};
    PrismaAnalysis*    prismaAnalysis{nullptr};
    SpiderAnalysis*    spiderAnalysis{nullptr};
    MonitorAnalysis*   monitorAnalysis{nullptr};
    SauronFwAnalysis*  sauronFwAnalysis{nullptr};
    SauronBwAnalysis*  sauronBwAnalysis{nullptr};
    EXSSiDeAnalysis*   exssideAnalysis{nullptr};
    OscarUpAnalysis*   oscarUpAnalysis{nullptr};
    OscarDownAnalysis* oscarDownAnalysis{nullptr};
    DanteAnalysis*     danteAnalysis{nullptr};
    McpAnalysis*       mcpAnalysis{nullptr};
    EuclidesAnalysis*  euclidesAnalysis{nullptr};
};

#ifdef AgataSelector_cxx
void   AgataSelector::Init(TTree* setTree) { fReader.SetTree(setTree); }

Bool_t AgataSelector::Notify() { return kTRUE; }
#endif // #ifdef AgataSelector_cxx
