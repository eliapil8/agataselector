#define AgataSelector_cxx
#include "AgataSelector.h"
#include "Header.h"

#include <TStyle.h>
#include <string>
#include <string_view>

void AgataSelector::Begin(TTree * /*tree*/)
{
    TString option = GetOption();
}

void AgataSelector::SlaveBegin(TTree *treeToRead)
{
    TString option = GetOption();

    progress->store(-1.);

    midTargetBeamEnergy = conf.reaction.energy;

    fChain = treeToRead;

    // If beam->target eloss table is present use it!
    std::string sTemp = conf.replay.conf_path + "/EnergyLoss/Target-" + conf.target.material +
                        "_Ion-" + std::to_string(conf.reaction.beam.A) + "-" +
                        std::to_string(conf.reaction.beam.Z) + ".txt";
    if (settings.verbosity_level > 0)
    {
        std::cout << "Target: ELoss file: " << sTemp << " - is present: " << FileIsPresent(sTemp)
                  << std::endl;
    }

    std::string elossTableType{"SRIM"};
    switch (conf.target.elossTableType)
    {
    case 0:
        elossTableType = "SRIM";
        break;
    case 1:
        elossTableType = "G4Table";
        break;
    case 2:
        elossTableType = "LISE";
        break;
    default:
        throw std::runtime_error("Type of energy loss table type does not make sense\n");
    }

    if (FileIsPresent(sTemp))
    {
        targetELoss.emplace(conf.reaction.beam, EnergyLoss(sTemp, elossTableType, 100));
    }

    sTemp = conf.replay.conf_path + "/EnergyLoss/Target-" + conf.target.degMaterial + "_Ion-" +
            std::to_string(conf.reaction.beam.A) + "-" + std::to_string(conf.reaction.beam.Z) +
            ".txt";

    if (conf.target.degPresent && settings.verbosity_level > 0)
    {
        std::cout << "Degrader: ELoss file: " << sTemp << " - is present: " << FileIsPresent(sTemp)
                  << std::endl;
    }
    if (FileIsPresent(sTemp))
    {
        degraderELoss.emplace(conf.reaction.beam, EnergyLoss(sTemp, elossTableType, 100));
    }

    auto degraderELossIt = degraderELoss.find(conf.reaction.beam);
    auto targetELossIt   = targetELoss.find(conf.reaction.beam);

    if (conf.target.degPresent && conf.target.degPos == TargetConf::DegPos::BEFORE &&
        degraderELossIt != degraderELoss.end())
    {
        if (settings.verbosity_level > 0)
        {
            std::cout << "Target energy before degrader is " << midTargetBeamEnergy / UNITS::MeV
                      << " MeV with a degrader thickness of "
                      << conf.target.degThickness / (UNITS::mg / UNITS::cm2) << " mg/cm2 ("
                      << conf.target.degThickness << " ) and an angle of "
                      << conf.target.targetNorm.Theta() / UNITS::deg << " deg" << std::endl;
        }

        midTargetBeamEnergy = degraderELossIt->second.SlowWithDensity(
            midTargetBeamEnergy, conf.target.degThickness,
            TVector3(0, 0, 1).Angle(conf.target.targetNorm)); // TODO check
    }

    if (targetELossIt != targetELoss.end())
    {
        if (settings.verbosity_level > 0)
        {
            std::cout << "Target energy before target is " << midTargetBeamEnergy / UNITS::MeV
                      << " MeV with a target thickness of "
                      << conf.target.thickness / (UNITS::mg / UNITS::cm2) << " mg/cm2 ("
                      << conf.target.thickness << " ) and an angle of "
                      << conf.target.targetNorm.Theta() / UNITS::deg << " deg" << std::endl;
        }
        midTargetBeamEnergy = targetELossIt->second.SlowWithDensity(
            midTargetBeamEnergy, conf.target.thickness * conf.reaction.reaction_position,
            TVector3(0, 0, 1).Angle(conf.target.targetNorm)); // TODO check
    }

    if (settings.verbosity_level > 0)
    {
        std::cout << "Mid target energy " << midTargetBeamEnergy / UNITS::MeV << std::endl;
    }

    // Load reactions and ELoss table
    for (const auto &it : conf.reaction.fragmentsOfInterest)
    {
        // Reaction reconstruction part
        reactions.emplace(
            std::piecewise_construct, std::forward_as_tuple(it.first),
            std::forward_as_tuple(
                ReactionReconstruction2body<long double>::ReactionInput2body{
                    ReactionFragment::FragmentSettings(conf.reaction.beam.A, conf.reaction.beam.Z,
                                                       0, midTargetBeamEnergy, 0 * UNITS::MeV),
                    ReactionFragment::FragmentSettings(conf.reaction.target.A,
                                                       conf.reaction.target.Z, 0, 0 * UNITS::MeV,
                                                       0 * UNITS::MeV),
                    ReactionFragment::FragmentSettings(
                        conf.reaction.beam.A + conf.reaction.target.A - it.first.A,
                        conf.reaction.beam.Z + conf.reaction.target.Z - it.first.Z, 0,
                        0 * UNITS::MeV, 0 * UNITS::MeV),
                    ReactionFragment::FragmentSettings(it.first.A, it.first.Z, 0, 0,
                                                       0 * UNITS::MeV)},
                conf.replay.conf_path + "/NuclearData/nutab12"));
        reactions.at(it.first).ChooseFixed(4);
        reactions.at(it.first).ChooseExFixed(4);
        ReactionFragment::Ion ionBP;
        ionBP.A = conf.reaction.beam.A + conf.reaction.target.A - it.first.A;
        ionBP.Z = conf.reaction.beam.Z + conf.reaction.target.Z - it.first.Z;

        // Energy loss tables
        sTemp = conf.replay.conf_path + "/EnergyLoss/Target-" + conf.target.material + "_Ion-" +
                std::to_string(it.first.A) + "-" + std::to_string(it.first.Z) + ".txt";

        if (settings.verbosity_level > 0)
        {
            std::cout << "Target: ELoss file: " << sTemp
                      << " - is present: " << FileIsPresent(sTemp) << std::endl;
        }
        if (FileIsPresent(sTemp))
        {
            targetELoss.emplace(it.first, EnergyLoss(sTemp, elossTableType, 100));
        }

        sTemp = conf.replay.conf_path + "/EnergyLoss/Target-" + conf.target.material + "_Ion-" +
                std::to_string(ionBP.A) + "-" + std::to_string(ionBP.Z) + ".txt";

        if (settings.verbosity_level > 0)
        {
            std::cout << "Target: ELoss file: " << sTemp
                      << " - is present: " << FileIsPresent(sTemp) << std::endl;
        }
        if (FileIsPresent(sTemp))
        {
            targetELoss.emplace(ionBP, EnergyLoss(sTemp, elossTableType, 100));
        }

        sTemp = conf.replay.conf_path + "/EnergyLoss/Target-" + conf.target.degMaterial + "_Ion-" +
                std::to_string(it.first.A) + "-" + std::to_string(it.first.Z) + ".txt";
        if (conf.target.degPresent && settings.verbosity_level > 0)
        {
            std::cout << "Degrader: ELoss file: " << sTemp
                      << " - is present: " << FileIsPresent(sTemp) << std::endl;
        }
        if (conf.target.degPresent && FileIsPresent(sTemp))
        {
            degraderELoss.emplace(it.first, EnergyLoss(sTemp, elossTableType, 100));
        }

        sTemp = conf.replay.conf_path + "/EnergyLoss/Target-" + conf.target.degMaterial + "_Ion-" +
                std::to_string(ionBP.A) + "-" + std::to_string(ionBP.Z) + ".txt";
        if (conf.target.degPresent && settings.verbosity_level > 0)
        {
            std::cout << "Degrader: ELoss file: " << sTemp
                      << " - is present: " << FileIsPresent(sTemp) << std::endl;
        }
        if (conf.target.degPresent && FileIsPresent(sTemp))
        {
            degraderELoss.emplace(ionBP, EnergyLoss(sTemp, elossTableType, 100));
        }
    }

    // Load reactions and ELoss tables for n body reactions
    std::vector<ReactionFragment::FragmentSettings> settingsBase;
    settingsBase.emplace_back(conf.reaction.beam.A, conf.reaction.beam.Z, 0, midTargetBeamEnergy,
                              0);
    settingsBase.emplace_back(conf.reaction.target.A, conf.reaction.target.Z, 0, 0, 0);
    for (const auto &it : conf.reaction.nBodyFragmentsOfInterest)
    {
        // Reaction reconstruction part
        auto tmpSettings = settingsBase;

        ReactionFragment::Ion ionCompNucl; // Compoud nucleus
        ionCompNucl.A = conf.reaction.beam.A + conf.reaction.target.A;
        ionCompNucl.Z = conf.reaction.beam.Z + conf.reaction.target.Z;

        for (const auto &it2 : it.first)
        {
            tmpSettings.emplace_back(it2.A, it2.Z);

            // Particle evaporation from compound nucleus
            ionCompNucl.A -= it2.A;
            ionCompNucl.Z -= it2.Z;

            // Add energy loss table for evaporated particle in the target
            sTemp = conf.replay.conf_path + "/EnergyLoss/Target-" + conf.target.material + "_Ion-" +
                    std::to_string(it2.A) + "-" + std::to_string(it2.Z) + ".txt";

            if (settings.verbosity_level > 0)
            {
                std::cout << "Target: ELoss file: " << sTemp
                          << " - is present: " << FileIsPresent(sTemp) << std::endl;
            }
            if (FileIsPresent(sTemp))
            {
                targetELoss.emplace(it2, EnergyLoss(sTemp, elossTableType, 100));
            }

            // Add energy loss table for evaporated particle in the degrader
            sTemp = conf.replay.conf_path + "/EnergyLoss/Target-" + conf.target.degMaterial +
                    "_Ion-" + std::to_string(it2.A) + "-" + std::to_string(it2.Z) + ".txt";
            if (conf.target.degPresent && settings.verbosity_level > 0)
            {
                std::cout << "Degrader: ELoss file: " << sTemp
                          << " - is present: " << FileIsPresent(sTemp) << std::endl;
            }
            if (conf.target.degPresent && FileIsPresent(sTemp))
            {
                degraderELoss.emplace(it2, EnergyLoss(sTemp, "SRIM", 100));
            }
        }

        // Add energy loss table for compound nucleus after evaporation in the
        // target
        sTemp = conf.replay.conf_path + "/EnergyLoss/Target-" + conf.target.material + "_Ion-" +
                std::to_string(ionCompNucl.A) + "-" + std::to_string(ionCompNucl.Z) + ".txt";

        if (settings.verbosity_level > 0)
        {
            std::cout << "Target: ELoss file: " << sTemp
                      << " - is present: " << FileIsPresent(sTemp) << std::endl;
        }
        if (FileIsPresent(sTemp))
        {
            targetELoss.emplace(ionCompNucl, EnergyLoss(sTemp, elossTableType, 100));
        }

        // Add energy loss table for compound nucleus after evaporation in the
        // degrader
        sTemp = conf.replay.conf_path + "/EnergyLoss/Target-" + conf.target.degMaterial + "_Ion-" +
                std::to_string(ionCompNucl.A) + "-" + std::to_string(ionCompNucl.Z) + ".txt";
        if (conf.target.degPresent && settings.verbosity_level > 0)
        {
            std::cout << "Degrader: ELoss file: " << sTemp
                      << " - is present: " << FileIsPresent(sTemp) << std::endl;
        }
        if (conf.target.degPresent && FileIsPresent(sTemp))
        {
            degraderELoss.emplace(ionCompNucl, EnergyLoss(sTemp, elossTableType, 100));
        }

        if (settings.verbosity_level > 0)
        {
            std::cout << "Adding N body reaction :";
            for (const auto &it2 : it.first)
            {
                std::cout << " A=" << it2.A << " Z=" << it2.Z;
            }
            std::cout << std::endl;
        }

        nBodyReactions.emplace(
            std::piecewise_construct, std::forward_as_tuple(it.first),
            std::forward_as_tuple(tmpSettings, conf.replay.conf_path + "/NuclearData/nutab12"));
    }

    // Read list of enabled histos if present
    {
        const std::string enabledHistosPath =
            conf.replay.conf_path + "/" + conf.replay.enabled_histo_file_name;
        std::ifstream enabledHistosFile(enabledHistosPath);
        if (enabledHistosFile.is_open())
        {
            if (settings.verbosity_level > 0)
                std::cout << "enabled histo file called " << conf.replay.enabled_histo_file_name
                          << " is present" << std::endl;
            std::string line;
            std::size_t lineNumber{0};
            while (std::getline(enabledHistosFile, line))
            {
                ++lineNumber;

                const auto firstCharacter = line.find_first_not_of(" \t\r\n");
                if (firstCharacter == std::string::npos || line[firstCharacter] == '#')
                    continue;

                line.erase(0, firstCharacter);
                const auto lastCharacter = line.find_last_not_of(" \t\r\n");
                line.erase(lastCharacter + 1);

                constexpr std::string_view regexMarker{"#regex"};
                if (line.size() >= regexMarker.size() &&
                    line.compare(line.size() - regexMarker.size(), regexMarker.size(),
                                 regexMarker) == 0)
                {
                    line.erase(line.size() - regexMarker.size());
                    const auto expressionEnd = line.find_last_not_of(" \t\r\n");
                    if (expressionEnd == std::string::npos)
                    {
                        throw std::runtime_error("Empty regex in enabled histogram file " +
                                                 enabledHistosPath + ":" +
                                                 std::to_string(lineNumber) + "\n");
                    }
                    line.erase(expressionEnd + 1);

                    try
                    {
                        enabledHistoRegexes.emplace_back(line);
                    }
                    catch (const std::regex_error &error)
                    {
                        throw std::runtime_error("Invalid regex in enabled histogram file " +
                                                 enabledHistosPath + ":" +
                                                 std::to_string(lineNumber) + ": " + error.what() +
                                                 "\n");
                    }
                }
                else
                {
                    enabledHistos.emplace_back(std::move(line));
                }
            }
        }
        else
        {
            if (settings.only_enabled_histos)
                throw std::runtime_error("only_enabled_histos requested but no file found\n");
        }
    }

    // Define here output file, so it can flush histograms to disk
    // outFile = TFile(option, "recreate");
    outFile.cd();

    outFile.mkdir("Configurations")->cd();
    Header header;
    header.CreateHeader(settings.confFileName);
    header.Write(outFile.GetName());

    // Allocate tree of analyzed data
    if (conf.agata.enable_treeFill || conf.prisma.enable_treeFill ||
        conf.prisma.enable_preprotreeFill || conf.spider.enable_treeFill ||
        conf.sauronFw.enable_treeFill || conf.sauronBw.enable_treeFill ||
        conf.monitor.enable_treeFill || conf.exsside.enable_treeFill ||
        conf.dante.enable_treeFill || conf.oscarUp.enable_treeFill ||
        conf.oscarDown.enable_treeFill || conf.euclides.enable_treeFill ||
        conf.labr.enable_treeFill || conf.mcp.enable_treeFill || conf.monitor.enable_treeFill)
    {
        tree =
            new TTree("AnalyzedTree", "Tree Analyzed by Agata selector for near online analysis");
        tree->SetDirectory(&outFile);
        if (settings.verbosity_level > 0)
            std::cout << "AnalyzedTree will be filled" << std::endl;
    }

    DetectorAnalysis::Inputs inputs{
        tree,
        fChain,
        fReader,
        outFile,
        allHistos,
        enabledHistos,
        enabledHistoRegexes,
        firstTS,
        midTargetBeamEnergy,
        type,
        reactions,
        nBodyReactions,
        targetELoss,
        degraderELoss};

    // AGATA-only variables
    if (conf.detectorsPresent.Agata)
    {
        detectorAnaList.emplace_back(agataAnalysis = new AgataAnalysis(inputs, agataOutput));
        if (settings.verbosity_level > 0)
            std::cout << "agata initialised" << std::endl;
    }

    // Labr-only variables
    if (conf.detectorsPresent.Labr)
    {
        detectorAnaList.emplace_back(labrAnalysis = new LabrAnalysis(inputs, labrOutput));
        header.AddLut(conf.replay.conf_path + "/LUT/" + conf.labr.lutFile, "LABR");
        if (settings.verbosity_level > 0)
            std::cout << "labr initialised" << std::endl;
    }

    // PRISMA-only variables
    if (conf.detectorsPresent.Prisma)
    {
        detectorAnaList.emplace_back(
            prismaAnalysis = new PrismaAnalysis(inputs, prismaOutput, prismaInputOutput));
        if (settings.verbosity_level > 0)
            std::cout << "prisma initialised" << std::endl;
    }

    // SPIDER-only variables
    if (conf.detectorsPresent.Spider)
    {
        detectorAnaList.emplace_back(spiderAnalysis = new SpiderAnalysis(inputs, spiderOutput));
        header.AddLut(conf.replay.conf_path + "/LUT/" + conf.spider.lutFile, "SPIDER");
        if (settings.verbosity_level > 0)
            std::cout << "spider initialised" << std::endl;
    }

    // MONITOR-only variables
    if (conf.detectorsPresent.Monitor)
    {
        detectorAnaList.emplace_back(monitorAnalysis = new MonitorAnalysis(inputs, monitorOutput));
        header.AddLut(conf.replay.conf_path + "/LUT/" + conf.monitor.lutFile, "MONITOR");
        if (settings.verbosity_level > 0)
            std::cout << "monitor initialised" << std::endl;
    }

    // SAURON-only variables
    if (conf.detectorsPresent.SauronFw)
    {
        detectorAnaList.emplace_back(sauronFwAnalysis =
                                         new SauronFwAnalysis(inputs, sauronFwOutput));
        header.AddLut(conf.replay.conf_path + "/LUT/" + conf.sauronFw.lutFile, "SAURONFW");
        if (settings.verbosity_level > 0)
            std::cout << "sauron fw initialised" << std::endl;
    }

    // SAURON-only variables
    if (conf.detectorsPresent.SauronBw)
    {
        detectorAnaList.emplace_back(sauronBwAnalysis =
                                         new SauronBwAnalysis(inputs, sauronBwOutput));
        header.AddLut(conf.replay.conf_path + "/LUT/" + conf.sauronBw.lutFile, "SAURONBW");
        if (settings.verbosity_level > 0)
            std::cout << "sauron bw initialised" << std::endl;
    }

    // EXSSiDe-only variables
    if (conf.detectorsPresent.EXSSiDe)
    {
        detectorAnaList.emplace_back(exssideAnalysis = new EXSSiDeAnalysis(inputs, exssideOutput));
        header.AddLut(conf.replay.conf_path + "/LUT/" + conf.exsside.lutFile, "EXSSiDe");
        if (settings.verbosity_level > 0)
            std::cout << "exsside initialised" << std::endl;
    }

    // OSCAR-only variables
    if (conf.detectorsPresent.OscarUp)
    {
        detectorAnaList.emplace_back(oscarUpAnalysis = new OscarUpAnalysis(inputs, oscarUpOutput));
        header.AddLut(conf.replay.conf_path + "/LUT/" + conf.oscarUp.lutFile, "OSCARUP");
        if (settings.verbosity_level > 0)
            std::cout << "oscar up initialised with lut "
                      << conf.replay.conf_path + "/LUT/" + conf.oscarUp.lutFile << std::endl;
    }

    // OSCAR-only variables
    if (conf.detectorsPresent.OscarDown)
    {
        detectorAnaList.emplace_back(oscarDownAnalysis =
                                         new OscarDownAnalysis(inputs, oscarDownOutput));
        header.AddLut(conf.replay.conf_path + "/LUT/" + conf.oscarDown.lutFile, "OSCARDOWN");
        if (settings.verbosity_level > 0)
            std::cout << "oscar down initialised with lut "
                      << conf.replay.conf_path + "/LUT/" + conf.oscarDown.lutFile << std::endl;
    }

    // DANTE-only variables
    if (conf.detectorsPresent.Dante)
    {
        detectorAnaList.emplace_back(danteAnalysis = new DanteAnalysis(inputs, danteOutput));
        header.AddLut(conf.replay.conf_path + "/LUT/" + conf.dante.lutFile, "DANTE");
        if (settings.verbosity_level > 0)
            std::cout << "dante initialised" << std::endl;
    }

    // MCP-only variables
    if (conf.detectorsPresent.Mcp)
    {
        detectorAnaList.emplace_back(mcpAnalysis = new McpAnalysis(inputs, mcpOutput));
        header.AddLut(conf.replay.conf_path + "/LUT/" + conf.mcp.lutFile, "MCP");
        if (settings.verbosity_level > 0)
            std::cout << "Exotic MCP initialised" << std::endl;
    }

    // EUCLIDES-only variables
    if (conf.detectorsPresent.Euclides)
    {
        detectorAnaList.emplace_back(euclidesAnalysis =
                                         new EuclidesAnalysis(inputs, euclidesOutput));
        header.AddLut(conf.replay.conf_path + "/LUT/" + conf.euclides.lutFile, "EUCLIDES");
        if (settings.verbosity_level > 0)
            std::cout << "euclides initialised" << std::endl;
    }

    // Agata & Dante variables
    if (conf.detectorsPresent.Agata && conf.detectorsPresent.Dante)
    {
        detectorAnaList.emplace_back(
            new AgataDanteAnalysis(inputs, agataOutput, danteOutput, agataAnalysis->agataReader,
                                   danteAnalysis->danteReader, gammaAgataDante));
        if (settings.verbosity_level > 0)
            std::cout << "agata+dante initialised" << std::endl;
    }

    // Agata & Euclides variables
    if (conf.detectorsPresent.Agata && conf.detectorsPresent.Euclides)
    {
        detectorAnaList.emplace_back(new AgataEuclidesAnalysis(
            inputs, agataOutput, euclidesOutput, agataAnalysis->agataReader,
            euclidesAnalysis->euclidesReader, gammaAgataEuclides));
        if (settings.verbosity_level > 0)
            std::cout << "agata+euclides initialised" << std::endl;
    }

    // Agata & Labr variables
    if (conf.detectorsPresent.Agata && conf.detectorsPresent.Labr)
    {
        detectorAnaList.emplace_back(new AgataLabrAnalysis(
            inputs, agataOutput, labrOutput, agataAnalysis->agataReader, labrAnalysis->labrReader));
        if (settings.verbosity_level > 0)
            std::cout << "agata+labr initialised" << std::endl;
    }

    // Agata & Prisma variables
    if (conf.detectorsPresent.Agata && conf.detectorsPresent.Prisma)
    {
        detectorAnaList.emplace_back(
            new AgataPrismaAnalysis(inputs, agataOutput, prismaOutput, agataAnalysis->agataReader,
                                    prismaAnalysis->prismaReader, gammaAgataPrisma));
        if (settings.verbosity_level > 0)
            std::cout << "agata+prisma initialised" << std::endl;
    }

    // Prisma & Dante variables
    if (conf.detectorsPresent.Prisma && conf.detectorsPresent.Dante)
    {
        detectorAnaList.emplace_back(new PrismaDanteAnalysis(inputs, prismaOutput, danteOutput,
                                                             prismaAnalysis->prismaReader,
                                                             danteAnalysis->danteReader));
        if (settings.verbosity_level > 0)
            std::cout << "agata+prisma+dante initialised" << std::endl;
    }

    // Agata & Prisma & Dante variables
    if (conf.detectorsPresent.Agata && conf.detectorsPresent.Prisma && conf.detectorsPresent.Dante)
    {
        detectorAnaList.emplace_back(new AgataPrismaDanteAnalysis(
            inputs, agataOutput, prismaOutput, danteOutput, agataAnalysis->agataReader,
            prismaAnalysis->prismaReader, danteAnalysis->danteReader, gammaAgataPrismaDante));
        if (settings.verbosity_level > 0)
            std::cout << "agata+prisma+dante initialised" << std::endl;
    }

    // Agata & Spider variables
    if (conf.detectorsPresent.Agata && conf.detectorsPresent.Spider)
    {
        detectorAnaList.emplace_back(
            new AgataSpiderAnalysis(inputs, agataOutput, spiderOutput, agataAnalysis->agataReader,
                                    spiderAnalysis->spiderReader, gammaAgataSpider));
        if (settings.verbosity_level > 0)
            std::cout << "agata+spider initialised" << std::endl;
    }

    // Agata & SauronFw variables
    if (conf.detectorsPresent.Agata && conf.detectorsPresent.SauronFw)
    {
        detectorAnaList.emplace_back(new AgataSauronFwAnalysis(
            inputs, agataOutput, sauronFwOutput, agataAnalysis->agataReader,
            sauronFwAnalysis->sauronReader, gammaAgataSauronFw));
        if (settings.verbosity_level > 0)
            std::cout << "agata+sauronFw initialised" << std::endl;
    }

    // Agata & SauronBw variables
    if (conf.detectorsPresent.Agata && conf.detectorsPresent.SauronBw)
    {
        detectorAnaList.emplace_back(new AgataSauronBwAnalysis(
            inputs, agataOutput, sauronBwOutput, agataAnalysis->agataReader,
            sauronBwAnalysis->sauronReader, gammaAgataSauronBw));
        if (settings.verbosity_level > 0)
            std::cout << "agata+sauronBw initialised" << std::endl;
    }

    // Agata & EXSSiDe variables
    if (conf.detectorsPresent.Agata && conf.detectorsPresent.EXSSiDe)
    {
        detectorAnaList.emplace_back(
            new AgataEXSSiDeAnalysis(inputs, agataOutput, exssideOutput, agataAnalysis->agataReader,
                                     exssideAnalysis->exssideReader, gammaAgataEXSSiDe));
        if (settings.verbosity_level > 0)
            std::cout << "agata+exsside initialised" << std::endl;
    }

    // Mcp & EXSSiDe variables
    if (conf.detectorsPresent.Mcp && conf.detectorsPresent.EXSSiDe)
    {
        detectorAnaList.emplace_back(new McpEXSSiDeAnalysis(inputs, mcpOutput, exssideOutput,
                                                            mcpAnalysis->mcpReader,
                                                            exssideAnalysis->exssideReader));
        if (settings.verbosity_level > 0)
            std::cout << "mcp+exsside initialised" << std::endl;
    }

    // Mcps & Monitor variables
    if (conf.detectorsPresent.Mcp && conf.detectorsPresent.Monitor)
    {
        detectorAnaList.emplace_back(
            new McpMonitorAnalysis(inputs, mcpOutput, monitorOutput, mcpAnalysis->mcpReader,
                                   monitorAnalysis->monitorReader)); //, gammaAgataSpider));
        if (settings.verbosity_level > 0)
            std::cout << "mcp+monitor initialised" << std::endl;
    }

    // Mcps & Dante variables
    if (conf.detectorsPresent.Mcp && conf.detectorsPresent.Dante)
    {
        detectorAnaList.emplace_back(
            new McpDanteAnalysis(inputs, mcpOutput, danteOutput, mcpAnalysis->mcpReader,
                                 danteAnalysis->danteReader)); //, gammaAgataSpider));
        if (settings.verbosity_level > 0)
            std::cout << "mcp+dante initialised" << std::endl;
    }

    // Mcps & SauronFw variables
    if (conf.detectorsPresent.Mcp && conf.detectorsPresent.SauronFw)
    {
        detectorAnaList.emplace_back(
            new McpSauronFwAnalysis(inputs, mcpOutput, sauronFwOutput, mcpAnalysis->mcpReader,
                                    sauronFwAnalysis->sauronReader, sauronFwMcpOutput));
        if (settings.verbosity_level > 0)
            std::cout << "mcp+sauronFw initialised" << std::endl;
    }

    // Mcps & SauronBw variables
    if (conf.detectorsPresent.Mcp && conf.detectorsPresent.SauronBw)
    {
        detectorAnaList.emplace_back(
            new McpSauronBwAnalysis(inputs, mcpOutput, sauronBwOutput, mcpAnalysis->mcpReader,
                                    sauronBwAnalysis->sauronReader, sauronBwMcpOutput));
        if (settings.verbosity_level > 0)
            std::cout << "mcp+sauronBw initialised" << std::endl;
    }

    // Agata & SauronFw & Mcp variables
    if (conf.detectorsPresent.Agata && conf.detectorsPresent.SauronFw && conf.detectorsPresent.Mcp)
    {
        detectorAnaList.emplace_back(new AgataMcpSauronFwAnalysis(
            inputs, agataOutput, sauronFwMcpOutput, agataAnalysis->agataReader,
            sauronFwAnalysis->sauronReader, gammaAgataSauronFw, sauronFwOutput, mcpOutput));
        if (settings.verbosity_level > 0)
            std::cout << "agata+sauronFw+mcp initialised" << std::endl;
    }

    // Agata & SauronBw & Mcp variables
    if (conf.detectorsPresent.Agata && conf.detectorsPresent.SauronBw && conf.detectorsPresent.Mcp)
    {
        detectorAnaList.emplace_back(new AgataMcpSauronBwAnalysis(
            inputs, agataOutput, sauronBwMcpOutput, agataAnalysis->agataReader,
            sauronBwAnalysis->sauronReader, gammaAgataSauronBw, sauronBwOutput, mcpOutput));
        if (settings.verbosity_level > 0)
            std::cout << "agata+sauronBw+mcp initialised" << std::endl;
    }

    // Agata & OscarUp variables
    if (conf.detectorsPresent.Agata && conf.detectorsPresent.OscarUp)
    {
        detectorAnaList.emplace_back(
            new AgataOscarUpAnalysis(inputs, agataOutput, oscarUpOutput, agataAnalysis->agataReader,
                                     oscarUpAnalysis->oscarReader, gammaAgataOscarUp));
        if (settings.verbosity_level > 0)
            std::cout << "agata+oscarUp initialised" << std::endl;
    }

    // Agata & OscarDown variables
    if (conf.detectorsPresent.Agata && conf.detectorsPresent.OscarDown)
    {
        detectorAnaList.emplace_back(new AgataOscarDownAnalysis(
            inputs, agataOutput, oscarDownOutput, agataAnalysis->agataReader,
            oscarDownAnalysis->oscarReader, gammaAgataOscarDown));
        if (settings.verbosity_level > 0)
            std::cout << "agata+oscarDown initialised" << std::endl;
    }

    // Labr & Dante variables
    if (conf.detectorsPresent.Labr && conf.detectorsPresent.Dante)
    {
        detectorAnaList.emplace_back(
            new LabrDanteAnalysis(inputs, labrOutput, danteOutput, labrAnalysis->labrReader,
                                  danteAnalysis->danteReader, gammaLabrDante));
        if (settings.verbosity_level > 0)
            std::cout << "labr+dante initialised" << std::endl;
    }

    // Labr & Euclides variables
    if (conf.detectorsPresent.Labr && conf.detectorsPresent.Euclides)
    {
        detectorAnaList.emplace_back(
            new LabrEuclidesAnalysis(inputs, labrOutput, euclidesOutput, labrAnalysis->labrReader,
                                     euclidesAnalysis->euclidesReader, gammaLabrEuclides));
        if (settings.verbosity_level > 0)
            std::cout << "labr+euclides initialised" << std::endl;
    }

    // Labr & Prisma variables
    if (conf.detectorsPresent.Labr && conf.detectorsPresent.Prisma)
    {
        detectorAnaList.emplace_back(
            new LabrPrismaAnalysis(inputs, labrOutput, prismaOutput, labrAnalysis->labrReader,
                                   prismaAnalysis->prismaReader, gammaLabrPrisma));
        if (settings.verbosity_level > 0)
            std::cout << "labr+prisma initialised" << std::endl;
    }

    // Labr & Prisma & Dante variables
    if (conf.detectorsPresent.Labr && conf.detectorsPresent.Prisma && conf.detectorsPresent.Dante)
    {
        detectorAnaList.emplace_back(new LabrPrismaDanteAnalysis(
            inputs, labrOutput, prismaOutput, danteOutput, labrAnalysis->labrReader,
            prismaAnalysis->prismaReader, danteAnalysis->danteReader, gammaLabrPrismaDante));
        if (settings.verbosity_level > 0)
            std::cout << "labr+prisma+dante initialised" << std::endl;
    }

    // Labr & Spider variables
    if (conf.detectorsPresent.Labr && conf.detectorsPresent.Spider)
    {
        detectorAnaList.emplace_back(
            new LabrSpiderAnalysis(inputs, labrOutput, spiderOutput, labrAnalysis->labrReader,
                                   spiderAnalysis->spiderReader, gammaLabrSpider));
        if (settings.verbosity_level > 0)
            std::cout << "labr+spider initialised" << std::endl;
    }

    // Labr & SauronFw variables
    if (conf.detectorsPresent.Labr && conf.detectorsPresent.SauronFw)
    {
        detectorAnaList.emplace_back(
            new LabrSauronFwAnalysis(inputs, labrOutput, sauronFwOutput, labrAnalysis->labrReader,
                                     sauronFwAnalysis->sauronReader, gammaLabrSauronFw));
        if (settings.verbosity_level > 0)
            std::cout << "labr+sauronFw initialised" << std::endl;
    }

    // Labr & SauronBw variables
    if (conf.detectorsPresent.Labr && conf.detectorsPresent.SauronBw)
    {
        detectorAnaList.emplace_back(
            new LabrSauronBwAnalysis(inputs, labrOutput, sauronBwOutput, labrAnalysis->labrReader,
                                     sauronBwAnalysis->sauronReader, gammaLabrSauronBw));
        if (settings.verbosity_level > 0)
            std::cout << "labr+sauronBw initialised" << std::endl;
    }

    // Labr & EXSSiDe variables
    if (conf.detectorsPresent.Labr && conf.detectorsPresent.EXSSiDe)
    {
        detectorAnaList.emplace_back(
            new LabrEXSSiDeAnalysis(inputs, labrOutput, exssideOutput, labrAnalysis->labrReader,
                                    exssideAnalysis->exssideReader, gammaLabrEXSSiDe));
        if (settings.verbosity_level > 0)
            std::cout << "labr+exsside initialised" << std::endl;
    }

    // Labr & OscarUp variables
    if (conf.detectorsPresent.Labr && conf.detectorsPresent.OscarUp)
    {
        detectorAnaList.emplace_back(
            new LabrOscarUpAnalysis(inputs, labrOutput, oscarUpOutput, labrAnalysis->labrReader,
                                    oscarUpAnalysis->oscarReader, gammaLabrOscarUp));
        if (settings.verbosity_level > 0)
            std::cout << "labr+oscarUp initialised" << std::endl;
    }

    // Labr & OscarDown variables
    if (conf.detectorsPresent.Labr && conf.detectorsPresent.OscarDown)
    {
        detectorAnaList.emplace_back(
            new LabrOscarDownAnalysis(inputs, labrOutput, oscarDownOutput, labrAnalysis->labrReader,
                                      oscarDownAnalysis->oscarReader, gammaLabrOscarDown));
        if (settings.verbosity_level > 0)
            std::cout << "labr+oscarDown initialised" << std::endl;
    }

    // Allocate histos
    for (auto &it : detectorAnaList)
    {
        it->Instantiate();
        it->AllocateHistos();
    }

    { // Write enabed histo file if not present
        std::ifstream enabledHistosFile(conf.replay.conf_path + "/" +
                                        conf.replay.enabled_histo_file_name);
        if (!enabledHistosFile.is_open())
        {
            std::ofstream newEnabledHistosFile(conf.replay.conf_path + "/" +
                                               conf.replay.enabled_histo_file_name);
            for (const auto &it : allHistos)
            {
                newEnabledHistosFile << it->GetName() << std::endl;
                enabledHistos.emplace_back(it->GetName());
            }
        }
    }
}

Bool_t AgataSelector::Process(Long64_t entry)
{
    // fReader.SetLocalEntry(entry);

    fReader.SetEntry(entry);

    if (settings.verbosity_level > 0 && entry % 100000 == 0)
        std::cout << "Processing event : " << entry << std::endl;
    // Check if StopFile is present, in case call terminate
    if (entry % 10000 == 0 && FileIsPresent("StopFile"))
    {
        Terminate();
        Abort("StopFile created");
        return kFALSE;
    }
    if (entry % 10000 == 0 && tree != nullptr)
    {
        tree->FlushBaskets();
        outFile.Flush();
    }

    // Log progress
    if (fReader.GetEntries(true) > 1000)
    {
        if (entry % (fReader.GetEntries(true) / 1000) == 0)
            progress->store((100. * static_cast<double>(entry + 1)) /
                            static_cast<double>(fReader.GetEntries(true)));
    }
    else
    {
        progress->store((100. * static_cast<double>(entry + 1)) /
                        static_cast<double>(fReader.GetEntries(true)));
    }

    // Perform the analysis
    for (std::size_t i = 0; i < detectorAnaList.size(); ++i)
    {
        auto *analysis = detectorAnaList[i];
        analysis->Clear();
    }

    for (std::size_t i = 0; i < detectorAnaList.size(); ++i)
    {
        auto *analysis = detectorAnaList[i];
        analysis->Analyze();
        analysis->FillHistos();
    }

    if (tree != nullptr)
        tree->Fill();
    return kTRUE;
}

void AgataSelector::SlaveTerminate() {}

void AgataSelector::Terminate()
{
    progress->store(101.); // Means writing data for the printer thread

    // Finalize histos
    for (auto &it : detectorAnaList)
    {
        it->Finalize();
    }

    // outFile->cd();
    outFile.Write();
    outFile.Close();
}

bool AgataSelector::FileIsPresent(const std::string &fileName)
{
    std::ifstream fileCheck(fileName);
    return fileCheck.good();
}
