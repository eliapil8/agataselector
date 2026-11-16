#pragma once

#include "../../Tools/MemCheck.h"
#include "ConfManager.h"
#include "Container.h"
#include "DetectorConf.h"
#include "EnergyLoss.h"
#include "ReactionReconstruction.h"
#include "Settings.h"

#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <TTreeReaderValue.h>

#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

#include <algorithm>
#include <regex>
#include <unordered_map>
#include <vector>

class DetectorAnalysis
{
  public:
    DetectorAnalysis()                         = delete;
    virtual ~DetectorAnalysis()                = default;
    DetectorAnalysis(const DetectorAnalysis &) = delete;
    DetectorAnalysis(DetectorAnalysis &&)      = delete;
#include <string>
#include <unordered_map>
#include <utility> // for std::move
#include <vector>

    struct Inputs
    {
        TTree *const              tree;
        TTree *const              treeToRead;
        TTreeReader              &fReader;
        TFile                    &outFile;
        std::vector<TObject *>   &allHistos;
        std::vector<std::string> &enabledHistos;
        std::vector<std::regex>  &enabledHistoRegexes;
        unsigned long long       &firstTS;
        double                    midTargetBeamEnergy;
        enum Type
        {
            SIGNAL,
            BKG_LEFT,
            BKG_RIGHT
        } type;

        std::unordered_map<ReactionFragment::Ion,
                           ReactionReconstruction2body<long double>,
                           ReactionFragment::Ion::hash_fn> &reactions;

        std::unordered_map<std::vector<ReactionFragment::Ion>,
                           ReactionReconstructionNbody<long double>,
                           ReactionFragment::Ion::vec_hash_fn> &nBodyReactions;

        std::unordered_map<ReactionFragment::Ion, EnergyLoss, ReactionFragment::Ion::hash_fn>
            &targetELoss;

        std::unordered_map<ReactionFragment::Ion, EnergyLoss, ReactionFragment::Ion::hash_fn>
            &degraderELoss;
    };

  protected:
    struct HistogramStructure
    {
        std::vector<TObject *> ptrs;
        TDirectory            *dir{nullptr};

      protected:
        HistogramStructure() = default;
    };
    struct DetectorHistograms : public HistogramStructure
    {
        TH1D *h_CR{nullptr};
        TH1D *h_TdiffLastEvent{nullptr};
    };
    unsigned long long lastTS{0};

    DetectorAnalysis(Inputs &, Container &, DetectorConf &, DetectorHistograms &);
    Inputs input;

  public:
    virtual void Instantiate();
    virtual bool AllocateHistos();
    virtual void Clear();
    virtual void Analyze();
    virtual bool FillHistos();
    virtual void Finalize();

  protected:
    Container          &data;
    DetectorConf       &detectorConf;
    DetectorHistograms &detectorHistograms;
    double              windowLeft{std::numeric_limits<double>::min()};
    double              windowRight{std::numeric_limits<double>::min()};

  protected:
    Settings    &settings;
    ConfManager &conf;

    struct Readers
    {
    };

    bool isEnabled(const std::string &name)
    {
        if (std::find(input.enabledHistos.begin(), input.enabledHistos.end(), name) !=
            input.enabledHistos.end())
        {
            return true;
        }

        return std::any_of(
            input.enabledHistoRegexes.begin(), input.enabledHistoRegexes.end(),
            [&name](const std::regex &expression) { return std::regex_match(name, expression); });
    };

    std::vector<TH1 *> listOfHistos;

    template <class T, class Factory>
    void InitializeLazy(T                                       *&hist,
                        const std::string                        &name,
                        Factory                                 &&factory,
                        std::vector<TObject *>                   &ptr,
                        const std::initializer_list<std::string> &enableAliases = {})
    {
        if (settings.verbosity_level > 0)
            std::cout << "Histo " << name;

        bool enabled = isEnabled(name);
        for (const auto &alias : enableAliases)
        {
            enabled = enabled || isEnabled(alias);
        }

        if (settings.only_enabled_histos && !enabled)
        {
            if (settings.verbosity_level > 0)
                std::cout << ": DISABLED" << std::endl;
            hist = nullptr;
            return;
        }

        if (settings.mem_check && MemCheck::getPercentFreePhysical() < 5)
            throw MemCheck::OutOfMemoryException();

        hist = std::forward<Factory>(factory)();
        input.allHistos.push_back(hist);
        listOfHistos.emplace_back(hist);
        ptr.push_back(hist);

        if (settings.verbosity_level > 0)
            std::cout << ": ENABLED" << std::endl;
    }

    template <class T>
    void Initialize(T *&hist, T *const allocation, std::vector<TObject *> &ptr)
    {
        if (settings.verbosity_level > 0)
            std::cout << "Histo " << allocation->GetName();
        if (settings.mem_check && MemCheck::getPercentFreePhysical() < 5)
            throw MemCheck::OutOfMemoryException();

        if (settings.only_enabled_histos && !isEnabled(allocation->GetName()))
        {
            if (settings.verbosity_level > 0)
                std::cout << ": DISABLED" << std::endl;
            allocation->Delete();
            hist = nullptr;
            return;
        }

        input.allHistos.push_back(allocation);
        if (settings.verbosity_level > 0)
            std::cout << ": ENABLED" << std::endl;
        hist = allocation;
        listOfHistos.emplace_back(allocation);
        // hist->SetBuffer(500);
        ptr.push_back(hist);
    }
    // Double fill
    static inline void Fill(TH1 *hist, const double &x, const double &w = 1)
    {
        if (hist != nullptr)
            hist->Fill(x, w);
    };
    static inline void Fill(TH2 *matr, const double &x, const double &y, const double &w = 1)
    {
        if (matr != nullptr)
            matr->Fill(x, y, w);
    };
    static inline void Fill(
        TH3 *matr, const double &x, const double &y, const double &z, const double &w = 1)
    {
        if (matr != nullptr)
            matr->Fill(x, y, z, w);
    };
    // Char fill
    static inline void Fill(TH1 *hist, const char *x, const double &w = 1)
    {
        if (hist != nullptr)
            hist->Fill(x, w);
    };
    static inline void Fill(TH2 *matr, const char *x, const double &y, const double &w = 1)
    {
        if (matr != nullptr)
            matr->Fill(x, y, w);
    };
    static inline void Fill(TH2 *matr, const double &x, const char *y, const double &w = 1)
    {
        if (matr != nullptr)
            matr->Fill(x, y, w);
    };
    static inline void Fill(TH2 *matr, const char *x, const char *y, const double &w = 1)
    {
        if (matr != nullptr)
            matr->Fill(x, y, w);
    };

    template <class T, class UT>
    T unsignedDiff(const UT &val1, const UT &val2)
    {
        UT res{0};
        if (val1 > val2)
            res = val1 - val2;
        else
            res = val2 - val1;

        if (res <= std::numeric_limits<T>::max())
        {
            if (val1 > val2)
                return static_cast<T>(res);
            else
                return -static_cast<T>(res);
        }

        throw std::runtime_error("Signed to unsigned conversion problem\n");
    }
    template <class T, class UT>
    T signedToUnsigned(const UT &val)
    {
        if (val <= std::numeric_limits<T>::max())
            return static_cast<T>(val);

        throw std::runtime_error("Signed to unsigned conversion problem\n");
    }
    template <class C>
    void SetupLabelsForChannelID(const C &confToLabel, TH1 *h, const bool doYaxis = false)
    {
        if (h != nullptr)
        {
            for (int i = 1; i < h->GetXaxis()->GetNbins(); i++)
            {
                try
                {
                    h->GetXaxis()->SetBinLabel(
                        static_cast<int>(i),
                        confToLabel.getName((i - 1) / Lut::MAXCH, (i - 1) % Lut::MAXCH).c_str());
                }
                catch (std::runtime_error &e)
                {
                    // Ignore missing channels
                    h->GetXaxis()->SetBinLabel(
                        static_cast<int>(i),
                        Form("nu (%d.%d)", (i - 1) / Lut::MAXCH, (i - 1) % Lut::MAXCH));
                }
            }
            if (h->InheritsFrom(TH2::Class()) && doYaxis)
            {
                for (int i = 1; i < h->GetYaxis()->GetNbins(); i++)
                {
                    try
                    {
                        h->GetYaxis()->SetBinLabel(
                            static_cast<int>(i),
                            confToLabel.getName((i - 1) / Lut::MAXCH, (i - 1) % Lut::MAXCH)
                                .c_str());
                    }
                    catch (std::runtime_error &e)
                    {
                        // Ignore missing channels
                        h->GetYaxis()->SetBinLabel(
                            static_cast<int>(i),
                            Form("nu (%d.%d)", (i - 1) / Lut::MAXCH, (i - 1) % Lut::MAXCH));
                    }
                }
            }
        }
    }

    // Used to normalize histograms for background subtraction
    template <class C>
    void NormalizeHistos(const C &c)
    {
        if (input.type != Inputs::Type::SIGNAL &&
            windowLeft != std::numeric_limits<double>::max() &&
            windowRight != std::numeric_limits<double>::max())
        {
            double scale = c.windowRight - c.windowLeft;
            scale /= (c.windowRightLeftBkg - c.windowLeftLeftBkg + c.windowRightRightBkg -
                      c.windowLeftRightBkg);
            for (auto &it : listOfHistos)
            {
                it->Scale(-scale);
            }
        }
    }

    template <class C>
    void SetupTimeWindow(const C &c)
    {
        switch (input.type)
        {
        case Inputs::Type::SIGNAL:
            windowLeft  = c.windowLeft;
            windowRight = c.windowRight;
            break;
        case Inputs::Type::BKG_LEFT:
            windowLeft  = c.windowLeftLeftBkg;
            windowRight = c.windowRightLeftBkg;
            break;
        case Inputs::Type::BKG_RIGHT:
            windowLeft  = c.windowLeftRightBkg;
            windowRight = c.windowRightRightBkg;
            break;
        default:
            throw std::runtime_error("Something wrong with selector job type, not signal or bkg\n");
        }
        if (windowLeft >= windowRight)
            throw std::runtime_error("Left window: " + std::to_string(windowLeft) +
                                     " does not make sense with respect to right window: " +
                                     std::to_string(windowRight) + "\n");
    }

    TLorentzVector SlowInTarget(TLorentzVector                p,
                                const CoincidenceConf::DcPos &pos,
                                const ReactionFragment::Ion  &ion) const;
    void           EvalInTarget(TLorentzVector &p, const ReactionFragment::Ion &ion) const;
    static bool    FileIsPresent(const std::string &);

  public:
    template <class C>
    C const *GetHisto(const std::string &histoName) const
    {
        for (const auto &it : listOfHistos)
        {
            if (histoName == it->GetName())
            {
                auto ptr = dynamic_cast<C *>(it);
                if (ptr == nullptr)
                    throw std::runtime_error("Provided hisrogram " + histoName +
                                             " does not match with provided type\n");
                else
                    return ptr;
            }
        }
        return nullptr;
    }
};
