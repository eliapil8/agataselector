#pragma once

#include "CalibrationSelector.h"
#include "DetectorAnalysis.h"
#include "GateSelector.h"
#include "IDLine.h"
#include "Mapping.h"
#include "PrismaInput.h"

#include <TLine.h>

class PrismaInputAnalysis : virtual public DetectorAnalysis
{
  public:
    explicit PrismaInputAnalysis(Inputs, PrismaInput &);
    void Instantiate() override;
    bool AllocateHistos() override;
    void Analyze() override;
    bool FillHistos() override;
    void Finalize() override;

    void   ReadTof();
    void   PrintTof(std::ostream &);
    void   FillTofOffset();
    void   CalculateTofOffset();
    double GetUnitsLength() const { return units_length; }
    double GetUnitsAoQ() const { return units_aoq; }
    double GetUnitsTime() const { return units_time; }

  protected:
    // Container data
    PrismaInput &prismaInput;
    PrismaInput  prismaInputRef;
    ConfManager &conf;

    // HistogramStructure
    struct PrismaInputHistograms : public DetectorHistograms
    {
        TH2D *TofOffsetNew_vs_colNr{nullptr};
    } prismaInputHistograms;

  private:
    // Trajector reconstruction
    class PrismaSolver
    {

      private:
        PrismaConf &confPrisma;

        // Prisma geometry
        TVector3 posTarget{0, 0, 0};
        TVector3 posDipoleInZero{0, 0, 0};
        TVector3 normDipoleInPlane{0, 0, 0};
        TVector3 posDipoleOutZero{0, 0, 0};
        TVector3 normDipoleOutPlane{0, 0, 0};
        TVector3 lineOutDipole{0, 0, 0};
        TVector3 posPPACZero{0, 0, 0};
        TVector3 normPPACPlane{0, 0, 0};
        TVector3 coordPPACZero{0, 0, 0};
        double   fp_dispersion = 0;
        double   B_ratio       = 0;
        double   B_quadrupole  = 0;
        double   B_dipole      = 0;

        // Calculated trajectory

        std::ofstream trajFile;
        // MCP start detector
        TVector3 posMCP;
        TVector3 posMCPZero;
        double   mcpAngle{};
        double   TargetMCPDist{};
        // entrance of the quadrupole
        TVector3 posQuadIn;
        TVector3 velQuadIn;
        double   TargetQuadDist{};
        double   geomRadius{};
        double   quadLength{};
        double   quadRadius{};
        // exit of the quadrupole
        double   kConstNoRadius{};
        double   argConstRefNoRadius{};
        TVector3 posQuadOutZero;
        TVector3 posQuadOut;
        TVector3 velQuadOut;
        // dipole entrance
        TVector3 posDipoleIn;
        TVector3 velDipoleIn;
        double   TargetDipDist{};
        double   dipoleHeight{};
        double   dipoleRadius{};
        // dipole exit
        TVector3 centreTraj;
        double   radius{};
        TVector3 posDipoleOut;
        TVector3 velDipoleOut;
        TVector3 beamAfterDipole;
        // focal plane
        TVector3 posFP;
        TVector3 velFP;
        TVector2 coordinatesFP;
        double   DipFPDist{};
        double   deltaThetaTraj{};
        double   archLength{};
        double   trajLength{};

        // Matrix
        TMatrixD targetToMcp;
        TMatrixD mcpToQuadIn;
        TMatrixD quadInToQuadOut;
        TMatrixD quadOutToDipoleIn;
        TMatrixD dipoleInToDipoleOut;
        TMatrixD dipoleOutToFP;

        TMatrixD targetToFP;

        double              deltaP{};
        double              trajectoryMatrices{};
        std::vector<double> tofAlign{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        std::vector<double> tofAlignRef{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        MappingPrisma mappingPrisma;

        bool            GetQuadrupoleEntrance(double x_mcp, double y_mcp);
        bool            CalculateQuadrupoleExit();
        bool            CalculateDipoleEntrance();
        bool            CalculateDipoleExit();
        bool            CalculateFocalPlane(bool dispersion);
        static TVector3 IntersectIonPlane(const TVector3 &ionPos,
                                          const TVector3 &ionDir,
                                          const TVector3 &planePos,
                                          const TVector3 &planeNorm);
        static TVector2 IntersectCircleLine(const TVector2 &center,
                                            double         &radius,
                                            const TVector2 &point,
                                            const TVector2 &direction);
        double          CalculateTrajLength();

      public:
        PrismaSolver();
        void InitGeometry(bool ref = false);
        bool CalculateFocalPlaneDispersion();
        bool RunSolver(const double &mcp_x,
                       const double &mcp_y,
                       const double &fp_x,
                       const double &fp_y);
        // bool          RunSolver2(const double& mcp_x, const double& mcp_y,
        //                          const double& fp_x, const double& fp_y);
        bool          debug{false};
        inline double GetRadius() const { return radius; };
        inline double GetTrajLength() const { return trajLength; };
        inline double GetArchLength() const { return archLength; };
        inline double CalculateArchLength(double R) const { return R * deltaThetaTraj; };
        inline double GetTofAlign(bool ref = false, int i = -1) const
        {
            return ref ? tofAlignRef[i] : tofAlign[i];
        };
        inline void SetTofAlign(bool ref = false, int i = -1, double value = 0)
        {
            if (!ref)
                tofAlign[i] = value;
            else
                tofAlignRef[i] = value;
        };
        inline TVector3 GetPosMcp() const { return posMCP; };
        inline TVector3 GetVelFP() const { return velFP; };
        inline double   GetdeltaP() const { return deltaP; };
        inline double   GetBdipole() const { return B_dipole; };
        inline double   GetBquadrupole() const { return B_quadrupole; };
        inline double   GetlengthMatr() const { return trajectoryMatrices; };
        void            Dump();

        void readMapping()
        {
            mappingPrisma.ReadMappingFromFile(ConfManager::getInstance().replay.conf_path + "/" +
                                              confPrisma.mapping_file);
        }
        MappingPoint SearchMapping(const double &mcp_x,
                                   const double &mcp_y,
                                   const double &fp_x,
                                   const double &fp_y)
        {
            if (fp_x == -1000 && fp_y == -1000)
                return MappingPoint();
            auto p = MappingPoint(mcp_x, mcp_y, 0, 0, 0, 0, 0, 0);
            p.Print();
            int id = mappingPrisma.FindClosestPoint(p);
            std::cout << "ID: " << id << std::endl;
            return mappingPrisma.GetPoint(id);
        };
        void ResetVelFP() { velFP = TVector3(0, 0, 0); };
        void Clear()
        {
            posMCP          = TVector3(0, 0, 0);
            posQuadIn       = TVector3(0, 0, 0);
            velQuadIn       = TVector3(0, 0, 0);
            posQuadOut      = TVector3(0, 0, 0);
            velQuadOut      = TVector3(0, 0, 0);
            posDipoleIn     = TVector3(0, 0, 0);
            velDipoleIn     = TVector3(0, 0, 0);
            centreTraj      = TVector3(0, 0, 0);
            radius          = 0;
            posDipoleOut    = TVector3(0, 0, 0);
            velDipoleOut    = TVector3(0, 0, 0);
            beamAfterDipole = TVector3(0, 0, 0);
            posFP           = TVector3(0, 0, 0);
            velFP           = TVector3(0, 0, 0);
            coordinatesFP   = TVector2(0, 0);
            deltaThetaTraj  = 0;
            archLength      = 0;
            trajLength      = 0;
        };

    } prismaSolver;

    // GateSelector
    GateSelector<1, 1> TOFGate; // BGS
    GateSelector<2, 1> qGates;
    GateSelector<1, 1> zGates;
    GateSelector<2, 1> aGates; // only gated in Z
    GateSelector<1, 1> validQGate;

    GateSelector<1, 1> mcpGate;
    GateSelector<1, 1> ppacGates;

    // GateSelector<1,1> massGates;
    GateSelector<2, 1, TGraph> aoqCorrXmcp;
    GateSelector<2, 1, TGraph> aoqCorrYmcp;
    GateSelector<2, 1, TGraph> aoqCorrXfp;
    GateSelector<1, 1, TGraph> icDEACorrXfp;
    GateSelector<1, 1, TGraph> icDEACorrYfp;
    GateSelector<1, 1, TGraph> icDEABCorrXfp;
    GateSelector<1, 1, TGraph> icDEABCorrYfp;
    GateSelector<1, 1, TGraph> icECorrXfp;
    GateSelector<1, 1, TGraph> icECorrYfp;
    GateSelector<1, 1, TGraph> icEnmBeta;
    GateSelector<2, 1, TGraph> icEnmZedBeta;
    GateSelector<1, 1, TGraph> aoqCorrTime;

    IDGrid zLines;
    IDGrid qLines;
    IDGrid zMassLines;

    // Calibrations
    CalibrationSelector<1> tofCal;

    CalibrationSelector<2> ppacThresh; // 0 : right, 1 : left, 2 : cathode, 3 : y

    CalibrationSelector<2> ppacPosCal; // 0 : R-L, 1 : R-C, 2 : C-L, 3 : U-D

    CalibrationSelector<2> icECal;     // 0 : A, 1 : B, 2 : C, 3 : D
    CalibrationSelector<2> icSidesCal; // 0 : A, 1 : B, 2 : C, 3 : D
    CalibrationSelector<2> icDriftCal; // 0 : A, 1 : B, 2 : C

    CalibrationSelector<1> mcpCal;
    CalibrationSelector<1> mcpRotCal;

    CalibrationSelector<1> monitorCal;

    CalibrationSelector<1> aoqCal;
    CalibrationSelector<2> chargeCal;
    CalibrationSelector<1> icECorr;
    CalibrationSelector<1> xmcpLength;
    CalibrationSelector<1> lengthRecal;

    EnergyLoss *beamInTarget{nullptr};

    double units_length{UNITS::mm};
    double units_aoq{100.};
    double units_time{UNITS::ns};
    std::unordered_map<ReactionFragment::Ion, ReactionFragment, ReactionFragment::Ion::hash_fn>
        fragments;

    // Helper functions
    double CalculateAoverQ(const double &radius, const double &beta);
    double CalculateTOF(const double &a_over_q, const double &length, const double &radius);
    double CalculateLength(const double &a_over_q, const double &tof, const double &radius);
    void   RecalLengthBrho(PrismaInput &);

  public:
    struct PrismaReader : public Readers
    {
        TTreeReaderValue<ULong64_t> *TSPrisma{nullptr}; // 42

        TTreeReaderArray<UShort_t> *MCP_raw{nullptr};          // 43
        TTreeReaderArray<UShort_t> *PPAC_Xleft_raw{nullptr};   // 44
        TTreeReaderArray<UShort_t> *PPAC_Xright_raw{nullptr};  // 45
        TTreeReaderArray<UShort_t> *PPAC_Cathode_raw{nullptr}; // 46
        TTreeReaderArray<UShort_t> *PPAC_Y_raw{nullptr};       // 47
        TTreeReaderArray<UShort_t> *TOF_raw{nullptr};          // 48
        TTreeReaderArray<UShort_t> *IC_A_raw{nullptr};         // 49
        TTreeReaderArray<UShort_t> *IC_B_raw{nullptr};         // 50
        TTreeReaderArray<UShort_t> *IC_C_raw{nullptr};         // 51
        TTreeReaderArray<UShort_t> *IC_D_raw{nullptr};         // 52
        TTreeReaderArray<UShort_t> *IC_A_Drift_raw{nullptr};   // 53
        TTreeReaderArray<UShort_t> *IC_B_Drift_raw{nullptr};   // 53
        TTreeReaderArray<UShort_t> *IC_C_Drift_raw{nullptr};   // 54
        TTreeReaderArray<UShort_t> *Side_A_raw{nullptr};       // 55
        TTreeReaderArray<UShort_t> *Side_B_raw{nullptr};       // 56
        TTreeReaderArray<UShort_t> *Side_C_raw{nullptr};       // 57
        TTreeReaderArray<UShort_t> *Side_D_raw{nullptr};       // 58
        TTreeReaderArray<UShort_t> *Monitors_raw{nullptr};     // 59
        TTreeReaderValue<UShort_t> *TAC_LT_VTS{nullptr};       // 60

        // TO BE REMOVED IN THE FUTURE
        TTreeReaderValue<Float_t> *MCP_X{nullptr};
        TTreeReaderValue<Float_t> *MCP_Y{nullptr};
        TTreeReaderValue<Float_t> *X_FP{nullptr};
        TTreeReaderValue<Float_t> *Y_FP{nullptr};
        TTreeReaderValue<Float_t> *MONITOR_1{nullptr};
        TTreeReaderValue<Float_t> *MONITOR_2{nullptr};
        TTreeReaderValue<Float_t> *MCP_Q{nullptr};
        TTreeReaderValue<Float_t> *MCP_Theta{nullptr};
        TTreeReaderValue<Float_t> *MCP_Phi{nullptr};
        TTreeReaderValue<Float_t> *TOF{nullptr};
        TTreeReaderValue<Float_t> *IC_E{nullptr};
        TTreeReaderValue<Float_t> *IC_DE_A{nullptr};
        TTreeReaderValue<Float_t> *IC_DE_AB{nullptr};
        TTreeReaderValue<Float_t> *IC_RANGE{nullptr};
        TTreeReaderValue<Float_t> *IC_Drift_A{nullptr};
        TTreeReaderValue<Float_t> *IC_Drift_B{nullptr};
        TTreeReaderValue<Float_t> *IC_Drift_C{nullptr};
        TTreeReaderValue<UChar_t> *IC_a_numpads{nullptr};
        TTreeReaderValue<UChar_t> *IC_b_numpads{nullptr};
        TTreeReaderValue<UChar_t> *IC_c_numpads{nullptr};
        TTreeReaderValue<UChar_t> *IC_d_numpads{nullptr};
        TTreeReaderValue<UChar_t> *IC_col_a{nullptr};
        TTreeReaderValue<UChar_t> *IC_col_b{nullptr};
        TTreeReaderValue<UChar_t> *IC_col_c{nullptr};
        TTreeReaderValue<UChar_t> *IC_col_d{nullptr};
        TTreeReaderArray<Float_t> *IC_Pads{nullptr};
        TTreeReaderValue<Float_t> *Theta{nullptr};
        TTreeReaderValue<Float_t> *Phi{nullptr};
        TTreeReaderValue<Float_t> *Beta{nullptr};
        TTreeReaderValue<Float_t> *Length{nullptr};
        TTreeReaderValue<Float_t> *Radius{nullptr};
        TTreeReaderValue<Float_t> *RBeta{nullptr};
        TTreeReaderValue<Float_t> *A_over_q{nullptr};
        TTreeReaderValue<Float_t> *A_over_q_uncal{nullptr};
        TTreeReaderValue<Float_t> *Mass{nullptr};
        TTreeReaderValue<Float_t> *Qvalue{nullptr};
        TTreeReaderValue<Float_t> *TAC_LT_LTS{nullptr};
        TTreeReaderValue<UChar_t> *Z_Nr{nullptr};
        TTreeReaderValue<UChar_t> *Q_Nr{nullptr};
        TTreeReaderValue<UChar_t> *A_Nr{nullptr};
        TTreeReaderValue<UChar_t> *mcp_ok{nullptr};
        TTreeReaderValue<UChar_t> *tof_ok{nullptr};
        TTreeReaderValue<UChar_t> *traj_ok{nullptr};
        TTreeReaderValue<UChar_t> *side_ok{nullptr};
        TTreeReaderValue<UChar_t> *ic_ok{nullptr};
    } prismaReader;

  private:
    void RunSolver(bool ref = false);
    void CheckGatesZ();
    void CheckGatesQ();
    void CalibrateRaw(bool ref = false);
    void CalibrateAna();
    void LoadReactions();
    void FissionCMReconstruction();
};
