#pragma once

#include "Conf.h"
#include "ReactionConf.h"

class TargetConf : public Conf {
  public:
    TargetConf();

    double      thickness{};    //(mg/cm^2)
    double      tilt{};         //(degrees)
    double      rotationX{};    //(degrees)
    double      rotationZ{};    //(degrees)
    double      degThickness{}; //(mg/cm^2)
    double      degDistance{};  //(um)
    bool        degPresent{false};
    int         elossTableType{0};
    std::string material{"none"};
    std::string degMaterial{"none"};
    TVector3    targetNorm{0, 0, 1};
    enum DegPos { AFTER, BEFORE };
    DegPos        degPos{DegPos::AFTER};

    std::ostream& operator<<(std::ostream& os) const override;
    std::istream& operator>>(std::istream& is) override;

  protected:
    std::unordered_map<std::string, Property<DegPos&>> parDEG;

  private:
    static DegPos      ClassifyPos(const std::string& val);
    static std::string PosName(const DegPos& s);
};
