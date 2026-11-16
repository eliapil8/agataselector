//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Jan 23 11:24:52 2023 by ROOT version 6.26/06
// from TTree TreeMaster/TreeMaster
// found on file: ../Out/Analysis/Tree_0000.root
//////////////////////////////////////////////////////////

#ifndef AgaSelector_h
#define AgaSelector_h

#include <TChain.h>
#include <TF2.h>
#include <TFile.h>
#include <TH2.h>
#include <TROOT.h>
#include <TSelector.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <TTreeReaderValue.h>
#include <map>

#include <TF1.h>
#include <TH1.h>

// Headers needed by this particular selector

class AgaSelector : public TSelector {
  public:
    TTreeReader fReader;    //! the tree reader
    TTree*      fChain = 0; //! pointer to the analyzed TTree or TChain

    // Readers to access the data (delete the ones you do not need).
    TTreeReaderValue<Int_t>     nbHits         = {fReader, "nbHits"};
    TTreeReaderArray<Float_t>   hitT           = {fReader, "hitT"};
    TTreeReaderArray<Float_t>   hitE           = {fReader, "hitE"};
    TTreeReaderArray<Float_t>   hitX           = {fReader, "hitX"};
    TTreeReaderArray<Float_t>   hitY           = {fReader, "hitY"};
    TTreeReaderArray<Float_t>   hitZ           = {fReader, "hitZ"};
    TTreeReaderArray<Float_t>   hitGX          = {fReader, "hitGX"};
    TTreeReaderArray<Float_t>   hitGY          = {fReader, "hitGY"};
    TTreeReaderArray<Float_t>   hitGZ          = {fReader, "hitGZ"};
    TTreeReaderArray<Int_t>     hitId          = {fReader, "hitId"};
    TTreeReaderArray<Int_t>     hitSg          = {fReader, "hitSg"};
    TTreeReaderValue<Int_t>     nbCores        = {fReader, "nbCores"};
    TTreeReaderArray<Int_t>     nbHitsperCry   = {fReader, "nbHitsperCry"};
    TTreeReaderArray<Int_t>     coreId         = {fReader, "coreId"};
    TTreeReaderArray<Float_t>   coreE0         = {fReader, "coreE0"};
    TTreeReaderArray<Float_t>   coreE1         = {fReader, "coreE1"};
    TTreeReaderArray<Float_t>   coreT0         = {fReader, "coreT0"};
    TTreeReaderArray<ULong64_t> coreTS         = {fReader, "coreTS"};
    TTreeReaderValue<Int_t>     nbAdd          = {fReader, "nbAdd"};
    TTreeReaderArray<Int_t>     AddId          = {fReader, "AddId"};
    TTreeReaderArray<Float_t>   AddE           = {fReader, "AddE"};
    TTreeReaderArray<Float_t>   AddX           = {fReader, "AddX"};
    TTreeReaderArray<Float_t>   AddY           = {fReader, "AddY"};
    TTreeReaderArray<Float_t>   AddZ           = {fReader, "AddZ"};
    TTreeReaderArray<ULong64_t> AddTS          = {fReader, "AddTS"};
    TTreeReaderValue<ULong64_t> TSHit          = {fReader, "TSHit"};
    TTreeReaderValue<Int_t>     nbTrack        = {fReader, "nbTrack"};
    TTreeReaderArray<Float_t>   trackE         = {fReader, "trackE"};
    TTreeReaderArray<Float_t>   trackX1        = {fReader, "trackX1"};
    TTreeReaderArray<Float_t>   trackY1        = {fReader, "trackY1"};
    TTreeReaderArray<Float_t>   trackZ1        = {fReader, "trackZ1"};
    TTreeReaderArray<Float_t>   trackhitE1     = {fReader, "trackhitE1"};
    TTreeReaderArray<Float_t>   trackX2        = {fReader, "trackX2"};
    TTreeReaderArray<Float_t>   trackY2        = {fReader, "trackY2"};
    TTreeReaderArray<Float_t>   trackZ2        = {fReader, "trackZ2"};
    TTreeReaderArray<Float_t>   trackhitE2     = {fReader, "trackhitE2"};
    TTreeReaderArray<Int_t>     tracknbHits    = {fReader, "tracknbHits"};
    TTreeReaderArray<Int_t>     trackType      = {fReader, "trackType"};
    TTreeReaderArray<Float_t>   trackFOM       = {fReader, "trackFOM"};
    TTreeReaderArray<Float_t>   trackT         = {fReader, "trackT"};
    TTreeReaderArray<Int_t>     trackCrystalID = {fReader, "trackCrystalID"};
    TTreeReaderValue<ULong64_t> TStrack        = {fReader, "TStrack"};
    // TTreeReaderValue<ULong64_t> TSPrisma = {fReader, "TSPrisma"};
    // TTreeReaderArray<UShort_t> MCP_raw = {fReader, "MCP_raw"};
    // TTreeReaderArray<UShort_t> PPAC_Xleft_raw = {fReader, "PPAC_Xleft_raw"};
    // TTreeReaderArray<UShort_t> PPAC_Xright_raw = {fReader,
    // "PPAC_Xright_raw"}; TTreeReaderArray<UShort_t> PPAC_Cathode_raw =
    // {fReader, "PPAC_Cathode_raw"}; TTreeReaderArray<UShort_t> PPAC_Y_raw =
    // {fReader, "PPAC_Y_raw"}; TTreeReaderArray<UShort_t> TOF_raw = {fReader,
    // "TOF_raw"}; TTreeReaderArray<UShort_t> IC_A_raw = {fReader, "IC_A_raw"};
    // TTreeReaderArray<UShort_t> IC_B_raw = {fReader, "IC_B_raw"};
    // TTreeReaderArray<UShort_t> IC_C_raw = {fReader, "IC_C_raw"};
    // TTreeReaderArray<UShort_t> IC_D_raw = {fReader, "IC_D_raw"};
    // TTreeReaderArray<UShort_t> IC_A_Drift_raw = {fReader, "IC_A_Drift_raw"};
    // TTreeReaderArray<UShort_t> IC_B_Drift_raw = {fReader, "IC_B_Drift_raw"};
    // TTreeReaderArray<UShort_t> IC_C_Drift_raw = {fReader, "IC_C_Drift_raw"};
    // TTreeReaderArray<UShort_t> Side_A_raw = {fReader, "Side_A_raw"};
    // TTreeReaderArray<UShort_t> Side_B_raw = {fReader, "Side_B_raw"};
    // TTreeReaderArray<UShort_t> Side_C_raw = {fReader, "Side_C_raw"};
    // TTreeReaderArray<UShort_t> Side_D_raw = {fReader, "Side_D_raw"};
    // TTreeReaderArray<UShort_t> Monitors_raw = {fReader, "Monitors_raw"};
    // TTreeReaderValue<UShort_t> TAC_LT_VTS = {fReader, "TAC_LT_VTS"};
    // TTreeReaderValue<Float_t> MONITOR_1 = {fReader, "MONITOR_1"};
    // TTreeReaderValue<Float_t> MONITOR_2 = {fReader, "MONITOR_2"};
    // TTreeReaderValue<Float_t> MCP_X = {fReader, "MCP_X"};
    // TTreeReaderValue<Float_t> MCP_Y = {fReader, "MCP_Y"};
    // TTreeReaderValue<Float_t> MCP_Q = {fReader, "MCP_Q"};
    // TTreeReaderValue<Float_t> MCP_Theta = {fReader, "MCP_Theta"};
    // TTreeReaderValue<Float_t> MCP_Phi = {fReader, "MCP_Phi"};
    // TTreeReaderValue<Float_t> X_FP = {fReader, "X_FP"};
    // TTreeReaderValue<Float_t> Y_FP = {fReader, "Y_FP"};
    // TTreeReaderValue<Float_t> TOF = {fReader, "TOF"};
    // TTreeReaderArray<Float_t> IC_Pads = {fReader, "IC_Pads"};
    // TTreeReaderValue<Float_t> IC_E = {fReader, "IC_E"};
    // TTreeReaderValue<Float_t> IC_DE_A = {fReader, "IC_DE_A"};
    // TTreeReaderValue<Float_t> IC_DE_AB = {fReader, "IC_DE_AB"};
    // TTreeReaderValue<Float_t> IC_RANGE = {fReader, "IC_RANGE"};
    // TTreeReaderValue<Float_t> IC_Drift_B = {fReader, "IC_Drift_B"};
    // TTreeReaderValue<UChar_t> IC_a_numpads = {fReader, "IC_a_numpads"};
    // TTreeReaderValue<UChar_t> IC_b_numpads = {fReader, "IC_b_numpads"};
    // TTreeReaderValue<Float_t> Theta = {fReader, "Theta"};
    // TTreeReaderValue<Float_t> Phi = {fReader, "Phi"};
    // TTreeReaderValue<Float_t> Beta = {fReader, "Beta"};
    // TTreeReaderValue<Float_t> Length = {fReader, "Length"};
    // TTreeReaderValue<Float_t> Radius = {fReader, "Radius"};
    // TTreeReaderValue<Float_t> RBeta = {fReader, "RBeta"};
    // TTreeReaderValue<Float_t> A_over_q = {fReader, "A_over_q"};
    // TTreeReaderValue<Float_t> Mass = {fReader, "Mass"};
    // TTreeReaderValue<Float_t> Qvalue = {fReader, "Qvalue"};
    // TTreeReaderValue<Float_t> Theta_BP = {fReader, "Theta_BP"};
    // TTreeReaderValue<Float_t> Phi_BP = {fReader, "Phi_BP"};
    // TTreeReaderValue<Float_t> Beta_BP = {fReader, "Beta_BP"};
    // TTreeReaderValue<Float_t> TAC_LT_LTS = {fReader, "TAC_LT_LTS"};
    // TTreeReaderValue<UChar_t> Z_Nr = {fReader, "Z_Nr"};
    // TTreeReaderValue<UChar_t> Q_Nr = {fReader, "Q_Nr"};
    // TTreeReaderValue<UChar_t> A_Nr = {fReader, "A_Nr"};
    // TTreeReaderValue<UChar_t> mcp_ok = {fReader, "mcp_ok"};
    // TTreeReaderValue<UChar_t> tof_ok = {fReader, "tof_ok"};
    // TTreeReaderValue<UChar_t> traj_ok = {fReader, "traj_ok"};
    // TTreeReaderValue<UChar_t> side_ok = {fReader, "side_ok"};
    // TTreeReaderValue<UChar_t> ic_ok = {fReader, "ic_ok"};
    // TTreeReaderValue<UChar_t> z_ok = {fReader, "z_ok"};
    // TTreeReaderValue<UChar_t> q_ok = {fReader, "q_ok"};
    // TTreeReaderValue<UChar_t> a_ok = {fReader, "a_ok"};
    // TTreeReaderValue<Int_t> nbLABR = {fReader, "nbLABR"};
    // TTreeReaderArray<UShort_t> LABRBoardId = {fReader, "LABRBoard"};
    // TTreeReaderArray<UShort_t> LABRChannelId = {fReader, "LABRChannel"};
    // TTreeReaderArray<Float_t> LABREnergy = {fReader, "LABREnergy"};
    // TTreeReaderArray<Float_t> LABRQshort = {fReader, "LABRQshort"};
    // TTreeReaderArray<Float_t> LABRTime = {fReader, "LABRTime"};
    // TTreeReaderArray<ULong64_t> TSLABR_Hit = {fReader, "TSLABR_Hit"};
    // TTreeReaderValue<ULong64_t> TSLABR = {fReader, "TSLABR"};
    // TTreeReaderValue<Int_t> nbDANTE = {fReader, "nbDANTE"};
    // TTreeReaderArray<UShort_t> DANTEBoardId = {fReader, "DANTEBoard"};
    // TTreeReaderArray<UShort_t> DANTEChannelId = {fReader, "DANTEChannel"};
    // TTreeReaderArray<Float_t> DANTEEnergy = {fReader, "DANTEEnergy"};
    // TTreeReaderArray<Float_t> DANTETime = {fReader, "DANTETime"};
    // TTreeReaderArray<ULong64_t> TSDANTE_Hit = {fReader, "TSDANTE_Hit"};
    // TTreeReaderValue<ULong64_t> TSDANTE = {fReader, "TSDANTE"};

    TH1F*                       GSing;
    TH2F*                       GGMatT0;
    TH2F*                       GGMatTS;
    TH1F*                       T0diff;
    TH1F*                       TSdiff;

    // Julgen Benito
    TH2F*                       hEn_vs_Id;
    TH2F*                       hAgata_hits;
    TH1F*                       hTime_diff_T0;
    TH1F*                       hTime_diff_TS;
    std::map<UInt_t, TH2F*>     hPSA_hits_XY;
    std::map<UInt_t, TH2F*>     hPSA_hits_XZ;
    std::map<UInt_t, TH2F*>     E_vs_Ts;
    std::map<UInt_t, TH2F*>     Esgm_vs_Ts;

    string                      opt;
    Int_t                       cryID;
    Int_t                       tot;
    Int_t                       tree_counter = 0;
    Int_t                       run_nr;

    AgaSelector(TTree* /*tree*/ = 0) {}
    virtual ~AgaSelector() {}
    virtual Int_t  Version() const { return 2; }
    virtual void   Begin(TTree* tree);
    virtual void   SlaveBegin(TTree* tree);
    virtual void   Init(TTree* tree);
    virtual Bool_t Notify();
    virtual Bool_t Process(Long64_t entry);
    virtual Int_t  GetEntry(Long64_t entry, Int_t getall = 0) {
        return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0;
    }
    virtual void   SetOption(const char* option) { fOption = option; }
    virtual void   SetObject(TObject* obj) { fObject = obj; }
    virtual void   SetInputList(TList* input) { fInput = input; }
    virtual TList* GetOutputList() const { return fOutput; }
    virtual void   SlaveTerminate();
    virtual void   Terminate();

    ClassDef(AgaSelector, 0);
};

#endif

#ifdef AgaSelector_cxx
void AgaSelector::Init(TTree* tree) {
    // The Init() function is called when the selector needs to initialize
    // a new tree or chain. Typically here the reader is initialized.
    // It is normally not necessary to make changes to the generated
    // code, but the routine can be extended by the user if needed.
    // Init() will be called many times when running on PROOF
    // (once per file to be processed).

    fReader.SetTree(tree);
}

Bool_t AgaSelector::Notify() {
    // The Notify() function is called when a new file is opened. This
    // can be either for a new TTree in a TChain or when when a new TTree
    // is started when using PROOF. It is normally not necessary to make changes
    // to the generated code, but the routine can be extended by the
    // user if needed. The return value is currently not used.

    return kTRUE;
}

#endif // #ifdef AgaSelector_cxx
