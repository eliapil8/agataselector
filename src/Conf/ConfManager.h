#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <TCutG.h>
#include <TFile.h>
#include <TKey.h>
#include <TVector3.h>

#include "DanteLut.h"
#include "EuclidesLut.h"
#include "GammadetectorSpiderLut.h"
#include "LabrLut.h"
#include "McpLut.h"

#include "EXSSiDeLut.h"
#include "MonitorLut.h"
#include "OscarLut.h"
#include "ReactionFragment.h"
#include "SauronLut.h"
#include "Settings.h"
#include "SpiderLut.h"

#include "AgataConf.h"
#include "AgataDanteConf.h"
#include "AgataEXSSiDeConf.h"
#include "AgataEuclidesConf.h"
#include "AgataLabrConf.h"
#include "AgataOscarConf.h"
#include "AgataPrismaConf.h"
#include "AgataSauronConf.h"
#include "AgataSpiderConf.h"

#include "LabrConf.h"
#include "LabrDanteConf.h"
#include "LabrEXSSiDeConf.h"
#include "LabrEuclidesConf.h"
#include "LabrOscarConf.h"
#include "LabrPrismaConf.h"
#include "LabrSauronConf.h"
#include "LabrSpiderConf.h"
#include "McpDanteConf.h"
#include "McpEXSSiDeConf.h"
#include "McpMonitorConf.h"
#include "McpSauronConf.h"

#include "Conf.h"
#include "DanteConf.h"
#include "DetectorsPresentConf.h"
#include "EXSSiDeConf.h"
#include "EuclidesConf.h"
#include "McpConf.h"
#include "MonitorConf.h"
#include "OptimizerConf.h"
#include "OscarConf.h"
#include "PrismaConf.h"
#include "PrismaDanteConf.h"
#include "ReactionConf.h"
#include "ReplayConf.h"
#include "SauronConf.h"
#include "SpiderConf.h"
#include "TargetConf.h"
#include "UserConf.h"

class ConfManager
{
  public:
  public:
    static ConfManager &getInstance()
    {
        static ConfManager instance;
        return instance;
    }

    ConfManager(const ConfManager &)            = delete;
    ConfManager &operator=(const ConfManager &) = delete;

  private:
    ConfManager() : settings(Settings::getInstance())
    {
        std::ifstream const confFile(settings.confFileName);
        if (!confFile.is_open())
        {
            std::cout << "Could not open file: " << settings.confFileName << std::endl;
            int const val = symlink(("./Conf/" + settings.confFileName).c_str(),
                                    ("./" + settings.confFileName).c_str());
            if (val != 0)
                throw std::runtime_error("Unable to generate selector.conf symbolic link");
        }
        try
        {
            ReadConf(settings.confFileName);
        }
        catch (std::runtime_error &e)
        {
            std::cerr << e.what() << std::endl;
            throw std::runtime_error("Could not read conf file: " + settings.confFileName);
        }
    };
    ~ConfManager() = default;

  public:
    void ReadConf(const std::string &);
    void PrintConf(const std::string &) const;

    DetectorsPresentConf detectorsPresent;
    ReplayConf           replay;
    ReactionConf         reaction;
    TargetConf           target;
    AgataConf            agata;
    PrismaConf           prisma;
    SpiderConf           spider;
    MonitorConf          monitor;
    SauronFwConf         sauronFw;
    SauronBwConf         sauronBw;
    EXSSiDeConf          exsside;
    OscarUpConf          oscarUp;
    OscarDownConf        oscarDown;
    LabrConf             labr;
    DanteConf            dante;
    McpConf              mcp;
    EuclidesConf         euclides;
    AgataPrismaConf      agataPrisma;
    AgataDanteConf       agataDante;
    AgataSpiderConf      agataSpider;
    AgataSauronFwConf    agataSauronFw;
    AgataSauronBwConf    agataSauronBw;
    AgataEXSSiDeConf     agataEXSSiDe;
    AgataOscarUpConf     agataOscarUp;
    AgataOscarDownConf   agataOscarDown;
    AgataEuclidesConf    agataEuclides;
    LabrEuclidesConf     labrEuclides;
    AgataLabrConf        agataLabr;
    LabrPrismaConf       labrPrisma;
    LabrDanteConf        labrDante;
    LabrSpiderConf       labrSpider;
    LabrSauronFwConf     labrSauronFw;
    LabrSauronBwConf     labrSauronBw;
    LabrEXSSiDeConf      labrEXSSiDe;
    LabrOscarUpConf      labrOscarUp;
    LabrOscarDownConf    labrOscarDown;
    PrismaDanteConf      prismaDante;
    McpEXSSiDeConf       mcpExsside;
    McpMonitorConf       mcpMonitor;
    McpSauronFwConf      mcpSauronFw;
    McpSauronBwConf      mcpSauronBw;
    McpDanteConf         mcpDante;
    OptimizerConf        optimizer;
    UserConf             user;

  private:
    std::string     confFileName;
    std::mutex      initMtx;
    std::mutex      readMtx;
    const Settings &settings;
    void            ReadLut();

  public:
    void Initialize();

  public:
    template <class T, class C = std::vector<T>>
    void AddCuts(const std::string &path, C &arr) const
    {
        std::string inFilePath = replay.conf_path + path;
        struct stat info{};
        if (stat(inFilePath.c_str(), &info) != 0)
            return;

        DIR           *dir;
        struct dirent *ent;
        if ((dir = opendir(inFilePath.c_str())) != nullptr)
        {
            std::cout << inFilePath << std::endl;
            /* print all the files and directories within directory */
            while ((ent = readdir(dir)) != nullptr)
            {
                // printf ("%s\n", ent->d_name);
                std::string tmpName = ent->d_name;
                std::cout << tmpName << std::endl;
                if (tmpName.substr(tmpName.find_last_of('.') + 1) == "root")
                {
                    TFile    cutsFile((inFilePath + tmpName).c_str(), "read");
                    TIter    contents(cutsFile.GetListOfKeys());
                    TKey    *key;
                    TObject *obj;
                    while ((key = dynamic_cast<TKey *>(contents())))
                    {
                        obj = cutsFile.Get(key->GetName());
                        if (obj->InheritsFrom("TCutG"))
                        {
                            auto *tmp = dynamic_cast<TCutG *>(obj);
                            if (settings.verbosity_level > 3)
                                std::cout << "Found cut: " << tmp->GetName()
                                          << " in file: " << tmpName << std::endl;
                            //: unsigned int duplicateCounter{0};
                            // while(std::find_if(arr.begin(), arr.end(),
                            //           [&](const T& a)->bool{
                            //                     //return
                            //                     std::string(a.cut->GetName()).compare(tmp->GetName())==0;
                            //     		return false;
                            //                 })){//cut with name already
                            //                 present
                            //     tmp->SetName((tmp->GetName()+std::to_string(++duplicateCounter)).c_str());
                            // }
                            arr.emplace_back(static_cast<TCutG *>(tmp->Clone()));
                        }
                        else if (obj->InheritsFrom("TGraph"))
                        {
                            auto  *_g = dynamic_cast<TGraph *>(obj);
                            TCutG *cut =
                                new TCutG(_g->GetName(), _g->GetN(), _g->GetX(), _g->GetY());
                            if (settings.verbosity_level > 3)
                                std::cout << "Found cut: " << cut->GetName()
                                          << " in file: " << tmpName << std::endl;
                            arr.emplace_back(cut);
                        }
                    }
                }
            }
            closedir(dir);
        }
        else
        {
            throw std::runtime_error("Could not open directory :" + inFilePath);
        }
    }

  private:
    bool initialized{false};
};
