
#include "Optimizer.h"

#include <fstream>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <iomanip>

#include <Math/Factory.h>
#include <Math/Functor.h>
#include <Math/Minimizer.h>
#include <TGraph.h>
#include <TH2D.h>

#include "App.h"

Optimizer::Optimizer()
    : settings(Settings::getInstance()), conf(ConfManager::getInstance()) {
    int status{};
    int childToParent[2]{};
    int parentToChild[2]{};
    if (pipe(childToParent)<0) throw std::runtime_error("Pipe error");
    if (pipe(parentToChild)<0) throw std::runtime_error("Pipe error");

    // Unfortunately we need to fork to keep the minimizer and the fitter on a
    // different memory space Because you can only have one minimizer in a
    // program due to ROOT.. yay
    pid = fork();
    switch(pid) {
    case -1: throw std::runtime_error("Fork error");
    case 0: // Child process
    {
        close(childToParent[0]);
        sendFd = childToParent[1];
        close(parentToChild[1]);
        receiveFd = parentToChild[0];
        receiveF  = fdopen(receiveFd, "r");

        Child();

        fclose(receiveF);
        exit(0);
    }
    default: // Parent
        close(childToParent[1]);
        receiveFd = childToParent[0];
        close(parentToChild[0]);
        sendFd   = parentToChild[1];
        receiveF = fdopen(receiveFd, "r");

        Parent();

        fclose(receiveF);
        while((pid = wait(&status)) != -1)
            fprintf(stderr, "Process %d exits with %d\n", pid,
                    WEXITSTATUS(status));
        break;
    }
}

double Optimizer::Cost(const double* pars) {
    // Send new parameters
    auto& p = conf.optimizer.parameters;
    for(unsigned int i{0}; i < p.size(); ++i) { newData.pars[i] = pars[i]; }
    if(log == nullptr)
        throw std::runtime_error("Unable to use log for minimizer");
    else {
        *log << "=============== it: " << iteration++
             << " =================" << std::endl;
        for(unsigned int i{0}; i < p.size(); ++i) {
            *log << "par: " << p.at(i).detector << " " << p.at(i).name
                 << " ---> ";
            *log << "val: " << pars[i] << std::endl;
        }
    }
    sendData(newData);
    Data data = receiveData();
    if(log == nullptr)
        throw std::runtime_error("Unable to use log for minimizer");
    else {
        *log << "COST " << data.cost << std::endl;
        *log << "FITTED MEAN " << data.mean << std::endl;
        *log << "FITTED SIGMA " << data.sigma << std::endl;
        //*log << "--------------- it: " << iteration++ << " -----------------"
        //     << std::endl;
    }
    return data.cost;
}

std::vector<TH1D> Optimizer::retrieveSpectrum(const std::string& file) {
    TFile f(file.c_str(), "read");
    if(!f.IsOpen())
        throw std::runtime_error("Unable to find file for fitting\n");

    std::vector<TH1D> spectra;
    std::cout << "Retrieving " << conf.optimizer.transition.size() << " spectra\n";  
    for(const auto& s : conf.optimizer.transition) {
        auto spectrumName = s.detector + "/" + s.name;
        auto h            = dynamic_cast<TH1D*>(f.Get(spectrumName.c_str()));
        if(h == nullptr)
            throw std::runtime_error("Provided spectrum named " + spectrumName
                                     + " is not of TH1D type\n");
        spectra.push_back(*h);
        std::cout << "Spectrum " << spectrumName << " added\n";
    }
    std::cout << spectra.size() << " spectra retrieved\n";
    f.Close();
    return spectra;
}

void Optimizer::Child() {
    // Fitter configuration
    RooMsgService::instance().setSilentMode(true);

    std::unordered_map<std::string,
                       std::vector<std::pair<double, std::vector<TH1D>>>>
                scanHistograms;
    std::string originalFilePattern{conf.replay.sum_file_pattern};

    while(!(newData = receiveData()).terminateChild) {
        std::cout << "Received data "  << std::endl;
        auto& p = conf.optimizer.parameters;
        for(unsigned int i{0}; i < p.size(); ++i) {
            if(p.at(i).ptr == nullptr)
                throw std::runtime_error("Got an unexpected nullptr\n");
            *(p.at(i).ptr) = newData.pars[i];
        }
        std::cout << "Checked pars "  << std::endl;

        if(conf.optimizer.saveScanFiles) {
            conf.replay.sum_file_pattern = originalFilePattern;
            if(newData.isScan) {
		std::ostringstream oss;
		oss << conf.replay.sum_file_pattern;
		oss <<  conf.replay.sum_file_pattern.append(newData.parameterName);
		oss << "_";
		oss << std::fixed << std::setprecision(11) << newData.pars[newData.scanParNr];
		oss << "_";
		
		conf.replay.sum_file_pattern = oss.str();
            }
        }

        const App app;

        auto      files = app.finalFiles;
        if(files.empty())
            throw std::runtime_error("Something is wrong with added files\n");

        std::cout << "Added files "  << std::endl;
        std::cout << "Files size " << files.size()  << std::endl;
        // Add spectra
        std::vector<TH1D> allhistos = retrieveSpectrum(files.at(0));
        std::cout << "Retrieved spectra "  << std::endl;
        for(unsigned int i{1}; i < files.size(); ++i) {
            auto tmpSpec = retrieveSpectrum(files.at(i));
            std::cout << "Retrieved spectra "  << i << std::endl;
            for(unsigned int h{0}; h < allhistos.size(); ++h) {
                allhistos.at(h).Add(&tmpSpec.at(h), 1);
            }
        }
        std::cout << "Added spectra "  << std::endl;

        if(newData.isScan)
            scanHistograms[newData.parameterName].emplace_back(
                newData.pars[newData.scanParNr], allhistos);

        std::string parsFileName = conf.replay.conf_path + "/Optimizer/"
                                 + conf.optimizer.fitParFile;
        std::ifstream fileCheck(parsFileName);
        bool          readParsFromFile{fileCheck.is_open()};

        struct FitterPars{
            TH1D histo;
            std::vector<std::pair<double,double>> centr;
            std::vector<double> sigma;
            std::vector<double> tail;
            std::vector<double> bias;
        };

        std::vector<FitterPars> fitterParsVec;

        double totbias{0};
        for(const auto& it: conf.optimizer.transition) {
            totbias += it.step;
        }

        std::cout << "Total bias: " << totbias << std::endl;

        std::set<unsigned int> setId;

        // Create a map with all the histograms to filter them by name
        for(unsigned int i{0}; i < allhistos.size(); ++i) {
            std::string histoName = allhistos.at(i).GetName(); //HISTOGRAM NAME
            if(setId.find(i) != setId.end()) continue; // If the histogram is already in the set, skip it
            fitterParsVec.emplace_back(FitterPars{ allhistos.at(i),
                                                {std::make_pair(conf.optimizer.transition.at(i).initialValue,2000)},
                                                {conf.optimizer.transition.at(i).minValue}, // sigma
                                                {conf.optimizer.transition.at(i).maxValue}, // tail
                                                {conf.optimizer.transition.at(i).step/totbias}}); // bias
            setId.insert(i);
            for(unsigned int j{i+1}; j < allhistos.size(); ++j) {
                if( allhistos.at(j).GetName() != histoName) continue; // If the histogram is not the same, skip it
                if (abs(conf.optimizer.transition.at(i).initialValue - conf.optimizer.transition.at(j).initialValue)
                    < 3. * (conf.optimizer.transition.at(i).minValue + conf.optimizer.transition.at(j).minValue)) { 
                    setId.insert(j);
                    fitterParsVec.back().centr.push_back(std::make_pair(conf.optimizer.transition.at(j).initialValue,2000));
                    fitterParsVec.back().sigma.push_back(conf.optimizer.transition.at(j).minValue);
                    fitterParsVec.back().tail.push_back(conf.optimizer.transition.at(j).maxValue);
                    fitterParsVec.back().bias.push_back(conf.optimizer.transition.at(j).step/totbias);
                }
            }
        }

        double totmeanCost{0};
        double totsigmaCost{0};
        double totnonconvergengecost{0};
        double mean{0};
        double sigma{0};

        std::cout << "FitterParsVec size: " << fitterParsVec.size() << std::endl;

        for( unsigned int i{0}; i < fitterParsVec.size(); ++i ){

            std::cout << "FitterParsVec id : " << i << std::endl;

            auto& fP = fitterParsVec.at(i); // Fitter parameters

            std::cout << "Fitting " << fP.histo.GetName() << std::endl;

            Fitter fitter({fP.centr}, 
                          static_cast<Fitter::Tail>(conf.optimizer.tail),
                          conf.optimizer.printLevel);

            // Get the fitter units from the histogram label
            std::string x_axis_label = fP.histo.GetXaxis()->GetTitle();
            if(settings.verbosity_level > 0){
                std::cout << "X axis label: " << x_axis_label << std::endl;
            }

            double fitter_unit = 1;
            if(!x_axis_label.empty()){
                fitter_unit = UNITS::classify(x_axis_label.substr(x_axis_label.find_last_of('[')+1,
                                        x_axis_label.find_last_of(']')-x_axis_label.find_last_of('[')-1));    
            }

            fitter.SetEnergyUnits(fitter_unit);
            fitter.SetBkgOrder(conf.optimizer.bkgPolOrder);

            // Read parameter values
            if(readParsFromFile) {
                fitter.ReadParsFromFile(parsFileName, i);
            } else {
                // Added units for the initial values
                fitter.SetInitialSigma(fP.sigma.at(0), fitter_unit); 
                fitter.SetSigmaLimits({fP.sigma.at(0)*0.33, fP.sigma.at(0)*3}, fitter_unit);
                fitter.SetInitialTau(fP.tail.at(0));
                fitter.WriteParsOnFile(parsFileName, i);
            }
            if(settings.debug_canvas) fitter.EnableCanvas();

            if(settings.verbosity_level > 2) std::cout << " @@@@@@@@@@  Ready to fit" << std::endl;

            // Fit result
            fitter.SetSpec(fP.histo);
            auto results = fitter.Fit();

            if(settings.verbosity_level > 2) std::cout << "@@@@@@@@@ Done Fititng" << std::endl;

            // Check if the fit converged
            if(results.front().status != 0) {
                std::cout << "Fit did not converge\n";
            }

            // Sigma
            double sigmacost{0};
            double meancost{0};

            for(unsigned int j = 0; j < results.size(); ++j) {
                // double val = it.sigma_gauss.first/it.sigma_gauss.second;
                double sig   = results.at(j).sigma_gauss.first;
                double discr = results.at(j).mean_gauss.first - results.at(j).energy;
                switch(conf.optimizer.tail) {
                case Fitter::NOTAIL: {
                    break;
                }
                case Fitter::RIGHT: {
                    sig += results.at(j).tail_right.first;
                    break;
                }
                case Fitter::LEFT: {
                    sig += results.at(j).tail_left.first;
                    break;
                }
                case Fitter::LEFT_RIGHT: {
                    sig += results.at(j).tail_left.first;
                    sig += results.at(j).tail_right.first;
                    break;
                }
                case Fitter::SYMMETRIC: {
                    sig += 2 * results.at(j).tail_left.first;
                    break;
                    }
                    default:
                        throw std::runtime_error(
                            "Tail settings in fitter does not make sense\n");
                    }
                    sigmacost += pow(sig, 2)  * fP.bias.at(j);
                    meancost += pow(discr, 2) * fP.bias.at(j);
            }
            sigmacost = sqrt(sigmacost);
            meancost  = sqrt(meancost);
            totsigmaCost += sigmacost;
            totmeanCost  += meancost;
            mean = results.back().mean_gauss.first;
            sigma = results.back().sigma_gauss.first;

            if(results.front().status == 5)
                totnonconvergengecost += conf.optimizer.nonConvergenceCost;
        }

        double cost = totmeanCost * (1-conf.optimizer.sigmaWeight) + totsigmaCost * conf.optimizer.sigmaWeight;

        cost += totnonconvergengecost;

        //newData.mean  = totmeanCost; // printed in the log
        //newData.sigma = totsigmaCost;
        newData.mean  = mean; // Mean of the last peak
        newData.sigma  = sigma; // Sigma of the last peak
        newData.cost  = cost;   // Minimzer cost

        if(newData.printConf)
            conf.PrintConf(settings.confFileName + "_optimized");
        sendData(newData);
    }

    // Write scan histograms in the optimizer output file
    TFile outOptimizer(
        (conf.replay.out_path + "/" + conf.optimizer.optimizerOutputFile)
            .c_str(),
        "update");
    for(const auto& it : scanHistograms) {
        std::unordered_map<std::string, TH2D> scan2DHistograms;
        for(const auto& h : it.second.front().second) {
            int    nBins = it.second.size() - 1;
            double minVal{
                std::min_element(
                    it.second.begin(), it.second.end(),
                    [](const std::pair<double, std::vector<TH1D>>& a,
                       const std::pair<double, std::vector<TH1D>>& b) {
                        return a.first < b.first;
                    })
                    ->first};
            double maxVal{
                std::max_element(
                    it.second.begin(), it.second.end(),
                    [](const std::pair<double, std::vector<TH1D>>& a,
                       const std::pair<double, std::vector<TH1D>>& b) {
                        return a.first < b.first;
                    })
                    ->first};

            double step = (maxVal - minVal) / nBins;
            scan2DHistograms.emplace(
                it.first + h.GetName(),
                TH2D{(it.first + h.GetName()).c_str(),
                     (it.first + h.GetName()).c_str(), nBins,
                     minVal - step / 2., maxVal + step / 2.,
                     it.second.front().second.front().GetNbinsX(),
                     it.second.front().second.front().GetXaxis()->GetXmin(),
                     it.second.front().second.front().GetXaxis()->GetXmax()});
        }

        for(const auto& xBin : it.second) {
            for(const auto& h : xBin.second) {
                auto& ref{scan2DHistograms[it.first + h.GetName()]};
                for(int yBin{0}; yBin < h.GetXaxis()->GetNbins(); ++yBin) {
                    ref.SetBinContent(ref.GetXaxis()->FindBin(xBin.first),
                                      yBin, h.GetBinContent(yBin));
                }
            }
        }

        outOptimizer.mkdir(it.first.c_str(), it.first.c_str())->cd();
        for(const auto& h : scan2DHistograms) { h.second.Write(); }
    }
    outOptimizer.Write();
}

void Optimizer::Parent() {
    // Minimizer configuration
    // minimizer = ROOT::Math::Factory::CreateMinimizer("Genetic", "");
    ROOT::Math::Minimizer* minimizer = ROOT::Math::Factory::CreateMinimizer(
        conf.optimizer.minimizerName, conf.optimizer.minimizerAlgorithm);

    if(minimizer == nullptr)
        throw std::runtime_error("Minimizer not found by factory");

    ROOT::Math::Functor fun(
        this, &Optimizer::Cost,
        static_cast<unsigned int>(conf.optimizer.parameters.size()));
    minimizer->SetFunction(fun);

    minimizer->SetMaxFunctionCalls(
        static_cast<unsigned int>(conf.optimizer.maxCalls));
    minimizer->SetTolerance(conf.optimizer.tolerance);
    minimizer->SetPrintLevel(conf.optimizer.printLevel);
    minimizer->SetPrecision(conf.optimizer.precision);
    minimizer->SetValidError(conf.optimizer.validErrors);

    for(unsigned int i{0}; i < conf.optimizer.parameters.size(); ++i) {
        const auto& p{conf.optimizer.parameters.at(i)};
        minimizer->SetVariable(i, p.detector + "_" + p.name, p.initialValue,
                               p.step);

        if(conf.optimizer.useIntervals)
            minimizer->SetVariableLimits(i, p.minValue, p.maxValue);
    }

    std::string logFileName
        = conf.replay.conf_path + "/Optimizer/" + conf.optimizer.logFile;
    log = new std::ofstream(logFileName);
    *log << "##################Start of minimizer###################\n";
    if(conf.replay.sum_file_pattern.size() > 400)
        throw std::runtime_error("Sum file pattern too long\n");
    std::strcpy(newData.parameterName, "");
    if(!conf.optimizer.onlyScan) minimizer->Minimize();
    *log << "##################End of minimizer#####################\n";
    // newData.printConf = true;
    // Cost(minimizer->X());
    // newData.printConf = false;

    // Data  newData;
    // auto& p = conf.optimizer.parameters;
    // for(unsigned int i{0}; i < p.size(); ++i) {
    //     newData.pars[i] = minimizedPars[i];
    //     *log << i << "  " << p.at(i).name << "  "<< minimizedPars[i] <<
    //     "\n"; std::cout << i << "  " << p.at(i).name << "  "<<
    //     minimizedPars[i] << "\n";
    // }
    // newData.printConf = true;
    // newData.terminateChild = false;
    // sendData(newData);
    // auto dummy = receiveData();
    // minimizer->PrintResults();

    // Parameter scanning
    TFile outOptimizer(
        (conf.replay.out_path + "/" + conf.optimizer.optimizerOutputFile)
            .c_str(),
        "recreate");
    // Scans
    newData.isScan = true;
    for(const auto& it : conf.optimizer.parameterScan) {
        // Resets parameters to the default values at the beginning of each scan
        for(unsigned int i{0}; i < conf.optimizer.parameters.size(); ++i) {
            const auto& p{conf.optimizer.parameters.at(i)};
            minimizer->SetVariable(i, p.name, p.initialValue, p.step);
        }

        //    if(conf.optimizer.useIntervals)
        //        minimizer->SetVariableLimits(i, p.minValue, p.maxValue);
        //}
        for(unsigned int i{0}; i < conf.optimizer.parameters.size(); ++i) {
            if(conf.optimizer.parameters.at(i).name != it.name
               || conf.optimizer.parameters.at(i).detector != it.detector)
                continue;

            std::strcpy(newData.parameterName,
                        (it.detector + "_" + it.name).c_str());
            newData.scanParNr = i;

            auto nSteps = static_cast<unsigned int>((it.maxValue - it.minValue)
                                                    / it.step)
                        + 1;
            std::vector<double> x(nSteps, 0.);
            std::vector<double> y(nSteps, 0.);
            *log << "##################Scan of ";
            *log << it.detector << " of " << it.name << std::endl;
            bool featureEnabled = minimizer->Scan(
                i, nSteps, x.data(), y.data(), it.minValue, it.maxValue);
            if(!featureEnabled)
                throw std::runtime_error("The chosen optimizer does not "
                                         "support the scan feature\n");
            TGraph graph(static_cast<int>(x.size()), x.data(), y.data());
            graph.SetTitle(
                ("par: " + it.name + " det: " + it.detector).c_str());
            graph.SetName(("scan_" + it.name + "_" + it.detector).c_str());
            graph.Write();
            break;
        }
    }
    newData.isScan = false;

    // Reset ouput file name to the default one
    std::strcpy(newData.parameterName, "");
    for(const auto& it : conf.optimizer.parameterContour) {
        // Resets parameters to the default values at the beginning of each
        // contour for(unsigned int i{0}; i < conf.optimizer.parameters.size();
        // ++i) {     const auto& p{conf.optimizer.parameters.at(i)};
        //    minimizer->SetVariable(i, p.name, p.initialValue, p.step);

        //    if(conf.optimizer.useIntervals)
        //        minimizer->SetVariableLimits(i, p.minValue, p.maxValue);
        //}
        int idx1{-1};
        int idx2{-1};
        for(unsigned int i{0}; i < conf.optimizer.parameters.size(); ++i) {
            if(conf.optimizer.parameters.at(i).name == it.name1
               && conf.optimizer.parameters.at(i).detector == it.detector1) {
                idx1 = static_cast<int>(i);
            } else if(conf.optimizer.parameters.at(i).name == it.name2
                      && conf.optimizer.parameters.at(i).detector
                             == it.detector2) {
                idx2 = static_cast<int>(i);
            }
        }
        if(idx1 == -1 || idx2 == -1)
            throw std::runtime_error(
                "Unable to find reuested parameters for contour\n");
        if(idx1 == idx2)
            throw std::runtime_error(
                "There is no point of correlating the same parameter!");

        *log << "##################Contour of ";
        *log << it.detector1 << " of " << it.name1;
        *log << " vs ";
        *log << it.detector2 << " of " << it.name2 << std::endl;

        auto                nPoints{it.nPoints};
        std::vector<double> x(nPoints, 0.);
        std::vector<double> y(nPoints, 0.);
        bool                featureEnabled
            = minimizer->Contour(idx1, idx2, nPoints, x.data(), y.data());
        if(!featureEnabled) std::cerr << "Contour returned false" << std::endl;
        TGraph graph(static_cast<int>(x.size()), x.data(), y.data());
        graph.SetTitle(("par1: " + it.name1 + " det1: " + it.detector1
                        + " par2: " + it.name2 + " det2: " + it.detector2)
                           .c_str());
        graph.SetName(("coutour_" + it.name1 + "_" + it.detector1 + "_vs_"
                       + it.name2 + "_" + it.detector2)
                          .c_str());
        graph.Write();
    }
    // Coutours
    outOptimizer.Write();
    outOptimizer.Close();

    // Final Minimization
    // minimizer->Minimize();

    if(!conf.optimizer.onlyScan) {
        *log << "#################Running with optimal parameters\n";
        newData.printConf = true;
        Cost(minimizer->X());
    
        std::cout << "\n\nEnd of minimizer procedure " << std::endl;    
        std::cout << "Correlation matrix" << std::endl;
        for(unsigned int i{0}; i < conf.optimizer.parameters.size(); ++i) {
            const auto& p1{conf.optimizer.parameters.at(i)};
            std::cout << p1.detector + "_" + p1.name << " ->\t\t";
            for(unsigned int j{0}; j < conf.optimizer.parameters.size(); ++j) {
                // const auto &p2{conf.optimizer.parameters.at(j)};
                std::cout << minimizer->Correlation(i, j) << "\t\t";
            }
            std::cout << std::endl;
        }
        std::cout << "\n\nResults" << std::endl;
        minimizer->PrintResults();
    }

    Data term;
    term.terminateChild = true;
    sendData(term);
    delete minimizer;
}

Optimizer::Data Optimizer::receiveData() const {
    Data   data;
    size_t dataRead = fread(&data, 1, sizeof(data), receiveF);
    if(dataRead != sizeof(data)) {
        throw std::runtime_error("Not enough bytes read from pipe, the other thread has probably crashed\n");
    }
    return data;
}

void Optimizer::sendData(const Optimizer::Data& data) const {
    if (write(sendFd, &data, sizeof(data)) != sizeof(data)) {
        throw std::runtime_error("Not enough bytes written to pipe, the other thread has probably crashed\n");
    }
}
