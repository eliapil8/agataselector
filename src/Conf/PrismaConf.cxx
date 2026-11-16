#include "PrismaConf.h"

PrismaConf::PrismaConf() : DetectorConf("PRISMA_CONF") {
    lutFile = "";

    parPI.emplace("Z_RANGE", Conf::Property<std::pair<int, int>&>{
                                 Zrange, "#", "Z for plots: min max"});
    parPI.emplace("A_RANGE", Conf::Property<std::pair<int, int>&>{
                                 Arange, "#", "A for plots: min max"});
    parI.emplace("DE_TYPE",
                 Conf::Property<int&>{de_type, "#",
                                      "DE variable used for Z gates: 0 -> "
                                      "DE_AB, 1 -> DE_A, 2 -> RANGE"});
    parI.emplace("CHARGE_TYPE",
                 Conf::Property<int&>{charge_type, "#",
                                      "Graph used for Q gates : 0 -> "
                                      "E vs RBeta, 1 -> E/RBeta vs E"});
    parI.emplace("MASS_TYPE",
		 Conf::Property<int&>{mass_type, "#",
		                      "Method to assign mass value: 0 -> round to next "
				      "integer, 1 -> TCutG on Mass vs Xfp"});
    parI.emplace("AQ_NBINS",
                 Conf::Property<int&>{aq_nbins, "#",
                                      "Number of bins in A over Q plots "});
    parLL.emplace("NR_REF",
                    Conf::Property<long long&>{nrRef, "#",
                                                "Number of events for reference"});
    parLL.emplace("TS_CORRECT",
                    Conf::Property<long long&>{ts_correct, "#",
                                                "Shift on TSPrisma with respect to AGATA (--update_prisma only)"});
    parD.emplace("X_SCALE",
                 Conf::Property<double&>{xScale, "#", "Scale of X of MCP"});
    parD.emplace("Y_SCALE",
                 Conf::Property<double&>{yScale, "#", "Scale of Y of MCP"});
    parD.emplace(
        "MCP_THETA_X",
        Conf::Property<double&>{mcpThetaX, "deg",
                                "Intrinsic rotation around X axis of MCP"});
    parD.emplace(
        "Z_ROT_ANGLE",
        Conf::Property<double&>{z_rot_angle, "deg",
                                "Rotation of IC_DE_A vs IC_Eres Z matrix for energy correction"});
    parD.emplace("TOF_GAIN",
                 Conf::Property<double&>{tofGain, "#",
                                         "Gain for recalibration of TOF"});
    parD.emplace("TOF_OFFSET",
                 Conf::Property<double&>{tofOffset, "ns",
                                         "Offset for recalibration of TOF"});
    parD.emplace("TAC_GAIN",
                 Conf::Property<double&>{tacGain, "#", "Gain for prisma tac"});
    parD.emplace("TAC_OFFSET", Conf::Property<double&>{
                                   tacOffset, "#", "Offset for prisma tac"});
    parD.emplace("MAX_IC_ENE",
                 Conf::Property<double&>{maxICEne, "#",
                                         "Maximum IC energy in histograms"});
    parD.emplace(
        "MAX_IC_dENE",
        Conf::Property<double&>{maxICdEne, "#",
                                "Maximum IC energy of deltaE in histograms"});
    parD.emplace(
        "MAX_EX_ENE",
        Conf::Property<double&>{
            maxExEne, "#", "Maximum Total Excitation energy in histograms"});
    parD.emplace(
        "MIN_EX_ENE",
        Conf::Property<double&>{
            minExEne, "#", "Minimum Total Excitation energy in histograms"});
    parD.emplace(
        "MAX_AOVERQ",
        Conf::Property<double&>{
            maxAoverQ, "#", "Maximum value of a over q in histograms"});
    parD.emplace(
        "MIN_AOVERQ",
        Conf::Property<double&>{
            minAoverQ, "#", "Minimum value of a over q in histograms"});
    parD.emplace(
        "MAX_RBETA",
        Conf::Property<double&>{
            maxRBeta, "#", "Maximum value of R*Beta in histograms"});
    parD.emplace(
        "MIN_RBETA",
        Conf::Property<double&>{
            minRBeta, "#", "Minimum value of R*Beta in histograms"});
    parB.emplace(
        "PRISMAFILTER_TKEL",
        Conf::Property<bool&>{
            prismafilter_tkel, "#",
            "Use prismafilter TKEL instead of internal calculations"});
    parB.emplace("RAW_HISTS",
                 Conf::Property<bool&>{raw, "#", "Enable raw histos"});
    parB.emplace("ANA_HISTS",
                 Conf::Property<bool&>{ana, "#", "Enable ana histos"});
    parB.emplace("MISC_HISTS",
                 Conf::Property<bool&>{misc, "#", "Enable misc histos"});
    parB.emplace(
        "AOVERQ_HISTS",
        Conf::Property<bool&>{Aoverq_hists, "#", "Enable A over Q histos"});
    parB.emplace(
        "AOVERQ_TEVO_HISTS",
        Conf::Property<bool&>{Aoverq_tevo_hists, "#",
                              "Enable time evolution A over Q histos"});

    parB.emplace("REQUIRE_MCPOK",
                 Conf::Property<bool&>{require_mcpok, "#",
                                       "Require mcp ok in analysis"});
    parB.emplace(
        "REQUIRE_TOFOK",
        Conf::Property<bool&>{require_tofok, "#",
                              "Require time of flight ok in analysis"});
    parB.emplace("REQUIRE_SIDEOK",
                 Conf::Property<bool&>{
                     require_sideok, "#",
                     "Require ionization chamber side ok in analysis"});
    parB.emplace("REQUIRE_TRAJOK",
                 Conf::Property<bool&>{require_trajok, "#",
                                       "Require trajectory ok in analysis"});
    parB.emplace(
        "REQUIRE_ICOK",
        Conf::Property<bool&>{require_icok, "#",
                              "Require ionization chamber ok in analysis"});
    parB.emplace(
        "ENABLE_PREPROTREE",
        Conf::Property<bool&>{enable_preprotreeFill, "#",
                              "Enable preprocess tree filling, similar to the "
                              "output of the prismafilters"});
    parB.emplace(
        "ENABLE_REFTREE",
        Conf::Property<bool&>{enable_reftreeFill, "#",
                              "Enable preprocess reference tree filling. For "
                              "tuning reconstruction parameters"});
    parB.emplace(
        "ENABLE_TOF_ALIGN",
        Conf::Property<bool&>{enable_tof_align, "#",
                              "Enable TOF alignment from TOF_ALIGN"});
    parB.emplace(
        "ENABLE_TRAJPLOT",
        Conf::Property<bool&>{enable_trajPlot, "#", "Enable trajectory plot"});
    parB.emplace("ENABLE_MAPPING",
                 Conf::Property<bool&>{
                     enable_mapping, "#",
                     "Enable trajectory reconstruction with mapping"});
    parB.emplace("PRISMA_UNITS_CUTS",
                 Conf::Property<bool&>{
                     prismaUnitsCuts, "#",
                     "Enable prisma units cuts for lengths in the cuts"});
    parB.emplace("ENABLE_CM_RECONSTRUCTION",
                 Conf::Property<bool&>{
                     enable_cm_reconstruction, "#",
                        "Enable CM reconstruction for fission analysis"});
    parB.emplace("AGATA_OPPOSITE",
                 Conf::Property<bool&>{
                     agata_opposite, "#",
                        "Fixes the agata angle to be 180 - prisma angle"});
    parS.emplace("MAPPING_FILE",
                 Conf::Property<std::string&>{
                     mapping_file, "#",
                     "File with mapping for trajectory reconstruction"});
    parS.emplace("TOF_FILE",
                 Conf::Property<std::string&>{
                     tof_file, "#",
                     "File with TOF offsets for each section"});
    parS.emplace("TOF_REF_FILE",
                 Conf::Property<std::string&>{
                     tof_ref_file, "#",
                     "File with reference TOF offsets for each section for automatic TOF alignment"});
    parS.emplace("MANAGER_PATH",
                 Conf::Property<std::string&>{
                     managerPath, "#",
                     "Path to manager file"});
    parS.emplace("ACCEPTANCE_MASK_FILE",
                    Conf::Property<std::string&>{
                        acceptance_mask_file, "#",
                        "File with acceptance mask"});
    parAQ.emplace("AQ_PLOTS",
                  Conf::Property<std::vector<AqConf>&>{
                      aqConfs, "#", "A/Q conf for histos. Z qmin qmax"});
    parAS.emplace("ANALYSIS_STAGES",
                  Conf::Property<AnalysisStage&>{
                      analysis_stages, "#",
                      "Analysis stages: RAWPROCESS, ANAPROCESS, RAW_ANA, HIGHLEVEL, ANA_HIGH, ALL"});

    parMI.emplace("BROKEN_IC_CHANNELS",
                  Conf::Property<std::unordered_map<int, int>&>{
                      broken_IC_channels, "#",
                      "Broken ionization channel segments: 1 -> broken, 0-> "
                      "not broken"});

    parMI.emplace("BROKEN_PPAC_CHANNELS",
                  Conf::Property<std::unordered_map<int, int>&>{
                      broken_PPAC_channels, "#",
                      "Broken MWPPAC segments: 1 -> broken, 0-> not broken"});

    parS.emplace("PRISMA_IN_PATH",
                 Conf::Property<std::string&>{
                     prisma_in_path, "#",
                     "Prisma input path (for --update_prisma option only)"});
    parS.emplace(
        "PRISMA_DIR_PATTERN",
        Conf::Property<std::string&>{
            prisma_dir_pattern, "#",
            "Prisma directory pattern (for --update_prisma option only)"});
    parS.emplace(
        "PRISMA_FILE_PATTERN",
        Conf::Property<std::string&>{
            prisma_file_pattern, "#",
            "Prisma input file pattern (for --update_prisma option only)"});
    // solver parameters
    parD.emplace(
        "TARGET_SHIFT_X",
        Conf::Property<double&>{targetShiftX, "mm", "Target X shift in mm"});
    parD.emplace(
        "TARGET_SHIFT_Y",
        Conf::Property<double&>{targetShiftY, "mm", "Target Y shift in mm"});
    parD.emplace(
        "TARGET_SHIFT_Z",
        Conf::Property<double&>{targetShiftZ, "mm", "Target Z shift in mm"});
    parD.emplace(
        "FP_SHIFT_X",
        Conf::Property<double&>{FPShiftX, "mm", "Focal plane X shift in mm"});
    parD.emplace(
        "FP_SHIFT_Y",
        Conf::Property<double&>{FPShiftY, "mm", "Focal plane Y shift in mm"});
    parD.emplace(
        "FP_SHIFT_Z",
        Conf::Property<double&>{FPShiftZ, "mm", "Focal plane Z shift in mm"});
    parD.emplace("TARGET_MCP_DISTANCE",
                 Conf::Property<double&>{target_mcp_distance, "mm",
                                         "Distance from target to MCP"});
    parD.emplace("MCP_ANGLE", Conf::Property<double&>{mcp_angle, "deg",
                                                      "MCP angle in degrees"});
    parD.emplace(
        "TARGET_QUAD_DISTANCE",
        Conf::Property<double&>{target_quad_distance, "mm",
                                "Distance from target to quadrupole"});
    parD.emplace(
        "QUAD_LENGTH",
        Conf::Property<double&>{quad_length, "mm", "Length of quadrupole"});
    parD.emplace(
        "QUAD_RADIUS",
        Conf::Property<double&>{quad_radius, "mm", "Radius of quadrupole"});
    parD.emplace("GEOM_RADIUS",
                 Conf::Property<double&>{geom_radius, "mm",
                                         "Geometrical radius of quadrupole"});
    parD.emplace("B_QUADRUPOLE",
                 Conf::Property<double&>{B_quadrupole, "tesla",
                                         "Magnetic field of quadrupole"});
    parD.emplace("B_DIPOLE",
                 Conf::Property<double&>{B_dipole, "tesla",
                                         "Magnetic field of dipole"});
    parD.emplace("REF_BRHO",
                 Conf::Property<double&>{refBrho, "tesla*m",
                                         "Reference Brho"});
    parD.emplace("TARGET_DIPOLE_DISTANCE",
                 Conf::Property<double&>{target_dipole_distance, "mm",
                                         "Distance from target to dipole"});
    parD.emplace("DIPOLE_RADIUS", Conf::Property<double&>{dipole_radius, "mm",
                                                          "Radius of dipole"});
    parD.emplace("DIPOLE_HEIGHT", Conf::Property<double&>{dipole_height, "mm",
                                                          "Height of dipole"});
    parD.emplace("IN_DIPOLE_ANGLE",
                 Conf::Property<double&>{in_dipole_angle, "deg",
                                         "Angle of dipole in degrees"});
    parD.emplace("OUT_DIPOLE_ANGLE",
                 Conf::Property<double&>{out_dipole_angle, "deg",
                                         "Angle of dipole in degrees"});
    parD.emplace(
        "DIPOLE_FP_DISTANCE",
        Conf::Property<double&>{dipole_fp_distance, "mm",
                                "Distance from dipole to focal plane"});
    parD.emplace("FP_HALF_LENGTH",
                 Conf::Property<double&>{fp_half_length, "mm",
                                         "Half length of focal plane"});
    parD.emplace("FP_HALF_HEIGHT",
                 Conf::Property<double&>{fp_half_height, "mm",
                                         "Half height of focal plane"});
    parD.emplace("FP_TOLERANCE",
                 Conf::Property<double&>{fp_tolerance, "mm",
                                         "Tolerance of focal plane"});
    parD.emplace("TRAJ_ANGULAR_TOLERANCE",
                 Conf::Property<double&>{traj_angular_tolerance, "deg",
                                         "Angular tolerance of trajectory"});        
    parD.emplace("GASICA_THICKNESS",
                 Conf::Property<double&>{gasICA_thickness, "mm",
                                         "Assumed thickness of first layer od IC (for non-measured energy calculations)"});  
}

std::ostream& PrismaConf::operator<<(std::ostream& os) const {
    DetectorConf::operator<<(os);
    for(const auto& it : parAQ) {
        if(it.second.val.empty())
            throw std::runtime_error("Should not be initialized empty\n");
        for(const auto& it2 : it.second.val) {
            // Property<decltype(it.second.val)> tmp{it2, it.second.unit,
            // it.second.comment};
            Property<AqConf> tmp{it2, it.second.unit, it.second.comment};
            os << it.first << "\t\t\t";
            tmp << os;
        }
    }
    for(const auto& it : parAS) {
        os << it.first << "\t\t\t";
        it.second << os;
    }
    return os;
}

std::istream& PrismaConf::operator>>(std::istream& is) {
    DetectorConf::operator>>(is);
    std::string line;

    // Erase previous map
    if(!parAQ.empty()) parAQ.begin()->second.val.clear();

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

        { // AQconf type parameters
            auto it = parAQ.find(name);
            if(it != parAQ.end()) {
                Property<AqConf> tmp;
                tmp >> sstr;
                it->second.val.emplace_back(tmp.val);
                continue;
            }
        }
        { // AS type parameters
            auto it = parAS.find(name);
            if(it != parAS.end()) {
                it->second >> sstr;
                continue;
            }
        }
    }
    //ReadManager(managerPath); //It is now read by confmanager
    return is;
}

std::istream& operator>>(std::istream& is, PrismaConf::AqConf& v) {
    is >> v.Z >> v.qmin >> v.qmax;
    return is;
}

std::ostream& operator<<(std::ostream& os, const PrismaConf::AqConf& v) {
    os << v.Z << " " << v.qmin << " " << v.qmax;
    return os;
}
PrismaConf::AqConf operator*=(PrismaConf::AqConf& v, const double& d) {
    v.Z *= static_cast<int>(d);
    v.qmin *= static_cast<int>(d);
    v.qmax *= static_cast<int>(d);
    return v;
}

std::istream& operator>>(std::istream& is, PrismaConf::AnalysisStage& v) {
    std::string s;
    is >> s;
    if(s == "RAWPROCESS")
        v = PrismaConf::AnalysisStage::RAWPROCESS;
    else if(s == "ANAPROCESS")
        v = PrismaConf::AnalysisStage::ANAPROCESS;
    else if(s == "HIGHLEVEL")
        v = PrismaConf::AnalysisStage::HIGHLEVEL;
    else if(s == "HIGHLEVEL")
        v = PrismaConf::AnalysisStage::HIGHLEVEL;
    else if(s == "ANA_HIGH")
        v = PrismaConf::AnalysisStage::ANA_HIGH;
    else if(s == "ALL")
        v = PrismaConf::AnalysisStage::ALL;
    else throw std::runtime_error("Provided keyword for prisma analysis stage not recognizead\n");
    return is;
}

std::ostream&
operator<<(std::ostream& os, const PrismaConf::AnalysisStage& v) {
    switch(v) {
    case PrismaConf::AnalysisStage::RAWPROCESS: os << "RAWPROCESS"; break;
    case PrismaConf::AnalysisStage::ANAPROCESS: os << "ANAPROCESS"; break;
    case PrismaConf::AnalysisStage::HIGHLEVEL: os << "HIGHLEVEL"; break;
    case PrismaConf::AnalysisStage::ANA_HIGH: os << "ANA_HIGH"; break;
    case PrismaConf::AnalysisStage::ALL: os << "ALL"; break;
    }
    return os;
}

PrismaConf::AnalysisStage
operator*=(PrismaConf::AnalysisStage& v, const double& d) {
    (void)d;
    return v;
}
