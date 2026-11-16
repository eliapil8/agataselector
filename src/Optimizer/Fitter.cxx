#include "Fitter.h"

#include <TVirtualFitter.h>

Fitter::Fitter(std::vector<std::pair<double, double>> energies,
               const Tail&                            settings_tail,
               const int&                             pLevel)
    : settings_tail(settings_tail), energies(std::move(energies)),
      printLevel(pLevel) {
        switch(printLevel){
            case 0:
            RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);
            break;
            case 1:
            RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
            break;
            case 2:
            RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);
            break;
            case 3:
            RooMsgService::instance().setGlobalKillBelow(RooFit::PROGRESS);
            break;
            case 4:
            RooMsgService::instance().setGlobalKillBelow(RooFit::INFO);
            break;
            default:
            break;
        }
    }

std::vector<Fitter::FitRes> Fitter::Fit() {
    TVirtualFitter::SetDefaultFitter("Minuit2");
    if(parameters.sigma.empty()) FindParameters();

    //  Declare observable x
    RooRealVar  x("x", "x", parameters.left_interval,
                  parameters.right_interval);

    // Create a binned dataset that imports contents of TH1 and associates its
    // contents to observable 'x'
    RooDataHist dh(spec.GetName(), spec.GetTitle(), x, RooFit::Import(spec));

    // Make plot of binned dataset showing Poisson error bars (RooFit default)
    std::string title = "Fit at energies ";
    for(const auto& it : energies) {
        title += " ";
        title += std::to_string(it.first);
    }

    auto ngauss = static_cast<unsigned int>(energies.size());
    std::vector<RooFitParams> params;
    // alternative to decay: RooAbsPdf* beta =
    // bindPdf("beta",ROOT::Math::beta_pdf,x2,a,b) ;
    params.reserve(ngauss);
    for(unsigned int i = 0; i < ngauss; ++i) {
        params.emplace_back(
            new RooRealVar(parameters.ampl[i].name.c_str(),
                           parameters.ampl[i].name.c_str(),
                           parameters.ampl[i].initial_value,
                           parameters.ampl[i].min, parameters.ampl[i].max),
            new RooRealVar(parameters.mean[i].name.c_str(),
                           parameters.mean[i].name.c_str(),
                           parameters.mean[i].initial_value,
                           parameters.mean[i].min, parameters.mean[i].max),
            new RooRealVar(parameters.sigma[i].name.c_str(),
                           parameters.sigma[i].name.c_str(),
                           parameters.sigma[i].initial_value,
                           parameters.sigma[i].min, parameters.sigma[i].max),
            new RooRealVar(parameters.tau_left[i].name.c_str(),
                           parameters.tau_left[i].name.c_str(),
                           parameters.tau_left[i].initial_value,
                           parameters.tau_left[i].min,
                           parameters.tau_left[i].max),
            new RooRealVar(parameters.tau_right[i].name.c_str(),
                           parameters.tau_right[i].name.c_str(),
                           parameters.tau_right[i].initial_value,
                           parameters.tau_right[i].min,
                           parameters.tau_right[i].max));
        params.back().fracsignal->setConstant(parameters.ampl[i].fixed);
        params.back().mean->setConstant(parameters.mean[i].fixed);
        params.back().sigma->setConstant(parameters.sigma[i].fixed);
        params.back().tau_left->setConstant(parameters.tau_left[i].fixed);
        params.back().tau_right->setConstant(parameters.tau_right[i].fixed);
    }
    params[ngauss - 1].fracsignal->setConstant(false);

    std::vector<RooAbsPdf*> gaussians;
    // std::vector<RooGExpModel*>      partial_smeared_gaussians;
    // std::vector<RooDecay*>      smeared_gaussians;
    RooArgList              gaussians_list;

    // Creates the gaussians
    for(unsigned int i = 0; i < ngauss; ++i) {
        switch(settings_tail) {
        case NOTAIL: {
            gaussians.push_back(new RooGaussModel(
                Form("gaussian_%i", i), Form("gaussian_%i", i), x,
                *params[i].mean, *params[i].sigma));
            break;
        }
        case RIGHT: {
            gaussians.push_back(new RooGExpModel(
                Form("gaussian_right_%i", i), Form("gaussian_right_%i", i), x,
                *params[i].mean, *params[i].sigma, *params[i].tau_right,
                RooFit::RooConst(1.), RooFit::RooConst(1.),
                RooFit::RooConst(1.), true, RooGExpModel::Type::Flipped));
            break;
        }
        case LEFT: {
            gaussians.push_back(new RooGExpModel(
                Form("gaussian_left_%i", i), Form("gaussian_left_%i", i), x,
                *params[i].mean, *params[i].sigma, *params[i].tau_left,
                // RooFit::RooConst(1.), RooFit::RooConst(1.),
                // RooFit::RooConst(1.),
                RooFit::RooConst(1.), RooFit::RooConst(1.),
                RooFit::RooConst(1.), true, RooGExpModel::Type::Normal));
            break;
        }
        case LEFT_RIGHT: {
            gaussians.push_back(new RooDecay(
                Form("gaussian_left_right_%i", i),
                Form("gaussian_left_right_%i", i), x, *params[i].tau_left,
                *new RooGExpModel(
                    Form("gaussian_right_%i", i), Form("gaussian_right_%i", i),
                    x, *params[i].mean, *params[i].sigma, *params[i].tau_right,
                    // RooFit::RooConst(1.), RooFit::RooConst(1.),
                    // RooFit::RooConst(1.),
                    RooFit::RooConst(1.), RooFit::RooConst(1.),
                    RooFit::RooConst(1.), true, RooGExpModel::Type::Normal),
                RooDecay::SingleSided));
            break;
        }
        case SYMMETRIC: {
            gaussians.push_back(new RooDecay(
                Form("gaussian_symmtail_%i", i),
                Form("gaussian_symmtail_%i", i), x, *params[i].tau_left,
                *new RooGaussModel(Form("gaussian_%i", i),
                                   Form("gaussian_%i", i), x, *params[i].mean,
                                   *params[i].sigma),
                RooDecay::DoubleSided));
            break;
        }
        default:
            throw std::runtime_error(
                "Tail settings in fitter does not make sense\n");
        }
        gaussians_list.add(*gaussians.back());
    }

    // Background (polN)
    std::vector<RooRealVar*> bkg_components;
    RooArgList               bkg_list;
    for(unsigned int ii{0}; ii < bkgInitialParameters.size(); ++ii) {
        bkg_components.push_back(new RooRealVar(
            Form("pol_%u", ii), Form("pol_%u", ii),
            bkgInitialParameters[ii].initial_value,
            bkgInitialParameters[ii].min, bkgInitialParameters[ii].max));
        bkg_list.add(*bkg_components.back());
    }
    RooPolynomial bkg("bkg", "bkg", x, bkg_list);

    // signal+bkg, will amount to total counts of each
    RooRealVar    sign_y("sign_y", "sign_y", 1E3, 0., 1E9);
    RooRealVar    bkg_y("bkg_y", "bkg_y", 1E3, 0., 1E9);

    // List of signals to be added
    RooArgList    signals_list;
    for(unsigned int i = 0; i < ngauss - 1; ++i) {
        signals_list.add(*params[i].fracsignal);
    }
    // Last parameter will be the total amount of signal
    params[ngauss - 1].fracsignal = &sign_y;

    // Total signal PDF
    auto*         signal = new RooAddPdf(Form("signal"), Form("signal"),
                                         gaussians_list, signals_list);

    // Signal + bkg model, final model
    auto          model = RooAddPdf("model", "model", RooArgList(*signal, bkg),
                                    RooArgList(sign_y, bkg_y));

    // FIT
    RooFitResult* result;
    if(minos) {
        result = model.fitTo(dh, RooFit::Minos(kTRUE), RooFit::Save(),
                             RooFit::NumCPU(4));
    } else {
        result = model.fitTo(dh, RooFit::Save(), RooFit::NumCPU(4));
    }

    // Reads parameters and errors
    parameters.GetParameters(params);

    // Extracts the results and saves them in the FitRes structure
    std::vector<FitRes> results;
    for(unsigned int ii = 0; ii < ngauss; ++ii) {
        double integral{0};
        double integral_err{0};
        if(ii != ngauss - 1) {
            auto& frac{params[ii].fracsignal};
            integral       = frac->getVal() * sign_y.getVal();
            double deltaF2 = sqrt(pow(sign_y.getVal() * frac->getError(), 2)
                                  + pow(sign_y.getError() * frac->getVal(), 2)
                                  + 2 * sign_y.getVal() * frac->getVal()
                                        * result->correlation(sign_y, *frac));
            integral_err   = sqrt(deltaF2);
        } else {
            if(ngauss == 1) {
                integral     = sign_y.getVal();
                integral_err = sign_y.getError();
            } else {
                double remainingFraction{1.};
                double deltaF2{0.};
                for(unsigned int jj = 0; jj < ngauss - 1; ++jj) {
                    auto& frac{params[jj].fracsignal};
                    remainingFraction -= frac->getVal();
                    deltaF2 += pow(sign_y.getVal() * frac->getError(), 2);
                }
                integral = remainingFraction * sign_y.getVal();

                deltaF2 += pow(remainingFraction * sign_y.getError(), 2);

                for(unsigned int jj = 0; jj < ngauss - 1; ++jj) {
                    auto& frac{params[jj].fracsignal};
                    deltaF2 -= 2 * sign_y.getVal() * remainingFraction
                             * result->correlation(sign_y, *frac);
                    for(unsigned int kk = 0; kk < jj; ++kk) {
                        deltaF2 += 2 * pow(frac->getVal(), 2)
                                 * result->correlation(*frac,
                                                       *params[kk].fracsignal);
                    }
                }
                integral_err = sqrt(deltaF2);
            }
        }
        // Simply getting the results here
        double counts          = integral / spec.GetBinWidth(1);
        double counts_err      = integral_err / spec.GetBinWidth(1);
        double mean_gauss      = parameters.mean[ii].fitted_value.first;
        double mean_gauss_err  = parameters.mean[ii].fitted_value.second;
        double sigma_gauss     = parameters.sigma[ii].fitted_value.first;
        double sigma_gauss_err = parameters.sigma[ii].fitted_value.second;
        double tail_left       = parameters.tau_left[ii].fitted_value.first;
        double tail_left_err   = parameters.tau_left[ii].fitted_value.second;
        double tail_right      = parameters.tau_right[ii].fitted_value.first;
        double tail_right_err  = parameters.tau_right[ii].fitted_value.second;

        // Setting the extracted values in the structure
        results.emplace_back(energies[ii].first, integral, integral_err,
                             counts, counts_err, mean_gauss, mean_gauss_err,
                             sigma_gauss, sigma_gauss_err, tail_left,
                             tail_left_err, tail_right, tail_right_err);
        results.back().status = result->status();
    }

    // The canvas is plotted if the option is enabled
    if(canvas_enabled) {
        std::unique_ptr<RooPlot> frame(x.frame(RooFit::Title(title.c_str())));
        dh.plotOn(frame.get());
        // model->plotOn(frame, RooFit::VisualizeError(*result));

        // Plots red bands for errors
        model.plotOn(frame.get(), RooFit::VisualizeError(*result, 1),
                     RooFit::LineColor(kRed), RooFit::FillColor(kRed));
        model.plotOn(frame.get(), RooFit::VisualizeError(*result, sign_y),
                     RooFit::Components(*signal), RooFit::LineColor(kBlue),
                     RooFit::FillColor(kBlue));
        model.plotOn(frame.get(), RooFit::VisualizeError(*result, bkg_y),
                     RooFit::Components(bkg), RooFit::LineColor(kGray),
                     RooFit::FillColor(kGray));

        // Plots model with blue line
        model.plotOn(frame.get(), RooFit::LineWidth(2),
                     RooFit::LineColor(kRed + 2));

        // Plots fit values and errors
        model.paramOn(frame.get(), RooFit::Layout(0.7, 0.9, 0.9));

        // Plots individual gaussian component
        for(unsigned int i = 0; i < ngauss; ++i) {
            model.plotOn(frame.get(), RooFit::Components(*gaussians[i]),
                         RooFit::LineStyle(kDashed),
                         RooFit::LineColor(kBlue + 2));
        }
        // Plots background
        model.plotOn(frame.get(), RooFit::Components(bkg),
                     RooFit::LineStyle(kDashed), RooFit::LineColor(kGray + 2));

        // Plot histogram again to stay on top
        dh.plotOn(frame.get());

        // Print stuff
        result->covarianceMatrix().Print();
        result->Print();

        // Start canvas
        TCanvas c;
        frame->Draw();
        c.Update();
        std::cout << "status: " << result->status() << std::endl;
        std::cout << "chi2: " << model.createChi2(dh)->getVal() << std::endl;
        std::cout << "chi2: "
                  << frame->chiSquare(result->correlationMatrix().GetNcols())
                  << std::endl;
        // std::cout << "chi2: "<< frame->chiSquare(Form("bkg_gaus%i", ngauss),
        // "dh") << std::endl;
        c.WaitPrimitive();
        c.Close();
    }

    // For as much as I hate it, I had to use pointers
    delete signal;
    for(auto& it : gaussians) delete it;
    // RooRealVar::cleanup();
    // params.clear();

    return results;
}

void Fitter::FindParameters() {
    std::vector<double> fractions;
    double              total_int{0};
    for(const auto& it : energies) { total_int += it.second; }

    fractions.reserve(energies.size());
    for(const auto& it : energies) {
        fractions.push_back(it.second / total_int);
    }

    auto number_of_gaussians = static_cast<unsigned int>(energies.size());
    parameters.Resize(number_of_gaussians);
    parameters.left_interval  = energies.front().first - fit_limits.first;
    parameters.right_interval = energies.back().first + fit_limits.second;

    for(unsigned int ii = 0; ii < number_of_gaussians; ++ii) {
        parameters.ampl[ii].initial_value = fractions[ii];
        // parameters.ampl[ii].initial_value = 0.5;
        parameters.ampl[ii].min           = 0.;
        parameters.ampl[ii].max           = 1.;
        parameters.ampl[ii].fixed         = relative_amplitude_fixed;
        parameters.ampl[ii].name          = "Ampl_" + std::to_string(ii);

        parameters.mean[ii].initial_value = energies[ii].first;
        parameters.mean[ii].min = energies[ii].first - mean_around_center_min;
        parameters.mean[ii].max = energies[ii].first + mean_around_center_max;
        parameters.mean[ii].fixed             = mean_fixed;
        parameters.mean[ii].name              = "Mean_" + std::to_string(ii);

        parameters.sigma[ii].initial_value    = parameters.initial_sigma;
        parameters.sigma[ii].min              = parameters.min_sigma;
        parameters.sigma[ii].max              = parameters.max_sigma;
        parameters.sigma[ii].fixed            = false;
        parameters.sigma[ii].name             = "Sigma_" + std::to_string(ii);

        parameters.tau_left[ii].initial_value = parameters.initial_tau;
        parameters.tau_left[ii].min           = parameters.min_tau;
        parameters.tau_left[ii].max           = parameters.max_tau;
        parameters.tau_left[ii].fixed         = false;
        parameters.tau_left[ii].name = "Tau_left_" + std::to_string(ii);

        parameters.tau_right[ii].initial_value = parameters.initial_tau;
        parameters.tau_right[ii].min           = parameters.min_tau;
        parameters.tau_right[ii].max           = parameters.max_tau;
        parameters.tau_right[ii].fixed         = false;
        parameters.tau_right[ii].name = "Tau_right_" + std::to_string(ii);
    }

    for(unsigned int ii{0}; ii < bkg_pol_order + 1; ++ii) {
        InitialState polInit;
        polInit.initial_value = 0;
        polInit.fitted_value  = {0, 0};
        polInit.min           = -0.1;
        polInit.max           = 0.1;
        polInit.fixed         = false;
        polInit.name          = Form("pol_%u", ii);
        bkgInitialParameters.emplace_back(polInit);
    }

    parameters.ampl[number_of_gaussians - 1].initial_value = 1E4;
    parameters.ampl[number_of_gaussians - 1].min           = 0.;
    parameters.ampl[number_of_gaussians - 1].max           = 1E8;
    parameters.ampl[number_of_gaussians - 1].fixed         = false;
    parameters.ampl[number_of_gaussians - 1].name
        = "Ampl_" + std::to_string(number_of_gaussians - 1);
}

void Fitter::SaveParsOnFile(const std::vector<Fitter::FitRes>& par,
                            char*                              fileName) {
    auto  ngauss = static_cast<unsigned int>(energies.size());

    FILE* fRes;
    fRes = fopen(fileName, "a");
    for(unsigned int i = 0; i < ngauss; ++i) {
        if(settings_tail == NOTAIL)
            fprintf(fRes, "%f %f %f %f %f %f\n", par[i].mean_gauss.first,
                    par[i].mean_gauss.second, par[i].sigma_gauss.first,
                    par[i].sigma_gauss.second, par[i].counts.first,
                    par[i].counts.second);
        else if(settings_tail == LEFT || settings_tail == SYMMETRIC)
            fprintf(fRes, "%f %f %f %f %f %f %f %f\n", par[i].mean_gauss.first,
                    par[i].mean_gauss.second, par[i].sigma_gauss.first,
                    par[i].sigma_gauss.second, par[i].counts.first,
                    par[i].counts.second, par[i].tail_left.first,
                    par[i].tail_left.second);
        else if(settings_tail == RIGHT)
            fprintf(fRes, "%f %f %f %f %f %f %f %f\n", par[i].mean_gauss.first,
                    par[i].mean_gauss.second, par[i].sigma_gauss.first,
                    par[i].sigma_gauss.second, par[i].counts.first,
                    par[i].counts.second, par[i].tail_right.first,
                    par[i].tail_right.second);
        else if(settings_tail == LEFT_RIGHT)
            fprintf(fRes, "%f %f %f %f %f %f %f %f %f %f\n",
                    par[i].mean_gauss.first, par[i].mean_gauss.second,
                    par[i].sigma_gauss.first, par[i].sigma_gauss.second,
                    par[i].counts.first, par[i].counts.second,
                    par[i].tail_left.first, par[i].tail_left.second,
                    par[i].tail_right.first,
                    par[i]
                        .tail_righ\
t.second);
    }
    fclose(fRes);
}

void Fitter::WriteParsOnFile(const std::string&  file_name,
                             const unsigned int& idx) {
    if(parameters.sigma.empty()) FindParameters();
    std::ofstream parfile(file_name, std::ios::app);

    if(!parfile.is_open()) throw std::runtime_error(Form("could not write file %s\n", file_name.c_str()));

    parfile << "---------------------------" << std::endl;
    parfile << "idx: " << idx << std::endl;
    parfile << "name "
            << ":\t\t\t"
            << " init_val "
            << "\t\t"
            << " min "
            << "\t\t\t"
            << " max "
            << "\t\t"
            << " fixed " << std::endl;
    for(unsigned long int ii = 0; ii < parameters.ampl.size(); ++ii) {
        parfile << std::scientific << parameters.ampl[ii].name << "\t\t\t"
                << std::scientific << parameters.ampl[ii].initial_value
                << "\t\t" << std::scientific << parameters.ampl[ii].min
                << "\t\t" << std::scientific << parameters.ampl[ii].max
                << "\t\t" << std::scientific << parameters.ampl[ii].fixed
                << std::endl;

        parfile << std::scientific << parameters.mean[ii].name << "\t\t\t"
                << std::scientific << parameters.mean[ii].initial_value
                << "\t\t" << std::scientific << parameters.mean[ii].min
                << "\t\t" << std::scientific << parameters.mean[ii].max
                << "\t\t" << std::scientific << parameters.mean[ii].fixed
                << std::endl;

        parfile << std::scientific << parameters.sigma[ii].name << "\t\t\t"
                << std::scientific << parameters.sigma[ii].initial_value
                << "\t\t" << std::scientific << parameters.sigma[ii].min
                << "\t\t" << std::scientific << parameters.sigma[ii].max
                << "\t\t" << std::scientific << parameters.sigma[ii].fixed
                << std::endl;

        parfile << std::scientific << parameters.tau_left[ii].name << "\t\t\t"
                << std::scientific << parameters.tau_left[ii].initial_value
                << "\t\t" << std::scientific << parameters.tau_left[ii].min
                << "\t\t" << std::scientific << parameters.tau_left[ii].max
                << "\t\t" << std::scientific << parameters.tau_left[ii].fixed
                << std::endl;

        parfile << std::scientific << parameters.tau_right[ii].name << "\t\t\t"
                << std::scientific << parameters.tau_right[ii].initial_value
                << "\t\t" << std::scientific << parameters.tau_right[ii].min
                << "\t\t" << std::scientific << parameters.tau_right[ii].max
                << "\t\t" << std::scientific << parameters.tau_right[ii].fixed
                << std::endl;
    }

    for(const auto& it : bkgInitialParameters) {
        parfile << std::scientific << it.name << "\t\t\t" << std::scientific
                << it.initial_value << "\t\t" << std::scientific << it.min
                << "\t\t" << std::scientific << it.max << "\t\t"
                << std::scientific << it.fixed << std::endl;
    }

    parfile << std::scientific << "min_max"
            << "\t\t\t" << std::scientific << parameters.left_interval
            << "\t\t" << std::scientific << parameters.right_interval
            << std::endl;

    parfile.close();
}

void Fitter::ReadParsFromFile(const std::string&  parfile_name,
                              const unsigned int& idx) {
    std::ifstream parfile(parfile_name);
    if(!parfile.is_open())
        throw std::runtime_error("Parfile: " + parfile_name + " not opened\n");

    InitializationParameters  tmp_pars;
    std::string               line;
    std::vector<InitialState> initial_states;
    bkgInitialParameters.resize(bkg_pol_order + 1);
    bool         found = false;
    unsigned int gaussCounter{0};
    unsigned int polCounter{0};
    while(std::getline(parfile, line)) {
        if(line.find("idx:") == std::string::npos && !found) continue;
        if(!found
           && std::stoi(line.substr(line.find("idx:") + 4))
                  == static_cast<int>(idx)) {
            found = true;
            continue;
        } else if(!found)
            continue;
        if(line.find("-----") == 0) break;

        if(line.find("name") != std::string::npos) continue;

        std::istringstream linestream(line);
        InitialState       tmp;
        linestream >> tmp.name >> tmp.initial_value >> tmp.min >> tmp.max
            >> tmp.fixed;

        if(tmp.name.find("Mean_") == 0)
            gaussCounter++;
        else if(tmp.name.find("pol_") == 0)
            polCounter++;

        initial_states.emplace_back(tmp);
    }

    if(gaussCounter == 0)
        throw std::runtime_error("Fitter found 0 Gaussians, try removing the "
                                 "Conf/Optimizer/parameters.dat file\n");

    parameters.Resize(gaussCounter);
    bkg_pol_order = polCounter + 1;

    for(const auto& it : initial_states) {
        if(it.name.find("Ampl_") != std::string::npos) {
            auto nr = std::stoul(it.name.substr(it.name.find("Ampl_") + 5));
            parameters.ampl[nr] = it;
        }
        if(it.name.find("Mean_") != std::string::npos) {
            auto nr = std::stoul(it.name.substr(it.name.find("Mean_") + 5));
            parameters.mean[nr] = it;
        }
        if(it.name.find("Sigma_") != std::string::npos) {
            auto nr = std::stoul(it.name.substr(it.name.find("Sigma_") + 6));
            parameters.sigma[nr] = it;
        }
        if(it.name.find("Tau_left_") != std::string::npos) {
            auto nr
                = std::stoul(it.name.substr(it.name.find("Tau_left_") + 9));
            parameters.tau_left[nr] = it;
        }
        if(it.name.find("Tau_right_") != std::string::npos) {
            auto nr
                = std::stoul(it.name.substr(it.name.find("Tau_right_") + 10));
            parameters.tau_right[nr] = it;
        }
        if(it.name.find("pol_") != std::string::npos) {
            auto nr = std::stoul(it.name.substr(it.name.find("pol_") + 4));
            bkgInitialParameters.at(nr) = it;
        }
        if(it.name.find("min_max") != std::string::npos) {
            parameters.left_interval  = it.initial_value;
            parameters.right_interval = it.min;
        }
    }
}

void Fitter::EnableCanvas(bool cv_enabled) {
    this->canvas_enabled = cv_enabled;
}

void Fitter::SetEnergyUnits(const double& val) {
    for(auto& it : energies) { it.first /= val; }
}

void Fitter::SetMeanIntervalAroundCenter(const double& min,
                                         const double& max) {
    mean_around_center_min = min;
    mean_around_center_max = max;
}

void Fitter::SetFitLimits(const double& min,
                          const double& max) {
    fit_limits = std::make_pair(min, max);
}

TH1D& Fitter::GetSpecRef() { return (TH1D&)spec; }

TF1&  Fitter::GetFitRef() { return (TF1&)fitfunc; }

void  Fitter::SetSpec(const TH1D& s) { spec = s; }

void  Fitter::SetMeanFixed(const bool& mean_fix) {
    this->mean_fixed = mean_fix;
}

void Fitter::SetRelativeAmplitudeFixed(const bool& relative_amplitude_fix) {
    this->relative_amplitude_fixed = relative_amplitude_fix;
}

void Fitter::SetBkgOrder(const int& pol_order) {
    this->bkg_pol_order = pol_order;
}
void Fitter::UseMinos(const bool& use_minos) { this->minos = use_minos; }

void Fitter::SetSigmaLimits(const std::pair<double, double>& val, const double& unit) {
    parameters.min_sigma = val.first/unit;
    parameters.max_sigma = val.second/unit;
}

void Fitter::SetTauLimits(const std::pair<double, double>& val) {
    parameters.min_tau = val.first;
    parameters.max_tau = val.second;
}

void Fitter::SetInitialSigma(const double& val, const double& unit) {
    parameters.initial_sigma = val/unit;
}

void Fitter::SetInitialTau(const double& val) { parameters.initial_tau = val; }

Fitter::~Fitter() = default;

// ClassImp(Fitter)

// Comments i'm too scared to delete

// Save as TF1 to calculate integral (this does not appear to be
// working)
// std::vector<TF1*> root_gaussians;
// for(unsigned int i = 0; i < ngauss; ++i) {
//    if(left_tail){
//    root_gaussians.push_back(smeared_gaussians[i].asTF(
//            RooArgList(x),
//            *smeared_gaussians[i].getParameters(RooArgList(x))));
//    }else {
//        root_gaussians.push_back(gaussians[i].asTF(
//                RooArgList(x),
//                *gaussians[i].getParameters(RooArgList(x))));
//    }
//}
// auto integralTest=
// root_gaussians[0]->Integral(parameters.left_interval,
//                                               parameters.right_interval,
//                                               1.e-12);
// auto integralError =
// root_gaussians[0]->IntegralError(parameters.left_interval,
//                                                      parameters.right_interval,
//                                                        nullptr,
//                                                      result->covarianceMatrix().GetMatrixArray());
// std::cout << "-----------> integral " << integralTest << " pm " <<
// integralError << std::endl;

// if(ii == ngauss - 1) {
//     double remaining_ampl       = 1.;
//     double remaining_ampl_sqerr = 0;
//     for(unsigned int jj = 0; jj < ngauss - 1; ++jj) {
//         remaining_ampl -= parameters.ampl[jj].fitted_value.first;
//         remaining_ampl_sqerr
//             += pow(parameters.ampl[jj].fitted_value.second, 2);

//        for(unsigned int kk = 0; kk < ngauss - 1; ++kk) {
//            remaining_ampl_sqerr
//                += 2 * result->correlation(parameters.ampl.at(jj).)
//        }
//    }
//    ampl     = remaining_ampl;
//    ampl_err = sqrt(remaining_ampl_sqerr);
//}
// double integral, integral_err;
////double integral     = int_signal * ampl;
////double integral_err = sqrt(
////    pow(int_signal * ampl_err, 2) + pow(int_signal_err * ampl, 2)
////    + 2 * (int_signal * ampl_err) * (int_signal_err * ampl)
////          * result->correlation(sign_y, ampl));

//        double integral_err = sqrt(
//        pow(parameters.ampl[ii].fitted_value.first *
//        parameters.common_ampl.fitted_value.second ,2)+
//                                    pow(parameters.ampl[ii].fitted_value.second
//                                    * parameters.common_ampl.fitted_value.first ,2));
//
//        double sigma =
//        sqrt(pow(parameters.sigma[ii].fitted_value.first,2)+pow(parameters.tau[ii].fitted_value.first,2));
//        double sigma_err = 1./sigma*
//                           sqrt(
//                           pow(parameters.sigma[ii].fitted_value.first*parameters.sigma[ii].fitted_value.second,2)+
//                                    pow(parameters.tau[ii].fitted_value.first*parameters.tau[ii].fitted_value.second,2)+
//                                    2*parameters.sigma[ii].fitted_value.first*parameters.tau[ii].fitted_value.first*cov[3+npars*ii][2+npars*ii]);
//
//        double sigma_gauss = parameters.sigma[ii].fitted_value.first;
//        double sigma_gauss_err =
//        parameters.sigma[ii].fitted_value.second;
//
//        double tail = parameters.tau[ii].fitted_value.first;
//        double tail_err = parameters.tau[ii].fitted_value.second;
//
// RooAbsReal
//    RooAbsMoment *aa = smeared_gaussians[0]->sigma(x);
//    std::cout << "sigma : " << aa->sigma(x)->getVal(x);
//    std::cout << " pm. " << aa->mean()->getVal(x) << std::endl;

//    RooAbsMoment * mean=
//    gaussians[0]->moment(RooArgList(x,*params[0].mean,
//    *params[0].sigma),1, false, true); std::cout << "sigma_gauss : " <<
//    mean->mean(); std::cout << " pm. " << mean->mean()->sigma(x) <<
//    std::endl;
// smeared_gaussians[0]->sigma

//    x.setRange("increased_range", parameters.left_interval - 10,
//		   parameters.right_interval + 10);
//    RooAbsReal* integral_res =
//    model.createIntegral(RooArgSet(x),RooArgSet(x),"x");

//////  double int_bkg = bkg_y->getVal()/spec.GetBinWidth(0);
//    double integral_err =
//    integral_res->getPropagatedError(*result,RooArgSet(x));

//    std::cout << "----------------->" << integral_res->getVal() << " pm
//    "<<integral_err << std::endl;
//    //RooAbsReal*
//  model_counts = model->createIntegral(x,RooFit::NormSet(x),
//  RooFit::Range("x"));
// double int_signal     = sign_y.getVal() / spec.GetBinWidth(0);
// double int_signal_err = sign_y.getError() / spec.GetBinWidth(0);
// double integral =int_signal*
// auto integralTest=
// root_smeared_gaussians[ii]->Integral(parameters.left_interval,
//                                     parameters.right_interval,
//                                     1.e-12);

// double integral_err =int_signal*

// root_smeared_gaussians[ii]->IntegralError(parameters.left_interval,
//                                                                            parameters.right_interval,
//                                                                            0,
//                                                                            result->covarianceMatrix().GetMatrixArray());

// double int_signal     = sign_y.getVal() / spec.GetBinWidth(0);
// double int_signal_err = sign_y.getError() / spec.GetBinWidth(0);
// double int_signal     = sign_y.getVal();
// double int_signal_err = sign_y.getError();
// double ampl     = parameters.ampl[ii].fitted_value.first;
// double ampl_err = parameters.ampl[ii].fitted_value.second;
