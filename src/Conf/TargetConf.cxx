#include "TargetConf.h"

TargetConf::TargetConf() : Conf("TARGET_CONF") {
    parD.emplace("THICKNESS",
                 Conf::Property<double&>{
                     thickness, "mg/cm2",
                     "Target thickness-density in mg/cm2 or units alike"});
    parD.emplace("TILT",
                 Conf::Property<double&>{tilt, "deg",
                                         "Target tilt in degrees; Negative "
                                         "values for clockwise rotations"});
    parD.emplace("ROTATIONX", Conf::Property<double&>{
                                  rotationX, "deg",
                                  "Target rotation on the X axis in degrees"});
    parD.emplace("ROTATIONZ", Conf::Property<double&>{
                                  rotationZ, "deg",
                                  "Target rotation on the Z axis in degrees"});
    parD.emplace("DEG_THICKNESS",
                 Conf::Property<double&>{degThickness, "mg/cm2",
                                         "Degrader thickness in mg"});
    parD.emplace(
        "DEG_DISTANCE",
        Conf::Property<double&>{degDistance, "um", "Degrader distance in um"});

    parB.emplace(
        "DEG_PRESENT",
        Conf::Property<bool&>{degPresent, "#", "Degrader present YES/NO"});

    parI.emplace("ELOSS_TABLE_TYPE",
                 Conf::Property<int&>{
                     elossTableType, "#",
                     "Type of eloss table 0 -> SRIM, 1-> Geant4, 2 ->Lise"});

    parS.emplace("MATERIAL", Conf::Property<std::string&>{material, "#",
                                                          "Target material"});
    parS.emplace("DEG_MATERIAL", Conf::Property<std::string&>{
                                     degMaterial, "#", "Degrader material"});

    parDEG.emplace("DEG_POS",
                   Conf::Property<DegPos&>{degPos, "#",
                                           "Degrader position BEFORE/AFTER"});
}
std::ostream& TargetConf::operator<<(std::ostream& os) const {
    Conf::operator<<(os);
    for(const auto& it : parDEG) {
        os << it.first << "\t\t\t" << PosName(it.second.val) << "\t\t\t\t\t"
           << it.second.comment << std::endl;
    }
    return os;
}
std::istream& TargetConf::operator>>(std::istream& is) {
    Conf::operator>>(is);
    std::string line;
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
            auto it = parDEG.find(name);
            if(it != parDEG.end()) {
                std::string tmp;
                sstr >> tmp;
                it->second.val = ClassifyPos(tmp);
                continue;
            }
        }
    }
    return is;
}

TargetConf::DegPos TargetConf::ClassifyPos(const std::string& val) {
    if(val == "BEFORE")
        return DegPos::BEFORE;
    else if(val == "AFTER")
        return DegPos::AFTER;
    else
        throw std::runtime_error("Unknown deg pos type keyword");
}

std::string TargetConf::PosName(const TargetConf::DegPos& s) {
    switch(s) {
    case DegPos::BEFORE: return "BEFORE";
    case DegPos::AFTER: return "AFTER";
    default: throw std::runtime_error("Unknown Deg pos type ");
    }
}
