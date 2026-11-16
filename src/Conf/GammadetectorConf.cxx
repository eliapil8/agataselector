#include "GammadetectorConf.h"

GammadetectorConf::GammadetectorConf(const std::string& mark)
    : DetectorConf(mark) {
    windowRight = 10; // Default value for AGATA
    lutFile     = "";

    parI.emplace(
        "BIN_WIDTH",
        Conf::Property<int&>{binWidth, "#", "Bin width in gamma histograms"});
    parD.emplace("GAMMA_E_MAX",
                 Conf::Property<double&>{
                     gammaEmax, "keV", "Maximum gamma energy in histograms"});

    parST.emplace("SPECTRA_TYPE", Conf::Property<SpectraType&>{
                                      spectraType, "#",
                                      "Spectra type: SEGMENT, CORE, CORE1, "
                                      "ADDBACK, TRACKED, CALORIMETER"});

    parGG.emplace(
        "GAMMA_GATE",
        Conf::Property<std::vector<GammaGate>&>{
            gammaGates, "#",
            "EnergyGate(int, int) DC|POL|AB|none ExcludeAngle(int, int)"});

    parB.emplace(
        "EXCLUDE_ANCILLARY",
        Conf::Property<bool&>{
            excludeAncillary, "#",
            "Only analyze event in anticoincidence with ancillaries"});

    parVP.emplace("EXCLUDE_TS",
                  Conf::Property<std::vector<std::pair<double, double>>&>{
                      excludeTimestamp, "ms",
                      "Exclude events in this  timestamp region, "
                      "beginning of run is TS=0"});
}

std::ostream& GammadetectorConf::operator<<(std::ostream& os) const {
    DetectorConf::operator<<(os);
    for(const auto& it : parST) {
        os << it.first << "\t\t\t" << SpectraName(it.second.val)
           << "\t\t\t\t\t" << it.second.comment << std::endl;
    }
    return os;
}

std::istream& GammadetectorConf::operator>>(std::istream& is) {
    DetectorConf::operator>>(is);
    std::string line;

    // Erase previous map
    if(!parGG.empty()) parGG.begin()->second.val.clear();

    findMarker(is);
    while(std::getline(is, line)) {
        line = trim(line);
        if(line[0] == '#') continue; // Comment
        if(line.find('#') != std::string::npos) line.erase(line.find('#'));
        if(std::find(markers.begin(), markers.end(), line) != markers.end())
            break; // Next marker found

        std::istringstream sstr(line);
        std::string        name;
        sstr >> name;
        std::transform(name.begin(), name.end(), name.begin(),
                       (std::function<int(unsigned char)>)::toupper);

        { // Bool type parameters
            auto it = parST.find(name);
            if(it != parST.end()) {
                std::string tmp;
                sstr >> tmp;
                it->second.val = ClassifySpectra(tmp);
                continue;
            }
        }

        { // Gamma Gates
            auto it2 = parGG.find(name);
            if(it2 != parGG.end()) {
                Property<GammaGate> tmp;
                tmp >> sstr;
                it2->second.val.emplace_back(tmp.val);
                continue;
            }
        }
    }
    return is;
}

GammadetectorConf::SpectraType
GammadetectorConf::ClassifySpectra(const std::string& val) {
    if(val == "SEGMENT")
        return SpectraType::SEGMENT;
    else if(val == "CORE")
        return SpectraType::CORE;
    else if(val == "CORE1")
        return SpectraType::CORE1;
    else if(val == "ADDBACK")
        return SpectraType::ADDBACK;
    else if(val == "TRACKED")
        return SpectraType::TRACKED;
    else if(val == "CALORIMETER")
        return SpectraType::CALORIMETER;
    else
        throw std::runtime_error("Unknown spectra type keyword");
}

std::string
GammadetectorConf::SpectraName(const GammadetectorConf::SpectraType& s) {
    switch(s) {
    case SpectraType::SEGMENT: return "SEGMENT";
    case SpectraType::CORE: return "CORE";
    case SpectraType::CORE1: return "CORE1";
    case SpectraType::ADDBACK: return "ADDBACK";
    case SpectraType::TRACKED: return "TRACKED";
    case SpectraType::CALORIMETER: return "CALORIMETER";
    default: throw std::runtime_error("Unknown spectra type ");
    }
}

std::istream& operator>>(std::istream& is, GammadetectorConf::GammaGate& v) {
    is >> v.gate.first >> v.gate.second >> v.option >> v.exclude_angle.first
        >> v.exclude_angle.second;
    return is;
}

std::ostream&
operator<<(std::ostream& os, const GammadetectorConf::GammaGate& v) {
    os << v.gate.first << " " << v.gate.second << " " << v.option << " "
       << v.exclude_angle.first << " " << v.exclude_angle.second;
    return os;
}

GammadetectorConf::GammaGate
operator*=(GammadetectorConf::GammaGate& v, const double& d) {
    v.gate.first *= static_cast<int>(d);
    v.gate.second *= static_cast<int>(d);
    return v;
}
