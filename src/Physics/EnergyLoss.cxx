#include "EnergyLoss.h"

EnergyLoss::EnergyLoss(const std::string& Path, const std::string& source,
                       int numberOfSlice = 100, int liseColumn,
                       int numberOfMass) {
    fMax           = -10000;
    fMin           = 1e12;

    fNumberOfSlice = numberOfSlice;
    fNumberOfMass  = numberOfMass;

    std::ifstream TableFile;

    // Opening dE/dX file
    TableFile.open(Path.c_str());
    if(!TableFile.is_open()) {
        throw std::runtime_error("Table file not found: " + Path + "\n");
    }

    if(source == "G4Table" || source == "G4table") {
        // Reading Data
        double      energy, total;
        std::string line;
        // skipped first line
        while(getline(TableFile, line)) {
            std::string densityKeyword{"Density ="};
            if(line.find(densityKeyword) != std::string::npos) {
                std::string        dummyTmp;
                std::string        unitTmp;
                double             density;
                std::istringstream str(line.substr(line.find(densityKeyword)
                                                   + densityKeyword.size()));
                str >> density >> unitTmp;

                fDensity = density * UNITS::classify(unitTmp);
                continue;
            }

            std::istringstream str(line);
            str >> energy >> total;
            energy = energy * UNITS::MeV;
            fEnergy.push_back(energy);
            if(energy > fMax) { fMax = energy; }
            if(energy < fMin) { fMin = energy; }
            fdEdX_Total.push_back(total * (UNITS::MeV / UNITS::micrometer));
        }

        // Close File
        TableFile.close();
    } else if(source == "SRIM") {
        // Reading Data
        double      energy, nuclear, electronic;
        std::string unit, dummy;
        std::string line;

        while(std::getline(TableFile, line)) {
            // Remove commas for decimal units if present
            std::replace(line.begin(), line.end(), ',', '.');
            std::string densityKeyword{"Density ="};

            if(line.find("-----") != std::string::npos && !fEnergy.empty())
                break; // Reached end of table

            if(line.find(densityKeyword) != std::string::npos) {
                std::string        dummyTmp;
                std::string        unitTmp;
                double             density;
                std::istringstream str(line.substr(line.find(densityKeyword)
                                                   + densityKeyword.size()));
                str >> density >> unitTmp;

                fDensity = density * UNITS::classify(unitTmp);
                continue;
            }

            std::istringstream str(line);
            if(line.empty() || line == "\r") continue;
            str >> energy >> unit >> electronic >> nuclear >> dummy >> dummy
                >> dummy >> dummy >> dummy >> dummy;
            if(energy == 0 || electronic == 0) continue;

            if(unit == "keV") energy = energy * UNITS::keV;
            if(unit == "MeV") energy = energy * UNITS::MeV;
            if(unit == "GeV") energy = energy * UNITS::GeV;
            fEnergy.push_back(energy);

            if(energy > fMax) fMax = energy;
            if(energy < fMin || fMin < 0) fMin = energy;

            fdEdX_Nuclear.push_back(nuclear * UNITS::MeV / UNITS::mm);
            fdEdX_Electronic.push_back(electronic * UNITS::MeV / UNITS::mm);
            fdEdX_Total.push_back(nuclear * UNITS::MeV / UNITS::mm
                                  + electronic * UNITS::MeV / UNITS::mm);
        }
        // Close File
        TableFile.close();
        if(fDensity < 0)
            throw std::runtime_error("Unable to find density in SRIM table\n");
    }

    else if(source == "LISE") {
        // Reading Data
        double      energy = 0, energyloss = 0;
        std::string dummy;
        // skipping comment first line
        getline(TableFile, dummy);

        while(TableFile >> energy) {
            for(int k = 0; k < 11; k++) {
                TableFile >> dummy;
                if(k + 1 == liseColumn) energyloss = std::stod(dummy);
            }
            energy = energy * UNITS::MeV;
            fEnergy.push_back(energy);

            if(energy > fMax) fMax = energy;
            if(energy < fMin || fMin < 0) fMin = energy;

            fdEdX_Total.push_back(energyloss * UNITS::MeV / UNITS::micrometer);
        }

        // Close File
        TableFile.close();
    } else {
        throw std::runtime_error("ERROR : EnergyLoss Wrong Source Type");
    }

    fInter = TGraph(static_cast<int>(fEnergy.size()), &fEnergy[0],
                    &fdEdX_Total[0]);
    fInter.Sort();

    fInterNuclear = TGraph(static_cast<int>(fEnergy.size()), &fEnergy[0],
                           &fdEdX_Nuclear[0]);
    fInterNuclear.Sort();

    fInterElectronic = TGraph(static_cast<int>(fEnergy.size()), &fEnergy[0],
                              &fdEdX_Electronic[0]);
    fInterElectronic.Sort();

    fSpline = TSpline3("Energy Loss Spline", &fInter);
}

void EnergyLoss::Draw(const std::string& option) {
    fInter.Draw(option.c_str());
}
void EnergyLoss::DrawNuclear(const std::string& option) {
    // if(fInterNuclear == nullptr)
    //	throw std::runtime_error("Trying to draw nullptr\n");
    fInterNuclear.Draw(option.c_str());
}
void EnergyLoss::DrawElectronic(const std::string& option) {
    // if(fInterElectronic == nullptr)
    //	throw std::runtime_error("Trying to draw nullptr\n");
    fInterElectronic.Draw(option.c_str());
}

double EnergyLoss::EvaluateNuclearLoss(double energy) const {
    if(fEnergy.empty() || fdEdX_Nuclear.empty()) {
        std::cout << "No Nuclear table for this Energy loss";
        return -1000;
    }

    return Eval(energy);
}

double EnergyLoss::EvaluateElectronicLoss(double energy) const {
    if(fEnergy.empty() || fdEdX_Electronic.empty()) {
        std::cout << "No Electronic table for this Energy loss";
        return -1000;
    }
    return Eval(energy);
}

double EnergyLoss::EvaluateTotalLoss(double energy) const {
    if(fEnergy.empty() || fdEdX_Total.empty()) {
        std::cout << "No Total table for this Energy loss";
        return -1000;
    }
    return Eval(energy);
}

void EnergyLoss::Print() const {
    std::cout << "Total Energy Loss : " << std::endl;
    size_t size = fdEdX_Total.size();
    for(unsigned int i = 0; i < size; i++)
        std::cout << fEnergy[i] / UNITS::MeV << " MeV "
                  << fdEdX_Total[i] / UNITS::MeV * UNITS::micrometer
                  << " MeV/um " << std::endl;
}

double EnergyLoss::EnergyLossCalculation(
    double energy,          // Energy of the detected particle
    double targetThickness, // Target Thickness at 0 degree
    double angle)           // Angle between particle and target norm
    const {
    return (energy - Slow(energy, targetThickness, angle));
}

double EnergyLoss::Slow(double energy, // Energy of the detected particle
                        double targetThickness, // Target Thickness at 0 degree
                        double angle) // Angle between particle and target norm
    const {
    //   Lise file are given in MeV/u
    //   For SRIM and geant4 file fNumberOfMass = 1 whatever is the nucleus,
    //   file are given in MeV

    energy = energy / (double)fNumberOfMass;

    if(angle > UNITS::CONSTANTS::halfpi) angle = UNITS::CONSTANTS::pi - angle;

    if(angle != UNITS::CONSTANTS::halfpi)
        targetThickness = targetThickness / (cos(angle));
    else
        targetThickness = 1 * UNITS::centimeter;

    double SliceThickness = targetThickness / (double)fNumberOfSlice;

    for(int i = 0; i < fNumberOfSlice; i++) {
        double de = Eval(energy) * SliceThickness;
        energy -= de / (double)fNumberOfMass;

        if(energy < 0) {
            energy = 0;
            break;
        }
    }
    return (energy * (double)fNumberOfMass);
}

double EnergyLoss::EvaluateInitialEnergy(
    double energy,          // Energy of the detected particle
    double targetThickness, // Target Thickness at 0 degree
    double angle)           // Angle between particle and target norm
    const {
    if(targetThickness == 0) return energy;
    //   Lise file are given in MeV/u
    //   For SRIM and geant4 file fNumberOfMass = 1 whatever is the nucleus,
    //   file are given in MeV
    energy = energy / (double)fNumberOfMass;

    if(angle > UNITS::CONSTANTS::halfpi) angle = UNITS::CONSTANTS::pi - angle;

    if(angle != UNITS::CONSTANTS::halfpi)
        targetThickness = targetThickness / (cos(angle));
    else
        targetThickness = 1 * UNITS::centimeter;

    double SliceThickness = targetThickness / (double)fNumberOfSlice;

    for(int i = 0; i < fNumberOfSlice; i++) {
        double de = Eval(energy) * SliceThickness;
        energy += de / fNumberOfMass;
    }
    return (energy * fNumberOfMass);
}

double EnergyLoss::EvaluateMaterialThickness(
    double initialEnergy, // Energy of the detected particle
    double finalEnergy, double thicknessLimit,
    double thicknessStep) // Target Thickness at 0 degree
    const {
    double Thickness = thicknessStep;
    double Energy    = initialEnergy;
    while(Energy < finalEnergy && Thickness > 0) {
        Energy = EvaluateInitialEnergy(Energy, thicknessStep, 0);
        Thickness += thicknessStep;
        if(Thickness > thicknessLimit) Thickness = -1;
    }

    return Thickness;
}

//   Evaluate Total Energy of particle from Energy loss in a giver thickness
double EnergyLoss::EvaluateEnergyFromDeltaE(
    double deltaE,           // Energy of the detected particle
    double targetThickness,  // Target Thickness at 0 degree
    double angle,            // Angle between particle and target norm
    double energyMin,        // Starting Energy
    double energyMax,        // Maximum Energy allowed
    double energyResolution, // Resolution at which function stop
    int    maxStep) // Stop after MaxStep Whatever Precision is reached
    const {
    double step_size  = 10. * UNITS::MeV;
    double Energy     = energyMax;
    double DE         = 0;
    bool   check_low  = false;
    bool   check_high = false;

    for(int i = 0; i < maxStep; i++) {
        if(Energy > 0)
            DE = Energy - Slow(Energy, targetThickness, angle);
        else
            return 0;

        if(abs(deltaE - DE) < energyResolution)
            return Energy;
        else if(deltaE - DE > 0) {
            if(Energy - step_size > energyMin) {
                Energy    = Energy - step_size;
                check_low = true;
            } else {
                step_size = step_size / 10.;
                Energy    = Energy - step_size;
            }
        }

        else if(deltaE - DE < 0) {
            if(Energy + step_size < energyMax) {
                Energy     = Energy + step_size;
                check_high = true;
            } else {
                step_size = step_size / 10.;
                Energy    = Energy + step_size;
            }
        }

        if(check_high && check_low) {
            step_size  = step_size / 10.;
            check_high = false;
            check_low  = false;
        }

        if(step_size < energyResolution) return Energy;
    }
    return Energy;
}

double EnergyLoss::Eval(double energy) const {
    if(energy < 0)
        throw std::runtime_error(
            "\033[1;31mERROR: negative energy given to EnergyLoss\033[0m\n");
    else if(std::isnan(energy)) {
        throw std::runtime_error(
            "\033[1;31mERROR: nan energy given to EnergyLoss\033[0m\n");
    }

    if(energy > fMax)
        throw std::runtime_error(
            "\033[1;31mERROR: energy of " + std::to_string(energy / UNITS::MeV)
            + " MeV is too high for given "
              "EnergyLoss table (max: "
            + std::to_string(fMax / UNITS::MeV) + " MeV)\033[0m\n");

    double retVal;

    if(energy < fMin) retVal = fSpline.Eval(fMin);
    // throw std::runtime_error("\033[1;31mERROR: energy too low for given "
    //						 "EnergyLoss table\033[0m\n");
    else
        retVal = fSpline.Eval(energy);

    if(retVal < 0) {
        std::cout << Form("Energy %f MeV, return value %f MeV",
                          energy / UNITS::MeV,
                          retVal / UNITS::MeV * UNITS::micrometer)
                  << std::endl;
        throw std::runtime_error("\033[1;31mERROR: spline failure - energy "
                                 "loss is negative \033[0m\n");
    }

    return retVal;
}
