#include "UserSelector.h"

void UserSelector::SlaveBegin(TTree* treeToRead) {
    AgataSelector::SlaveBegin(treeToRead);

    if(conf.user.eloss){

            if(conf.user.a_ranges.size() != conf.user.z_ranges.size()){
                std::cerr << "Ranges of A and Z are not the same size" << std::endl;
                return;
            }

            double thicknessC = 0.088770528 * UNITS::um ;
            double angleC = 45 * UNITS::deg;

            double thicknessPPACwindow =  1.5 * UNITS::um;

            double thicknessPPACgas =  10.8 * UNITS::mm;
            
            double thicknessICgas =  265 * UNITS::mm;

            std::ofstream * efile = new std::ofstream(conf.user.out_el_file+".dat");
            std::unordered_map<double,std::ofstream*> efiles_Athick;

            std::vector<double> betas;
            for(int i = 0; i < conf.user.n_beta; i++){
                betas.push_back(conf.user.min_beta + i * (conf.user.max_beta - conf.user.min_beta) / (conf.user.n_beta - 1));
            }

            std::pair<double,double> icAthickness = std::make_pair(conf.user.min_t_ica/UNITS::mm,conf.user.max_t_ica/UNITS::mm);
            for(double t = icAthickness.first; t <= icAthickness.second; t+=conf.user.step_t_ica/UNITS::mm){
                efiles_Athick[t]=new std::ofstream(Form("%s_Athick%3.0f.dat",conf.user.out_el_file.c_str(),t));
            }


            //efile  << "A\tZ\tBeta\tdE C\tdE W1 PPAC\tdE Gas PPAC\tdE W2 PPAC\tdE W IC\tdE IC A\tdE IC B\tdE IC C\tdE IC D"<<std::endl;


            for(unsigned int j = 0; j < betas.size(); j++){               

                TVector3 beta {0,0,betas[j]};
                for(unsigned i = 0; i < conf.user.a_ranges.size(); i++){
                    unsigned int A_min = conf.user.a_ranges[i].first;   
                    unsigned int A_max = conf.user.a_ranges[i].second;   
                    unsigned int Z_min = conf.user.z_ranges[i].first;   
                    unsigned int Z_max = conf.user.z_ranges[i].second;   
                    for(unsigned int z = Z_min; z <= Z_max; z++){
                        for(unsigned int a = A_min; a <= A_max; a++){

                            
                            ReactionFragment::FragmentSettings ion{a,z};

                            ReactionFragment fragment;
                            try {fragment = ReactionFragment(ion,conf.replay.conf_path+"/NuclearData/nutab12");} 
                            catch (const std::exception& e) {
                                if(settings.verbosity_level>2) std::cout << e.what() << std::endl;
                                continue;}
                            fragment.Set_Beta(beta);
                            double beamEn = fragment.Get_Ek();

                            std::ifstream file;

                            std::string sTempMCP = conf.replay.conf_path+conf.user.el_tables_path+"/Eloss_12C/Target-12-6_Ion-"+std::to_string(a)+"-"+std::to_string(z)+".txt";
                            file.open(sTempMCP);
                            if(!file.good()) {
                                std::cout << "Could not open file sTempMCP " << sTempMCP << std::endl;
                                continue;}
                            file.close();
                            elossPrisma.MCP_ELoss.emplace(ion,
                                    EnergyLoss(sTempMCP, "SRIM", 100));
                            auto MCP_ELossIt   = elossPrisma.MCP_ELoss.find(ion);
                            if (settings.verbosity_level > 0) std::cout << "Table file : " << sTempMCP << std::endl;


                            std::string sTempPPACwindow = conf.replay.conf_path+conf.user.el_tables_path+"/Eloss_Mylar/Target-Mylar_Ion-"+std::to_string(a)+"-"+std::to_string(z)+".txt";
                            file.open(sTempPPACwindow);
                            if(!file.good()) continue;
                            file.close();
                            elossPrisma.PPAC_window_ELoss.emplace(ion,
                                    EnergyLoss(sTempPPACwindow, "SRIM", 100));
                            auto PPAC_window_ELossIt   = elossPrisma.PPAC_window_ELoss.find(ion);
                            if (settings.verbosity_level > 0) std::cout << "Table file : " << sTempPPACwindow << std::endl;

                            std::string sTempPPACgas = conf.replay.conf_path+conf.user.el_tables_path+"/Eloss_C4H10/Target-C4H10_6mbar_Ion-"+std::to_string(a)+"-"+std::to_string(z)+".txt";
                            file.open(sTempPPACgas);
                            file.close();
                            if(!file.good()) continue;                        
                            elossPrisma.PPAC_gas_ELoss.emplace(ion,
                                    EnergyLoss(sTempPPACgas, "SRIM", 100));
                            auto PPAC_gas_ELossIt   = elossPrisma.PPAC_gas_ELoss.find(ion);
                            if (settings.verbosity_level > 0) std::cout << "Table file : " << sTempPPACgas << std::endl;

                            std::string sTempICgas = conf.replay.conf_path+conf.user.el_tables_path+"/Eloss_"+conf.user.ic_el_name+"/Target-"+conf.user.ic_el_name+"_Ion-"+std::to_string(a)+"-"+std::to_string(z)+".txt";
                            file.open(sTempICgas);
                            file.close();
                            if(!file.good()) continue;
                            elossPrisma.IC_gas_ELoss.emplace(ion,
                                    EnergyLoss(sTempICgas, "SRIM", 100));
                            auto IC_gas_ELossIt   = elossPrisma.IC_gas_ELoss.find(ion);
                            if (settings.verbosity_level > 0) std::cout << "Table file : " << sTempICgas << std::endl;


                            if(MCP_ELossIt != elossPrisma.MCP_ELoss.end() && PPAC_window_ELossIt != elossPrisma.PPAC_window_ELoss.end() && PPAC_gas_ELossIt != elossPrisma.PPAC_gas_ELoss.end() && IC_gas_ELossIt != elossPrisma.IC_gas_ELoss.end()) {

                                if(settings.verbosity_level > 0) std::cout << "Ion: " << a << " - " << z << std::endl;
                                if(settings.verbosity_level > 0) std::cout << "Initial energy : "<< beamEn / UNITS::MeV << " MeV" << std::endl;
                                if(settings.verbosity_level > 0) std::cout << "Thickness of C " << thicknessC / (UNITS::um) << " um (" << thicknessC << " ) and an angle of " << angleC / UNITS::deg << " deg"<< std::endl;
                                double afterC = MCP_ELossIt->second.Slow(beamEn , thicknessC, angleC );         
                                if(settings.verbosity_level > 0) std::cout << "Energy after C is "<< afterC / UNITS::MeV << " MeV" << std::endl;
                                if(settings.verbosity_level > 0) std::cout << "Thickness of Mylar window 1 " << thicknessPPACwindow / (UNITS::mm) << " mm (" << thicknessPPACwindow << " ) and an angle of " << 0 / UNITS::deg << " deg"<< std::endl;
                                double afterPPACw1 = PPAC_window_ELossIt->second.Slow(afterC, thicknessPPACwindow, 0 );         
                                if(settings.verbosity_level > 0) std::cout << "Energy after PPAC window 1 is "<< afterPPACw1 / UNITS::MeV << " MeV" << std::endl;
                                if(settings.verbosity_level > 0) std::cout << "Thickness of PPAC gas " << thicknessPPACgas / (UNITS::mm) << " mm (" << thicknessPPACgas << " ) and an angle of " << 0 / UNITS::deg << " deg"<< std::endl;
                                double afterPPACgas = PPAC_gas_ELossIt->second.Slow(afterPPACw1, thicknessPPACgas, 0 );         
                                if(settings.verbosity_level > 0) std::cout << "Energy after PPAC gas is "<< afterPPACgas / UNITS::MeV << " MeV" << std::endl;
                                if(settings.verbosity_level > 0) std::cout << "Thickness of Mylar window 2 " << thicknessPPACwindow / (UNITS::mm) << " mm (" << thicknessPPACwindow << " ) and an angle of " << 0 / UNITS::deg << " deg"<< std::endl;
                                double afterPPACw2 = PPAC_window_ELossIt->second.Slow(afterPPACgas, thicknessPPACwindow, 0 );         
                                if(settings.verbosity_level > 0) std::cout << "Energy after PPAC window 2 is "<< afterPPACw2 / UNITS::MeV << " MeV" << std::endl;
                                if(settings.verbosity_level > 0) std::cout << "Thickness of Mylar window 3 " << thicknessPPACwindow / (UNITS::mm) << " mm (" << thicknessPPACwindow << " ) and an angle of " << 0 / UNITS::deg << " deg"<< std::endl;
                                double afterICw = PPAC_window_ELossIt->second.Slow(afterPPACw2, thicknessPPACwindow, 0 );         
                                if(settings.verbosity_level > 0) std::cout << "Energy after IC window is "<< afterICw / UNITS::MeV << " MeV" << std::endl;
                                
                                
                                for(double t = icAthickness.first; t <= icAthickness.second; t+=conf.user.step_t_ica/UNITS::mm){
                                    
                                    if(t*UNITS::mm > thicknessICgas){
                                        if(settings.verbosity_level > 0) std::cout << "Thickness of IC gas before A " << t - thicknessICgas << " mm and an angle of " << 0 / UNITS::deg << " deg " << std::endl;
                                        double afterICGas = IC_gas_ELossIt->second.Slow(afterICw, t*UNITS::mm-thicknessICgas, 0 );
                                        if(settings.verbosity_level > 0) std::cout << "Energy after IC gas before A is "<< afterICGas / UNITS::MeV << " MeV" << std::endl;
                                        
                                        if(settings.verbosity_level > 0) std::cout << "Thickness of IC gas A " << thicknessICgas << " mm and an angle of " << 0 / UNITS::deg << " deg " << std::endl;
                                        double afterICA = IC_gas_ELossIt->second.Slow(afterICGas, thicknessICgas, 0 ); 
                                        if(settings.verbosity_level > 0) std::cout << "Energy after IC gas A is "<< afterICA / UNITS::MeV << " MeV" << std::endl;
                                        *efiles_Athick[t]  << a << " " << z << " " << beta.Z() << " " << (beamEn-afterC) / UNITS::MeV << " " << (afterC-afterPPACw1) / UNITS::MeV << " " << (afterPPACw1-afterPPACgas) / UNITS::MeV << " " << (afterPPACgas-afterPPACw2) / UNITS::MeV << " " << (afterPPACw2-afterICGas) / UNITS::MeV << " " << (afterICGas-afterICA) / UNITS::MeV << " " << afterICA / UNITS::MeV << std::endl;                               

                                    }
                                    else{
                                        if(settings.verbosity_level > 0) std::cout << "Thickness of IC gas A " << t << " mm and an angle of " << 0 / UNITS::deg << " deg " << std::endl;
                                        double afterICA = IC_gas_ELossIt->second.Slow(afterICw, t*UNITS::mm, 0 ); 
                                        if(settings.verbosity_level > 0) std::cout << "Energy after IC gas A is "<< afterICA / UNITS::MeV << " MeV" << std::endl;
                                        *efiles_Athick[t]  << a << " " << z << " " << beta.Z() << " " << (beamEn-afterC) / UNITS::MeV << " " << (afterC-afterPPACw1) / UNITS::MeV << " " << (afterPPACw1-afterPPACgas) / UNITS::MeV << " " << (afterPPACgas-afterPPACw2) / UNITS::MeV << " " << (afterPPACw2-afterICw) / UNITS::MeV << " " << (afterICw-afterICA) / UNITS::MeV << " " << afterICA / UNITS::MeV << std::endl;                               
                                    }
                                }
                                
                                //if(settings.verbosity_level > 0) std::cout << "Thickness of IC gas A" << thicknessICgas / (UNITS::mm) << " mm (" << thicknessICgas << " ) and an angle of " << 0 / UNITS::deg << " deg"<< std::endl;
                                //double afterICA = IC_gas_ELossIt->second.Slow(afterICw, thicknessICgas, 0 ); 
                                //if(settings.verbosity_level > 0) std::cout << "Energy after IC gas A is "<< afterICA / UNITS::MeV << " MeV" << std::endl;
                                //if(settings.verbosity_level > 0) std::cout << "Thickness of IC gas B " << thicknessICgas / (UNITS::mm) << " mm (" << thicknessICgas << " ) and an angle of " << 0 / UNITS::deg << " deg"<< std::endl;
                                //double afterICB = IC_gas_ELossIt->second.Slow(afterICA, thicknessICgas, 0 );         
                                //if(settings.verbosity_level > 0) std::cout << "Energy after IC gas B is "<< afterICB / UNITS::MeV << " MeV" << std::endl;
                                //if(settings.verbosity_level > 0) std::cout << "Thickness of IC gas C " << thicknessICgas / (UNITS::mm) << " mm (" << thicknessICgas << " ) and an angle of " << 0 / UNITS::deg << " deg"<< std::endl;
                                //double afterICC = IC_gas_ELossIt->second.Slow(afterICB, thicknessICgas, 0 );         
                                //if(settings.verbosity_level > 0) std::cout << "Energy after IC gas C is "<< afterICC / UNITS::MeV << " MeV" << std::endl;
                                //if(settings.verbosity_level > 0) std::cout << "Thickness of IC gas D " << thicknessICgas / (UNITS::mm) << " mm (" << thicknessICgas << " ) and an angle of " << 0 / UNITS::deg << " deg"<< std::endl;
                                //double afterICD = IC_gas_ELossIt->second.Slow(afterICC, thicknessICgas, 0 );         
                                //if(settings.verbosity_level > 0) std::cout << "Energy after IC gas D is "<< afterICD / UNITS::MeV << " MeV" << std::endl;
                                //efile  << a << " " << z << " " << beta.Z() << " " << (beamEn-afterC) / UNITS::MeV << " " << (afterC-afterPPACw1) / UNITS::MeV << " " << (afterPPACw1-afterPPACgas) / UNITS::MeV << " " << (afterPPACgas-afterPPACw2) / UNITS::MeV << " " << (afterPPACw2-afterICw) / UNITS::MeV << " " << (afterICw-afterICA) / UNITS::MeV << " " << (afterICA-afterICB) / UNITS::MeV << " " << (afterICB-afterICC) / UNITS::MeV  << " " << (afterICC-afterICD) / UNITS::MeV  << std::endl;                               

                            }


                        }
                    }
                }
                //efile <<  std::endl;
                for(double t = icAthickness.first; t <= icAthickness.second; t+=5) *efiles_Athick[t] << std::endl;

            }

        }
    
}

Bool_t UserSelector::Process(Long64_t entry) {

    return kTRUE;
}

void UserSelector::SlaveTerminate() {
    AgataSelector::SlaveTerminate();
    // User terminate
}
