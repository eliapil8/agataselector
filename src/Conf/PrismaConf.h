#pragma once

#include <unordered_map>
#include <vector>

#include "DetectorConf.h"

class PrismaConf : public DetectorConf {
  public:
    PrismaConf();


    std::string                  prisma_in_path{"./PrismaData"};
    std::string                  prisma_dir_pattern{"run_"};
    std::string                  prisma_file_pattern{"Tree_"};
    std::pair<int, int>          Zrange{0, 2};
    std::pair<int, int>          Arange{0, 4};
    int                          de_type{0};
    int                          charge_type{0};
    int                          mass_type{0};
    int                          aq_nbins{1000};
    long long                    nrRef{100000};
    long long                    ts_correct{0};
    double                       xScale{1.};
    double                       yScale{1.};
    double                       mcpThetaX{0.};
    double                       z_rot_angle{9*UNITS::deg};
    double                       tofOffset{0.};
    double                       tofGain{1.};
    double                       tacOffset{0};
    double                       tacGain{1};
    double                       maxICEne{10000};
    double                       maxICdEne{7000};
    double                       maxExEne{100};
    double                       minExEne{-100};
    double                       maxAoverQ{500};
    double                       minAoverQ{100};
    double                       maxRBeta{4000};
    double                       minRBeta{0.01};
    bool                         prismafilter_tkel{false};
    bool                         raw{false};
    bool                         ana{false};
    bool                         misc{false};
    bool                         Aoverq_hists{true};
    bool                         Aoverq_tevo_hists{false};
    bool                         require_mcpok{true};
    bool                         require_tofok{true};
    bool                         require_sideok{true};
    bool                         require_trajok{true};
    bool                         require_icok{true};
    bool                         enable_trajPlot{false};
    bool                         enable_preprotreeFill{false};
    bool                         enable_reftreeFill{false};
    bool                         enable_tof_align{false};
    bool                         enable_mapping{false};
    bool                         doneTofAlign {false};
    bool                         prismaUnitsCuts {true};
    bool                         enable_cm_reconstruction{false};
    bool                         agata_opposite{false};
    std::string                  mapping_file{"Prisma/mapping.dat"};
    std::string                  tof_file{"Prisma/PrismaTOF.dat"};
    std::string                  tof_ref_file{"Prisma/PrismaRefTOF.dat"};
    std::string                  managerPath{"./Conf/prismaManager.conf"};
    std::string                  acceptance_mask_file{""};

    //solver parameters
    double                       targetShiftX           {0.*UNITS::mm};
    double                       targetShiftY           {0.*UNITS::mm};
    double                       targetShiftZ           {0.*UNITS::mm};
    double                       FPShiftX               {0.*UNITS::mm};
    double                       FPShiftY               {0.*UNITS::mm};
    double                       FPShiftZ               {0.*UNITS::mm}; 
    double                       target_mcp_distance    {250.*UNITS::mm};
    double                       mcp_angle              {135.*UNITS::deg};
    double                       target_quad_distance   {470.*UNITS::mm};
    double                       quad_length            {400.*UNITS::mm};
    double                       quad_radius            {160.*UNITS::mm};
    double                       geom_radius            {150.*UNITS::mm};
    double                       B_quadrupole           {1.*UNITS::tesla};
    double                       B_dipole               {1.*UNITS::tesla};
    double                       refBrho                {1.*UNITS::tesla*UNITS::m};
    double                       target_dipole_distance {1600.*UNITS::mm};
    double                       dipole_radius          {1200.*UNITS::mm};
    double                       dipole_height          {200.*UNITS::mm};
    double                       in_dipole_angle        {20.*UNITS::deg};
    double                       out_dipole_angle       {125.*UNITS::deg};
    double                       dipole_fp_distance     {3285.*UNITS::mm};
    double                       fp_half_length         {500.*UNITS::mm};
    double                       fp_half_height         {65.*UNITS::mm};
    double                       fp_tolerance           {1.*UNITS::mm};
    double                       traj_angular_tolerance {1.3*UNITS::deg};
    double                       gasICA_thickness       {265*UNITS::mm};   
    double                       ic_fp_distance         {720*UNITS::mm};
    double                       ic_depth               {250*UNITS::mm};

 
    std::unordered_map<int, int> broken_IC_channels{
        {2, 1}
    };
    std::unordered_map<int, int> broken_PPAC_channels{
        {9, 0}
    };

    enum AnalysisStage{
        RAWPROCESS = 1,         //Calculated Calibrated and Run Solver
        ANAPROCESS = 2,         //Read Calibrated and Run Solver
        HIGHLEVEL = 3,          //Read All and Make Histos
        ANA_HIGH = 4,           //Run Solver and Make Histos
        ALL = 5,                //Calculate Calibrated, Run Solver and Make Histos
    };
    AnalysisStage                analysis_stages{AnalysisStage::HIGHLEVEL};

    struct AqConf { // A/q plot parameters (fixed Z)
        int Z, qmin, qmax;
    };
    std::vector<AqConf> aqConfs{
        {1, 0, 1}
    };

    std::ostream& operator<<(std::ostream& os) const override;
    std::istream& operator>>(std::istream& is) override;

    std::string mcpGatePath =       "/CUT/PRISMA/RAW/MCP";
    std::string ppacGatesPath =     "/CUT/PRISMA/RAW/PPAC";
    std::string TOFGatePath =       "/CUT/PRISMA/ANA/TOF";
    std::string qGatesPath =        "/CUT/PRISMA/ANA/CHARGE";
    std::string validQGatePath =    "/CUT/PRISMA/ANA/VALID_Q";
    std::string zGatesPath =        "/CUT/PRISMA/ANA/ZED";
    std::string aGatesPath =        "/CUT/PRISMA/ANA/MASS";
    std::string zLinesPath =        "/CUT/PRISMA/ANA/ZED_LINES";
    std::string qLinesPath =        "/CUT/PRISMA/ANA/Q_LINES";
    std::string zMassLinesPath =    "/CUT/PRISMA/ANA/Z_MASS_LINES";
    std::string aoqCorrXmcpPath =   "/CUT/PRISMA/ANA/AOVERQ_XMCP";
    std::string aoqCorrYmcpPath =   "/CUT/PRISMA/ANA/AOVERQ_YMCP";
    std::string aoqCorrXfpPath =    "/CUT/PRISMA/ANA/AOVERQ_XFP";
    std::string icDEACorrXfpPath =  "/CUT/PRISMA/ANA/ICDEA_XFP";
    std::string icDEACorrYfpPath =  "/CUT/PRISMA/ANA/ICDEA_YFP";
    std::string icDEABCorrXfpPath =  "/CUT/PRISMA/ANA/ICDEAB_XFP";
    std::string icDEABCorrYfpPath =  "/CUT/PRISMA/ANA/ICDEAB_YFP";
    std::string icECorrXfpPath =    "/CUT/PRISMA/ANA/ICECORR_XFP";
    std::string icECorrYfpPath =    "/CUT/PRISMA/ANA/ICECORR_YFP";
    std::string icEnmBetaPath =     "/CUT/PRISMA/ANA/ICENM_BETA";
    std::string icEnmBetaZedPath =  "/CUT/PRISMA/ANA/ICENM_BETA_ZED";
    std::string aoqCorrTimePath =   "/CUT/PRISMA/ANA/AOVERQ_TIME";
    std::string tofCalPath =        "/CALIBRATION/PRISMA/RAW/tof.cal";
    std::string ppacThreshPath =    "/CALIBRATION/PRISMA/RAW/ppac_tresh.cal";
    std::string ppacPosCalPath =    "/CALIBRATION/PRISMA/RAW/ppac_pos_cal.cal";
    std::string icECalPath =        "/CALIBRATION/PRISMA/RAW/ice.cal";
    std::string icSidesCalPath =    "/CALIBRATION/PRISMA/RAW/icsides.cal";
    std::string icDriftCalPath =    "/CALIBRATION/PRISMA/RAW/icdrift.cal";
    std::string mcpCalPath =        "/CALIBRATION/PRISMA/RAW/mcp.cal";
    std::string mcpRotCalPath =     "/CALIBRATION/PRISMA/RAW/mcprot.cal";
    std::string monitorCalPath =    "/CALIBRATION/PRISMA/RAW/monitor.cal";
    std::string aoqCalPath =        "/CALIBRATION/PRISMA/ANA/aoverq.cal";
    std::string chargeCalPath =     "/CALIBRATION/PRISMA/ANA/charge.cal";
    std::string icECorrPath =       "/CALIBRATION/PRISMA/ANA/icecorr.cal";
    std::string xmcpLengthPath =    "/CALIBRATION/PRISMA/ANA/xmcp_length.cal";
    std::string lengthRecalPath =   "/CALIBRATION/PRISMA/ANA/length_recal.cal";
    std::string thetaCMLimitsPath = "/CALIBRATION/PRISMA/ANA/thetaCM_limits.cal";


    void ReadManager(std::string managerpath = "./Conf/prismaManager.conf") {
        std::ifstream file(managerpath);
        if(Settings::getInstance().verbosity_level > 1) std::cout << "Reading Prisma manager file " << managerpath << std::endl;
        if(!file.is_open()) {
            std::cerr << "Could not open manager file " << managerpath << std::endl;
            return;
        }
        std::string line;
        std::stringstream sstr;
        while(std::getline(file, line)) {
            if(line[0] == '#') continue;
            sstr.clear();
            sstr.str(line);
            std::string token;
            sstr >> token;

            if(token == "MCP_GATE")             { sstr >> mcpGatePath;          if(Settings::getInstance().verbosity_level > 1)    std::cout << "MCP_GATE " << mcpGatePath << std::endl;                }     
            if(token == "PPAC_GATE")            { sstr >> ppacGatesPath;        if(Settings::getInstance().verbosity_level > 1)    std::cout << "PPAC_GATE " << ppacGatesPath << std::endl;             }         
            if(token == "TOF_GATE")             { sstr >> TOFGatePath;          if(Settings::getInstance().verbosity_level > 1)    std::cout << "TOF_GATE " << TOFGatePath << std::endl;                }     
            if(token == "Q_GATE")               { sstr >> qGatesPath;           if(Settings::getInstance().verbosity_level > 1)    std::cout << "Q_GATE " << qGatesPath << std::endl;                   } 
            if(token == "Z_GATE")               { sstr >> zGatesPath;           if(Settings::getInstance().verbosity_level > 1)    std::cout << "Z_GATE " << zGatesPath << std::endl;                   }
	    if(token == "A_GATE")               { sstr >> aGatesPath;           if(Settings::getInstance().verbosity_level > 1)    std::cout << "A_GATE " << aGatesPath << std::endl;                   } 
            if(token == "VALID_Q")              { sstr >> validQGatePath;       if(Settings::getInstance().verbosity_level > 1)    std::cout << "VALID_Q " << validQGatePath << std::endl;              } 
            if(token == "Z_LINES")              { sstr >> zLinesPath;           if(Settings::getInstance().verbosity_level > 1)    std::cout << "Z_LINES " << zLinesPath << std::endl;                  } 
            if(token == "Q_LINES")              { sstr >> qLinesPath;           if(Settings::getInstance().verbosity_level > 1)    std::cout << "Q_LINES " << qLinesPath << std::endl;                  } 
            if(token == "Z_MASS_LINES")         { sstr >> zMassLinesPath;       if(Settings::getInstance().verbosity_level > 1)    std::cout << "Z_MASS_LINES " << zMassLinesPath << std::endl;         } 
            if(token == "AOQXMCP_CORR")         { sstr >> aoqCorrXmcpPath;      if(Settings::getInstance().verbosity_level > 1)    std::cout << "AOQXMCP_CORR " << aoqCorrXmcpPath << std::endl;        }             
            if(token == "AOQYMCP_CORR")         { sstr >> aoqCorrYmcpPath;      if(Settings::getInstance().verbosity_level > 1)    std::cout << "AOQYMCP_CORR " << aoqCorrYmcpPath << std::endl;        }             
            if(token == "AOQXFP_CORR")          { sstr >> aoqCorrXfpPath;       if(Settings::getInstance().verbosity_level > 1)    std::cout << "AOQXFP_CORR " << aoqCorrXfpPath << std::endl;          }         
            if(token == "ICDEAXFP_CORR")        { sstr >> icDEACorrXfpPath;     if(Settings::getInstance().verbosity_level > 1)    std::cout << "ICDEAXFP_CORR " << icDEACorrXfpPath << std::endl;      }             
            if(token == "ICDEAYFP_CORR")        { sstr >> icDEACorrYfpPath;     if(Settings::getInstance().verbosity_level > 1)    std::cout << "ICDEAYFP_CORR " << icDEACorrYfpPath << std::endl;      }                     
            if(token == "ICDEABXFP_CORR")       { sstr >> icDEABCorrXfpPath;    if(Settings::getInstance().verbosity_level > 1)    std::cout << "ICDEABXFP_CORR " << icDEABCorrXfpPath << std::endl;    }             
            if(token == "ICDEABYFP_CORR")       { sstr >> icDEABCorrYfpPath;    if(Settings::getInstance().verbosity_level > 1)    std::cout << "ICDEABYFP_CORR " << icDEABCorrYfpPath << std::endl;    }             
            if(token == "ICECORR_XFP")          { sstr >> icECorrXfpPath;       if(Settings::getInstance().verbosity_level > 1)    std::cout << "ICECORR_XFP " << icECorrXfpPath << std::endl;          }         
            if(token == "ICECORR_YFP")          { sstr >> icECorrYfpPath;       if(Settings::getInstance().verbosity_level > 1)    std::cout << "ICECORR_YFP " << icECorrYfpPath << std::endl;          }         
            if(token == "ICENM_BETA")           { sstr >> icEnmBetaPath;        if(Settings::getInstance().verbosity_level > 1)    std::cout << "ICENM_BETA " << icEnmBetaPath << std::endl;            }         
            if(token == "ICENM_BETA_ZED")       { sstr >> icEnmBetaZedPath;     if(Settings::getInstance().verbosity_level > 1)    std::cout << "ICENM_BETA_ZED " << icEnmBetaZedPath << std::endl;     }         
            if(token == "AOQTIME_CORR")         { sstr >> aoqCorrTimePath;      if(Settings::getInstance().verbosity_level > 1)    std::cout << "AOQTIME_CORR " << aoqCorrTimePath << std::endl;        }             
            if(token == "TOF_CAL")              { sstr >> tofCalPath;           if(Settings::getInstance().verbosity_level > 1)    std::cout << "TOF_CAL " << tofCalPath << std::endl;                  } 
            if(token == "PPAC_THRESH")          { sstr >> ppacThreshPath;       if(Settings::getInstance().verbosity_level > 1)    std::cout << "PPAC_THRESH " << ppacThreshPath << std::endl;          }         
            if(token == "PPACPOS_CAL")          { sstr >> ppacPosCalPath;       if(Settings::getInstance().verbosity_level > 1)    std::cout << "PPACPOS_CAL " << ppacPosCalPath << std::endl;          }         
            if(token == "ICE_CAL")              { sstr >> icECalPath;           if(Settings::getInstance().verbosity_level > 1)    std::cout << "ICE_CAL " << icECalPath << std::endl;                  } 
            if(token == "ICSIDES_CAL")          { sstr >> icSidesCalPath;       if(Settings::getInstance().verbosity_level > 1)    std::cout << "ICSIDES_CAL " << icSidesCalPath << std::endl;          }         
            if(token == "ICDRIFT_CAL")          { sstr >> icDriftCalPath;       if(Settings::getInstance().verbosity_level > 1)    std::cout << "ICDRIFT_CAL " << icDriftCalPath << std::endl;          }         
            if(token == "MCP_CAL")              { sstr >> mcpCalPath;           if(Settings::getInstance().verbosity_level > 1)    std::cout << "MCP_CAL " << mcpCalPath << std::endl;                  } 
            if(token == "MCPROT_CAL")           { sstr >> mcpRotCalPath;        if(Settings::getInstance().verbosity_level > 1)    std::cout << "MCPROT_CAL " << mcpRotCalPath << std::endl;            }         
            if(token == "MONITOR_CAL")          { sstr >> monitorCalPath;       if(Settings::getInstance().verbosity_level > 1)    std::cout << "MONITOR_CAL " << monitorCalPath << std::endl;          }         
            if(token == "AOQ_CAL")              { sstr >> aoqCalPath;           if(Settings::getInstance().verbosity_level > 1)    std::cout << "AOQ_CAL " << aoqCalPath << std::endl;                  } 
            if(token == "CHARGE_CAL")           { sstr >> chargeCalPath;        if(Settings::getInstance().verbosity_level > 1)    std::cout << "CHARGE_CAL " << chargeCalPath << std::endl;            }         
            if(token == "ICE_CORR")             { sstr >> icECorrPath;          if(Settings::getInstance().verbosity_level > 1)    std::cout << "ICE_CORR " << icECorrPath << std::endl;                }     
            if(token == "XMCP_LENGTH")          { sstr >> xmcpLengthPath;       if(Settings::getInstance().verbosity_level > 1)    std::cout << "XMCP_LENGTH " << xmcpLengthPath << std::endl;          }         
            if(token == "LENGTH_RECAL")         { sstr >> lengthRecalPath;      if(Settings::getInstance().verbosity_level > 1)    std::cout << "LENGTH_RECAL " << lengthRecalPath << std::endl;        }         
            if(token == "THETA_CM_LIMITS")      { sstr >> thetaCMLimitsPath;    if(Settings::getInstance().verbosity_level > 1)    std::cout << "THETA_CM_LIMITS " << thetaCMLimitsPath << std::endl;   }         
        }
    }

  protected:
    std::unordered_map<std::string, Property<std::vector<AqConf>&>> parAQ;
    std::unordered_map<std::string, Property<AnalysisStage&>> parAS;
};

std::istream&      operator>>(std::istream& is, PrismaConf::AqConf& vec);
std::ostream&      operator<<(std::ostream& os, const PrismaConf::AqConf& vec);
PrismaConf::AqConf operator*=(PrismaConf::AqConf& v, const double& d);

std::istream&      operator>>(std::istream& is, PrismaConf::AnalysisStage& v);
std::ostream&      operator<<(std::ostream& os, const PrismaConf::AnalysisStage& v);
PrismaConf::AnalysisStage operator*=(PrismaConf::AnalysisStage& v, const double& d);
