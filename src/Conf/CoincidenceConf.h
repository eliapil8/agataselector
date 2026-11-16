#pragma once

#include "Conf.h"

class CoincidenceConf : public Conf {
  public:
    CoincidenceConf() = delete;

  protected:
    explicit CoincidenceConf(const std::string& mark);

  public:
    enum DcPos {
        BEGIN_TARGET,
        MID_TARGET,
        END_TARGET,
        MID_DEGRADER,
        END_DEGRADER
    };

    std::ostream& operator<<(std::ostream& os) const override;
    std::istream& operator>>(std::istream& is) override;

    DcPos         dcPos{DcPos::MID_TARGET};
    DcPos         bpDcPos{DcPos::MID_TARGET};

    int           windowLeft{};
    int           windowRight{};
    int           windowLeftLeftBkg{};
    int           windowRightLeftBkg{};
    int           windowLeftRightBkg{};
    int           windowRightRightBkg{};
    bool          enable_histos{true};
    bool          enable_treeFill{false};
    bool          enable_redBranch{false};
    bool          event_by_event{true};
    double        thetaMin{0.};
    double        thetaMax{180.};
    int           thetaBins{1800};

  
  protected:
    std::unordered_map<std::string, Property<DcPos&>> parDC;

  private:
    static DcPos ClassifyPos(const std::string& val) {
        if(val == "BEGIN_TARGET") return DcPos::BEGIN_TARGET;
        if(val == "MID_TARGET") return DcPos::MID_TARGET;
        if(val == "END_TARGET") return DcPos::END_TARGET;
        if(val == "MID_DEGRADER") return DcPos::MID_DEGRADER;
        if(val == "END_DEGRADER")
            return DcPos::END_DEGRADER;
        else
            throw std::runtime_error("Unknown position type");
    }
    static std::string PosName(const DcPos& s) {
        switch(s) {
        case DcPos::BEGIN_TARGET: return "BEGIN_TARGET";
        case DcPos::MID_TARGET: return "MID_TARGET";
        case DcPos::END_TARGET: return "END_TARGET";
        case DcPos::MID_DEGRADER: return "MID_DEGRADER";
        case DcPos::END_DEGRADER: return "END_DEGRADER";
        default: throw std::runtime_error("Unknown DC position type \n");
        }
    }
};
