#pragma once

#include "DetectorAnalysis.h"
#include "Monitor.h"

class MonitorAnalysis : public DetectorAnalysis
{
  public:
    explicit MonitorAnalysis(Inputs, Monitor &);
    void Instantiate() override;
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  protected:
    // Container data
    Monitor &monitor;

    // Histograms
    struct MonitorHistograms : public DetectorHistograms
    {
        TH2D *m_Energy_TS{nullptr};
        TH2D *m_Energy_monitorID{nullptr};
        TH2D *m_Energy_monitorID_uncal{nullptr};
        TH1D *h_E{nullptr};
    } monitorHistograms;

    // Readers
  public:
    struct MonitorReader : public Readers
    {
        TTreeReaderValue<Int_t>     *nbMONITOR{nullptr};
        TTreeReaderValue<ULong64_t> *TSMONITOR{nullptr};
        TTreeReaderArray<ULong64_t> *TSMONITOR_Hit{nullptr};
        TTreeReaderArray<UShort_t>  *MONITORChannel{nullptr};
        TTreeReaderArray<UShort_t>  *MONITORBoard{nullptr};
        TTreeReaderArray<Float_t>   *MONITOREnergy{nullptr};
        TTreeReaderArray<Float_t>   *MONITORTime{nullptr};
    } monitorReader;

  private:
    std::pair<bool, EnergyLoss> monitorDeadlayerELoss{false, {}};
};
