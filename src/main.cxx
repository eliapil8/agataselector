#include <iostream>
#include <unistd.h>

#include <TApplication.h>

#include "App.h"
#include "ConfManager.h"
#include "Optimizer.h"
#include "Settings.h"
#include "GateSelector.h"

#ifdef USE_MPI
#include "MpiManager.h"

#include <boost/mpi/environment.hpp>
#endif

int main(int argc, char* argv[]) {
#ifdef USE_MPI
    // The mpi environment is set up in the singleton MpiManager class
    MpiManager::getInstance();
    // system("echo $PATH");
    // system("echo $LD_LIBRARY_PATH");
    // system("echo $ROOTSYS");
#endif

    // Initialize singletons
    Settings& settings = Settings::getInstance();
    settings.parseOptions(argc, argv);
    
    // If no args simply print help
    if(argc == 1 || settings.printHelp) {
        settings << std::cout;
        return 0;
    }

    // Initialize ConfManager
    try{
        ConfManager::getInstance();
    }catch (std::runtime_error& e){
        // Print default conf file of necessary
        if(!settings.printFileName.empty()) {
            std::cout << "Printing default conf to file: "
                      << settings.printFileName << "\n";
            ConfManager::getInstance().PrintConf(settings.printFileName);
            return 0;
        }else {
				std::cout << e.what() << std::endl;
				throw std::runtime_error("Could not initialize ConfManager");
			}
    }
    ConfManager& conf = ConfManager::getInstance();    

    if(!settings.printFileName.empty()) {
        std::cout << "Printing default conf to file: "
                  << settings.printFileName << "\n";
        ConfManager::getInstance().PrintConf(settings.printFileName);
        return 0;
    }


    if(!settings.set_gates.empty()){
        TApplication const root_app("GateSetter", &argc, argv);
            if(conf.detectorsPresent.Prisma){
                std::cout << "Setting gates for prisma : " << conf.detectorsPresent.Prisma<< std::endl;
                GateSelector<1,1>::SetGates(settings.set_gates, "Prisma/Raw/MCP", "m_MCPX_MCPY_raw", conf.replay.conf_path+conf.prisma.mcpGatePath+"/", "mcp");
                GateSelector<1,1>::SetGates(settings.set_gates, "Prisma/Ana", "m_TOF_XFP", conf.replay.conf_path+conf.prisma.TOFGatePath+"/", "tofvsxfp");
                switch (conf.prisma.de_type)
                {
                case 0:
                    GateSelector<1,1>::SetGates(settings.set_gates, "Prisma/Ana/DE_E", "m_ICDEAB_ICE_tot_0", conf.replay.conf_path+conf.prisma.zGatesPath+"/", "deabvse");
                    break;
                case 1:
                    GateSelector<1,1>::SetGates(settings.set_gates, "Prisma/Ana/DE_E", "m_ICDEA_ICE_tot_1", conf.replay.conf_path+conf.prisma.zGatesPath+"/", "deavse");
                    break;
                case 3:
                    GateSelector<1,1>::SetGates(settings.set_gates, "Prisma/Ana/DE_E", "m_ICDEA_ICEres_corr_tot_3", conf.replay.conf_path+conf.prisma.zGatesPath+"/", "deabvseres");
                    break;
                default:
                    break;
                }
                switch (conf.prisma.charge_type)
                {
                case 0:
                    GateSelector<2,1>::SetGates(settings.set_gates, "Prisma/Ana/ICE_RBeta", "m_ICE_RBeta_0", conf.replay.conf_path+conf.prisma.qGatesPath+"/", "evsrbeta");
                    break;
                case 1:
                    GateSelector<2,1>::SetGates(settings.set_gates, "Prisma/Ana/ICE_RBeta", "m_ICEoverRBeta_ICE_1", conf.replay.conf_path+conf.prisma.qGatesPath+"/", "erbetavse");
                    break;
                case 2:
                    GateSelector<2,1>::SetGates(settings.set_gates, "Prisma/Ana/ICE_XFP", "m_ICE_XFP_", conf.replay.conf_path+conf.prisma.qGatesPath+"/", "evsxfp");
                    break;
                default:
                    break;
                }
                
                GateSelector<2,1, TGraph>::SetGates(settings.set_gates, "Prisma/Ana/Aoverq/ZQ_Gated/2D_Xmcp","m_Aoverq_Xmcp", conf.replay.conf_path+conf.prisma.aoqCorrXmcpPath+"/", "aoqvsxmcp");
                GateSelector<2,1, TGraph>::SetGates(settings.set_gates, "Prisma/Ana/Aoverq/ZQ_Gated/2D_Ymcp","m_Aoverq_Ymcp", conf.replay.conf_path+conf.prisma.aoqCorrYmcpPath+"/", "aoqvsymcp");
                GateSelector<2,1, TGraph>::SetGates(settings.set_gates, "Prisma/Ana/Aoverq/ZQ_Gated/2D_Xfp", "m_Aoverq_Xfp",  conf.replay.conf_path+conf.prisma.aoqCorrXfpPath+"/",  "aoqvsxfp");
            }
            if(conf.detectorsPresent.Dante){
                GateSelector<1,1>::SetGates(settings.set_gates, "Dante", "h_tof_Theta", conf.replay.conf_path+"/CUT/DANTE/", "dantetof");

                if(conf.detectorsPresent.Prisma && (conf.detectorsPresent.Agata || conf.detectorsPresent.Labr)){
                    GateSelector<1,1>::SetGates(settings.set_gates, "PrismaDante/TOF", "m_TOF_Qval", conf.replay.conf_path+"/CUT/PRISMA_DANTE/TOF_QVAL/", "dantetofqval");
                }
            }
            if(conf.detectorsPresent.SauronFw){
                GateSelector<2,2>::SetGates(settings.set_gates, "SauronFw/PID", "m_Imax_Energy", conf.replay.conf_path+"/CUT/SAURONFW/EImax/", "evsimax");
            }
            if(conf.detectorsPresent.SauronBw){
                GateSelector<2,2>::SetGates(settings.set_gates, "SauronBw/PID", "m_Imax_Energy", conf.replay.conf_path+"/CUT/SAURONBW/EImax/", "evsimax");
            }
            if(conf.detectorsPresent.Mcp){
                GateSelector<2,1>::SetGates(settings.set_gates, "Mcp", "m_tof_mcpRawX", conf.replay.conf_path+"/CUT/MCP/", "rawxmcptof");
                GateSelector<2,1>::SetGates(settings.set_gates, "Mcp", "m_tof_mcpX", conf.replay.conf_path+"/CUT/MCP/", "xmcptof");
                GateSelector<1,1>::SetGates(settings.set_gates, "Mcp", "m_XY_raw", conf.replay.conf_path+"/CUT/MCP/","rawxy");
                if(conf.detectorsPresent.Monitor){
                    GateSelector<2,1>::SetGates(settings.set_gates, "McpMonitor", "m_Energy_TOF_monitorID", conf.replay.conf_path+"/CUT/MCP/","etof");
                }
            }
        return 0;
    }

    // Launch either simple selector or optimizer
    if(!settings.optimize) {
        const App app;
    } else {
        TApplication const root_app("Optimizer", &argc, argv);
        Optimizer const    optimizer;
    }

#ifdef USE_MPI
    if(settings.distribute) return EXIT_SUCCESS;
#endif
    return 0;
}
