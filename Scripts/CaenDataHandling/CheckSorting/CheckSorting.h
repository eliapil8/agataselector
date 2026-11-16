//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Oct 20 15:05:45 2023 by ROOT version 6.28/04
// from TTree Data_R/Data_R
// found on file: Runs/TimeSorted_aBeam_10MeV_12K_100mbar_0.root
//////////////////////////////////////////////////////////

#ifndef CheckSorting_h
#define CheckSorting_h

#include <TChain.h>
#include <TFile.h>
#include <TROOT.h>
#include <TSelector.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <TTreeReaderValue.h>

// Headers needed by this particular selector

class CheckSorting : public TSelector {
  public:
    TTreeReader fReader;    //! the tree reader
    TTree*      fChain = 0; //! pointer to the analyzed TTree or TChain

    // Readers to access the data (delete the ones you do not need).
    TTreeReaderValue<UShort_t>  Channel   = {fReader, "Channel"};
    TTreeReaderValue<ULong64_t> Timestamp = {fReader, "Timestamp"};
    TTreeReaderValue<UShort_t>  Board     = {fReader, "Board"};
    TTreeReaderValue<UShort_t>  Energy    = {fReader, "Energy"};
    TTreeReaderValue<UInt_t>    Flags     = {fReader, "Flags"};

    CheckSorting(TTree* /*tree*/ = 0) {}
    virtual ~CheckSorting() {}
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

    ClassDef(CheckSorting, 0);

    ULong64_t PreviousTS{0};
};

#endif

#ifdef CheckSorting_cxx
void CheckSorting::Init(TTree* tree) {
    // The Init() function is called when the selector needs to initialize
    // a new tree or chain. Typically here the reader is initialized.
    // It is normally not necessary to make changes to the generated
    // code, but the routine can be extended by the user if needed.
    // Init() will be called many times when running on PROOF
    // (once per file to be processed).

    fReader.SetTree(tree);
}

Bool_t CheckSorting::Notify() {
    // The Notify() function is called when a new file is opened. This
    // can be either for a new TTree in a TChain or when when a new TTree
    // is started when using PROOF. It is normally not necessary to make changes
    // to the generated code, but the routine can be extended by the
    // user if needed. The return value is currently not used.

    return kTRUE;
}

#endif // #ifdef CheckSorting_cxx
