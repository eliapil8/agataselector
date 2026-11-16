#pragma once

#include "Dante.h"
#include "DetectorAnalysis.h"
#include "GateSelector.h"

class DanteAnalysis : public DetectorAnalysis
{
  public:
    explicit DanteAnalysis(Inputs, Dante &);
    void Instantiate() override;
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

  protected:
    // Container data
    Dante &dante;

    // Histograms
    struct DanteHistograms : public DetectorHistograms
    {
        TH3D                                    *m_layout_XYZ{nullptr};
        TH2D                                    *m_layout_front{nullptr};
        TH2D                                    *m_layout_top{nullptr};
        TH2D                                    *m_layout_side{nullptr};
        TH1D                                    *h_danteID{nullptr};
        TH2D                                    *m_goodEvtsStat_danteID{nullptr};
        std::unordered_map<unsigned int, TH2D *> m_dante;
        std::unordered_map<unsigned int, TH1D *> h_TOF;
        std::unordered_map<unsigned int, TH2D *> h_TOF_Theta;
        TH2D                                    *m_dante_dante_theta{nullptr};
        TH2D                                    *m_dante_dante_theta_tof{nullptr};
        TH2D                                    *m_dante_dante_phi{nullptr};
    } danteHistograms;

    // Readers
  public:
    struct DanteReader : public Readers
    {
        TTreeReaderValue<Int_t>     *nbDANTE{nullptr};
        TTreeReaderValue<ULong64_t> *TSDANTE{nullptr};
        TTreeReaderArray<ULong64_t> *TSDANTE_Hit{nullptr};
        TTreeReaderArray<UShort_t>  *DANTEChannel{nullptr};
        TTreeReaderArray<UShort_t>  *DANTEBoard{nullptr};
        TTreeReaderArray<Float_t>   *DANTEEnergy{nullptr};
        TTreeReaderArray<Float_t>   *DANTETime{nullptr};

        // cms edit 2024: Function to debug hanging executable while processing Dante analyze()
        inline void printDanteReader() const
        {
            // Use ->Get() to access TTreeReaderValue pointers
            if (nbDANTE && nbDANTE->IsValid())
            {
                std::cout << "nbDANTE: " << nbDANTE->Get() << std::endl;
            }
            else
            {
                std::cout << "nbDANTE is nullptr or invalid" << std::endl;
            }

            if (TSDANTE && TSDANTE->IsValid())
            {
                std::cout << "TSDANTE: " << TSDANTE->Get() << std::endl;
            }
            else
            {
                std::cout << "TSDANTE is nullptr or invalid" << std::endl;
            }

            // For TTreeReaderArray, loop through each element
            if (TSDANTE_Hit)
            {
                auto size = TSDANTE_Hit->GetSize();
                std::cout << "TSDANTE_Hit: " << size << " counts, ";
                if (size > 0)
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        std::cout << (*TSDANTE_Hit)[i] << " ";
                    }
                }
                else
                {
                    std::cout << "No hits available";
                }
                std::cout << std::endl;
            }
            else
            {
                std::cout << "TSDANTE_Hit is nullptr" << std::endl;
            }

            if (DANTEChannel)
            {
                auto size = DANTEChannel->GetSize();
                std::cout << "DANTEChannel: " << size << " counts, ";
                if (size > 0)
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        std::cout << (*DANTEChannel)[i] << " ";
                    }
                }
                else
                {
                    std::cout << "No channels available";
                }
                std::cout << std::endl;
            }
            else
            {
                std::cout << "DANTEChannel is nullptr" << std::endl;
            }

            if (DANTEBoard)
            {
                auto size = DANTEBoard->GetSize();
                std::cout << "DANTEBoard: " << size << " counts, ";
                if (size > 0)
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        std::cout << (*DANTEBoard)[i] << " ";
                    }
                }
                else
                {
                    std::cout << "No boards available";
                }
                std::cout << std::endl;
            }
            else
            {
                std::cout << "DANTEBoard is nullptr" << std::endl;
            }

            if (DANTEEnergy)
            {
                auto size = DANTEEnergy->GetSize();
                std::cout << "DANTEEnergy: " << size << " counts, ";
                if (size > 0)
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        std::cout << (*DANTEEnergy)[i] << " ";
                    }
                }
                else
                {
                    std::cout << "No energy readings available";
                }
                std::cout << std::endl;
            }
            else
            {
                std::cout << "DANTEEnergy is nullptr" << std::endl;
            }

            if (DANTETime)
            {
                auto size = DANTETime->GetSize();
                std::cout << "DANTETime: " << size << " counts, ";
                if (size > 0)
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        std::cout << (*DANTETime)[i] << " ";
                    }
                }
                else
                {
                    std::cout << "No time readings available";
                }
                std::cout << std::endl;
            }
            else
            {
                std::cout << "DANTETime is nullptr" << std::endl;
            }
        }
    } danteReader;

  private:
    std::unordered_map<unsigned int, std::array<int, 4>> dantePresentMask;
    GateSelector<1, 1>                                   TOFgate;
};
