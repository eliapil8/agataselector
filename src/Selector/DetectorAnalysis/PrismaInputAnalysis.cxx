#include "PrismaInputAnalysis.h"

#include <iomanip>

#include "Prisma.h"
#include "PrismaInput.h"
#include "ReactionFragment.h"

PrismaInputAnalysis::PrismaInputAnalysis(DetectorAnalysis::Inputs in, PrismaInput &pi)
    : DetectorAnalysis(in, pi, ConfManager::getInstance().prisma, prismaInputHistograms),
      prismaInput(pi), conf(ConfManager::getInstance()),
      TOFGate(conf.replay.conf_path + conf.prisma.TOFGatePath),
      qGates(conf.replay.conf_path + conf.prisma.qGatesPath),
      zGates(conf.replay.conf_path + conf.prisma.zGatesPath),
      aGates(conf.replay.conf_path + conf.prisma.aGatesPath),
      validQGate(conf.replay.conf_path + conf.prisma.validQGatePath),
      zLines(conf.replay.conf_path + conf.prisma.zLinesPath),
      qLines(conf.replay.conf_path + conf.prisma.qLinesPath),
      zMassLines(conf.replay.conf_path + conf.prisma.zMassLinesPath),
      mcpGate(conf.replay.conf_path + conf.prisma.mcpGatePath),
      ppacGates(conf.replay.conf_path + conf.prisma.ppacGatesPath),
      aoqCorrXmcp(conf.replay.conf_path + conf.prisma.aoqCorrXmcpPath),
      aoqCorrYmcp(conf.replay.conf_path + conf.prisma.aoqCorrYmcpPath),
      aoqCorrXfp(conf.replay.conf_path + conf.prisma.aoqCorrXfpPath),
      icDEACorrXfp(conf.replay.conf_path + conf.prisma.icDEACorrXfpPath),
      icDEABCorrXfp(conf.replay.conf_path + conf.prisma.icDEABCorrXfpPath),
      icDEACorrYfp(conf.replay.conf_path + conf.prisma.icDEACorrYfpPath),
      icDEABCorrYfp(conf.replay.conf_path + conf.prisma.icDEABCorrYfpPath),
      icECorrXfp(conf.replay.conf_path + conf.prisma.icECorrXfpPath),
      icECorrYfp(conf.replay.conf_path + conf.prisma.icECorrYfpPath),
      icEnmBeta(conf.replay.conf_path + conf.prisma.icEnmBetaPath),
      icEnmZedBeta(conf.replay.conf_path + conf.prisma.icEnmBetaZedPath),
      aoqCorrTime(conf.replay.conf_path + conf.prisma.aoqCorrTimePath),
      tofCal(conf.replay.conf_path + conf.prisma.tofCalPath),
      ppacThresh(conf.replay.conf_path + conf.prisma.ppacThreshPath),
      ppacPosCal(conf.replay.conf_path + conf.prisma.ppacPosCalPath),
      icECal(conf.replay.conf_path + conf.prisma.icECalPath),
      icSidesCal(conf.replay.conf_path + conf.prisma.icSidesCalPath),
      icDriftCal(conf.replay.conf_path + conf.prisma.icDriftCalPath),
      mcpCal(conf.replay.conf_path + conf.prisma.mcpCalPath),
      mcpRotCal(conf.replay.conf_path + conf.prisma.mcpRotCalPath),
      monitorCal(conf.replay.conf_path + conf.prisma.monitorCalPath),
      aoqCal(conf.replay.conf_path + conf.prisma.aoqCalPath),
      chargeCal(conf.replay.conf_path + conf.prisma.chargeCalPath),
      icECorr(conf.replay.conf_path + conf.prisma.icECorrPath),
      xmcpLength(conf.replay.conf_path + conf.prisma.xmcpLengthPath),
      lengthRecal(conf.replay.conf_path + conf.prisma.lengthRecalPath)
{

    if (input.tree && conf.prisma.enable_preprotreeFill && input.type == Inputs::Type::SIGNAL)
        input.tree->Branch("PrismaInput", &prismaInput);
    if (input.tree && conf.prisma.enable_reftreeFill && input.type == Inputs::Type::SIGNAL)
        input.tree->Branch("PrismaInputRef", &prismaInputRef);
    prismaInput.clear();
    prismaInputRef.clear();

    if (conf.prisma.enable_tof_align)
        ReadTof();

    if (!conf.prisma.prismaUnitsCuts)
    {
        units_aoq    = 1.;
        units_length = 1.;
        // units_time = 1.;
    }

    // PrismaInputHistograms& h{prismaInputHistograms};
}

bool PrismaInputAnalysis::AllocateHistos()
{
    PrismaInputHistograms &h = prismaInputHistograms;

    h.dir = input.outFile.mkdir("PrismaInput", "Prisma preprocessing histograms");
    h.dir->cd();

    Initialize<TH2D>(
        h.TofOffsetNew_vs_colNr,
        new TH2D("TofOffsetNew_vs_colNr", "TofOffsetNew_vs_colNr", 10, -0.5, 9.5, 4000, -20, 20),
        h.ptrs);
    if (!DetectorAnalysis::AllocateHistos())
        return false;

    return true;
}

void PrismaInputAnalysis::Analyze()
{
    DetectorAnalysis::Analyze();

    if (settings.verbosity_level > 15)
    {
        std::cout << "PrismaInputAnalysis::Analyze" << std::endl;
    }

    prismaInput.TS           = **prismaReader.TSPrisma;
    prismaInput.prisma_angle = conf.prisma.angle;

    // Read stuff from the readers
    if (conf.prisma.analysis_stages == PrismaConf::AnalysisStage::ANAPROCESS ||
        conf.prisma.analysis_stages == PrismaConf::AnalysisStage::ANA_HIGH ||
        conf.prisma.analysis_stages == PrismaConf::AnalysisStage::HIGHLEVEL)
    {
        // Reading readers
        prismaInput.mcp_x = static_cast<float>(
            (**prismaReader.MCP_X * UNITS::mm + conf.prisma.xShift) * conf.prisma.xScale);
        prismaInput.mcp_y = static_cast<float>(
            (**prismaReader.MCP_Y * UNITS::mm + conf.prisma.yShift) * conf.prisma.yScale);
        prismaInput.x_fp = static_cast<float>(**prismaReader.X_FP * UNITS::mm);
        prismaInput.y_fp = static_cast<float>(**prismaReader.Y_FP * UNITS::mm);

        prismaInput.mcp_theta = **prismaReader.MCP_Theta * UNITS::deg;
        prismaInput.mcp_phi   = **prismaReader.MCP_Phi * UNITS::deg;

        // Beta settings
        prismaInput.tof =
            (**prismaReader.TOF * UNITS::ns); // * conf.prisma.tofGain + conf.prisma.tofOffset;
        prismaInput.beta =
            (**prismaReader.Length * UNITS::mm) / prismaInput.tof / UNITS::PHYSICS::c_light;
        prismaInput.gamma = 1. / sqrt(1 - prismaInput.beta * prismaInput.beta);

        prismaInput.a_over_q = **prismaReader.A_over_q;

        prismaInput.ic_e     = **prismaReader.IC_E;
        prismaInput.ic_de_a  = **prismaReader.IC_DE_A;
        prismaInput.ic_de_ab = **prismaReader.IC_DE_AB;
        prismaInput.ic_range = **prismaReader.IC_RANGE;

        prismaInput.z_nbr = **prismaReader.Z_Nr;
        prismaInput.q_nbr = **prismaReader.Q_Nr;
        prismaInput.a_nbr = **prismaReader.A_Nr;

        prismaInput.tof_ok  = **prismaReader.tof_ok;
        prismaInput.mcp_ok  = **prismaReader.mcp_ok;
        prismaInput.traj_ok = **prismaReader.traj_ok;
        prismaInput.side_ok = **prismaReader.side_ok;
        prismaInput.ic_ok   = **prismaReader.ic_ok;
        prismaInput.ppac_ok = true;

        prismaInput.monitor_0 = **prismaReader.MONITOR_1;
        prismaInput.monitor_1 = **prismaReader.MONITOR_2;

        for (int i = 0; i < 40; i++)
        {
            prismaInput.ic_pads[i] = static_cast<float>(prismaReader.IC_Pads->At(i));
        }
        prismaInput.radius = **prismaReader.Radius * UNITS::mm;
        prismaInput.length = **prismaReader.Length * UNITS::mm;
        prismaInput.qvalue = **prismaReader.Qvalue * UNITS::MeV;
        // prismaInput.rbeta  = prismaInput.radius*prismaInput.beta;
        prismaInput.rbeta = **prismaReader.RBeta;

        prismaInput.mass = **prismaReader.Mass;
        prismaInput.brho = prismaInput.radius * conf.prisma.B_dipole / (UNITS::tesla * UNITS::m);

        prismaInput.ic_a_numpads = **prismaReader.IC_a_numpads;
        prismaInput.ic_b_numpads = **prismaReader.IC_b_numpads;
        prismaInput.ic_c_numpads = **prismaReader.IC_c_numpads;
        prismaInput.ic_d_numpads = **prismaReader.IC_d_numpads;

        prismaInput.ic_col_a = **prismaReader.IC_col_a;
        prismaInput.ic_col_b = **prismaReader.IC_col_b;
        prismaInput.ic_col_c = **prismaReader.IC_col_c;
        prismaInput.ic_col_d = **prismaReader.IC_col_d;
    }
    else
    {
        // Oherwise calibrate the raw data
        CalibrateRaw(false);
    }

    // If highlevel analysis stop here
    if (conf.prisma.analysis_stages == PrismaConf::AnalysisStage::HIGHLEVEL)
        return;

    if (conf.prisma.enable_reftreeFill && !conf.prisma.doneTofAlign)
    {
        // Fill reference branch
        RunSolver(true);
    }
    // Fill actual branch with paramteres from the conf

    RunSolver(false);

    // if(!prismaInput.traj_ok) return;

    CheckGatesZ();

    CheckGatesQ();

    CalibrateAna();

    if (!conf.prisma.doneTofAlign && conf.prisma.enable_reftreeFill)
    {
        FillTofOffset();
    }
}

double PrismaInputAnalysis::CalculateAoverQ(const double &radius, const double &beta)
{
    return (radius * prismaSolver.GetBdipole()) /
           (3.107 * UNITS::tesla * UNITS::m * beta / std::sqrt(1 - beta * beta));
}

double PrismaInputAnalysis::CalculateTOF(const double &a_over_q,
                                         const double &length,
                                         const double &radius)
{
    double betagamma =
        (radius * prismaSolver.GetBdipole()) / (3.107 * UNITS::tesla * UNITS::m * a_over_q);
    double beta = sqrt(pow(betagamma, 2) / (1 + pow(betagamma, 2)));
    return length / (beta * UNITS::PHYSICS::c_light);
}

double PrismaInputAnalysis::CalculateLength(const double &a_over_q,
                                            const double &tof,
                                            const double &radius)
{
    double betagamma =
        (radius * prismaSolver.GetBdipole()) / (3.107 * UNITS::tesla * UNITS::m * a_over_q);
    double beta = sqrt(pow(betagamma, 2) / (1 + pow(betagamma, 2)));
    return beta * UNITS::PHYSICS::c_light * tof;
}

//================= Functions for automatic TOF alignment
//==========================================================

void PrismaInputAnalysis::FillTofOffset()
{
    if (abs(prismaInputRef.a_over_q - 3) < 0.0005)
    {
        double BetaGamma = prismaInput.radius * prismaSolver.GetBdipole() /
                           (3.107 * UNITS::tesla * UNITS::m * prismaInputRef.a_over_q);
        double Beta      = sqrt(pow(BetaGamma, 2) / (1 + pow(BetaGamma, 2)));
        double tof       = prismaInput.length / (Beta * UNITS::PHYSICS::c_light);
        double tofOffset = prismaInput.tof - tof;

        if (settings.verbosity_level > 5)
            std::cout << std::setprecision(3) << "A/Q: " << prismaInput.a_over_q << "   "
                      << prismaInputRef.a_over_q * 1e12 << std::endl;
        if (settings.verbosity_level > 5)
            std::cout << std::setprecision(3) << "Tof: " << tof << std::endl;
        if (settings.verbosity_level > 5)
            std::cout << std::setprecision(3) << "TofOffset: " << tofOffset << std::endl;
        if (settings.verbosity_level > 5)
            std::cout << std::setprecision(3) << "Offset : " << prismaInput.col_nbr << " "
                      << tofOffset * 1e9 << std::endl;

        Fill(prismaInputHistograms.TofOffsetNew_vs_colNr, prismaInput.col_nbr, tofOffset * 1e9);
    }
}

void PrismaInputAnalysis::CalculateTofOffset()
{
    if (prismaInputHistograms.TofOffsetNew_vs_colNr == nullptr)
        return;
    for (int i = 0; i < 10; i++)
    {
        auto h_col = prismaInputHistograms.TofOffsetNew_vs_colNr->ProjectionY("h_py", i + 1, i + 1);
        double mean = h_col->GetMean();
        // double sigma = h_col->GetRMS();
        // TF1 *gaus = new TF1("gaus","gaus",mean-3*sigma,mean+3*sigma);
        // h_col->Fit("gaus","R");
        double tofOffset = prismaSolver.GetTofAlign(false, i) - mean; // gaus->GetParameter(1);
        if (settings.verbosity_level > 5)
            std::cout << std::setprecision(3) << "Column: " << i << "  Offset: " << tofOffset
                      << std::endl;
        prismaSolver.SetTofAlign(false, i, tofOffset);
    }
    std::ofstream tof_out_file(conf.replay.conf_path + "/" + conf.prisma.tof_file);
    PrintTof(tof_out_file);
    tof_out_file.close();
    return;
}

void PrismaInputAnalysis::ReadTof()
{
    std::string   filename = conf.replay.conf_path + "/" + conf.prisma.tof_file;
    std::ifstream tof_file(filename);
    if (!tof_file.is_open())
        std::cout << "Could not open tof offset file: " + filename << std::endl;
    else
    {
        // std::cout << "Reading tof file: " << filename << std::endl;
        std::string line;
        while (std::getline(tof_file, line))
        {
            std::stringstream iss(line);
            int               colNr;
            double            tof;
            iss >> colNr >> tof;
            prismaSolver.SetTofAlign(false, colNr, tof);
        }
    }
    tof_file.close();

    if (conf.prisma.enable_reftreeFill)
    {
        std::string   filename_ref = conf.replay.conf_path + "/" + conf.prisma.tof_ref_file;
        std::ifstream tof_file_ref(filename_ref);
        if (!tof_file_ref.is_open())
            std::cout << "Could not open tof offset file: " + filename_ref << std::endl;
        else
        {
            // std::cout << "Reading tof file: " << filename << std::endl;
            std::string line;
            while (std::getline(tof_file_ref, line))
            {
                std::stringstream iss(line);
                int               colNr;
                double            tof;
                iss >> colNr >> tof;
                prismaSolver.SetTofAlign(true, colNr, tof);
                prismaSolver.SetTofAlign(false, colNr, tof);
            }
        }
        tof_file_ref.close();
    }

    return;
}

void PrismaInputAnalysis::PrintTof(std::ostream &outstream = std::cout)
{
    for (int i = 0; i < 10; i++)
    {
        outstream << std::setprecision(3) << i << " " << prismaSolver.GetTofAlign(false, i)
                  << std::endl;
    }
}

void PrismaInputAnalysis::RecalLengthBrho(PrismaInput &Input)
{

    auto lengthrecal = lengthRecal.GetCalibration({1}, Input.length);
    if (lengthrecal.first)
    {

        // std::cout << std::setprecision(6) << "Recalibrating length: " << Input.length << " " <<
        // lengthrecal.second << std::endl;

        double alpha = lengthrecal.second / Input.length;
        Input.length = lengthrecal.second;

        // std::cout << std::setprecision(6) << "Radius: " << Input.radius  << " " <<
        // sqrt(1-pow(Input.beta,2))/sqrt(pow(1./alpha,2)-pow(Input.beta,2))*Input.radius  << " " <<
        // Input.beta << " " << alpha << std::endl;

        Input.radius = sqrt(1 - pow(Input.beta, 2)) /
                       sqrt(pow(1. / alpha, 2) - pow(Input.beta, 2)) * Input.radius;

        double new_beta = Input.beta * alpha;

        Input.beta  = new_beta;
        Input.gamma = 1 / sqrt(1 - Input.beta * Input.beta);
        Input.rbeta = Input.radius * Input.beta * Input.gamma;
        Input.brho  = Input.radius * conf.prisma.B_dipole / (UNITS::tesla * UNITS::m);

        // std::cout << std::setprecision(6) << "B " << conf.prisma.B_dipole / (UNITS::tesla) <<
        // std::endl;
    }
}

void PrismaInputAnalysis::RunSolver(bool ref)
{

    if (conf.prisma.enable_reftreeFill)
    {
        CalibrateRaw(ref);
        prismaSolver.InitGeometry(ref);
    }

    PrismaInput &Input = ref ? prismaInputRef : prismaInput;

    if (settings.verbosity_level > 15)
    {
        std::cout << "PrismaInputAnalysis::Solver::RunSolver - ref: " << ref << std::endl;
    }
    auto resetVariables = [&]() {
        Input.traj_ok        = false;
        Input.radius         = 0;
        Input.length         = 0;
        Input.a_over_q       = 0;
        Input.a_over_q_uncal = 0;
        Input.mass           = 0;
        Input.beta           = 0;
        Input.rbeta          = 0;
    };

    if (!Input.mcp_ok || Input.x_fp < 0)
    {
        resetVariables();
        return;
    }

    if (!conf.prisma.enable_mapping)
    {

        bool solved = false;
        solved      = prismaSolver.RunSolver(Input.mcp_x, Input.mcp_y, Input.x_fp, Input.y_fp);

        if (settings.verbosity_level > 3)
            std::cout << "Solved " << solved << std::endl;
        if (solved)
        {
            Input.traj_ok = true;

            if (conf.prisma.enable_tof_align)
                Input.tof = (Input.tof) +
                            prismaSolver.GetTofAlign(ref, static_cast<int>(prismaInput.col_nbr)) *
                                UNITS::ns;

            Input.radius = static_cast<float>(prismaSolver.GetRadius());
            Input.length = static_cast<float>(prismaSolver.GetTrajLength());

            Input.beta = (Input.length) / Input.tof / UNITS::PHYSICS::c_light;

            RecalLengthBrho(Input);

            Input.gamma = 1 / sqrt(1 - Input.beta * Input.beta);
            Input.rbeta = Input.radius * Input.beta * Input.gamma;
            Input.brho  = Input.radius * conf.prisma.B_dipole / (UNITS::tesla * UNITS::m);

            Input.a_over_q_uncal = CalculateAoverQ(Input.radius, Input.beta);
            Input.a_over_q       = Input.a_over_q_uncal;

            auto timeEval = aoqCorrTime.Eval({1}, Input.TS / (60. * 1e8));
            if (timeEval.second[0] != -1)
            {

                double temp_aoq = Input.a_over_q / timeEval.first * 3.;

                double prevLength = Input.length;
                Input.length      = CalculateLength(temp_aoq, Input.tof, Input.radius);

                auto xmcplength = xmcpLength.GetCalibration({1}, Input.length - prevLength);
                if (xmcplength.first)
                    Input.mcp_x = Input.mcp_x + xmcplength.second;

                if (settings.verbosity_level > 7)
                    std::cout << std::setprecision(7) << "cal " << xmcplength.first << " "
                              << xmcplength.second << std::endl;
                if (settings.verbosity_level > 7)
                    std::cout << std::setprecision(7) << "Time Eval Aoq " << Input.TS / (60. * 1e8)
                              << " " << timeEval.first << std::endl;
                if (settings.verbosity_level > 7)
                    std::cout << std::setprecision(7) << "New Aoq: " << temp_aoq << " "
                              << Input.a_over_q << std::endl;
                if (settings.verbosity_level > 7)
                    std::cout << std::setprecision(7) << "New Length: " << Input.length << " "
                              << prevLength << std::endl;
                if (settings.verbosity_level > 7)
                    std::cout << std::setprecision(7)
                              << "New MCP_X: " << Input.mcp_x + xmcplength.second << " "
                              << Input.mcp_x << std::endl;

                bool newSolved =
                    prismaSolver.RunSolver(Input.mcp_x, Input.mcp_y, Input.x_fp, Input.y_fp);

                Input.radius = static_cast<float>(prismaSolver.GetRadius());
                Input.length = static_cast<float>(prismaSolver.GetTrajLength());

                RecalLengthBrho(Input);

                Input.beta  = Input.length / Input.tof / UNITS::PHYSICS::c_light;
                Input.gamma = 1 / sqrt(1 - Input.beta * Input.beta);
                Input.rbeta = Input.radius * Input.beta * Input.gamma;
                Input.brho  = Input.radius * conf.prisma.B_dipole / (UNITS::tesla * UNITS::m);
                Input.a_over_q_uncal = CalculateAoverQ(Input.radius, Input.beta);
                Input.a_over_q       = Input.a_over_q_uncal;

                // std::cout << std::setprecision(6) << " B " << conf.prisma.B_dipole /
                // (UNITS::tesla) << std::endl;

                if (settings.verbosity_level > 7)
                    std::cout << std::setprecision(7) << "Second A/Q: " << Input.a_over_q
                              << std::endl;
            }

            auto xmcpEval = aoqCorrXmcp.Eval({0, 0}, Input.mcp_x / GetUnitsLength());

            if (xmcpEval.second[0] != -1)
            {

                if (settings.verbosity_level > 10)
                    std::cout << std::setprecision(5) << "AOQ_XMCP_CORR: ref " << xmcpEval.first
                              << " " << xmcpEval.second[0];

                double temp_aoq   = Input.a_over_q / xmcpEval.first * xmcpEval.second[0];
                double prevLength = Input.length;
                Input.length      = CalculateLength(temp_aoq, Input.tof, Input.radius);
                // if(settings.verbosity_level > 7) std::cout << std::setprecision(5) <<"TOF: " <<
                // Input.tof << " " << prevTOF << std::endl;
                if (settings.verbosity_level > 10)
                    std::cout << std::setprecision(5) << " " << Input.a_over_q << " " << temp_aoq
                              << std::endl;

                Input.beta           = Input.length / Input.tof / UNITS::PHYSICS::c_light;
                Input.gamma          = 1 / sqrt(1 - Input.beta * Input.beta);
                Input.rbeta          = Input.radius * Input.beta * Input.gamma;
                Input.a_over_q_uncal = CalculateAoverQ(Input.radius, Input.beta);
                Input.a_over_q       = Input.a_over_q_uncal;
            }
            else
            {
                if (settings.verbosity_level > 10)
                    std::cout << std::setprecision(5) << "NOT FOUND AOQ_XMCP_CORR" << std::endl;
            }

            auto ymcpEval = aoqCorrYmcp.Eval({0, 0}, Input.mcp_y / GetUnitsLength());

            if (ymcpEval.second[0] != -1)
            {

                if (settings.verbosity_level > 10)
                    std::cout << std::setprecision(5) << "AOQ_YMCP_CORR: ref " << ymcpEval.first
                              << " " << ymcpEval.second[0];

                double temp_aoq   = Input.a_over_q / ymcpEval.first * ymcpEval.second[0];
                double prevLength = Input.length;
                Input.length      = CalculateLength(temp_aoq, Input.tof, Input.radius);
                // if(settings.verbosity_level > 7) std::cout << std::setprecision(5) <<"TOF: " <<
                // Input.tof << " " << prevTOF << std::endl;

                if (settings.verbosity_level > 10)
                    std::cout << std::setprecision(5) << " " << Input.a_over_q << " " << temp_aoq
                              << std::endl;

                Input.beta           = Input.length / Input.tof / UNITS::PHYSICS::c_light;
                Input.gamma          = 1 / sqrt(1 - Input.beta * Input.beta);
                Input.rbeta          = Input.radius * Input.beta * Input.gamma;
                Input.a_over_q_uncal = CalculateAoverQ(Input.radius, Input.beta);
                Input.a_over_q       = Input.a_over_q_uncal;
            }
            else
            {
                if (settings.verbosity_level > 10)
                    std::cout << std::setprecision(5) << "NOT FOUND AOQ_YMCP_CORR" << std::endl;
            }

            if (settings.verbosity_level > 7)
                std::cout << std::setprecision(7) << "Aberrations A/Q: " << Input.a_over_q << " Z "
                          << static_cast<int>(prismaInput.z_nbr) << std::endl;

            auto xfpEval = aoqCorrXfp.Eval({0, 0}, Input.x_fp / GetUnitsLength());

            // auto xfpEval = aoqCorrXfp.Eval({0,0}, Input.x_fp/GetUnitsLength());
            if (xfpEval.second[0] != -1)
            {

                if (settings.verbosity_level > 10)
                    std::cout << std::setprecision(5) << "AOQ_XFP_CORR: ref " << xfpEval.first
                              << " " << xfpEval.second[0];

                double temp_aoq = Input.a_over_q / xfpEval.first * xfpEval.second[0];

                if (settings.verbosity_level > 10)
                    std::cout << std::setprecision(5) << " " << Input.a_over_q << " " << temp_aoq
                              << std::endl;

                double prevTOF = Input.tof;
                Input.tof      = CalculateTOF(temp_aoq, Input.length, Input.radius);

                Input.beta  = Input.length / Input.tof / UNITS::PHYSICS::c_light;
                Input.gamma = 1 / sqrt(1 - Input.beta * Input.beta);
                Input.rbeta = Input.radius * Input.beta * Input.gamma;

                Input.a_over_q_uncal = CalculateAoverQ(Input.radius, Input.beta);
                Input.a_over_q       = Input.a_over_q_uncal;
            }
            else
            {
                if (settings.verbosity_level > 10)
                    std::cout << std::setprecision(5) << "NOT FOUND AOQ_XFP_CORR" << std::endl;
            }
        }
        else
        {
            resetVariables();
            prismaSolver.ResetVelFP();
        }

        Input.mcp_theta  = static_cast<float>(prismaSolver.GetPosMcp().Theta());
        Input.mcp_phi    = static_cast<float>(prismaSolver.GetPosMcp().Phi());
        TVector3 globMCP = prismaSolver.GetPosMcp();
        globMCP.RotateY(-conf.prisma.angle);
        Input.theta = static_cast<float>(globMCP.Theta());
        Input.phi   = static_cast<float>(globMCP.Phi());

        // Input.col_nbr = static_cast<int>(floor(Input.x_fp*10));
        // colNr = Input.col_nbr;

        TVector3 velFP_corr = prismaSolver.GetVelFP();
        if (velFP_corr.Mag() > 0)
        {
            velFP_corr.RotateY(60 * UNITS::deg);
            double ThetaFP = static_cast<float>(velFP_corr.Theta());
            double PhiFP   = static_cast<float>(velFP_corr.Phi());
            Input.phi_fp   = atan(tan(ThetaFP) * sin(PhiFP));
            Input.theta_fp =
                PhiFP > 0 ? atan(tan(ThetaFP) * cos(PhiFP)) : -atan(tan(ThetaFP) * cos(PhiFP));

            Input.ic_de_a_traj  = Input.ic_de_a * (cos(Input.phi_fp) * cos(Input.theta_fp));
            Input.ic_de_ab_traj = Input.ic_de_ab * (cos(Input.phi_fp) * cos(Input.theta_fp));
        }

        if (settings.verbosity_level > 12)
            std::cout << std::setprecision(5) << "ref : " << ref << " a/q : " << Input.a_over_q
                      << " tof: " << Input.tof / UNITS::ns << " x_fp: " << Input.mcp_x << std::endl;
    }
    else
    {
        MappingPoint bestFit =
            prismaSolver.SearchMapping(Input.mcp_x, Input.mcp_y, Input.x_fp, Input.y_fp);
        bestFit.Print();
        Input.brho     = bestFit.GetDelta() * conf.prisma.refBrho;
        Input.radius   = bestFit.GetDelta() * conf.prisma.refBrho * conf.prisma.B_dipole;
        Input.length   = bestFit.GetPath();
        Input.theta_fp = bestFit.GetThetafp();
        Input.phi_fp   = bestFit.GetPhifp();
    }
}

void PrismaInputAnalysis::CheckGatesZ()
{

    // TOF gate (tof vs x_fp)
    if (TOFGate.size())
    {
        auto result =
            TOFGate.Id({0}, prismaInput.x_fp / GetUnitsLength(), prismaInput.tof / GetUnitsTime());
        if (result.first)
        {
            if (result.second.at(0) != 0)
            {
                prismaInput.tof_ok = true;
            }
            else
            {
                prismaInput.tof_ok = false;
            }
        }
        else
        {
            prismaInput.tof_ok = false;
        }
    }
    else
    {
        prismaInput.tof_ok = true;
    }

    // if(!prismaInput.side_ok || !prismaInput.ic_ok) return;

    prismaInput.ic_e_res_a  = prismaInput.ic_e - prismaInput.ic_de_a_traj;
    prismaInput.ic_e_res_ab = prismaInput.ic_e - prismaInput.ic_de_ab_traj;

    prismaInput.ic_de_a_corr     = prismaInput.ic_de_a_traj;
    prismaInput.ic_de_ab_corr    = prismaInput.ic_de_ab_traj;
    prismaInput.ic_e_res_a_corr  = prismaInput.ic_e_res_a;
    prismaInput.ic_e_res_ab_corr = prismaInput.ic_e_res_ab;
    prismaInput.ic_e_tot         = prismaInput.ic_e;

    if (conf.prisma.de_type == 3 || conf.prisma.de_type == 4 || conf.prisma.de_type == 5)
    {

        // Apply rotation of z_rot_angle for correction
        if (conf.prisma.z_rot_angle != 0)
        {
            prismaInput.ic_en1 = prismaInput.ic_de_a_traj * TMath::Cos(conf.prisma.z_rot_angle) +
                                 prismaInput.ic_e_res_a * TMath::Sin(conf.prisma.z_rot_angle);
            prismaInput.ic_en2 = -prismaInput.ic_de_a_traj * TMath::Sin(conf.prisma.z_rot_angle) +
                                 prismaInput.ic_e_res_a * TMath::Cos(conf.prisma.z_rot_angle);

            auto icdeacorryfp =
                icDEACorrYfp.Eval({0}, prismaInput.y_fp / GetUnitsLength()); /// UNITS::mm);
            auto icdeacorryfp_zero =
                icDEACorrYfp.Eval({0}, -0.02 / GetUnitsLength()); /// UNITS::mm);
            if (icdeacorryfp.second[0] == 0)
            {
                prismaInput.ic_en1_corr1 =
                    prismaInput.ic_en1 / icdeacorryfp.first * icdeacorryfp_zero.first;
            }
            else
                prismaInput.ic_en1_corr1 = prismaInput.ic_en1;

            auto icdeacorrxfp =
                icDEACorrXfp.Eval({0}, prismaInput.x_fp / GetUnitsLength());         /// UNITS::mm);
            auto icdeacorrxfp_zero = icDEACorrXfp.Eval({0}, 0.5 / GetUnitsLength()); /// UNITS::mm);
            if (icdeacorrxfp.second[0] == 0)
            {
                prismaInput.ic_en1_corr2 =
                    prismaInput.ic_en1_corr1 / icdeacorrxfp.first * icdeacorrxfp_zero.first;
            }
            else
                prismaInput.ic_en1_corr2 = prismaInput.ic_en1_corr1;

            prismaInput.ic_de_a_corr =
                prismaInput.ic_en1_corr2 * TMath::Cos(conf.prisma.z_rot_angle) -
                prismaInput.ic_en2 * TMath::Sin(conf.prisma.z_rot_angle);
            prismaInput.ic_e_res_a_corr =
                prismaInput.ic_en1_corr2 * TMath::Sin(conf.prisma.z_rot_angle) +
                prismaInput.ic_en2 * TMath::Cos(conf.prisma.z_rot_angle);
        }
        else
        {
            prismaInput.ic_de_a_corr    = prismaInput.ic_de_a_traj;
            prismaInput.ic_e_res_a_corr = prismaInput.ic_e - prismaInput.ic_de_a_traj;
        }
    }
    else if (conf.prisma.de_type == 6)
    {
        if (conf.prisma.z_rot_angle != 0)
        {
            prismaInput.ic_e_res_ab = prismaInput.ic_e - prismaInput.ic_de_ab_traj;

            prismaInput.ic_en1 = prismaInput.ic_de_ab_traj * TMath::Cos(conf.prisma.z_rot_angle) +
                                 prismaInput.ic_e_res_ab * TMath::Sin(conf.prisma.z_rot_angle);
            prismaInput.ic_en2 = -prismaInput.ic_de_ab_traj * TMath::Sin(conf.prisma.z_rot_angle) +
                                 prismaInput.ic_e_res_ab * TMath::Cos(conf.prisma.z_rot_angle);

            auto icdeabcorryfp =
                icDEABCorrYfp.Eval({0}, prismaInput.y_fp / GetUnitsLength()); /// UNITS::mm);
            auto icdeabcorryfp_zero =
                icDEABCorrYfp.Eval({0}, -0.02 / GetUnitsLength()); /// UNITS::mm);
            if (icdeabcorryfp.second[0] == 0)
            {
                prismaInput.ic_en1_corr1 =
                    prismaInput.ic_en1 / icdeabcorryfp.first * icdeabcorryfp_zero.first;
            }
            else
                prismaInput.ic_en1_corr1 = prismaInput.ic_en1;

            auto icdeabcorrxfp =
                icDEABCorrXfp.Eval({0}, prismaInput.x_fp / GetUnitsLength()); /// UNITS::mm);
            auto icdeabcorrxfp_zero =
                icDEABCorrXfp.Eval({0}, 0.5 / GetUnitsLength()); /// UNITS::mm);
            if (icdeabcorrxfp.second[0] == 0)
            {
                prismaInput.ic_en1_corr2 =
                    prismaInput.ic_en1_corr1 / icdeabcorrxfp.first * icdeabcorrxfp_zero.first;
            }
            else
                prismaInput.ic_en1_corr2 = prismaInput.ic_en1_corr1;

            prismaInput.ic_de_ab_corr =
                prismaInput.ic_en1_corr2 * TMath::Cos(conf.prisma.z_rot_angle) -
                prismaInput.ic_en2 * TMath::Sin(conf.prisma.z_rot_angle);
            prismaInput.ic_e_res_ab_corr =
                prismaInput.ic_en1_corr2 * TMath::Sin(conf.prisma.z_rot_angle) +
                prismaInput.ic_en2 * TMath::Cos(conf.prisma.z_rot_angle);
        }
        else
        {
            prismaInput.ic_de_a_corr     = prismaInput.ic_de_a;
            prismaInput.ic_e_res_ab_corr = prismaInput.ic_e - prismaInput.ic_de_ab_traj;
        }
    }

    ///////////////////////=============//////////////////////////
    ////////////// RANGE CALCULATIONS ////////////////////////////
    ///////////////==================////////////////////////////

    double etmp = 0.5 * conf.prisma.ic_depth * prismaInput.ic_mdE2 / prismaInput.ic_dE2;
    double tmp  = 0.5 * conf.prisma.ic_depth * prismaInput.ic_msdE / prismaInput.ic_sdE;
    double dEr;
    if (prismaInput.ic_e > 0.)
    {
        dEr = 0.2 * (prismaInput.ic_e_res_ab_corr + prismaInput.ic_de_ab_corr) /
              prismaInput.ic_de_ab_corr;
        prismaInput.ic_range =
            5. * (prismaInput.ic_range - 0.25 * (etmp + tmp - 2. * prismaInput.ic_range));
        prismaInput.ic_range =
            TMath::Power(dEr * (prismaInput.ic_range - 400. * UNITS::mm), 0.282830) * 400 *
            UNITS::mm;
    }
    else
        prismaInput.ic_range = -2000.;

    switch (conf.prisma.de_type)
    {
    case 0: { // IC_DE_AB vs IC_E
        auto result = zGates.Id({0}, prismaInput.ic_e, prismaInput.ic_de_ab_corr);
        if (result.first)
        {
            prismaInput.z_nbr = result.second[0];
        }
        else
        {
            prismaInput.z_nbr = 0;
        }
        break;
    }
    case 1: { // IC_DE_A vs IC_E
        auto result = zGates.Id({1}, prismaInput.ic_e, prismaInput.ic_de_a_corr);
        if (result.first)
        {
            prismaInput.z_nbr = result.second[0];
        }
        else
        {
            prismaInput.z_nbr = 0;
        }
        break;
    }
    case 2: // IC_E vs Range
        break;
    case 3: { // IC_DE_A vs IC_Eres
        auto result = zGates.Id({3}, prismaInput.ic_e_res_a_corr, prismaInput.ic_de_a_corr);
        if (result.first)
        {
            prismaInput.z_nbr = result.second[0];
            // std::cout << "Z: " << static_cast<unsigned int>(prismaInput.z_nbr) << std::endl;
        }
        else
        {
            prismaInput.z_nbr = 0;
        }
        break;
    }
    case 4: { // Continuous Zed
        Int_t initZ  = 0;
        auto  result = zGates.Id({3}, prismaInput.ic_e_res_a_corr, prismaInput.ic_de_a_corr);
        if (result.first)
        {
            initZ = result.second[0];
        }
        else
        {
            initZ = 0;
        }

        double icerescorr = prismaInput.ic_e_res_a_corr;
        double iceacorr   = prismaInput.ic_de_a_corr;

        prismaInput.zed = zLines.Identify(icerescorr, iceacorr, initZ);
        if (settings.verbosity_level > 20)
            std::cout << "Zfloat: " << static_cast<unsigned int>(prismaInput.zed) << std::endl;
        prismaInput.z_float = prismaInput.zed;
        prismaInput.z_nbr   = round(prismaInput.zed);

        break;
    }
    case 5: { // Zfloat corrected by mass: not working yet
        Int_t initZ  = 0;
        auto  result = zGates.Id({3}, prismaInput.ic_e_res_a_corr, prismaInput.ic_de_a_corr);
        if (result.first)
        {
            initZ = result.second[0];
        }
        else
        {
            initZ = 0;
        }

        double icerescorr = prismaInput.ic_e_res_a_corr;
        double iceacorr   = prismaInput.ic_de_a_corr;

        prismaInput.zed = zLines.Identify(icerescorr, iceacorr, initZ);
        if (settings.verbosity_level > 20)
            std::cout << "Zfloat: " << static_cast<unsigned int>(prismaInput.zed) << std::endl;
        prismaInput.z_nbr = round(prismaInput.zed);

        break;
    }

    case 6: {
        auto result = zGates.Id({6}, prismaInput.ic_e_res_ab_corr, prismaInput.ic_de_ab_corr);
        if (result.first)
        {
            prismaInput.z_nbr = result.second[0];
        }
        else
        {
            prismaInput.z_nbr = 0;
        }
        break;
    }

    default:
        throw std::runtime_error("Unknown de_type");
    }
}

void PrismaInputAnalysis::CheckGatesQ()
{

    // if(!prismaInput.side_ok || !prismaInput.ic_ok) return;

    // qGates.Print();
    // std::cout << " ic_e_tot " << prismaInput.ic_e_tot << " ic_e_nm " << prismaInput.ic_e_nm << "
    // ic_e_corr " << prismaInput.ic_e_corr << std::endl;
    auto icetot_cal            = icECorr.GetCalibration({5}, prismaInput.ic_e_tot);
    prismaInput.ic_e_tot       = icetot_cal.second;
    auto icdea_cal             = icECorr.GetCalibration({5}, prismaInput.ic_de_a_corr);
    prismaInput.ic_de_a_cal    = icdea_cal.second;
    auto iceres_a_cal          = icECorr.GetCalibration({5}, prismaInput.ic_e_res_a_corr);
    prismaInput.ic_e_res_a_cal = iceres_a_cal.second;

    // std::cout << " Then ic_e_tot " << prismaInput.ic_e_tot << " ic_e_nm " << prismaInput.ic_e_nm
    // << " ic_e_corr " << prismaInput.ic_e_corr << std::endl;

    // if(!prismaInput.tof_ok) return;

    // auto icecorrxfpEval = icECorrXfp.Eval({0}, prismaInput.x_fp);
    // if(icecorrxfpEval.second[0]!=-1){
    //     prismaInput.ic_e = prismaInput.ic_e / icecorrxfpEval.first * icecorrxfpEval.second[0];
    //     prismaInput.ic_e_tot = prismaInput.ic_e_tot / icecorrxfpEval.first *
    //     icecorrxfpEval.second[0];
    // }
    //
    // auto icecorryfpEval = icECorrYfp.Eval({0}, prismaInput.y_fp);
    // if(icecorryfpEval.second[0]!=-1){
    //     prismaInput.ic_e = prismaInput.ic_e / icecorryfpEval.first * icecorryfpEval.second[0];
    //     prismaInput.ic_e_tot = prismaInput.ic_e_tot / icecorryfpEval.first *
    //     icecorryfpEval.second[0];
    // }

    // auto icEnmBetaEval = icEnmBeta.Eval({0}, prismaInput.beta);
    ////auto icEnmBetaEvalZero = icEnmBeta.Eval({0}, 0.15);
    // double factor = icEnmBetaEval.first ;
    // //(icEnmBetaEval.first-icEnmBetaEvalZero.first)/2+icEnmBetaEvalZero.first;

    prismaInput.ic_e_nm = 0;

    // if(icEnmBetaEval.second[0]!=-1){
    //     //prismaInput.ic_e_nm =
    //     prismaInput.ic_de_a/(TMath::Cos(prismaInput.theta_fp)*TMath::Cos(prismaInput.phi_fp)) *
    //     icEnmBetaEval.first; prismaInput.ic_e_nm =
    //     prismaInput.ic_de_a_corr/(TMath::Cos(prismaInput.theta_fp)*TMath::Cos(prismaInput.phi_fp))
    //     * factor;
    // }

    auto icEnmBetaZedEval =
        icEnmZedBeta.Eval({static_cast<int>(round(conf.prisma.gasICA_thickness / UNITS::mm)),
                           static_cast<int>(prismaInput.z_nbr)},
                          prismaInput.beta);
    // if(settings.verbosity_level > 18) std::cout << std::setprecision(5) << "Gas thickness: " <<
    // round(conf.prisma.gasICA_thickness/UNITS::mm) << " z : " <<
    // static_cast<int>(prismaInput.z_nbr) << " beta: " << prismaInput.beta << "
    // icEnmBetaZedEval.second[0] " << icEnmBetaZedEval.second[0] << std::endl;
    if (icEnmBetaZedEval.second[0] != -1)
    {
        if (settings.verbosity_level > 25)
            std::cout << std::setprecision(5)
                      << "Gas thickness: " << round(conf.prisma.gasICA_thickness / UNITS::mm)
                      << "found! ic_e_nm: " << icEnmBetaZedEval.first << std::endl;
        prismaInput.ic_e_nm = icEnmBetaZedEval.first /
                              (TMath::Cos(prismaInput.theta_fp) * TMath::Cos(prismaInput.phi_fp));
        // prismaInput.ic_e_nm = icEnmBetaZedEval.first;
    }

    auto icenm_cal = icECorr.GetCalibration({0}, prismaInput.ic_de_a_cal);
    if (icenm_cal.first)
        prismaInput.ic_e_nm += icenm_cal.second;
    else
        prismaInput.ic_e_nm += 0.;

    if (settings.verbosity_level > 18)
        std::cout << std::setprecision(5) << "ic_e_nm: " << prismaInput.ic_e_nm << std::endl;

    prismaInput.ic_e_corr = prismaInput.ic_e_tot + prismaInput.ic_e_nm;

    auto icecorrxfpEval = icECorrXfp.Eval({0}, prismaInput.x_fp);
    if (icecorrxfpEval.second[0] != -1)
    {
        prismaInput.ic_e_corr =
            prismaInput.ic_e_corr / icecorrxfpEval.first * icecorrxfpEval.second[0];
    }

    auto icecorryfpEval = icECorrYfp.Eval({0}, prismaInput.y_fp);
    if (icecorryfpEval.second[0] != -1)
    {
        prismaInput.ic_e_corr =
            prismaInput.ic_e_corr / icecorryfpEval.first * icecorryfpEval.second[0];
    }

    auto chargecorr = icECorr.GetCalibration({1}, prismaInput.ic_e_nm);

    double corrCharge = 0;
    if (chargecorr.first)
        corrCharge = chargecorr.second;

    auto Energycorr = icECorr.GetCalibration({3}, prismaInput.ic_e_corr);

    if (Energycorr.first)
        prismaInput.ic_e_corr = Energycorr.second;

    prismaInput.ic_mass =
        prismaInput.ic_e_corr / (UNITS::PHYSICS::amu_c2 * (prismaInput.gamma - 1));

    prismaInput.charge = prismaInput.ic_mass / prismaInput.a_over_q; // + corrCharge;

    // std::cout << std::setprecision(5) << "ic_e_nm: " << prismaInput.ic_e_nm << " ic_e_corr " <<
    // prismaInput.ic_e_corr  << " gamma " << prismaInput.gamma  << " a_over_q " <<
    // prismaInput.a_over_q  << " " << UNITS::PHYSICS::amu_c2 << " Charge: " << prismaInput.charge
    // << " " << corrCharge << std::endl;

    auto chargecal = icECorr.GetCalibration({2}, prismaInput.charge);

    if (chargecal.first)
        prismaInput.charge_cal = chargecal.second;

    if (validQGate.size())
    {
        auto result = validQGate.Id({0}, prismaInput.beta, prismaInput.charge_cal);
        if (!result.first)
        {
            prismaInput.charge_cal = 0;
        }
    }

    if (prismaInput.z_nbr > 0)
    {
        switch (conf.prisma.charge_type)
        {

        case 0: { // IC_E vs RBeta
            auto result =
                qGates.Id({0, prismaInput.z_nbr}, prismaInput.rbeta, prismaInput.ic_e_corr);
            if (result.first)
            {
                prismaInput.q_nbr = result.second[0];
            }
            else
            {
                prismaInput.q_nbr = 0;
            }
            break;
        }
        case 1: { // IC_E/RBeta vs IC_E
            auto result = qGates.Id({1, prismaInput.z_nbr}, prismaInput.ic_e_corr,
                                    prismaInput.ic_e_corr /
                                        (prismaInput.radius * prismaInput.beta / GetUnitsLength()));
            if (result.first)
            {
                prismaInput.q_nbr = result.second[0];
            }
            else
            {
                prismaInput.q_nbr = 0;
            }
            break;
        }
        case 2: { // Beta vs X_FP
            auto result = qGates.Id({2, prismaInput.z_nbr}, prismaInput.x_fp / GetUnitsLength(),
                                    prismaInput.ic_e_corr);
            if (result.first)
            {
                prismaInput.q_nbr = result.second[0];
            }
            else
            {
                prismaInput.q_nbr = 0;
            }
            break;
        }
        case 3: { // Charge vs A/Q
            auto result =
                qGates.Id({3, prismaInput.z_nbr}, prismaInput.a_over_q, prismaInput.charge_cal);
            if (result.first)
            {
                prismaInput.q_nbr = result.second[0];
            }
            else
            {
                prismaInput.q_nbr = 0;
            }
            break;
        }
        case 4: { // From charge 1d calibration, float value
            prismaInput.q_float = prismaInput.charge_cal;
            prismaInput.q_nbr   = round(prismaInput.charge_cal);
            break;
        }
        case 5: { // From Lines grid, float value
            int    initQ  = round(prismaInput.charge_cal);
            double beta   = prismaInput.beta;
            double charge = prismaInput.charge_cal;
            if (prismaInput.beta > 0)
                prismaInput.q_float = qLines.Identify(beta, charge, initQ);
            if (settings.verbosity_level > 20)
                std::cout << "Qfloat: " << static_cast<unsigned int>(prismaInput.q_float)
                          << std::endl;
            prismaInput.q_nbr = round(prismaInput.q_float);

            break;
        }
        case 6: {

            break;
        }

        case 202: { // double check: means 0&2; good if beta vs x is contaminated and ic_e vs rbeta
                    // is not

            auto result =
                qGates.Id({0, prismaInput.z_nbr}, prismaInput.rbeta, prismaInput.ic_e_corr);

            if (result.first)
            {

                // LZ
                // in principle, the gates used to recheck the q can be overlapping, as anyways the
                // primary Q gates are not. so we need to get the list of gates where the point is
                // inside, and check if there is the one we are looking for in principle it could
                // have been done more easily just by putting another parameter to the function and
                // return a bool, but anyways...
                auto result2 =
                    qGates.AllId({2, prismaInput.z_nbr}, prismaInput.x_fp / GetUnitsLength(),
                                 prismaInput.ic_e_corr);

                if (result2.first)
                    for (int ii = 0; ii < result2.second.size(); ii++)
                        if (result2.second[ii][0] == result.second[0])
                            prismaInput.q_nbr = result2.second[ii][0];
                        else
                            prismaInput.q_nbr = 0;
                else
                    prismaInput.q_nbr = 0;
            }

            break;
        }

        case 220: { // double check: means 2&0; good if ic_e vs rbeta is contaminated and beta vs x
                    // is not

            auto result = qGates.Id({2, prismaInput.z_nbr}, prismaInput.x_fp / GetUnitsLength(),
                                    prismaInput.ic_e_corr);

            if (result.first)
            {

                auto result2 =
                    qGates.AllId({0, prismaInput.z_nbr}, prismaInput.rbeta, prismaInput.ic_e_corr);

                if (result2.first)
                    for (int ii = 0; ii < result2.second.size(); ii++)
                        if (result2.second[ii][0] == result.second[0])
                            prismaInput.q_nbr = result2.second[ii][0];
                        else
                            prismaInput.q_nbr = 0;
                else
                    prismaInput.q_nbr = 0;
            }

            break;
        }

        default:
            throw std::runtime_error("Unknown charge_type");
        }
    }
}

void PrismaInputAnalysis::CalibrateRaw(bool ref)
{

    PrismaInput &Input = ref ? prismaInputRef : prismaInput;

    // SIDES
    std::vector<TTreeReaderArray<UShort_t> *> ICSide_raw{
        prismaReader.Side_A_raw, prismaReader.Side_B_raw, prismaReader.Side_C_raw,
        prismaReader.Side_D_raw};

    // bool sidevalidEvent = true;
    // std::cout << "Check sides" << std::endl;
    // std::cout << ICSide_raw.size() << std::endl;
    // std::cout << prismaReader.Side_A_raw->At(0) << std::endl;
    // std::cout << prismaReader.Side_A_raw->GetSize() << std::endl;

    Input.side_ok = true;
    for (unsigned int i = 0; i < ICSide_raw.size(); i++)
    {
        for (unsigned int j = 0; j < ICSide_raw.at(i)->GetSize(); j++)
        {
            // std::cout << "Check sides " << i << " " << j << std::endl;
            bool sidesFired = icSidesCal.InsideThreshold({static_cast<int>(i), static_cast<int>(j)},
                                                         ICSide_raw.at(i)->At(j));
            if (sidesFired)
            {
                // sidevalidEvent = false;
                Input.side_ok = false;
            }
            else
            {
                continue;
            }
        }
    }

    // std::cout << "Check IC E" << std::endl;

    // IC E
    std::array<TTreeReaderArray<UShort_t> *, 4> icE_raw{
        prismaReader.IC_A_raw, prismaReader.IC_B_raw, prismaReader.IC_C_raw, prismaReader.IC_D_raw};

    std::array<double, 4> maxE{0, 0, 0, 0};

    Input.ic_a_numpads = 0;
    Input.ic_b_numpads = 0;
    Input.ic_c_numpads = 0;
    Input.ic_d_numpads = 0;

    std::array<double, 4> ic_dE{0, 0, 0, 0};

    bool icvalidEvent = false;

    Input.ic_range = 0.;
    Input.ic_mdE2  = 0.;
    Input.ic_dE2   = 0.;
    Input.ic_msdE  = 0.;
    Input.ic_sdE   = 0.;

    for (unsigned int i = 0; i < icE_raw.size(); i++)
    {
        for (unsigned int j = 0; j < icE_raw.at(i)->GetSize(); j++)
        {
            auto icEcal = icECal.GetCalibration({static_cast<int>(i), static_cast<int>(j)},
                                                icE_raw.at(i)->At(j));
            if (icEcal.first)
            {
                Input.ic_pads[4 * j + i] = icEcal.second;
                if (i == 0)
                {
                    Input.ic_a_numpads++;
                    if (icEcal.second > maxE.at(i))
                        Input.ic_col_a = j;
                    maxE.at(i) = icEcal.second;
                }
                if (i == 1)
                {
                    Input.ic_b_numpads++;
                    if (icEcal.second > maxE.at(i))
                        Input.ic_col_b = j;
                    maxE.at(i) = icEcal.second;
                }
                if (i == 2)
                {
                    Input.ic_c_numpads++;
                    if (icEcal.second > maxE.at(i))
                        Input.ic_col_c = j;
                    maxE.at(i) = icEcal.second;
                }
                if (i == 3)
                {
                    Input.ic_d_numpads++;
                    if (icEcal.second > maxE.at(i))
                        Input.ic_col_d = j;
                    maxE.at(i) = icEcal.second;
                }
                icvalidEvent = true;
            }
            else
            {
                Input.ic_pads[4 * j + i] = 0;
            }
            ic_dE[i] += Input.ic_pads[4 * j + i];
            // std::cout << "ic "<< i << " "<< j << " " << Input.ic_pads[4*j+i] << std::endl;
        }
        Input.ic_range +=
            (conf.prisma.ic_fp_distance + (i + 0.5) * conf.prisma.ic_depth) * ic_dE[i];
        Input.ic_mdE2 += (2 * i + 1) * ic_dE[i] * ic_dE[i];
        Input.ic_dE2 += ic_dE[i] * ic_dE[i];
        if (ic_dE[i] > 0)
        {
            Input.ic_msdE += (2 * i + 1) * sqrt(ic_dE[i]);
            Input.ic_sdE += sqrt(ic_dE[i]);
        }
        else
        {
            Input.ic_msdE += 0.;
            Input.ic_sdE += 0.;
        }
    }

    Input.ic_ok = icvalidEvent;

    Input.ic_de_a       = ic_dE[0];
    Input.ic_de_ab      = ic_dE[0] + ic_dE[1];
    Input.ic_de_a_traj  = ic_dE[0];
    Input.ic_de_ab_traj = ic_dE[0] + ic_dE[1];
    Input.ic_e          = ic_dE[0] + ic_dE[1] + ic_dE[2] + ic_dE[3];
    Input.ic_range      = Input.ic_range / Input.ic_e - conf.prisma.ic_fp_distance;

    // std::cout << "Check IC drift" << std::endl;

    // IC DRIFT
    std::array<TTreeReaderArray<UShort_t> *, 3> icDrift_raw{
        prismaReader.IC_A_Drift_raw, prismaReader.IC_B_Drift_raw, prismaReader.IC_C_Drift_raw};

    // std::cout << icDrift_raw.size() << std::endl;
    // std::cout << icDrift_raw.at(0)->GetSize() << std::endl;
    // std::cout << prismaReader.IC_A_Drift_raw->At(0) << std::endl;

    for (unsigned int i = 0; i < icDrift_raw.size(); i++)
    {
        for (unsigned int j = 0; j < icDrift_raw.at(i)->GetSize(); j++)
        {
            auto icDriftcal = icDriftCal.GetCalibration({static_cast<int>(i), static_cast<int>(j)},
                                                        icDrift_raw.at(i)->At(j));
            // std::cout << "Check drift " << i << " " << j << std::endl;
            if (icDriftcal.first)
            {
                switch (i)
                {
                case 0:
                    if (j == Input.ic_col_a)
                        Input.ic_drift_a = icDriftcal.second;
                    break;
                case 1:
                    if (j == Input.ic_col_b)
                        Input.ic_drift_b = icDriftcal.second;
                    break;
                case 2:
                    if (j == Input.ic_col_c)
                        Input.ic_drift_c = icDriftcal.second;
                    break;
                default:
                    break;
                }
                icDrift_raw.at(i)->At(j) = icDriftcal.second;
                // std::cout << "ic drift "<< i << " "<< j << " " << icDrift_raw.at(i)->At(j)<<
                // std::endl;
            }
            else
            {
                continue;
            }
        }
    }

    // std::cout << "Check PPAC" << std::endl;

    // PPAC
    bool ppacvalidEvent = false;

    for (int i = 0; i < 10; i++)
    {
        // auto tofcal = tofCal.GetCalibration({i}, prismaReader.TOF_raw->At(i));
        // if(tofcal.first){
        //     tempTof = (tofcal.second * conf.prisma.tofGain + conf.prisma.tofOffset)*UNITS::ns;
        //     //std::cout << "tof " << i << " " << Input.tof << std::endl;
        // } else {
        //     continue;
        // }

        if (!tofCal.InsideThreshold({i}, prismaReader.TOF_raw->At(i)))
        {
            continue;
        }

        if (!ppacThresh.InsideThreshold({2, i}, prismaReader.PPAC_Cathode_raw->At(i)))
        {
            continue;
        }

        bool ppacRightOk = false;
        bool ppacLeftOk  = false;

        if (ppacThresh.InsideThreshold({0, i}, prismaReader.PPAC_Xright_raw->At(i)))
            ppacRightOk = true;
        if (ppacThresh.InsideThreshold({1, i}, prismaReader.PPAC_Xleft_raw->At(i)))
            ppacLeftOk = true;

        if (ppacRightOk && ppacLeftOk)
        {
            auto ppacRmLcal = ppacPosCal.GetCalibration(
                {0, i}, prismaReader.PPAC_Xright_raw->At(i) - prismaReader.PPAC_Xleft_raw->At(i));
            if (ppacRmLcal.first)
            {
                Input.x_fp      = ppacRmLcal.second * UNITS::mm;
                Input.x_fp_type = 0;
            }
        }
        else if (ppacRightOk && !ppacLeftOk)
        {
            auto ppacRmCcal = ppacPosCal.GetCalibration(
                {1, i}, prismaReader.PPAC_Xright_raw->At(i) - prismaReader.PPAC_Cathode_raw->At(i));
            if (ppacRmCcal.first)
            {
                Input.x_fp      = ppacRmCcal.second * UNITS::mm;
                Input.x_fp_type = 1;
            }
        }
        else if (ppacLeftOk && !ppacRightOk)
        {
            auto ppacCmLcal = ppacPosCal.GetCalibration(
                {2, i}, prismaReader.PPAC_Cathode_raw->At(i) - prismaReader.PPAC_Xleft_raw->At(i));
            if (ppacCmLcal.first)
            {
                Input.x_fp      = ppacCmLcal.second * UNITS::mm;
                Input.x_fp_type = 2;
            }
        }
        else
            continue;

        auto ppacgate =
            ppacGates.Id({i}, prismaReader.PPAC_Cathode_raw->At(i),
                         prismaReader.PPAC_Xright_raw->At(i) + prismaReader.PPAC_Xleft_raw->At(i));
        if (ppacgate.first && ppacgate.second[0] != 1)
            continue;

        if (ppacvalidEvent)
        {
            // there are two valid events
            ppacvalidEvent = false;
            if (settings.verbosity_level > 1)
                std::cout << "Two valid events " << Input.col_nbr << " " << i << std::endl;
            break;
        }
        else
        {
            ppacvalidEvent = true;
            Input.col_nbr  = i;
            // std::cout << "One valid event " << i << std::endl;
        }
    }

    Input.ppac_ok = ppacvalidEvent;

    if (ppacvalidEvent)
    {
        int j = Input.col_nbr;
        if (j < prismaReader.TOF_raw->GetSize())
        {
            auto tofcal = tofCal.GetCalibration({j}, prismaReader.TOF_raw->At(j));
            if (tofcal.first)
            {
                // Input.tof = (tofcal.second * conf.prisma.tofGain)*UNITS::ns +
                // conf.prisma.tofOffset;
                Input.tof = tofcal.second * UNITS::ns;
                // std::cout << "tof " << i << " " << Input.tof << std::endl;
            }
        }
        else
        {
            // Handle the error or skip this part
            std::cerr << "Warning: Attempted to access TOF_raw out of bounds with index " << j
                      << std::endl;
        }
    }

    // std::cout << "PPAC " << Input.col_nbr << " " << Input.x_fp << " r " <<
    // prismaReader.PPAC_Xright_raw->At(Input.col_nbr) << " l " <<
    // prismaReader.PPAC_Xleft_raw->At(Input.col_nbr) << std::endl;

    if (ppacvalidEvent)
    {
        int ppacYOk = 0;
        for (unsigned int i = 0; i < prismaReader.PPAC_Y_raw->GetSize(); i++)
        {
            if (ppacThresh.InsideThreshold({3, static_cast<int>(i)},
                                           prismaReader.PPAC_Y_raw->At(i)))
            {
                ppacYOk++;
            }
        }
        if (ppacYOk == 2)
        {
            auto ppacUmDcal =
                ppacPosCal.GetCalibration({3, Input.col_nbr}, prismaReader.PPAC_Y_raw->At(0) -
                                                                  prismaReader.PPAC_Y_raw->At(1));
            if (ppacUmDcal.first)
            {
                Input.y_fp = ppacUmDcal.second * UNITS::mm;
            }
        }
    }

    // if(ppacvalidEvent && Input.tof > 0) Input.tof_ok = true;

    // std::cout << "Check MCP" << std::endl;

    auto mcpgate = mcpGate.Id({0}, prismaReader.MCP_raw->At(0), prismaReader.MCP_raw->At(1));

    if (mcpgate.first && mcpgate.second[0] == 1)
        Input.mcp_ok = true;
    else
        Input.mcp_ok = false;

    // if(!Input.mcp_ok) return;

    auto mcpRotcal = mcpRotCal.GetMatrixCalibration(
        {0}, std::make_pair(prismaReader.MCP_raw->At(0), prismaReader.MCP_raw->At(1)));

    if (mcpRotcal.first)
    {
        auto mcpxcal = mcpCal.GetCalibration({0}, mcpRotcal.second.first);
        auto mcpycal = mcpCal.GetCalibration({1}, mcpRotcal.second.second);
        // auto mcpqcal = mcpCal.GetCalibration({2}, prismaReader.MCP_raw->At(2));
        if (mcpxcal.first)
            Input.mcp_x = (mcpxcal.second * UNITS::mm + conf.prisma.xShift) * conf.prisma.xScale;
        if (mcpycal.first)
            Input.mcp_y = (mcpycal.second * UNITS::mm + conf.prisma.yShift) * conf.prisma.yScale;
    }

    // std::cout << "Check Monitors" << std::endl;

    // Monitors
    auto mon0cal = monitorCal.GetCalibration({0}, prismaReader.Monitors_raw->At(0));
    if (mon0cal.first)
        Input.monitor_0 = mon0cal.second;
    auto mon1cal = monitorCal.GetCalibration({1}, prismaReader.Monitors_raw->At(1));
    if (mon1cal.first)
        Input.monitor_1 = mon1cal.second;
}

void PrismaInputAnalysis::CalibrateAna()
{
    // Aberration correction
    std::array<int, 2> ids{static_cast<int>(prismaInput.z_nbr),
                           static_cast<int>(prismaInput.q_nbr)};

    auto xmcpEval = aoqCorrXmcp.Eval(ids, prismaInput.mcp_x / GetUnitsLength());
    if (xmcpEval.second[0] > 0)
    {
        prismaInput.a_over_q =
            prismaInput.a_over_q -
            (xmcpEval.first - static_cast<double>(xmcpEval.second[0]) / prismaInput.q_nbr);
    }

    auto ymcpEval = aoqCorrYmcp.Eval(ids, prismaInput.mcp_y / GetUnitsLength());
    if (ymcpEval.second[0] > 0)
        prismaInput.a_over_q =
            prismaInput.a_over_q -
            (ymcpEval.first - static_cast<double>(ymcpEval.second[0]) / prismaInput.q_nbr);

    auto xfpEval = aoqCorrXfp.Eval(ids, prismaInput.x_fp / GetUnitsLength());
    if (xfpEval.second[0] > 0)
        prismaInput.a_over_q =
            prismaInput.a_over_q -
            (xfpEval.first - static_cast<double>(xfpEval.second[0]) / prismaInput.q_nbr);

    // Mass calibration
    auto aoqcal          = aoqCal.GetCalibration({prismaInput.z_nbr}, prismaInput.a_over_q);
    prismaInput.a_over_q = aoqcal.second;

    auto qeff = chargeCal.GetCalibration(ids, prismaInput.q_nbr);

    if (conf.prisma.charge_type < 2 || conf.prisma.charge_type >= 100)
        prismaInput.charge =
            prismaInput.ic_e_corr /
            (UNITS::PHYSICS::amu_c2 * (prismaInput.gamma - 1) * prismaInput.a_over_q);

    if ((aoqcal.first || qeff.first) &&
        (conf.prisma.charge_type < 4 || conf.prisma.charge_type >= 100))
    {
        prismaInput.mass = prismaInput.a_over_q * qeff.second;
    }
    else if ((aoqcal.first || qeff.first) &&
             (conf.prisma.charge_type >= 4 && conf.prisma.charge_type < 100))
    {
        prismaInput.mass = prismaInput.a_over_q * prismaInput.q_nbr;
    }
    else
    {
        prismaInput.mass = 0;
    }

    switch (conf.prisma.mass_type)
    {
    case 0:
        prismaInput.a_nbr = std::round(prismaInput.mass);
        break;

    case 1:
        auto result = aGates.Id({1, prismaInput.z_nbr}, prismaInput.mass,
                                prismaInput.x_fp / GetUnitsLength());
        if (result.first)
            prismaInput.a_nbr = result.second[0];
        else
            prismaInput.a_nbr = 0;

        break;
    }

    if (zMassLines.size() && conf.prisma.de_type == 5 && prismaInput.mass > 0)
    {
        prismaInput.z_float =
            zMassLines.Identify(prismaInput.mass, prismaInput.zed, prismaInput.z_nbr, 1);
        prismaInput.z_nbr = round(prismaInput.z_float);
    }

    if (prismaInput.a_nbr > 0 && prismaInput.z_nbr > 0)
    {
        ReactionFragment::FragmentSettings ion{prismaInput.a_nbr, prismaInput.z_nbr};

        auto found = fragments.find(ion);
        if (found == fragments.end())
            try
            {
                fragments.emplace(
                    ion, ReactionFragment{ion, conf.replay.conf_path + "/NuclearData/nutab12"});
                found = fragments.find(ion);
            }
            catch (const std::exception &e)
            {
                return;
            }

        TVector3 beta3d{0, 0, prismaInput.beta};
        found->second.Set_Beta(beta3d);
        prismaInput.energy_m = found->second.Get_Ek();
    }
}

//====================================================================================================================

//================= Functions for Prisma Solver
//======================================================================

PrismaInputAnalysis::PrismaSolver::PrismaSolver()
    : confPrisma(ConfManager::getInstance().prisma), trajFile("traj.log")
{
    // debug = true;
    InitGeometry();
    if (confPrisma.enable_mapping)
        readMapping();
}

void PrismaInputAnalysis::PrismaSolver::InitGeometry(bool ref)
{

    TVector3 dipInRightTemp, dipInLeftTemp, dipOutRightTemp, dipOutLeftTemp, PPACRightTemp,
        PPACLeftTemp;

    if (!ref)
    {
        posDipoleInZero   = TVector3(0., 0., confPrisma.target_dipole_distance);
        normDipoleInPlane = TVector3(0., 0., 1.);
        normDipoleInPlane.RotateY(confPrisma.in_dipole_angle);
        double dipoleBendingAngle = 60 * UNITS::deg;
        beamAfterDipole           = TVector3(0., 0., 1.);
        beamAfterDipole.RotateY(-dipoleBendingAngle);
        posDipoleOutZero =
            posDipoleInZero +
            confPrisma.dipole_radius *
                TVector3(-sin(30. * UNITS::deg), 0, cos(30. * UNITS::deg)); // equilateral triangle
        normDipoleOutPlane = TVector3(0., 0., 1.);
        normDipoleOutPlane.RotateY(confPrisma.out_dipole_angle);
        lineOutDipole    = -normDipoleOutPlane.Cross(TVector3(0, 1, 0));
        TVector3 fpShift = TVector3(confPrisma.FPShiftX, confPrisma.FPShiftY, confPrisma.FPShiftZ);
        fpShift.RotateY(-dipoleBendingAngle);
        posPPACZero = posDipoleOutZero + confPrisma.dipole_fp_distance * beamAfterDipole + fpShift;
        normPPACPlane = beamAfterDipole;
        coordPPACZero =
            posPPACZero + beamAfterDipole.Cross(TVector3(0, -1, 0)) * confPrisma.fp_half_length;
        B_dipole     = confPrisma.B_dipole;
        B_quadrupole = confPrisma.B_quadrupole;
        B_ratio      = B_dipole / B_quadrupole;
        posTarget =
            TVector3(confPrisma.targetShiftX, confPrisma.targetShiftY, confPrisma.targetShiftZ);
        posMCPZero          = TVector3(0, 0, confPrisma.target_mcp_distance);
        mcpAngle            = confPrisma.mcp_angle;
        TargetMCPDist       = confPrisma.target_mcp_distance;
        TargetQuadDist      = confPrisma.target_quad_distance;
        geomRadius          = confPrisma.geom_radius;
        quadLength          = confPrisma.quad_length;
        quadRadius          = confPrisma.quad_radius;
        kConstNoRadius      = 1.0 / sqrt((B_ratio * quadRadius));
        argConstRefNoRadius = kConstNoRadius * quadLength;
        posQuadOutZero      = TVector3(0, 0, TargetQuadDist + quadLength);
        TargetDipDist       = confPrisma.target_dipole_distance;
        dipoleHeight        = confPrisma.dipole_height;
        dipoleRadius        = confPrisma.dipole_radius;
        DipFPDist           = confPrisma.dipole_fp_distance;
        auto dipInRightTemp = IntersectIonPlane(TVector3(-0.4, 0, 0), TVector3(0, 0, 1),
                                                posDipoleInZero, normDipoleInPlane);
        auto dipInLeftTemp  = IntersectIonPlane(TVector3(+0.4, 0, 0), TVector3(0, 0, 1),
                                                posDipoleInZero, normDipoleInPlane);
        auto dipOutRightTemp =
            IntersectIonPlane(posPPACZero + 0.4 * beamAfterDipole.Cross(TVector3(0, 1, 0)),
                              beamAfterDipole, posDipoleOutZero, normDipoleOutPlane);
        auto dipOutLeftTemp =
            IntersectIonPlane(posPPACZero - 0.4 * beamAfterDipole.Cross(TVector3(0, 1, 0)),
                              beamAfterDipole, posDipoleOutZero, normDipoleOutPlane);
        auto PPACRightTemp = posPPACZero + 0.5 * beamAfterDipole.Cross(TVector3(0, 1, 0));
        auto PPACLeftTemp  = posPPACZero - 0.5 * beamAfterDipole.Cross(TVector3(0, 1, 0));
    }
    else
    {
        posDipoleInZero = TVector3(0., 0., 1600. * UNITS::mm); // confPrisma.target_dipole_distance
        normDipoleInPlane = TVector3(0., 0., 1.);
        normDipoleInPlane.RotateY(20. * UNITS::deg); //(confPrisma.in_dipole_angle);
        double dipoleBendingAngle = 60 * UNITS::deg;
        beamAfterDipole           = TVector3(0., 0., 1.);
        beamAfterDipole.RotateY(-dipoleBendingAngle);
        posDipoleOutZero =
            posDipoleInZero +
            1200 * UNITS::mm *
                TVector3(
                    -sin(30. * UNITS::deg), 0,
                    cos(30. * UNITS::deg)); // confPrisma.dipole_radius  --- equilateral triangle
        normDipoleOutPlane = TVector3(0., 0., 1.);
        normDipoleOutPlane.RotateY(125.5 * UNITS::deg); // confPrisma.out_dipole_angle
        lineOutDipole = -normDipoleOutPlane.Cross(TVector3(0, 1, 0));
        posPPACZero   = posDipoleOutZero + 3285. * UNITS::mm * beamAfterDipole +
                      TVector3(0, 0, 0); // confPrisma.dipole_fp_distance ;
        normPPACPlane = beamAfterDipole;
        coordPPACZero = posPPACZero + beamAfterDipole.Cross(TVector3(0, -1, 0)) * 500. *
                                          UNITS::mm; // confPrisma.fp_half_length
        B_dipole       = 0.869423 * UNITS::tesla;    // confPrisma.B_dipole
        B_quadrupole   = 0.800782 * UNITS::tesla;    // confPrisma.B_quadrupole
        B_ratio        = B_dipole / B_quadrupole; // confPrisma.B_dipole / confPrisma.B_quadrupole
        posTarget      = TVector3(0, 0, 0);
        posMCPZero     = TVector3(0, 0, 250 * UNITS::mm); // confPrisma.target_mcp_distance
        mcpAngle       = 135 * UNITS::deg;                // confPrisma.mcp_angle;
        TargetMCPDist  = 250 * UNITS::mm;                 // confPrisma.target_mcp_distance;
        TargetQuadDist = 462 * UNITS::mm; // 457*UNITS::mm; //confPrisma.target_quad_distance;
        geomRadius     = 150 * UNITS::mm; // confPrisma.geom_radius;
        quadLength     = 477 * UNITS::mm; // 500 * UNITS::mm; //confPrisma.quad_lGeometryRength;
        quadRadius     = 160 * UNITS::mm; // confPrisma.quad_radius;
        kConstNoRadius = 1.0 / sqrt((B_ratio * quadRadius));
        argConstRefNoRadius = kConstNoRadius * quadLength;
        posQuadOutZero      = TVector3(0, 0, TargetQuadDist + quadLength);
        TargetDipDist       = 1600 * UNITS::mm; // confPrisma.target_dipole_distance;
        dipoleHeight        = 200 * UNITS::mm;  // confPrisma.dipole_height;
        dipoleRadius        = 1200 * UNITS::mm; // confPrisma.dipole_radius;
        DipFPDist           = 3285 * UNITS::mm; // confPrisma.dipole_fp_distance;
        auto dipInRightTemp = IntersectIonPlane(TVector3(-0.4, 0, 0), TVector3(0, 0, 1),
                                                posDipoleInZero, normDipoleInPlane);
        auto dipInLeftTemp  = IntersectIonPlane(TVector3(+0.4, 0, 0), TVector3(0, 0, 1),
                                                posDipoleInZero, normDipoleInPlane);
        auto dipOutRightTemp =
            IntersectIonPlane(posPPACZero + 0.4 * beamAfterDipole.Cross(TVector3(0, 1, 0)),
                              beamAfterDipole, posDipoleOutZero, normDipoleOutPlane);
        auto dipOutLeftTemp =
            IntersectIonPlane(posPPACZero - 0.4 * beamAfterDipole.Cross(TVector3(0, 1, 0)),
                              beamAfterDipole, posDipoleOutZero, normDipoleOutPlane);

        auto PPACRightTemp = posPPACZero + 0.5 * beamAfterDipole.Cross(TVector3(0, 1, 0));
        auto PPACLeftTemp  = posPPACZero - 0.5 * beamAfterDipole.Cross(TVector3(0, 1, 0));
    }

    if (confPrisma.enable_trajPlot)
    {
        trajFile << "GEOMETRY" << std::endl;
        trajFile << "postarget " << posTarget.X() << " " << posTarget.Y() << " " << posTarget.Z()
                 << std::endl;
        trajFile << "posquadin " << posTarget.X() << " " << posTarget.Y() << " "
                 << posTarget.Z() + confPrisma.target_quad_distance << std::endl;
        trajFile << "posquadou " << posTarget.X() << " " << posTarget.Y() << " "
                 << posTarget.Z() + confPrisma.target_quad_distance + confPrisma.quad_length
                 << std::endl;
        trajFile << "posdipin  " << posDipoleInZero.X() << " " << posDipoleInZero.Y() << " "
                 << posDipoleInZero.Z() << std::endl;
        trajFile << "posdipout " << posDipoleOutZero.X() << " " << posDipoleOutZero.Y() << " "
                 << posDipoleOutZero.Z() << std::endl;
        trajFile << "posfpzero " << posPPACZero.X() << " " << posPPACZero.Y() << " "
                 << posPPACZero.Z() << std::endl;
        trajFile << "cordfpzer " << coordPPACZero.X() << " " << coordPPACZero.Y() << " "
                 << coordPPACZero.Z() << std::endl;
        trajFile << "dipintemp " << dipInRightTemp.X() << " " << dipInRightTemp.Y() << " "
                 << dipInRightTemp.Z() << std::endl;
        trajFile << "dipinleft " << dipInLeftTemp.X() << " " << dipInLeftTemp.Y() << " "
                 << dipInLeftTemp.Z() << std::endl;
        trajFile << "dipoutrig " << dipOutRightTemp.X() << " " << dipOutRightTemp.Y() << " "
                 << dipOutRightTemp.Z() << std::endl;
        trajFile << "dipoutlef " << dipOutLeftTemp.X() << " " << dipOutLeftTemp.Y() << " "
                 << dipOutLeftTemp.Z() << std::endl;
        trajFile << "fprighemp " << PPACRightTemp.X() << " " << PPACRightTemp.Y() << " "
                 << PPACRightTemp.Z() << std::endl;
        trajFile << "fpleftemp " << PPACLeftTemp.X() << " " << PPACLeftTemp.Y() << " "
                 << PPACLeftTemp.Z() << std::endl;
    }

    CalculateFocalPlaneDispersion();
}

TVector3 PrismaInputAnalysis::PrismaSolver::IntersectIonPlane(const TVector3 &ionPos,
                                                              const TVector3 &ionDir,
                                                              const TVector3 &planePos,
                                                              const TVector3 &planeNorm)
{
    TVector3 diff = ionPos - planePos;
    double   prod = diff.Dot(planeNorm);
    if (prod == 0)
    {
        std::cout << "No intersection" << std::endl;
        return {0, 0, 0};
    }
    double   t            = -prod / ionDir.Dot(planeNorm);
    TVector3 intersection = ionPos + t * ionDir;
    return intersection;
}

// Function to calculate the intersection points between a circle and a line in 2D
TVector2 PrismaInputAnalysis::PrismaSolver::IntersectCircleLine(const TVector2 &center,
                                                                double         &radius,
                                                                const TVector2 &point,
                                                                const TVector2 &direction)
{
    // Calculate the coefficients of the line equation (y = mx + b)
    double m = direction.Y() / direction.X();
    double b = point.Y() - m * point.X();

    // Calculate the coefficients for the quadratic equation (Ax^2 + Bx + C = 0)
    double A = 1 + m * m;
    double B = center.X() - m * (b - center.Y());
    double C = center.X() * center.X() + (b - center.Y()) * (b - center.Y()) - radius * radius;

    // Calculate the discriminant
    double discriminant = B * B - A * C;

    // Calculate the intersection points
    if (discriminant >= 0)
    {
        double x1 = (B + sqrt(discriminant)) / A;
        double y1 = m * x1 + b;
        // double x2 = (B - sqrt(discriminant)) / A; //DANIELE: unused variable
        // double y2 = m * x2 + b; //DANIELE: unused variable
        return {x1, y1};
    }
    else
    {
        // No intersection
        // std::cout << "No intersection 2D" << std::endl;
        // std::cerr << "inters Center: " << center.X() << " " << center.Y() << std::endl;
        // std::cerr << "inters Radius: " << radius << std::endl;
        // std::cerr << "inters Point: " << point.X() << " " << point.Y() << std::endl;
        // std::cerr << "inters Direction: " << direction.X() << " " << direction.Y() << std::endl;
        return {};
    }
}

bool PrismaInputAnalysis::PrismaSolver::GetQuadrupoleEntrance(double x_mcp, double y_mcp)
{
    //    if(debug) std::cout << "New event" << std::endl;
    posMCP = posMCPZero + TVector3(-x_mcp * sin(mcpAngle), y_mcp, x_mcp * cos(mcpAngle));

    posQuadIn = posTarget + (posMCP - posTarget) *
                                ((TargetQuadDist - posTarget.Z()) / (posMCP - posTarget).Z());

    // should not continue if quadrupole is not entered!!!
    if (sqrt(pow(posQuadIn.X(), 2) + pow(posQuadIn.Y(), 2)) > geomRadius)
        return false;

    velQuadIn = (posQuadIn - posTarget).Unit();
    return true;
}

bool PrismaInputAnalysis::PrismaSolver::CalculateQuadrupoleExit()
{

    // coefficients for quadrupole
    double tg_alpha = velQuadIn.X() / velQuadIn.Z();
    double tg_beta  = velQuadIn.Y() / velQuadIn.Z();
    double coefB    = posQuadIn.X();
    double coefD    = posQuadIn.Y();

    // getQuadrupoleExit
    double k_const =
        kConstNoRadius / sqrt(radius); // 1.0/sqrt((B_ratio * confPrisma.quad_radius * radius));

    double coefA = tg_alpha / k_const;
    double coefC = tg_beta / k_const;

    double argument = argConstRefNoRadius / sqrt(radius);

    double sinhx = sinh(argument);
    double coshx = cosh(argument);
    double sinx  = sin(argument);
    double cosx  = cos(argument);

    double x_exit = coefA * sinhx + coefB * coshx;
    double y_exit = coefC * sinx + coefD * cosx;

    posQuadOut = posQuadOutZero + TVector3(x_exit, y_exit, 0);

    double vel_x = coefA * k_const * coshx + coefB * k_const * sinhx;
    double vel_y = coefC * k_const * cosx - coefD * k_const * sinx;

    velQuadOut = TVector3(vel_x, vel_y, 1.).Unit();

    return true;
}

bool PrismaInputAnalysis::PrismaSolver::CalculateDipoleEntrance()
{

    posDipoleIn = IntersectIonPlane(posQuadOut, velQuadOut, posDipoleInZero, normDipoleInPlane);
    velDipoleIn = velQuadOut;
    if (fabs(posDipoleIn.Y()) > dipoleHeight / 2)
        return false; // DANIELE: bug here!!!!

    return true;
}

bool PrismaInputAnalysis::PrismaSolver::CalculateDipoleExit()
{
    centreTraj = posDipoleIn - radius * (TVector3(velDipoleIn.Z(), 0., -velDipoleIn.X()).Unit());
    TVector2 posDipoleOutHorizontal =
        IntersectCircleLine(TVector2(centreTraj.X(), centreTraj.Z()), radius,
                            TVector2(posDipoleOutZero.X(), posDipoleOutZero.Z()),
                            TVector2(lineOutDipole.X(), lineOutDipole.Z()));

    TVector3 tangentOut = TVector3(posDipoleOutHorizontal.X() - centreTraj.X(), 0,
                                   (posDipoleOutHorizontal.Y() - centreTraj.Z()))
                              .Cross(TVector3(0, 1, 0))
                              .Unit();

    double verticalSlope =
        velDipoleIn.Y() / sqrt(pow(velDipoleIn.X(), 2) + pow(velDipoleIn.Z(), 2));
    velDipoleOut = TVector3(tangentOut.X(), verticalSlope, tangentOut.Z()).Unit();

    deltaThetaTraj =
        acos((velDipoleIn.X() * velDipoleOut.X() + velDipoleIn.Z() * velDipoleOut.Z()) /
             (TVector3(velDipoleIn.X(), 0., velDipoleIn.Z()).Mag() *
              TVector3(velDipoleOut.X(), 0., velDipoleOut.Z()).Mag()));

    archLength = radius * deltaThetaTraj;
    posDipoleOut =
        TVector3(posDipoleOutHorizontal.X(), posDipoleIn.Y() + verticalSlope * archLength,
                 posDipoleOutHorizontal.Y());

    // if(fabs(posDipoleOut.Y())>dipoleHeight/2) return false;

    return true;
}
bool PrismaInputAnalysis::PrismaSolver::CalculateFocalPlane(bool dispersion = false)
{

    posFP = IntersectIonPlane(posDipoleOut, velDipoleOut, posPPACZero, normPPACPlane);
    velFP = velDipoleOut;

    auto fp_temp = (posFP - coordPPACZero);

    fp_temp.RotateY(-120. * TMath::DegToRad());
    coordinatesFP = TVector2(fp_temp.X(), posFP.Y());

    if (dispersion)
        return true;

    return true;
}

bool PrismaInputAnalysis::PrismaSolver::CalculateFocalPlaneDispersion()
{
    // assume particles entering the dipole along the optical axis

    double var_radius = 100. * UNITS::mm;

    // First assume curv_radius = guess_value - 10 cm
    Clear();
    posDipoleIn = posDipoleInZero;
    velDipoleIn = TVector3(0., 0., 1.);
    radius      = dipoleRadius - var_radius;
    if (!CalculateDipoleExit())
        throw std::runtime_error("Problem in calculating the focal plane dispersion (dipole)!");
    if (!CalculateFocalPlane(true))
        throw std::runtime_error(
            "Problem in calculating the focal plane dispersion (focal plane)!");
    TVector3 fp_decreased = TVector3(posFP.X(), 0, posFP.Z());

    // Then assume curv_radius = guess_value + 10 cm
    Clear();
    posDipoleIn = posDipoleInZero;
    velDipoleIn = TVector3(0., 0., 1.);
    radius      = dipoleRadius + var_radius;
    if (!CalculateDipoleExit())
        throw std::runtime_error("Problem in calculating the focal plane dispersion (dipole)!");
    if (!CalculateFocalPlane(true))
        throw std::runtime_error(
            "Problem in calculating the focal plane dispersion (focal plane)!");
    TVector3 fp_increased = TVector3(posFP.X(), 0, posFP.Z());

    // Finally obtains the required value!
    fp_dispersion = (2. * var_radius) / ((fp_decreased - fp_increased).Mag());

    if (debug)
        std::cout << "Focal plane dispersion: " << fp_dispersion << "  " << std::endl;
    return true;
}

double PrismaInputAnalysis::PrismaSolver::CalculateTrajLength()
{

    trajLength           = 0.;
    TVector3 velQuadTemp = TVector3(velQuadIn.X(), velQuadIn.Y(), velQuadIn.Z());
    TVector3 posQuadTemp = TVector3(posQuadIn.X(), posQuadIn.Y(), posQuadIn.Z());
    TVector3 posQuadPrev = TVector3(posQuadIn.X(), posQuadIn.Y(), posQuadIn.Z());
    double   tg_alpha    = velQuadTemp.X() / velQuadTemp.Z();
    double   tg_beta     = velQuadTemp.Y() / velQuadTemp.Z();
    double   coefB       = posQuadTemp.X();
    double   coefD       = posQuadTemp.Y();

    double        k_const       = kConstNoRadius / sqrt(radius);
    double        coefA         = tg_alpha / k_const;
    double        coefC         = tg_beta / k_const;
    double        quad_distance = 0;
    constexpr int n_points      = 5;
    for (int j = 1; j < n_points + 1; ++j)
    {
        double argument = argConstRefNoRadius / sqrt(radius) * j / n_points;
        double sinhx    = sinh(argument);
        double coshx    = cosh(argument);
        double sinx     = sin(argument);
        double cosx     = cos(argument);

        double x_next = coefA * sinhx + coefB * coshx;
        double y_next = coefC * sinx + coefD * cosx;

        posQuadTemp =
            TVector3(x_next, y_next,
                     confPrisma.target_quad_distance + confPrisma.quad_length * j * 1.0 / n_points);
        quad_distance += (posQuadTemp - posQuadPrev).Mag();
        posQuadPrev = posQuadTemp;
    }

    trajLength += (posQuadIn - posMCP).Mag();
    trajLength += quad_distance;
    trajLength += (posDipoleIn - posQuadOut).Mag();
    trajLength += sqrt(pow(archLength, 2) + pow(posDipoleOut.Y() - posDipoleIn.Y(), 2));
    trajLength += (posFP - posDipoleOut).Mag();

    // double oldLength = 0;
    //
    // oldLength += ( posQuadIn - posMCP ).Mag();
    // oldLength += ( posQuadOut - posQuadIn ).Mag();
    // oldLength += (posDipoleIn - posQuadOut ).Mag();
    // oldLength += archLength;
    // oldLength += ( posFP - posDipoleOut).Mag();
    //
    // trajLength = oldLength;
    return trajLength;
}

bool PrismaInputAnalysis::PrismaSolver::RunSolver(const double &mcp_x,
                                                  const double &mcp_y,
                                                  const double &fp_x,
                                                  const double &fp_y)
{

    // Coordinates:
    // X left of the MCP, facing prisma
    // Y upwards
    // Z prisma entrance
    Clear();
    if (fp_x == -1000 * UNITS::mm)
        return false;
    if (fp_y < -1e8 * UNITS::mm)
        return false;

    if (!GetQuadrupoleEntrance(mcp_x, mcp_y))
        return false;

    // Shifts away the starting point a little bit to prevent a spike at 1.2 m
    radius = confPrisma.dipole_radius + (rand() % 1001 - 500) * 0.01 * UNITS::mm;

    constexpr unsigned short max_iterations = 50;

    for (unsigned short iterations{0}; iterations < max_iterations; ++iterations)
    {
        CalculateQuadrupoleExit();
        CalculateDipoleEntrance();

        if (!CalculateDipoleExit())
            return false;
        CalculateFocalPlane();
        double difference = coordinatesFP.X() - fp_x;
        // double difference = sqrt(pow(coordinatesFP.X() - fp_x,2)+pow(coordinatesFP.Y() -
        // fp_y,2));

        if (debug)
            Dump();

        if (fabs(difference) < confPrisma.fp_tolerance)
        {
            if (confPrisma.enable_trajPlot)
                Dump();

            trajLength = CalculateTrajLength();

            return true;
        }
        // Iterative step, updates the new tentative radius
        // negative difference --> should decrease the radius
        // positive difference --> should increase the radius
        // std::cout << "Radius: " << radius << std::endl;
        radius = radius + difference * fp_dispersion;
        if (radius < 0)
        {
            return false;
        }
    }
    if (Settings::getInstance().verbosity_level > 3)
        std::cout << "Max iterations reached" << std::endl;
    return false;
}

void PrismaInputAnalysis::PrismaSolver::Dump()
{
    trajFile << "NEW" << std::endl;
    trajFile << "postarget  " << posTarget.X() << " " << posTarget.Y() << " " << posTarget.Z()
             << std::endl;
    trajFile << "posmcp  " << posMCP.X() << " " << posMCP.Y() << " " << posMCP.Z() << std::endl;
    trajFile << "posquadin  " << posQuadIn.X() << " " << posQuadIn.Y() << " " << posQuadIn.Z()
             << std::endl;
    trajFile << "posquadou  " << posQuadOut.X() << " " << posQuadOut.Y() << " " << posQuadOut.Z()
             << std::endl;
    trajFile << "posdipin   " << posDipoleIn.X() << " " << posDipoleIn.Y() << " " << posDipoleIn.Z()
             << std::endl;
    trajFile << "posdipout  " << posDipoleOut.X() << " " << posDipoleOut.Y() << " "
             << posDipoleOut.Z() << std::endl;
    trajFile << "posfp      " << posFP.X() << " " << posFP.Y() << " " << posFP.Z() << std::endl;
}

/*
bool PrismaInputAnalysis::PrismaSolver::RunSolver2(const double& mcp_x,
                                                   const double& mcp_y,
                                                   const double& fp_x,
                                                   const double& fp_y) {

    //Solver2 uses transport matrices

    Clear();

    posMCP = TVector3( -mcp_x*sin(confPrisma.mcp_angle), mcp_y,
confPrisma.target_mcp_distance+mcp_x*cos(confPrisma.mcp_angle)); TVector3 dir = posMCP - posTarget;
    if(fp_x == -1 || fp_y == -1) return false;

    TMatrixD inputVector(6,1);
    inputVector(0,0) = posTarget.X()/UNITS::cm;
    inputVector(1,0) = std::atan(dir.X()/dir.Z())/UNITS::mrad;
    inputVector(2,0) = posTarget.Y()/UNITS::cm;
    inputVector(3,0) = std::atan(dir.Y()/dir.Z())/UNITS::mrad;
    inputVector(4,0) = posTarget.Z()/UNITS::cm;

    double fp_coord = fp_x-500*UNITS::mm;

    //Guessing the 5th component based on the analytical formula
    inputVector(5,0) =
(fp_coord-(targetToFP(0,0)*posTarget.X()+targetToFP(0,1)*inputVector(1,0)))/targetToFP(0,5); deltaP
= inputVector(5,0);

    int cnt{0};
    float learningRate = 1e-3;
    //The loop is needed only for matrixes where the anlytical solution is not good enough

    //std::cout << "-----------------------------------------Solver2 start" << std::endl;
    do{
        auto posPredFp = targetToFP * inputVector;
        auto xdiff = posPredFp(0,0) - fp_coord;

        if(abs(xdiff)<confPrisma.fp_tolerance){
            deltaP = inputVector(5,0);
            trajectoryMatrices = inputVector(4,0) +
                                 confPrisma.target_dipole_distance +
                                 confPrisma.dipole_radius*UNITS::CONSTANTS::pi/3. +
                                 confPrisma.dipole_fp_distance +
                                 posPredFp(4,0);
            return true;
        }
        inputVector(5,0) -= xdiff*learningRate;
    }while(cnt++<100);
    return false;
}
*/

bool PrismaInputAnalysis::FillHistos()
{
    if (!DetectorAnalysis::FillHistos())
        return false;
    // PrismaInputHistograms& h = prismaInputHistograms;

    return true;
}

void PrismaInputAnalysis::Instantiate()
{
    DetectorAnalysis::Instantiate();
    TTreeReader &fReader{input.fReader};

    if (conf.detectorsPresent.Prisma)
    {
        if (input.treeToRead->FindLeaf("TSPrisma") == nullptr)
            throw std::runtime_error("PRISMA enabled in the config but not found in the tree");
        prismaReader.TSPrisma = new TTreeReaderValue<ULong64_t>(fReader, "TSPrisma");

        if (input.treeToRead->FindLeaf("Raw.MCP_raw") == nullptr &&
            input.treeToRead->FindLeaf("MCP_raw") == nullptr)
        {
            conf.prisma.raw  = false;
            conf.prisma.misc = false;
        }

        if (input.treeToRead->FindLeaf("Analyzed.MCP_X") != nullptr ||
            input.treeToRead->FindLeaf("Raw.MCP_raw") != nullptr)
        {

            // PrismaFilters have been used to build the tree
            if (conf.prisma.raw ||
                conf.prisma.analysis_stages == PrismaConf::AnalysisStage::RAWPROCESS ||
                conf.prisma.analysis_stages == PrismaConf::AnalysisStage::ALL)
            {
                if (input.treeToRead->FindLeaf("Raw.MCP_raw") == nullptr)
                    throw std::runtime_error(
                        "PRISMA raw required by chosen analysis step but not found in the tree");

                prismaReader.MCP_raw = new TTreeReaderArray<UShort_t>(fReader, "Raw.MCP_raw");
                prismaReader.PPAC_Xleft_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "Raw.PPAC_Xleft_raw");
                prismaReader.PPAC_Xright_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "Raw.PPAC_Xright_raw");
                prismaReader.PPAC_Cathode_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "Raw.PPAC_Cathode_raw");
                prismaReader.PPAC_Y_raw = new TTreeReaderArray<UShort_t>(fReader, "Raw.PPAC_Y_raw");
                prismaReader.TOF_raw    = new TTreeReaderArray<UShort_t>(fReader, "Raw.TOF_raw");
                prismaReader.IC_A_raw   = new TTreeReaderArray<UShort_t>(fReader, "Raw.IC_A_raw");
                prismaReader.IC_B_raw   = new TTreeReaderArray<UShort_t>(fReader, "Raw.IC_B_raw");
                prismaReader.IC_C_raw   = new TTreeReaderArray<UShort_t>(fReader, "Raw.IC_C_raw");
                prismaReader.IC_D_raw   = new TTreeReaderArray<UShort_t>(fReader, "Raw.IC_D_raw");
                prismaReader.IC_A_Drift_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "Raw.IC_A_Drift_raw");
                prismaReader.IC_B_Drift_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "Raw.IC_B_Drift_raw");
                prismaReader.IC_C_Drift_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "Raw.IC_C_Drift_raw");
                prismaReader.Side_A_raw = new TTreeReaderArray<UShort_t>(fReader, "Raw.Side_A_raw");
                prismaReader.Side_B_raw = new TTreeReaderArray<UShort_t>(fReader, "Raw.Side_B_raw");
                prismaReader.Side_C_raw = new TTreeReaderArray<UShort_t>(fReader, "Raw.Side_C_raw");
                prismaReader.Side_D_raw = new TTreeReaderArray<UShort_t>(fReader, "Raw.Side_D_raw");
                prismaReader.Monitors_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "Raw.Monitors_raw");
                prismaReader.TAC_LT_VTS = new TTreeReaderValue<UShort_t>(fReader, "Raw.TAC_LT_VTS");
            }
            if (conf.prisma.analysis_stages == PrismaConf::AnalysisStage::ANAPROCESS ||
                conf.prisma.analysis_stages == PrismaConf::AnalysisStage::ANA_HIGH ||
                conf.prisma.analysis_stages == PrismaConf::AnalysisStage::HIGHLEVEL)
            {
                if (input.treeToRead->FindLeaf("Analyzed.MCP_X") == nullptr)
                    throw std::runtime_error("PRISMA analyzed data required by chosen analysis "
                                             "step but not found in the tree");

                // TO BE REMOVED IN THE FUTURE
                prismaReader.MONITOR_1 =
                    new TTreeReaderValue<Float_t>(fReader, "Analyzed.MONITOR_1");
                prismaReader.MONITOR_2 =
                    new TTreeReaderValue<Float_t>(fReader, "Analyzed.MONITOR_2");
                prismaReader.MCP_X = new TTreeReaderValue<Float_t>(fReader, "Analyzed.MCP_X");
                prismaReader.MCP_Y = new TTreeReaderValue<Float_t>(fReader, "Analyzed.MCP_Y");
                prismaReader.MCP_Q = new TTreeReaderValue<Float_t>(fReader, "Analyzed.MCP_Q");
                prismaReader.MCP_Theta =
                    new TTreeReaderValue<Float_t>(fReader, "Analyzed.MCP_Theta");
                prismaReader.MCP_Phi  = new TTreeReaderValue<Float_t>(fReader, "Analyzed.MCP_Phi");
                prismaReader.X_FP     = new TTreeReaderValue<Float_t>(fReader, "Analyzed.X_FP");
                prismaReader.Y_FP     = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Y_FP");
                prismaReader.TOF      = new TTreeReaderValue<Float_t>(fReader, "Analyzed.TOF");
                prismaReader.IC_E     = new TTreeReaderValue<Float_t>(fReader, "Analyzed.IC_E");
                prismaReader.IC_DE_A  = new TTreeReaderValue<Float_t>(fReader, "Analyzed.IC_DE_A");
                prismaReader.IC_DE_AB = new TTreeReaderValue<Float_t>(fReader, "Analyzed.IC_DE_AB");
                prismaReader.IC_RANGE = new TTreeReaderValue<Float_t>(fReader, "Analyzed.IC_RANGE");
                prismaReader.IC_Drift_A =
                    new TTreeReaderValue<Float_t>(fReader, "Analyzed.IC_Drift_A");
                prismaReader.IC_Drift_B =
                    new TTreeReaderValue<Float_t>(fReader, "Analyzed.IC_Drift_B");
                prismaReader.IC_Drift_C =
                    new TTreeReaderValue<Float_t>(fReader, "Analyzed.IC_Drift_C");
                prismaReader.IC_a_numpads =
                    new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_a_numpads");
                prismaReader.IC_b_numpads =
                    new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_b_numpads");
                prismaReader.IC_c_numpads =
                    new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_c_numpads");
                prismaReader.IC_d_numpads =
                    new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_d_numpads");
                prismaReader.IC_col_a = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_col_a");
                prismaReader.IC_col_b = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_col_b");
                prismaReader.IC_col_c = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_col_c");
                prismaReader.IC_col_d = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.IC_col_d");
                prismaReader.IC_Pads  = new TTreeReaderArray<Float_t>(fReader, "Analyzed.IC_Pads");
                prismaReader.Theta    = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Theta");
                prismaReader.Phi      = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Phi");
                prismaReader.Beta     = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Beta");
                prismaReader.Length   = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Length");
                prismaReader.Radius   = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Radius");
                prismaReader.RBeta    = new TTreeReaderValue<Float_t>(fReader, "Analyzed.RBeta");
                prismaReader.A_over_q = new TTreeReaderValue<Float_t>(fReader, "Analyzed.A_over_q");
                if (input.treeToRead->FindLeaf("Analyzed.A_over_q_uncal") != nullptr)
                {
                    prismaReader.A_over_q_uncal =
                        new TTreeReaderValue<Float_t>(fReader, "Analyzed.A_over_q_uncal");
                }
                else
                {
                    prismaReader.A_over_q_uncal = prismaReader.A_over_q;
                }
                prismaReader.Mass   = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Mass");
                prismaReader.Qvalue = new TTreeReaderValue<Float_t>(fReader, "Analyzed.Qvalue");
                prismaReader.TAC_LT_LTS =
                    new TTreeReaderValue<Float_t>(fReader, "Analyzed.TAC_LT_LTS");
                prismaReader.Z_Nr    = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.Z_Nr");
                prismaReader.Q_Nr    = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.Q_Nr");
                prismaReader.A_Nr    = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.A_Nr");
                prismaReader.mcp_ok  = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.mcp_ok");
                prismaReader.tof_ok  = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.tof_ok");
                prismaReader.traj_ok = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.traj_ok");
                prismaReader.side_ok = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.side_ok");
                prismaReader.ic_ok   = new TTreeReaderValue<UChar_t>(fReader, "Analyzed.ic_ok");
            }
        }
        else
        { // femul has been used to build the tree
            prismaReader.TSPrisma = new TTreeReaderValue<ULong64_t>(fReader, "TSPrisma");
            if (conf.prisma.raw ||
                conf.prisma.analysis_stages == PrismaConf::AnalysisStage::RAWPROCESS ||
                conf.prisma.analysis_stages == PrismaConf::AnalysisStage::ALL)
            {
                if (input.treeToRead->FindLeaf("MCP_raw") == nullptr)
                    throw std::runtime_error(
                        "PRISMA raw required by chosen analysis step but not found in the tree");

                prismaReader.MCP_raw = new TTreeReaderArray<UShort_t>(fReader, "MCP_raw");
                prismaReader.PPAC_Xleft_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "PPAC_Xleft_raw");
                if (input.treeToRead->FindLeaf("PPAC_Xright_raw") != nullptr)
                    prismaReader.PPAC_Xright_raw =
                        new TTreeReaderArray<UShort_t>(fReader, "PPAC_Xright_raw");
                else
                    prismaReader.PPAC_Xright_raw =
                        new TTreeReaderArray<UShort_t>(fReader, "PPAC_XRight_raw");
                prismaReader.PPAC_Cathode_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "PPAC_Cathode_raw");
                prismaReader.PPAC_Y_raw = new TTreeReaderArray<UShort_t>(fReader, "PPAC_Y_raw");
                prismaReader.TOF_raw    = new TTreeReaderArray<UShort_t>(fReader, "TOF_raw");
                prismaReader.IC_A_raw   = new TTreeReaderArray<UShort_t>(fReader, "IC_A_raw");
                prismaReader.IC_B_raw   = new TTreeReaderArray<UShort_t>(fReader, "IC_B_raw");
                prismaReader.IC_C_raw   = new TTreeReaderArray<UShort_t>(fReader, "IC_C_raw");
                prismaReader.IC_D_raw   = new TTreeReaderArray<UShort_t>(fReader, "IC_D_raw");
                prismaReader.IC_A_Drift_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "IC_A_Drift_raw");
                prismaReader.IC_B_Drift_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "IC_B_Drift_raw");
                prismaReader.IC_C_Drift_raw =
                    new TTreeReaderArray<UShort_t>(fReader, "IC_C_Drift_raw");
                prismaReader.Side_A_raw   = new TTreeReaderArray<UShort_t>(fReader, "Side_A_raw");
                prismaReader.Side_B_raw   = new TTreeReaderArray<UShort_t>(fReader, "Side_B_raw");
                prismaReader.Side_C_raw   = new TTreeReaderArray<UShort_t>(fReader, "Side_C_raw");
                prismaReader.Side_D_raw   = new TTreeReaderArray<UShort_t>(fReader, "Side_D_raw");
                prismaReader.Monitors_raw = new TTreeReaderArray<UShort_t>(fReader, "Monitors_raw");
                prismaReader.TAC_LT_VTS   = new TTreeReaderValue<UShort_t>(fReader, "TAC_LT_VTS");
            }
            if (conf.prisma.analysis_stages == PrismaConf::AnalysisStage::ANAPROCESS ||
                conf.prisma.analysis_stages == PrismaConf::AnalysisStage::ANA_HIGH ||
                conf.prisma.analysis_stages == PrismaConf::AnalysisStage::HIGHLEVEL)
            {
                if (input.treeToRead->FindLeaf("MCP_X") == nullptr)
                    throw std::runtime_error("PRISMA analyzed data required by chosen analysis "
                                             "step but not found in the tree");

                // TO BE REMOVED IN THE FUTURE
                prismaReader.TSPrisma     = new TTreeReaderValue<ULong64_t>(fReader, "TSPrisma");
                prismaReader.MONITOR_1    = new TTreeReaderValue<Float_t>(fReader, "MONITOR_1");
                prismaReader.MONITOR_2    = new TTreeReaderValue<Float_t>(fReader, "MONITOR_2");
                prismaReader.MCP_X        = new TTreeReaderValue<Float_t>(fReader, "MCP_X");
                prismaReader.MCP_Y        = new TTreeReaderValue<Float_t>(fReader, "MCP_Y");
                prismaReader.MCP_Q        = new TTreeReaderValue<Float_t>(fReader, "MCP_Q");
                prismaReader.MCP_Theta    = new TTreeReaderValue<Float_t>(fReader, "MCP_Theta");
                prismaReader.MCP_Phi      = new TTreeReaderValue<Float_t>(fReader, "MCP_Phi");
                prismaReader.X_FP         = new TTreeReaderValue<Float_t>(fReader, "X_FP");
                prismaReader.Y_FP         = new TTreeReaderValue<Float_t>(fReader, "Y_FP");
                prismaReader.TOF          = new TTreeReaderValue<Float_t>(fReader, "TOF");
                prismaReader.IC_E         = new TTreeReaderValue<Float_t>(fReader, "IC_E");
                prismaReader.IC_DE_A      = new TTreeReaderValue<Float_t>(fReader, "IC_DE_A");
                prismaReader.IC_DE_AB     = new TTreeReaderValue<Float_t>(fReader, "IC_DE_AB");
                prismaReader.IC_RANGE     = new TTreeReaderValue<Float_t>(fReader, "IC_RANGE");
                prismaReader.IC_Drift_A   = new TTreeReaderValue<Float_t>(fReader, "IC_Drift_A");
                prismaReader.IC_Drift_B   = new TTreeReaderValue<Float_t>(fReader, "IC_Drift_B");
                prismaReader.IC_Drift_C   = new TTreeReaderValue<Float_t>(fReader, "IC_Drift_C");
                prismaReader.IC_a_numpads = new TTreeReaderValue<UChar_t>(fReader, "IC_a_numpads");
                prismaReader.IC_b_numpads = new TTreeReaderValue<UChar_t>(fReader, "IC_b_numpads");
                prismaReader.IC_c_numpads = new TTreeReaderValue<UChar_t>(fReader, "IC_c_numpads");
                prismaReader.IC_d_numpads = new TTreeReaderValue<UChar_t>(fReader, "IC_d_numpads");
                prismaReader.IC_col_a     = new TTreeReaderValue<UChar_t>(fReader, "IC_col_a");
                prismaReader.IC_col_b     = new TTreeReaderValue<UChar_t>(fReader, "IC_col_b");
                prismaReader.IC_col_c     = new TTreeReaderValue<UChar_t>(fReader, "IC_col_c");
                prismaReader.IC_col_d     = new TTreeReaderValue<UChar_t>(fReader, "IC_col_d");
                prismaReader.IC_Pads      = new TTreeReaderArray<Float_t>(fReader, "IC_Pads");
                prismaReader.Theta        = new TTreeReaderValue<Float_t>(fReader, "Theta");
                prismaReader.Phi          = new TTreeReaderValue<Float_t>(fReader, "Phi");
                prismaReader.Beta         = new TTreeReaderValue<Float_t>(fReader, "Beta");
                prismaReader.Length       = new TTreeReaderValue<Float_t>(fReader, "Length");
                prismaReader.Radius       = new TTreeReaderValue<Float_t>(fReader, "Radius");
                prismaReader.RBeta        = new TTreeReaderValue<Float_t>(fReader, "RBeta");
                prismaReader.A_over_q     = new TTreeReaderValue<Float_t>(fReader, "A_over_q");
                if (input.treeToRead->FindLeaf("A_over_q_uncal") != nullptr)
                {
                    prismaReader.A_over_q_uncal =
                        new TTreeReaderValue<Float_t>(fReader, "A_over_q_uncal");
                }
                else
                {
                    prismaReader.A_over_q_uncal = prismaReader.A_over_q;
                }
                prismaReader.Mass       = new TTreeReaderValue<Float_t>(fReader, "Mass");
                prismaReader.Qvalue     = new TTreeReaderValue<Float_t>(fReader, "Qvalue");
                prismaReader.TAC_LT_LTS = new TTreeReaderValue<Float_t>(fReader, "TAC_LT_LTS");
                prismaReader.Z_Nr       = new TTreeReaderValue<UChar_t>(fReader, "Z_Nr");
                prismaReader.Q_Nr       = new TTreeReaderValue<UChar_t>(fReader, "Q_Nr");
                prismaReader.A_Nr       = new TTreeReaderValue<UChar_t>(fReader, "A_Nr");
                prismaReader.tof_ok     = new TTreeReaderValue<UChar_t>(fReader, "tof_ok");
                prismaReader.mcp_ok     = new TTreeReaderValue<UChar_t>(fReader, "mcp_ok");
                prismaReader.side_ok    = new TTreeReaderValue<UChar_t>(fReader, "side_ok");
                prismaReader.traj_ok    = new TTreeReaderValue<UChar_t>(fReader, "traj_ok");
                prismaReader.ic_ok      = new TTreeReaderValue<UChar_t>(fReader, "ic_ok");
            }
        }
    }
}

void PrismaInputAnalysis::Finalize()
{
    DetectorAnalysis::Finalize();
}
