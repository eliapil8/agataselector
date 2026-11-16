#include "ConfManager.h"

#include "Units.h"
#include <functional>

#include <iostream>

void ConfManager::ReadConf(const std::string &fileName)
{
    if (initialized)
        return;
    std::lock_guard<std::mutex> guard(readMtx);
    confFileName = fileName;

    std::ifstream confFile(confFileName);
    if (!confFile.is_open())
        throw std::runtime_error("Unable to open conf file");
    // Mandatory
    detectorsPresent >> confFile;
    replay >> confFile;
    reaction >> confFile;
    target >> confFile;

    // Only if present
    if (detectorsPresent.Agata)
        agata >> confFile;

    if (detectorsPresent.Prisma)
    {
        prisma >> confFile;
        prisma.ReadManager(prisma.managerPath);
    }

    if (detectorsPresent.Spider)
        spider >> confFile;

    if (detectorsPresent.Monitor)
        monitor >> confFile;

    if (detectorsPresent.SauronFw)
        sauronFw >> confFile;

    if (detectorsPresent.SauronBw)
        sauronBw >> confFile;

    if (detectorsPresent.EXSSiDe)
        exsside >> confFile;

    if (detectorsPresent.OscarUp)
        oscarUp >> confFile;

    if (detectorsPresent.OscarDown)
        oscarDown >> confFile;

    if (detectorsPresent.Labr)
        labr >> confFile;

    if (detectorsPresent.Dante)
        dante >> confFile;

    if (detectorsPresent.Euclides)
        euclides >> confFile;

    if (detectorsPresent.Mcp)
        mcp >> confFile;

    if (detectorsPresent.Agata && detectorsPresent.Prisma)
        agataPrisma >> confFile;

    if (detectorsPresent.Agata && detectorsPresent.Spider)
        agataSpider >> confFile;

    if (detectorsPresent.Agata && detectorsPresent.SauronFw)
        agataSauronFw >> confFile;

    if (detectorsPresent.Agata && detectorsPresent.SauronBw)
        agataSauronBw >> confFile;

    if (detectorsPresent.Agata && detectorsPresent.EXSSiDe)
        agataEXSSiDe >> confFile;

    if (detectorsPresent.Agata && detectorsPresent.OscarUp)
        agataOscarUp >> confFile;

    if (detectorsPresent.Agata && detectorsPresent.OscarDown)
        agataOscarDown >> confFile;

    if (detectorsPresent.Labr && detectorsPresent.Spider)
        labrSpider >> confFile;

    if (detectorsPresent.Labr && detectorsPresent.SauronFw)
        labrSauronFw >> confFile;

    if (detectorsPresent.Labr && detectorsPresent.SauronBw)
        labrSauronBw >> confFile;

    if (detectorsPresent.Labr && detectorsPresent.EXSSiDe)
        labrEXSSiDe >> confFile;

    if (detectorsPresent.Labr && detectorsPresent.OscarUp)
        labrOscarUp >> confFile;

    if (detectorsPresent.Labr && detectorsPresent.OscarDown)
        labrOscarDown >> confFile;

    if (detectorsPresent.Agata && detectorsPresent.Euclides)
        agataEuclides >> confFile;

    if (detectorsPresent.Labr && detectorsPresent.Euclides)
        labrEuclides >> confFile;

    if (detectorsPresent.Agata && detectorsPresent.Dante)
        agataDante >> confFile;

    if (detectorsPresent.Agata && detectorsPresent.Labr)
        agataLabr >> confFile;

    if (detectorsPresent.Labr && detectorsPresent.Prisma)
        labrPrisma >> confFile;

    if (detectorsPresent.Labr && detectorsPresent.Dante)
        labrDante >> confFile;

    if (detectorsPresent.Prisma && detectorsPresent.Dante)
        prismaDante >> confFile;

    if (detectorsPresent.Mcp && detectorsPresent.EXSSiDe)
        mcpExsside >> confFile;

    if (detectorsPresent.Mcp && detectorsPresent.Monitor)
        mcpMonitor >> confFile;

    if (detectorsPresent.Mcp && detectorsPresent.Dante)
        mcpDante >> confFile;

    if (detectorsPresent.Mcp && detectorsPresent.SauronFw)
        mcpSauronFw >> confFile;

    if (detectorsPresent.Mcp && detectorsPresent.SauronBw)
        mcpSauronBw >> confFile;

    // Can be ignored
    if (settings.optimize)
        optimizer >> confFile;

    if (settings.userSelector)
        user >> confFile;

    ReadLut();
    Initialize();

    if (settings.optimize)
    {
        // Set optimizable variables
        std::vector<Conf *> optimConfs{&reaction, &target};

        if (detectorsPresent.Agata)
        {
            optimConfs.emplace_back(&agata);
        }
        if (detectorsPresent.Prisma)
        {
            optimConfs.emplace_back(&prisma);
        }
        if (detectorsPresent.Spider)
        {
            optimConfs.emplace_back(&spider);
        }
        if (detectorsPresent.Monitor)
        {
            optimConfs.emplace_back(&monitor);
        }
        if (detectorsPresent.SauronFw)
        {
            optimConfs.emplace_back(&sauronFw);
        }
        if (detectorsPresent.SauronBw)
        {
            optimConfs.emplace_back(&sauronBw);
        }
        if (detectorsPresent.EXSSiDe)
        {
            optimConfs.emplace_back(&exsside);
        }
        if (detectorsPresent.OscarUp)
        {
            optimConfs.emplace_back(&oscarUp);
        }
        if (detectorsPresent.OscarDown)
        {
            optimConfs.emplace_back(&oscarDown);
        }
        if (detectorsPresent.Labr)
        {
            optimConfs.emplace_back(&labr);
        }
        if (detectorsPresent.Dante)
        {
            optimConfs.emplace_back(&dante);
        }
        if (detectorsPresent.Euclides)
        {
            optimConfs.emplace_back(&euclides);
        }
        if (detectorsPresent.Mcp)
        {
            optimConfs.emplace_back(&mcp);
        }
        if (detectorsPresent.Agata && detectorsPresent.Prisma)
        {
            optimConfs.emplace_back(&agataPrisma);
        }
        optimConfs.emplace_back(&user);

        for (const auto &it : optimConfs)
        {
            for (const auto &it2 : it->getListOfOptimizableParameters())
                optimizer.setOptimizablePtr(it->marker, it2, it->getParameterPtr(it2));
        }
        optimizer.RetrieveParameters();
    }
    initialized = true;
}

void ConfManager::ReadLut()
{
    if (detectorsPresent.Spider)
    {
        spider.lut.ReadLut(replay.conf_path + "/LUT/" + spider.lutFile);

        for (auto &it : agataSpider.lutFiles)
        {
            if (it == "none")
                continue;
            if (settings.verbosity_level > 1)
                std::cout << "Reading file: " << replay.conf_path + "/CUT/SPIDER/Ex/" + it
                          << std::endl;
            agataSpider.lut.ReadLut(replay.conf_path + "/CUT/SPIDER/Ex/" + it);
        }
        for (auto &it : labrSpider.lutFiles)
        {
            if (it == "none")
                continue;
            if (settings.verbosity_level > 1)
                std::cout << "Reading file: " << replay.conf_path + "/CUT/SPIDER/Ex/" + it
                          << std::endl;
            labrSpider.lut.ReadLut(replay.conf_path + "/CUT/SPIDER/Ex/" + it);
        }
    }

    if (detectorsPresent.Monitor)
    {
        monitor.lut.ReadLut(replay.conf_path + "/LUT/" + monitor.lutFile);
    }

    if (detectorsPresent.SauronFw)
    {
        sauronFw.lut.ReadLut(replay.conf_path + "/LUT/" + sauronFw.lutFile);
    }
    if (detectorsPresent.SauronBw)
    {
        sauronBw.lut.ReadLut(replay.conf_path + "/LUT/" + sauronBw.lutFile);
    }
    if (detectorsPresent.EXSSiDe)
    {
        exsside.lut.ReadLut(replay.conf_path + "/LUT/" + exsside.lutFile);
    }
    if (detectorsPresent.OscarUp)
    {
        oscarUp.lut.ReadLut(replay.conf_path + "/LUT/" + oscarUp.lutFile);
        std::vector<TCutG *> ions;
        AddCuts<TCutG *>("/CUT/OSCARUP/EdE/", ions);
        if (oscarUp.global_dE_E_Cuts)
        {
            oscarUp.lut.Load_dE_E_Cuts(ions);
        }
        else
        {
            oscarUp.lut.Load_dE_E_Cuts_TelescopeID(ions);
        }
        AddCuts<TCutG *>(oscarUp.energyThetaCutPath, oscarUp.energyThetaCuts);
    }
    if (detectorsPresent.OscarDown)
    {
        oscarDown.lut.ReadLut(replay.conf_path + "/LUT/" + oscarDown.lutFile);
        std::vector<TCutG *> ions;
        AddCuts<TCutG *>("/CUT/OSCARDOWN/EdE/", ions);
        if (oscarDown.global_dE_E_Cuts)
        {
            oscarDown.lut.Load_dE_E_Cuts(ions);
        }
        else
        {
            oscarDown.lut.Load_dE_E_Cuts_TelescopeID(ions);
        }
        AddCuts<TCutG *>(oscarDown.energyThetaCutPath, oscarDown.energyThetaCuts);
    }

    if (detectorsPresent.Labr)
    {
        labr.lut.ReadLut(replay.conf_path + "/LUT/" + labr.lutFile);
    }

    if (detectorsPresent.Dante)
    {
        dante.lut.ReadLut(replay.conf_path + "/LUT/" + dante.lutFile);
    }

    if (detectorsPresent.Mcp)
    {
        mcp.lut.ReadLut(replay.conf_path + "/LUT/" + mcp.lutFile);
    }

    if (detectorsPresent.Euclides)
    {
        euclides.lut.ReadLut(replay.conf_path + "/LUT/" + euclides.lutFile);
        std::vector<TCutG *> ions;
        AddCuts<TCutG *>("/CUT/EUCLIDES/EdE/", ions);
        euclides.lut.LoadCuts(ions);
    }
}

void ConfManager::Initialize()
{
    std::lock_guard<std::mutex> guard(initMtx);
    {
        target.targetNorm = TVector3(0, 0, 1);
        target.targetNorm.RotateY(target.tilt);
        target.targetNorm.RotateX(target.rotationX);
        target.targetNorm.RotateZ(target.rotationZ);
    }

    if (detectorsPresent.Agata)
        agata.betaAvg.SetMagThetaPhi(agata.betaAvgMag, agata.betaAvgTheta, agata.betaAvgPhi);
    if (detectorsPresent.Prisma)
        prisma.betaAvg.SetMagThetaPhi(prisma.betaAvgMag, prisma.betaAvgTheta, prisma.betaAvgPhi);
    if (detectorsPresent.Spider)
        spider.betaAvg.SetMagThetaPhi(spider.betaAvgMag, spider.betaAvgTheta, spider.betaAvgPhi);
    if (detectorsPresent.Monitor)
        monitor.betaAvg.SetMagThetaPhi(monitor.betaAvgMag, monitor.betaAvgTheta,
                                       monitor.betaAvgPhi);
    if (detectorsPresent.SauronFw)
        sauronFw.betaAvg.SetMagThetaPhi(sauronFw.betaAvgMag, sauronFw.betaAvgTheta,
                                        sauronFw.betaAvgPhi);
    if (detectorsPresent.SauronBw)
        sauronBw.betaAvg.SetMagThetaPhi(sauronBw.betaAvgMag, sauronBw.betaAvgTheta,
                                        sauronBw.betaAvgPhi);

    if (detectorsPresent.EXSSiDe)
        exsside.betaAvg.SetMagThetaPhi(exsside.betaAvgMag, exsside.betaAvgTheta,
                                       exsside.betaAvgPhi);
    if (detectorsPresent.OscarUp)
        oscarUp.betaAvg.SetMagThetaPhi(oscarUp.betaAvgMag, oscarUp.betaAvgTheta,
                                       oscarUp.betaAvgPhi);
    if (detectorsPresent.OscarDown)
        oscarDown.betaAvg.SetMagThetaPhi(oscarDown.betaAvgMag, oscarDown.betaAvgTheta,
                                         oscarDown.betaAvgPhi);
    if (detectorsPresent.Labr)
        labr.betaAvg.SetMagThetaPhi(labr.betaAvgMag, labr.betaAvgTheta, labr.betaAvgPhi);
    if (detectorsPresent.Dante)
        dante.betaAvg.SetMagThetaPhi(dante.betaAvgMag, dante.betaAvgTheta, dante.betaAvgPhi);
    if (detectorsPresent.Mcp)
        mcp.betaAvg.SetMagThetaPhi(mcp.betaAvgMag, mcp.betaAvgTheta, mcp.betaAvgPhi);
    if (detectorsPresent.Euclides)
        euclides.betaAvg.SetMagThetaPhi(euclides.betaAvgMag, euclides.betaAvgTheta,
                                        euclides.betaAvgPhi);

    if (detectorsPresent.Spider)
    {
        for (auto &brd : spider.lut.prop)
        {
            for (auto &chan : brd.second)
            {
                chan.second.pos =
                    chan.second.posOrig + TVector3(spider.xShift, spider.yShift, spider.zShift);

                // Small variations from lookup table
                chan.second.pos.RotateZ(spider.phi);
                chan.second.pos.RotateX(spider.thetaX);
                chan.second.pos.RotateY(spider.thetaY);

                // Major variation, most likely should be kept to 0 in the
                // selector.conf
                chan.second.pos.RotateY(spider.angle);
            }
        }
    }

    if (detectorsPresent.Monitor)
    {
        // nothing
    }

    if (detectorsPresent.Labr)
    {
        for (auto &brd : labr.lut.prop)
        {
            for (auto &chan : brd.second)
            {
                chan.second.pos =
                    chan.second.posOrig + TVector3(labr.xShift, labr.yShift, labr.zShift);

                // Small variations from lookup table
                chan.second.pos.RotateZ(labr.phi);
                chan.second.pos.RotateX(labr.thetaX);
                chan.second.pos.RotateY(labr.thetaY);

                // Major variation, most likely should be kept to 0 in the
                // selector.conf
                chan.second.pos.RotateY(labr.angle);
            }
        }
    }
    if (detectorsPresent.Dante)
    {
        dante.lut.SetRotation(-dante.angle);
    }
    if (detectorsPresent.Mcp)
    {
        //  mcp.lut.SetRotation(-mcp.angle);
    }
    if (detectorsPresent.Euclides)
    {
        for (auto &it : euclides.lut.prop)
        {
            // Small variations from lookup table
            it.second.pos =
                it.second.posOrig + TVector3(euclides.xShift, euclides.yShift, euclides.zShift);
            it.second.pos.RotateZ(euclides.phi);
            it.second.pos.RotateX(euclides.thetaX);
            it.second.pos.RotateY(euclides.thetaY);

            // Major variation, most likely should be kept to 0 in the
            // selector.conf
            it.second.pos.RotateY(euclides.angle);
        }
    }
}

void ConfManager::PrintConf(const std::string &outFileName) const
{
    std::ofstream confFile(outFileName);

    // Comments
    confFile << "#Configuration file for the selector\n";
    confFile << "#Format: | KEYWORD | value(s) | Unit of measure | Comment |\n";
    confFile << "#Comments are ignored unit of measure # means none\n";

    // Mandatory
    detectorsPresent << confFile;
    replay << confFile;
    reaction << confFile;
    target << confFile;

    // On occasion
    agata << confFile;
    prisma << confFile;
    spider << confFile;
    monitor << confFile;
    sauronFw << confFile;
    sauronBw << confFile;
    exsside << confFile;
    oscarUp << confFile;
    oscarDown << confFile;
    labr << confFile;
    dante << confFile;
    mcp << confFile;
    euclides << confFile;
    agataPrisma << confFile;
    labrPrisma << confFile;
    agataSpider << confFile;
    agataSauronFw << confFile;
    agataSauronBw << confFile;
    agataEXSSiDe << confFile;
    agataOscarUp << confFile;
    agataOscarDown << confFile;
    labrSpider << confFile;
    labrSauronFw << confFile;
    labrSauronBw << confFile;
    labrEXSSiDe << confFile;
    labrOscarUp << confFile;
    labrOscarDown << confFile;
    agataEuclides << confFile;
    labrEuclides << confFile;
    agataDante << confFile;
    agataLabr << confFile;
    labrDante << confFile;
    prismaDante << confFile;
    mcpExsside << confFile;
    mcpMonitor << confFile;
    mcpDante << confFile;
    mcpSauronFw << confFile;
    mcpSauronBw << confFile;
    user << confFile;

    optimizer << confFile;
}
