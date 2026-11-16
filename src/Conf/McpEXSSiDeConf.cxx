#include "McpEXSSiDeConf.h"

McpEXSSiDeConf::McpEXSSiDeConf()
    : CoincidenceConf("MCPEXSSIDE_CONF") {

        parB.emplace("DEBUG",
                 Conf::Property<bool&>{debug, "#",
                                       "Allow reverse Si XY position reconstruction based on given source position"});
        parD.emplace("SOURCE_X",
                 Conf::Property<double&>{sourcex, "#",
                                       "Source x position [mm]"});
        parD.emplace("SOURCE_Y",
                 Conf::Property<double&>{sourcey, "#",
                                       "Source y position [mm]"});
        parD.emplace("SOURCE_Z",
                 Conf::Property<double&>{sourcez, "#",
                                       "Source z position [mm]"});
                
        
    }
