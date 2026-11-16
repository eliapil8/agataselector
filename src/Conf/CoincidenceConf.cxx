#include "CoincidenceConf.h"

CoincidenceConf::CoincidenceConf(const std::string& mark) : Conf(mark) {
    parDC.emplace(
        "FRAGMENT_POSITION",
        Conf::Property<DcPos&>{
            dcPos, "#", "Doppler correction position for detected ion"});
    parDC.emplace(
        "BP_FRAGMENT_POSITION",
        Conf::Property<DcPos&>{
            bpDcPos, "#", "Doppler correction position for binary partner"});
    parI.emplace("COINC_W_LEFT",
                 Conf::Property<int&>{windowLeft, "#", "Time window left"});
    parI.emplace("COINC_W_RIGHT",
                 Conf::Property<int&>{windowRight, "#", "Time window right"});
    parI.emplace("COINC_W_LEFT_LEFT_BKG",
                 Conf::Property<int&>{windowLeftLeftBkg, "#",
                                      "Left Background time window on the "
                                      "left side of the coincidence peak"});
    parI.emplace("COINC_W_RIGHT_LEFT_BKG",
                 Conf::Property<int&>{windowRightLeftBkg, "#",
                                      "Right Background time window on the "
                                      "left side of the coincidence peak"});
    parI.emplace("COINC_W_LEFT_RIGHT_BKG",
                 Conf::Property<int&>{windowLeftRightBkg, "#",
                                      "Left Background time window on the "
                                      "right side of the coincidence peak"});
    parI.emplace("COINC_W_RIGHT_RIGHT_BKG",
                 Conf::Property<int&>{windowRightRightBkg, "#",
                                      "Right Background time window on the "
                                      "right side of the coincidence peak"});
    parB.emplace("ENABLE_HISTS",
                 Conf::Property<bool&>{enable_histos, "#",
                                       "Enable histograms of coincidence"});
    parB.emplace("ENABLE_TREE",
                 Conf::Property<bool&>{enable_treeFill, "#",
                                       "Enable or disable coincidence TTree branch to "
                                       "save memory (strongly encouraged)"});
    parB.emplace("ENABLE_REDBRANCH",
                 Conf::Property<bool&>{enable_redBranch, "#",
                                       "Enable saving simple gamma energies instead of TLorentzVectors"});
    parB.emplace("EVENT_BY_EVENT",
                 Conf::Property<bool&>{event_by_event, "#",
                                       "YES: only event by event combinations are validated"
                                       "NO: full built events are in coincidence if any combination of the detectors are in coincidence"});

    parD.emplace("THETA_MIN",
		 Conf::Property<double&>{thetaMin, "deg",
		   "Minimum relative angle for 2D E-theta histograms"});
    parD.emplace("THETA_MAX",
		 Conf::Property<double&>{thetaMin, "deg",
		   "Maximum relative angle for 2D E-theta histograms"});
    parI.emplace("THETA_BINS",
		 Conf::Property<int&>{thetaBins, "#",
		   "Number of bins for theta for 2D E-theta histograms"});

}
std::ostream& CoincidenceConf::operator<<(std::ostream& os) const {
    Conf::operator<<(os);
    for(const auto& it : parDC) {
        os << it.first << "\t\t\t" << PosName(it.second.val) << "\t\t\t\t\t"
           << it.second.comment << std::endl;
    }
    return os;
}

std::istream& CoincidenceConf::operator>>(std::istream& is) {
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

        { // DC type parameters
            auto it = parDC.find(name);
            if(it != parDC.end()) {
                std::string pos;
                sstr >> pos;
                it->second.val = ClassifyPos(pos);
                continue;
            }
        }
    }
    return is;
}
