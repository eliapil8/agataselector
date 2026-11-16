#include "OptimizerConf.h"

OptimizerConf::OptimizerConf() : Conf("OPTIMIZER_CONF") {
    parVPARS.emplace(
        "PARAMETER",
        Conf::Property<std::vector<OptimizerConf::Parameter>&>{
            parameters, "mm",
            "Parameter: |detector|par_name|initial_value|min|max|step|"});

    parVPARS.emplace("SCAN",
                     Conf::Property<std::vector<OptimizerConf::Parameter>&>{
                         parameterScan, "#",
                         "Scan a parameter: "
                         "|detector|par_name|initial_value|min|max|step|"});

    parVPARS.emplace("TRANSITION",
                     Conf::Property<std::vector<OptimizerConf::Parameter>&>{
                         transition, "#",
                         "Transition to optimize: "
                         "|folder|spec_name|centroid|sigma|tail||bias"});

    parVPARSC.emplace(
        "CONTOUR",
        Conf::Property<std::vector<OptimizerConf::ContourParameter>&>{
            parameterContour, "#",
            "Saves a countour of par1 vs par2: "
            "|detector1|par_name1|detector2|par_name2|nrpts|"});

    parS.emplace("FIT_PAR_FILE",
                 Conf::Property<std::string&>{
                     fitParFile, "#", "Name of parameter file for fitter"});

    parS.emplace("LOG_FILE",
                 Conf::Property<std::string&>{
                     logFile, "#", "Name of log file for minimizer"});
    parS.emplace("ROOT_FILE",
                 Conf::Property<std::string&>{optimizerOutputFile, "#",
                                              "Name optimizer output file"});

    parS.emplace("MINIMIZER", Conf::Property<std::string&>{
                                  minimizerName, "#",
                                  "Name of minimizer (Minuit/Minuit2, Fumili, "
                                  "GLSMultiMin, Genetic)"});

    parS.emplace("ALGORITHM",
                 Conf::Property<std::string&>{
                     minimizerAlgorithm, "#",
                     "Name of algorithm (Migrad, Simplex,...) see: "
                     "tutorials/fit/NumericalMinimization.C.html"});

    parI.emplace("MAX_CALLS",
                 Conf::Property<int&>{maxCalls, "#",
                                      "Maximum number of minimizer calls"});

    parI.emplace("PRINT_LEVEL",
                 Conf::Property<int&>{printLevel, "#", "Minimizer verbosity"});

    parI.emplace("BKG_POL_ORDER",
                 Conf::Property<int&>{bkgPolOrder, "#",
                                      "Polynomial order for background"});

    parD.emplace("TOLERANCE", Conf::Property<double&>{tolerance, "#",
                                                      "Minimizer tolerance"});

    parD.emplace("PRECISION",
                 Conf::Property<double&>{precision, "#",
                                         "Minimizer precision (likely leave 0 "
                                         "for optimally calculated value)"});

    parD.emplace("SIGMA_WEIGHT",
                 Conf::Property<double&>{
                     sigmaWeight, "#",
                     "Figure of merit weight on sigma. 0 means only the "
                     "centroid is optimized, 1 means only the sigma, values "
                     "in between are a combination"});

    parD.emplace("NON_CONVERGENCE_COST",
                 Conf::Property<double&>{
                     nonConvergenceCost, "#",
                     "Multiplier cost for fits that did not converge"});

    parI.emplace(
        "TAIL",
        Conf::Property<int&>{
            tail, "#",
            "0: notail, 1: right, 2: left, 3: left+right, 4: symmetric"});

    parB.emplace("USE_INTERVALS",
                 Conf::Property<bool&>{
                     useIntervals, "#",
                     "Use intervals in minimization (try what is best)"});

    parB.emplace("VALID_ERRORS",
                 Conf::Property<bool&>{
                     validErrors, "#",
                     "Performs error analysis (e.g. run Hesse for Minuit)"});

    parB.emplace(
        "SAVE_SCAN_FILES",
        Conf::Property<bool&>{
            saveScanFiles, "#",
            "Saves the output of all scan points for each parameter"});

    parB.emplace(
        "ONLY_SCAN",
        Conf::Property<bool&>{
            onlyScan, "#",
            "Skips the minimization and only performs the scan"});
}

std::ostream& OptimizerConf::operator<<(std::ostream& os) const {
    Conf::operator<<(os);
    for(const auto& it : parVPARS) {
        // if(it.second.val.empty())
        //	throw std::runtime_error("Should not be initialized empty\n");
        for(const auto& it2 : it.second.val) {
            // Property<decltype(it.second.val)> tmp{it2, it.second.unit,
            // it.second.comment};
            Property<Parameter> tmp{it2, it.second.unit, it.second.comment};
            os << it.first << "\t\t\t";
            tmp << os;
        }
    }
    for(const auto& it : parVPARSC) {
        // if(it.second.val.empty())
        //     throw std::runtime_error("Should not be initialized empty\n");
        for(const auto& it2 : it.second.val) {
            // Property<decltype(it.second.val)> tmp{it2, it.second.unit,
            // it.second.comment};
            Property<ContourParameter> tmp{it2, it.second.unit,
                                           it.second.comment};
            os << it.first << "\t\t\t";
            tmp << os;
        }
    }
    return os;
}

std::istream& OptimizerConf::operator>>(std::istream& is) {
    Conf::operator>>(is);
    std::string line;

    // Erase previous map
    if(!parVPARS.empty()) parVPARS.begin()->second.val.clear();

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

        { // VPARS type parameters
            auto it = parVPARS.find(name);
            if(it != parVPARS.end()) {
                Property<Parameter> tmp;
                tmp >> sstr;
                it->second.val.emplace_back(tmp.val);
                continue;
            }
        }
        { // VPARSC type parameters
            auto it = parVPARSC.find(name);
            if(it != parVPARSC.end()) {
                Property<ContourParameter> tmp;
                tmp >> sstr;
                it->second.val.emplace_back(tmp.val);
                continue;
            }
        }
    }
    return is;
}

void OptimizerConf::setOptimizablePtr(const std::string& det,
                                      const std::string& par,
                                      double* const&     ptr) {
    nameToPtr[det].emplace(par, ptr);
}

void OptimizerConf::RetrieveParameters() {
    for(auto& it : parVPARS.find("PARAMETER")->second.val) {
        const auto& det = nameToPtr.find(it.detector);
        if(det == nameToPtr.end())
            throw std::runtime_error(
                "Detector not find in optimization list\n");

        const auto& par = det->second.find(it.name);
        if(par == det->second.end())
            throw std::runtime_error(
                "Parameter not find in optimization list\n");

        it.ptr = par->second;
    }
}

std::istream& operator>>(std::istream& is, OptimizerConf::Parameter& v) {
    is >> v.detector >> v.name;
    is >> v.initialValue >> v.minValue >> v.maxValue >> v.step;
    v.ptr = nullptr;
    return is;
}
std::ostream& operator<<(std::ostream& os, const OptimizerConf::Parameter& v) {
    os << v.detector << " " << v.name << " " << v.initialValue << " "
       << v.minValue << " " << v.maxValue << " " << v.step;
    return os;
}
OptimizerConf::Parameter
operator*=(OptimizerConf::Parameter& v, const double& d) {
    v.initialValue *= d;
    v.minValue *= d;
    v.maxValue *= d;
    v.step *= d;
    return v;
}
std::istream&
operator>>(std::istream& is, OptimizerConf::ContourParameter& v) {
    is >> v.detector1 >> v.name1;
    is >> v.detector2 >> v.name2;
    is >> v.nPoints;
    return is;
}
std::ostream&
operator<<(std::ostream& os, const OptimizerConf::ContourParameter& v) {
    os << v.detector1 << " " << v.name1 << " ";
    os << v.detector2 << " " << v.name2 << " ";
    os << v.nPoints;
    return os;
}
OptimizerConf::ContourParameter
operator*=(OptimizerConf::ContourParameter& v, const double& d) {
    // Ignores units
    (void)d;
    return v;
}
