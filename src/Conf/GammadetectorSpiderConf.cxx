#include "GammadetectorSpiderConf.h"

GammadetectorSpiderConf::GammadetectorSpiderConf(const std::string& mark)
    : CoincidenceConf(mark) {
    parVP.emplace("EX_VALUES",
                  Conf::Property<std::vector<std::pair<double, double>>&>{
                      exValues, "MeV",
                      "Excitation energy gates, first value is the min, "
                      "second value is the max"});

    parVS.emplace("EX_VALUES_FILE",
                  Conf::Property<std::vector<std::string>&>{
                      lutFiles, "#",
                      "File name in the folder LUT containing the channel by "
                      "channel gate in excitation energy"});

    parGG.emplace(
        "GAMMA_GATE",
        Conf::Property<std::vector<GammaGate>&>{
            gammaGates, "#",
            "EnergyGate(int, int) DC|POL|none ExcludeAngle(int, int)"});

    parGG.emplace(
        "EX_GAMMA_GATE",
        Conf::Property<std::vector<GammaGate>&>{
            exgammaGates, "#",
            "EnergyGate(int, int) DC|POL|none ExcludeAngle(int, int)"});

}

std::ostream& GammadetectorSpiderConf::operator<<(std::ostream& os) const {
    CoincidenceConf::operator<<(os);
    return os;
}

std::istream& GammadetectorSpiderConf::operator>>(std::istream& is) {
    CoincidenceConf::operator>>(is);
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

std::istream&
operator>>(std::istream& is, GammadetectorSpiderConf::GammaGate& v) {
    is >> v.gate.first >> v.gate.second >> v.option >> v.exclude_angle.first
        >> v.exclude_angle.second;
    return is;
}

std::ostream&
operator<<(std::ostream& os, const GammadetectorSpiderConf::GammaGate& v) {
    os << v.gate.first << " " << v.gate.second << " " << v.option << " "
       << v.exclude_angle.first << " " << v.exclude_angle.second;
    return os;
}

GammadetectorSpiderConf::GammaGate
operator*=(GammadetectorSpiderConf::GammaGate& v, const double& d) {
    v.gate.first *= static_cast<int>(d);
    v.gate.second *= static_cast<int>(d);
    return v;
}
