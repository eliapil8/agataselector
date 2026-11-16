#pragma once

// Eloss calculations from NPTOOL: https://gitlab.in2p3.fr/np/nptool

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

//   ROOT
#include "TAxis.h"
#include "TFile.h"
#include "TGraph.h"
#include "TSpline.h"
#include "TVector3.h"

// Use CLHEP System of unit and Physical Constant
#include "Units.h"

class EnergyLoss {
  public: //   Constructor
    EnergyLoss() = default;
    EnergyLoss(
        const std::string& path,   //   Path to table file
        const std::string& source, // Type of file : Geant4,Lise,SRIM
        int numberOfSlice,         //   Low number = Faster, High Number = more
                                   //   accurate / typical: 100 to 1000
        int LiseColumns
        = 0, //   Indicate which model to read in a lise File, set to 0
             //   (Default value) for a SRIM / Geant4 file
        int NumberOfMass
        = 1); //   Number of mass A of the nucleus (used only for Lise file)
    ~EnergyLoss() = default;

  private:                //   dE/dX, slice parameter
    int fNumberOfSlice{}; // Number Of Slice used to evaluate energy loss
    int fNumberOfMass{}; // Number of mass A of the nucleus (used only for Lise
                         // file)
    double              fMax{-10000};     // Max energy value of the table
    double              fMin{1e12};       // Min energy value of the tahble
    double              fDensity{-1};     // Min energy value of the tahble
    std::vector<double> fEnergy;          // Energy
    std::vector<double> fdEdX_Nuclear;    // Nuclear Stopping Power
    std::vector<double> fdEdX_Electronic; // Electronic Stopping Power
    std::vector<double> fdEdX_Total;      // Total Stopping Power

    // Energy loss at given energy
    TGraph              fInter;           // Graph use to perform interpolation
    TGraph              fInterNuclear;    // Graph use to perform interpolation
    TGraph              fInterElectronic; // Graph use to perform interpolation
    TSpline3 fSpline; // Spline 3rd order used to perfom the interpolation

  public:                                  //   General Function on dE/dX table
    double        Eval(double ener) const; // return the evaluated energy
    double        EvaluateNuclearLoss(double ener) const;
    double        EvaluateElectronicLoss(double ener) const;
    double        EvaluateTotalLoss(double ener) const;
    inline double GetDensity() const { return fDensity; }

  public
      : //   Function to Slow down particle or reconstruct their initial energy
    //   Calculate Energy of a particle after crossing material
    double Slow(double energy,          // Energy of the detected particle
                double targetThickness, // Target Thickness at 0 degree
                double angle) // Angle between particle and target norm
        const;
    double SlowWithDensity(
        double energy,        // Energy of the detected particle
        double targetDensity, // Target density (mg/cm2) at 0 degree
        double angle)         // Angle between particle and target norm
        const {
        return Slow(energy, targetDensity / fDensity, angle);
    }

    //   Calculate Energy Loss of a particle inside a material
    double EnergyLossCalculation(
        double energy,          // Energy of the detected particle in MeV
        double targetThickness, // Target Thickness at 0 degree in mm
        double angle)           // Angle between particle and target norm
        const;

    //   Evaluate Initial Energy of particle before crossing material knowing
    //   Angle, final Energy and Target Thickness.
    double EvaluateInitialEnergy(
        double energy,          // Energy of the detected particle
        double targetThickness, // Target Thickness at 0 degree
        double angle)           // Angle between particle and target norm
        const;
    double EvaluateInitialEnergyWithDensity(
        double energy,        // Energy of the detected particle
        double targetDensity, // Target density (mg/cm2) at 0 degree
        double angle)         // Angle between particle and target norm
        const {
        return EvaluateInitialEnergy(energy, targetDensity / fDensity, angle);
    }

    //   Evaluate Total Energy of particle from Energy loss in a given thickness
    double EvaluateEnergyFromDeltaE(
        double deltaE,           // Energy of the detected particle
        double targetThickness,  // Target Thickness at 0 degree
        double angle,            // Angle between particle and target norm
        double energyMin,        // Starting Energy
        double energyMax,        // Maximum Energy allowed
        double energyResolution, // Resolution at which function stop
        int    maxStep
        = 1000000) // Stop after MaxStep Whatever Precision is reached
        const;

    // Evaluate the thickness the particle has been through using the energy
    // loss and initial energy usefull for thickness measurement using particle
    // sources
    double EvaluateMaterialThickness(
        double initialEnergy, // Energy of the detected particle
        double finalEnergy, double thicknessLimit,
        double thicknessStep
        = 0.1 * UNITS::micrometer) // Target Thickness at 0 degree
        const;

  public: //   Useful function
    void          Print() const;
    void          Draw(const std::string& option = "");
    void          DrawNuclear(const std::string& option = "");
    void          DrawElectronic(const std::string& option = "");
    inline double GetMax() const {
        return fMax;
    }; // Get maximum energy value of ELoss table
    inline double GetMin() const {
        return fMin;
    }; // Get minimum energy value of ELoss table
};
