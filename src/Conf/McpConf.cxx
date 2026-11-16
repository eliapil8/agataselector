#include "McpConf.h"

McpConf::McpConf() : DetectorConf("MCP_CONF") {
    lutFile  = "LUT_MCP.dat";
    cfd_unit = 1 * UNITS::picosecond;

    parION.emplace("ION", Conf::Property<ReactionFragment::Ion&>{
                              ion, "#", "Ion assumed in mcp"});
    parB.emplace("MANDATORY_X",
                 Conf::Property<bool&>{mandatoryX, "#",
                                       "request X for a valid mcp event"});
    parB.emplace("MANDATORY_Y",
                 Conf::Property<bool&>{mandatoryY, "#",
                                       "request Y for a valid mcp event"});
    parB.emplace("MANDATORY_TOF",
                 Conf::Property<bool&>{mandatoryTOF, "#",
                                       "request TOF for a valid mcp event"});
    parI.emplace("NR_MCP",
                 Conf::Property<int&>{nDetectors, "#",
                                       "Number of employed EXOTIC MCPs"});
    parD.emplace("BLINE1_ZMIN",
                Conf::Property<double&>{bline1zmin, "#",
                              "Start of beamline tube phi 100 mm"});
    parD.emplace("BLINE1_ZMAX",
                Conf::Property<double&>{bline1zmax, "#",
                              "End of beamline tube phi 100 mm"});
    parD.emplace("BLINE2_ZMIN",
                Conf::Property<double&>{bline2zmin, "#",
                                              "Start of beamline tube phi 50 mm "});
    parD.emplace("BLINE2_ZMAX",
                Conf::Property<double&>{bline2zmax, "#",
                                              "End of beamline tube phi 50 mm "});
    parD.emplace("BLINE2_RAD",
                Conf::Property<double&>{bline2radius, "#",
                            "Radius of beamline tube phi 50 mm "}); 
    parD.emplace("BLINE1_RAD",
                Conf::Property<double&>{bline1radius, "#",
                            "Radius of beamline tube phi 100 mm "});                                                                  
    
    parD.emplace("AVG_X1",
                 Conf::Property<double&>{avgx1, "#",
                                       "Average beam position in MCP1 X [channels]"});
    parD.emplace("AVG_X1",
                 Conf::Property<double&>{avgx1, "#",
                                       "Average beam position in MCP1 X [channels]"});
    parD.emplace("AVG_Y1",
                 Conf::Property<double&>{avgy1, "#",
                                       "Average beam position in MCP1 Y [channels]"});
    parD.emplace("AVG_X2",
                 Conf::Property<double&>{avgx2, "#",
                                       "Average beam position in MCP2 X [channels]"});
    parD.emplace("AVG_Y2",
                 Conf::Property<double&>{avgy2, "#",
                                       "Average beam position in MCP2 Y [channels]"});
    parD.emplace("SOURCE_X",
                 Conf::Property<double&>{sourcex, "#",
                                       "Source X pos [mm]"});
    parD.emplace("SOURCE_Y",
                 Conf::Property<double&>{sourcey, "#",
                                       "Source Y pos [mm]"});
    parD.emplace("SOURCE_Z",
                 Conf::Property<double&>{sourcez, "#",
                                       "Source Z pos [mm]"});
    parB.emplace("DEBUG",
                 Conf::Property<bool&>{debug, "#",
                                       "MCP2 position debug gated on MCP1"});
                                       
    parD.emplace("TOLERANCE",
                 Conf::Property<double&>{tolerance, "#",
                                       "Tolerance in [mm] to define a good event (hitting the target)"});

    parB.emplace("ENABLE_CUT_TOF",
                 Conf::Property<bool&>{enablecuttof, "#",
                                       "good events are inside the CUT_RAWX_TOF"});

    parB.emplace("ENABLE_CAL_CUT_TOF",
                 Conf::Property<bool&>{enableCalcuttof, "#",
                                       "good events are inside the CUT_X_TOF"});
    parD.emplace("DISTANCE_CENTRE_MCP1",
                 Conf::Property<double&>{dcentremcp1, "#",
                                       "distance of the centre of the 1st MCP foil in [mm]"});
    parD.emplace("DISTANCE_CENTRE_MCPS",
                 Conf::Property<double&>{dcentremcps, "#",
                                       "distance between the centres of the two MCP foils in [mm]"});
    
    parVPI.emplace("INDEX_GATES_XTOF",
                    Conf::Property<std::vector<std::pair<int, int>>&>{indexgates_xtof,"#",
                    "index (XTOF gate id, MCP id)"});

    parB.emplace("ENABLE_CUT_XY",
                 Conf::Property<bool&>{enablecutxy, "#",
                                       "good events are inside the CUT_RAWXY"});

    parVI.emplace("INDEX_GATES_XY",
                    Conf::Property<std::vector<int>&>{indexgates_xy,"#",
                    "index MCP id"});

    parVD.emplace("OFFSET_X",
                    Conf::Property<std::vector<double>&>{offx,"#",
                    "Offset x MCP 1,2"});
    parVD.emplace("OFFSET_Y",
                    Conf::Property<std::vector<double>&>{offy,"#",
                    "Offset y MCP 1,2"});
    parVD.emplace("OFFSET_Z",
                    Conf::Property<std::vector<double>&>{offz,"#",
                    "Offset z MCP 1,2"});

}
