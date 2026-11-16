#include "McpMonitorConf.h"

McpMonitorConf::McpMonitorConf()
    : CoincidenceConf("MCPMONITOR_CONF") {
                
        
    parB.emplace("ENABLE_CUT_ETOF",
                 Conf::Property<bool&>{enablecutetof, "#",
                                       "good events are inside the CUT_E_TOF"});
             
    parVPI.emplace("ID_CUT_ETOF",
                 Conf::Property<std::vector<std::pair<int, int>>&>{cut_etof_ids, "#",
                                       "IDs for E_TOF cuts (TOF id, Monitor ID)"});

    }
