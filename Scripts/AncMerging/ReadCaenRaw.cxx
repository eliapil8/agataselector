#include "ReadCaenRaw.h"
#include <algorithm>
#include <fstream>
#include <numeric>
#include <sstream>

#include <TApplication.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <iomanip>
#include <sys/stat.h>

std::string fourCharInt(int I) {
    std::stringstream ID;
    ID << std::setfill('0') << std::setw(4) << I;
    return ID.str();
}

FILE* openADFFile(FILE* file, std::string fname) {
    const std::string ext = ".adf";
    if(fname >= ext.size()
       && fname.compare(fname.size() - ext.size(), ext.size(), ext) == 0) {
        fname.erase(fname.size() - ext.size());
    }
    fname += "_" + fourCharInt(outADFFileIndex) + ".adf";
    ++outADFFileIndex;
    file = fopen(fname.c_str(), "wb");
    if(!file) {
        std::cerr << "Error opening ADF file: " << fname << std::endl;
        throw std::runtime_error("Cannot open ADF file for writing");
    }
    std::cout << "ADF file : " << fname << std::endl;
    return file;
}

long get_file_size(FILE* file) {
    // std::cout << "File size: " << ftell(file) << " bytes" << std::endl;
    return ftell(file);
}

template <std::size_t R, std::size_t L, std::size_t N>
std::bitset<N>                      project_range(std::bitset<N> b);

std::map<int, struct boardSettings> brdSettings;

struct dataFormatPSD                dataFormPSD;
struct dataFormatPHA                dataFormPHA;
struct agataKey                     prismaMergedKey;

struct subDataPHA_t;
struct caenEventPHA_t;
struct subDataPSD_t;
struct caenEventPSD_t;
struct internalData intEvt;

void                printHelp() {
                   std::cout << "Usage: ./ReadCaenRaw [OPTIONS]\n\n";
                   std::cout << "Options:\n";
                   std::cout << "  --help, -h                       Show this help message "
                                               "and exit\n";
                   std::cout << "  --root <OutputRootFileName>      Specify the output root "
                                               "file name\n";
                   std::cout
                       << "  --write_traces [nSamples]        Write traces in output files\n";
                   std::cout << "  --global-anc-tsoffset <value>    Set the global ancillary "
                                               "timestamp offset (double)\n";
                   std::cout << "  --nrevts <value>                 Set the number of events "
                                               "to process (integer)\n";
                   std::cout << "  --prisma <file1> [file2 ...]     Specify one or more "
                                               "input Prisma files\n";
                   std::cout << "  --input <file1> [file2 ...]      Specify one or more "
                                               "ancillary input files\n";
                   std::cout << "  --adf <outAdfFileName>           Specify the output ADF "
                                               "file name\n\n";
                   std::cout << "Example:\n";
                   std::cout << "  ./ReadCaenRaw --root output.root --global-anc-tsoffset "
                                               "1.5 --nrevts 100 \\\n";
                   std::cout << "                --dante 5 --prisma prisma1.dat prisma2.dat "
                                               "--input anc1.dat anc2.dat \\\n";
                   std::cout << "                --adf output.adf\n";
}

Arguments parseArguments(int argc, char* argv[]) {
    Arguments args;

    for(int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if(arg == "--help" || arg == "-h") {
            printHelp();
            exit(0);
        } else if(arg == "--verb") {
            args.verb = true;
        } else if(arg == "--write_traces") {
            args.write_traces = true;
            args.n_samples    = std::stoi(argv[++i]);
        } else if(arg == "--root") {
            if(i + 1 < argc) {
                args.outputRootFileName = argv[++i];
            } else {
                throw std::runtime_error("Missing value for --root");
            }
        } else if(arg == "--global-anc-tsoffset") {
            if(i + 1 < argc) {
                args.globalAncTsOffset = std::stol(argv[++i]);
                std::cout << "Setting global offset to "
                          << args.globalAncTsOffset << std::endl;
            } else {
                throw std::runtime_error(
                    "Missing value for --global-anc-tsoffset");
            }
        } else if(arg == "--nrevts") {
            if(i + 1 < argc) {
                args.nrevts = std::stoi(argv[++i]);
            } else {
                throw std::runtime_error("Missing value for --nrevts");
            }
        } else if(arg == "--plot") {
            if(i + 1 < argc) {
                args.plot.first  = std::stoi(argv[++i]);
                args.plot.second = std::stoi(argv[++i]);
            } else {
                throw std::runtime_error("Missing value for --plot");
            }
        } else if(arg == "--prisma") {
            while(i + 1 < argc
                  && argv[i + 1][0] != '-') { // Continue while next arg is not
                                              // another option
                FILE* file = fopen(argv[++i], "rb");
                if(file) {
                    args.prismaFiles.push_back(file);
                    std::cout << "Adding prisma file " << argv[i] << std::endl;
                } else {
                    throw std::runtime_error("Cannot open file for --prisma: "
                                             + std::string(argv[i]));
                }
            }
        } else if(arg == "--input") {
            while(i + 1 < argc
                  && argv[i + 1][0] != '-') { // Continue while next arg is not
                                              // another option
                FILE* file = fopen(argv[++i], "rb");
                if(file) {
                    std::cout << "Adding caen binary file " << argv[i]
                              << std::endl;
                    args.ancillaryFiles.push_back(file);
                } else {
                    throw std::runtime_error("Cannot open file for --input: "
                                             + std::string(argv[i]));
                }
            }
        } else if(arg == "--adf") {
            if(i + 1 < argc) {
                args.outAdfFileName = argv[++i];
            } else {
                throw std::runtime_error("Missing value for --adf");
            }
        } else {
            throw std::runtime_error("Unknown argument: " + arg);
        }
    }
    return args;
}

void readSetFile() {
    std::ifstream setFile("ReadCaenRaw.set");
    if(!setFile.is_open()) {
        throw std::runtime_error("Settings file not found ...\n");
    } else {
        std::string line("");
        while(std::getline(setFile, line)) {
            if(line.empty() || line.at(0) == '#') continue;

            // Existing parsing logic
            if(line.find("window") == 0) {
                std::string       dummy;
                std::stringstream ss(line);
                ss >> dummy >> coincWindowAnc;
                coincWindowAnc /= 10;
                std::cout << "Coincidence window for the building of the "
                             "ancillaries set to "
                          << coincWindowAnc << " timestamp ("
                          << coincWindowAnc * 10 << " ns)" << std::endl;
                continue;
            } /*
                      digitizer.second.theDAQ->process_start();
                      fileHeader.boardDef[board++] = (
                      digitizer.second.theDAQ->theDAQSetup_.nsPerSample &
                 (0x3F) ) | ( digitizer.second.theDAQ->theDAQSetup_.nsPerTimetag
                 & (0x3F) ) << 6 | (
                 digitizer.second.theDAQ->theDAQSetup_.dppVersion & (0xF) )
                      << 12 | ( digitizer.second.theDAQ->theDAQSetup_.channels
                 & (0xFF) ) << 16 | (
                 digitizer.second.theDAQ->theDAQSetup_.boardRegId & (0xFF) ) <<
                 24;

                    */
            else if(line.find("board") == 0) {
                std::stringstream ss(line);
                std::string       dummy, fw, datKey;
                uint32_t          nsxTS;
                uint32_t          nsxSamp;
                int               brd(-1), version(-1);
                uint16_t          nbChannel(0);
                ss >> dummy >> brd >> version >> nbChannel >> fw >> nsxTS
                    >> nsxSamp >> datKey;
                if(brd == -1) continue;
                brdSettings[brd].isV1         = (version == 1) ? true : false;
                brdSettings[brd].isPHA        = (fw == "PHA") ? true : false;
                brdSettings[brd].fNsPerTStamp = nsxTS;
                brdSettings[brd].fNsPerTStamp = nsxSamp;
                brdSettings[brd].key          = getKey(datKey);
                brdSettings[brd].nbChannels   = nbChannel;
                for(uint16_t ch = 0; ch < nbChannel; ++ch) {
                    brdSettings[brd].tsOffset[ch] = args.globalAncTsOffset;
                }
                if(brdSettings[brd].nbChannels == 64) {
                    for(int j = brd; j < brd + 4; j++) {
                        brdSettings[j].isV1  = (version == 1) ? true : false;
                        brdSettings[j].isPHA = (fw == "PHA") ? true : false;
                        brdSettings[j].fNsPerTStamp = nsxTS;
                        brdSettings[j].fNsPerTStamp = nsxSamp;
                        brdSettings[j].key          = getKey(datKey);
                        brdSettings[j].nbChannels   = 16;
                        for(uint16_t ch = 0; ch < nbChannel; ++ch) {
                            brdSettings[j].tsOffset[ch]
                                = args.globalAncTsOffset;
                        }
                    }
                }
                continue;
            } else if(line.find("waveAnalyzer") == 0) {
                std::string       dummy, paramName;
                int               brd{-1}, chan{-1};
                std::stringstream ss(line);
                float             floatValue;
                int               intValue;

                // Read board and channel identifiers
                ss >> dummy >> brd >> chan >> paramName;

                if(brd == -1 || chan == -1) continue;
                if(chan > 15) {
                    brd += chan / 16;
                    chan = chan % 16;
                }
                // Ensure board and channel are valid
                if(brdSettings.find(brd) == brdSettings.end() || chan < 0
                   || chan >= brdSettings[brd].nbChannels) {
                    std::cerr << "Invalid board or channel in waveAnalyzer "
                                 "settings: "
                              << brd << ", " << chan << std::endl;
                    continue;
                }

                // Parse parameter values
                if(paramName == "nDcwin") {
                    ss >> intValue;
                    waveAnalyzer[brd][chan].nDcwin = intValue;
                } else if(paramName == "nSmooth") {
                    ss >> intValue;
                    waveAnalyzer[brd][chan].nSmooth = intValue;
                } else if(paramName == "nWidthT") {
                    ss >> intValue;
                    waveAnalyzer[brd][chan].nWidthT = intValue;
                } else if(paramName == "nDelayCFD") {
                    ss >> intValue;
                    waveAnalyzer[brd][chan].nDelayCFD = intValue;
                } else if(paramName == "fFractCDF") {
                    ss >> floatValue;
                    waveAnalyzer[brd][chan].fFractCDF = floatValue;
                } else if(paramName == "fThreshTFA") {
                    ss >> floatValue;
                    waveAnalyzer[brd][chan].fThreshTFA = floatValue;
                } else if(paramName == "fThreshCFD") {
                    ss >> floatValue;
                    waveAnalyzer[brd][chan].fThreshCFD = floatValue;
                } else if(paramName == "nMinWidthP") {
                    ss >> intValue;
                    waveAnalyzer[brd][chan].nMinWidthP = intValue;
                } else if(paramName == "nMinWidthN") {
                    ss >> intValue;
                    waveAnalyzer[brd][chan].nMinWidthN = intValue;
                }
                continue;
            } else if(line.find("tsoffset") == 0) {
                std::string       dummy;
                int               brd{-1111}, chan{-1111}, offset{-1111};
                std::stringstream ss(line);
                ss >> dummy >> brd >> chan >> offset;
                if(brd == -1111 || chan == -1111 || offset == -1111) continue;
                if(chan > 15) {
                    brd += chan / 16;
                    chan = chan % 16;
                }
                brdSettings[brd].tsOffset[chan] += offset;

                std::cout << "Setting offset for board " << brd << ", channel "
                          << chan << " to " << offset
                          << " and total offset of "
                          << brdSettings[brd].tsOffset[chan] << std::endl;
                continue;
            } else if(line.find("minfold") == 0) {
                std::string       dummy;
                int               brd{-1111}, fold{-1111};
                std::stringstream ss(line);
                ss >> dummy >> brd >> fold;
                if(brd == -1111 || fold == -1111) continue;
                std::cout << "Setting min fold for board " << brd
                          << " to: " << fold << std::endl;
                brdSettings[brd].minMult = fold;
                continue;
            }
        }
        setFile.close();
    }
}

int main(int argc, char* argv[]) {
    TApplication app("app", 0, 0);
    if(argc == 1) {
        printHelp();
        return 0;
    }

    args            = parseArguments(argc, argv);

    mergeWithPrisma = !args.prismaFiles.empty();
    for(uint32_t brd = 0; brd < maxboards; ++brd) {
        for(uint32_t ch = 0; ch < maxchannels; ++ch) {
            int numSamples        = 128;
            waveAnalyzer[brd][ch] = MWD(numSamples, true);
            waveAnalyzer[brd][ch].nDcwin
                = 128 / 5; // the first 20% --> check if this is valid
            waveAnalyzer[brd][ch].nSmooth    = 5;
            waveAnalyzer[brd][ch].nWidthT    = 8;
            waveAnalyzer[brd][ch].nDelayCFD  = 6;
            waveAnalyzer[brd][ch].fFractCDF  = 0.25f;
            waveAnalyzer[brd][ch].fThreshTFA = 20;
            waveAnalyzer[brd][ch].fThreshCFD = 0;
            waveAnalyzer[brd][ch].nMinWidthP = 2;
            waveAnalyzer[brd][ch].nMinWidthN = 6;
        }
    }

    readSetFile();

    if(!args.outputRootFileName.empty()) {
        std::cout << "Root output file set to: " << args.outputRootFileName
                  << std::endl;
        fOutput   = new TFile(args.outputRootFileName.c_str(), "recreate");
        rootWrite = true;
        fOutput->cd();
    }
    fTree = new TTree("TreeMaster", "TreeMaster");
    std::cout << " ========= Board settings ========= " << std::endl;
    for(auto it = brdSettings.begin(); it != brdSettings.end(); ++it) {
        std::cout << "Board " << it->first << "\n" << std::endl;
        std::cout << "Key: " << std::hex << it->second.key << std::dec
                  << std::endl;
        std::cout << "Slave " << it->second.slave << std::endl;
        std::cout << "MinMult " << it->second.minMult << std::endl;
        std::cout << "IsPHA " << it->second.isPHA << std::endl;
        std::cout << "fNsPerTStamp " << it->second.fNsPerTStamp << std::endl;
        std::cout << "--------------------- \n\n" << std::endl;
        switch(it->second.key) {
        case keyDataVME: break;
        case keyDataPrisma: break;
        case keyDataSpider: createBranchSpider(); break;
        case keyDataDante: createBranchDante(); break;
        case keyDataEuclides: createBranchEuclides(); break;
        case keyDataLaBr: createBranchLaBr(); break;
        case keyDataSauron: createBranchSauron(); break;
        case keyDataOscar: createBranchOscar(); break;
        default:
            std::cerr << "Unknown key: 0x" << std::hex << it->second.key
                      << std::dec << std::endl;
        }
    }

    std::cout << " ================================== " << std::endl;
    uint16_t boardId(0), channelId(0);
    uint16_t energy(0), time(0), qshort(0);
    uint64_t tstamp, TStamp_10ns(0);
    bool     tsat(0), isat(0), pur(0);
    float    cfd     = 0.;
    bool     TreeEna = true;
    coincTree        = new TTree("coincTree", "coincTree");

    hitpatternPHA    = new TH2D("hitPHA", "hit PHA", maxboards, -.5,
                                maxboards - 0.5, 4096, 0, 36767);
    hitpatternPHA->SetXTitle("Channel+Board*16");
    hitpatternPHA->SetYTitle("Energy [ADC chan.]");
    hitpatternPSD = new TH2D("hitPSD", "hit PSD", maxboards, -.5,
                             maxboards - 0.5, 4096, 0, 36767);
    hitpatternPSD->SetXTitle("Channel+Board*16");
    hitpatternPSD->SetYTitle("Energy [ADC chan.]");

    buffer = new char[2048 * 2048];

    for(auto it = brdSettings.begin(); it != brdSettings.end(); ++it) {
        for(uint16_t ch = 0; ch < it->second.nbChannels; ch++) {
            uint16_t index                     = it->first << 8 | ch % 16;
            // eventContainer[index].set_capacity(50000);
            // eventContainer[index].set_capacity(5000);
            readCountRate[it->first][ch % 16]  = 0;
            writeCountRate[it->first][ch % 16] = 0;
        }
    }

    // RAW DATA file has a header containing the number of boards and the
    // AGAVA timestamp corresponding to the T=0 of the CAEN boards
    // i.e. the start of the run
    uint64_t tsOffset;
    uint16_t nbOfBoards;
    //  if(!sizeFrame) sizeFrame = new uint32_t;
    //	uint32_t *sizePrismaFrame = new uint32_t;

    if(args.ancillaryFiles.size() == 0)
        throw std::runtime_error("No input file specified\n");
    fread(&sizeFrame, sizeof(uint32_t), 1, args.ancillaryFiles[0]);
    sizeFrame = sizeFrame & 0x0FFFFFFF;
    std::cout << "Size to read = " << sizeFrame << std::endl;

    // FIRST FILE HAS AN HEADER
    fread(buffer, sizeof(uint32_t), sizeFrame - 1, args.ancillaryFiles[0]);
    //
    uint32_t* convBuff = (uint32_t*)buffer;
    fGlobalOffset      = (uint64_t)(convBuff[2]) << 32 | convBuff[3];
    //
    printf("fGlobalOffset = %lu\n", fGlobalOffset);
    time_t    rawtime = convBuff[4];
    struct tm ts;
    char      buf[80];
    //
    //// Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
    ts = *localtime(&rawtime);
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
    printf("sizeframe: %i", sizeFrame);
    printf("%s\n", buf);
    uint32_t aggregateSize   = 0;
    uint32_t nbAggr          = 0;
    uint32_t offset          = 5;
    uint32_t channelAggrSize = 0;

    pFile                    = openADFFile(pFile, args.outAdfFileName);
    // pFile                    = fopen(args.outAdfFileName.c_str(), "wb");
    // std::cout << "ADF file : " << args.outAdfFileName << std::endl;

    while(!isEof()) {
        sizeFrame = getSizeFrame();
        if(sizeFrame + MAX_OUT_ADF_FILE_SIZE < get_file_size(pFile)) {
            pFile = openADFFile(pFile, args.outAdfFileName);
        }
        // std::cout << "-----> size frame " << sizeFrame << std::endl;
        if((sizeFrame == 0)) break;
        aggregateSize = (sizeFrame & 0x0FFFFFFF);
        if(args.verb)
            printf("Size to read = %u (0x%08X)\n", aggregateSize, sizeFrame);
        bool dataOk = getData(aggregateSize);
        if(!dataOk) {
            std::cout << "Data not ok" << std::endl;
            break;
        }
        // decoding the aggregate:
        convBuff = (uint32_t*)buffer;
        boardId  = (convBuff[1] & 0xF8000000) >> 27;
        if(brdSettings[boardId].isPHA) {
            if(brdSettings[boardId].isV1) {
                if(args.verb) std::cout << "PHA v1" << std::endl;
                decode_PHA_v1(convBuff);
            } else {
                if(args.verb) std::cout << "PHA v2" << std::endl;
                decode_PHA_v2(convBuff);
            }
        } else if(!brdSettings[boardId].isPHA) {
            if(brdSettings[boardId].isV1) {
                if(args.verb) std::cout << "PSD v1" << std::endl;
                decode_PSD_v1(convBuff);
            } else {
                if(args.verb) std::cout << "PSD v2" << std::endl;
                decode_PSD_v2(convBuff);
            }
        }
        memset(buffer, 0, 512 * 512);
        nbAggr++;
        if(nbAggr % 10000 == 0) std::cout << nbAggr << "  \r" << std::flush;
        if(nbAggr > args.nrevts && args.nrevts > 0) break;
    };

    std::cout << "\nStarting to deplete buffers\n";
    depleteBuffers();

    std::cout << "Number of aggregate = " << nbAggr << std::endl;
    std::cout << "Number of events with Energy > 0 channels: " << std::endl;
    for(uint32_t bd = 0; bd < maxboards; bd++) {
        for(uint32_t ch = 0; ch < maxchannels; ch++) {
            if(readCountRate[bd][ch] > 0) {
                printf("Board %2i \t Channel %2i \t nEventsRead = %8i \t "
                       "nEventsWritten = %8i \t eff: %4.4f \n ",
                       bd, ch, readCountRate[bd][ch], writeCountRate[bd][ch],
                       1.0 * writeCountRate[bd][ch] / readCountRate[bd][ch]);
            }
        }
    }

    printf("Prisma Events \t nEventsRead = %8i \t nEventsWritten = %8i \t "
           "eff: %4.4f \n",
           prismaReadRate, prismaWriteRate,
           1.0 * prismaWriteRate / prismaReadRate);
    if(rootWrite) {
        fOutput->cd();
        fTree->Write(0, TObject::kOverwrite);
        // coincTree->Write(0, TObject::kOverwrite);
        hitpatternPHA->Write(0, TObject::kOverwrite);
        hitpatternPSD->Write(0, TObject::kOverwrite);
        fOutput->Close();
        delete fOutput;
    }
    fclose(pFile);
    while(args.ancillaryFiles.size() > 0) {
        fclose(args.ancillaryFiles.front());
        args.ancillaryFiles.pop_front();
    }
    while(args.prismaFiles.size() > 0) {
        fclose(args.prismaFiles.front());
        args.prismaFiles.pop_front();
    }
    std::cout << "Finished sorting" << std::endl;
    // delete coincTree;
    delete[] buffer;
    if(prismaBuffer) delete[] prismaBuffer;
    if(!convBuff) delete[] convBuff;
    //  delete sizeFrame;
    eventContainer.clear();
    timeSorted.clear();
    std::cout << "memory released ... Done" << std::endl;
    return 0;
}

void timeSortEvent(const internalData& evt) {
    uint16_t index = evt.board << 8 | evt.channel;
    // std::cout << "timesort begin" << std::endl;
    // printf("board = %2.i , channel = %2.i , index = %5.i, evt.energy
    // =%i\n",evt.board, evt.channel,index,evt.energy);
    if(evt.energy > 0) { eventContainer[index].push_back(evt); }
    // if(eventContainer[index].full()) {
    if(eventContainer[index].size() > channelsBuffersSize) {
        if(args.verb)
            std::cout << "One buffer hase reached the capacity" << std::endl;
        // one of the queue is full so we should start to sort the events
        do { // once all buffers are not full we go out of here
            internalData evtSmallestTS;
            uint16_t*    indexSTS   = nullptr;
            evtSmallestTS.timestamp = std::numeric_limits<uint64_t>::max();
            for(auto it = eventContainer.begin(); it != eventContainer.end();
                it++) {
                if(it->second.size() == 0) continue;
                if(evtSmallestTS.timestamp > it->second.front().timestamp) {
                    evtSmallestTS = it->second.front();
                    if(indexSTS == nullptr) indexSTS = new uint16_t;
                    *indexSTS = it->first;
                }
            }
            timeSorted.push_back(evtSmallestTS);

            if(indexSTS != nullptr) eventContainer[*indexSTS].pop_front();
            delete indexSTS;

        } while(eventContainer[index].size() > channelsBuffersSize);

        if(timeSorted.size() > 1000) {
            if(args.verb)
                std::cout << "The sorted buffer has reached the capacity"
                          << std::endl;
            while(timeSorted.size() > coincWindowAnc) {
                uint16_t nb_event = 1;
                bool     minMultOK[16];
                std::fill(minMultOK, minMultOK + 16, true);
                while(timeSorted[nb_event].timestamp - timeSorted[0].timestamp
                      < coincWindowAnc) {
                    if(nb_event >= timeSorted.size()) break;
                    nb_event++;
                }

                bool aloneAnc[16];
                std::fill(aloneAnc, aloneAnc + 16, false);
                for(int j = 0; j < 16; ++j) {
                    if(brdSettings[j]
                           .slave) { // if labr is requested as slave, checks
                        aloneAnc[j] = true;
                        for(int i{0}; i < nb_event; ++i) {
                            if(timeSorted[i].board != j) aloneAnc[j] = false;
                        }
                    }
                    if(brdSettings[j].minMult > 1) {
                        minMultOK[j]     = false;
                        int boardCounter = 0;
                        for(int i{0}; i < nb_event; ++i) {
                            if(timeSorted[i].board == j) boardCounter++;
                        }

                        if(boardCounter >= brdSettings[j].minMult
                           || boardCounter == 0)
                            minMultOK[j] = true;
                    }
                }

                agataKey mergedKey;
                mergedKey.size = sizeof(agataKey);
                for(size_t i{0}; i < nb_event; ++i) {
                    if(brdSettings[timeSorted[i].board].isPHA) {
                        mergedKey.size += sizeof(caenEventPHA_t);
                    } else {
                        mergedKey.size += sizeof(caenEventPSD_t);
                    }
                }
                mergedKey.key   = 0xCA020100;
                mergedKey.evnum = 0;
                mergedKey.ts_0
                    = uint32_t(timeSorted[0].timestamp & 0xFFFFFFFF);
                mergedKey.ts_1
                    = uint32_t((timeSorted[0].timestamp >> 32) & 0x0000FFFF);

                bool        prismaCoincidence = false;
                size_t      prismaToWrite{0};
                char        prismaSingleEvent[512 * 512];
                std::size_t size_buff = sizeof(prismaBuffer);
                if(mergeWithPrisma) {
                    if(args.verb)
                        std::cout << "Reading prisma buffer" << std::endl;
                    if(prismaBuffer == nullptr) {
                        if(!fread((char*)&prismaMergedKey, 1, sizeof(agataKey),
                                  args.prismaFiles.front())) {
                            fclose(args.prismaFiles.front());
                            std::cout << "End of prisma file "
                                      << prismaFileCounter << std::endl;
                            prismaFileCounter++;
                            args.prismaFiles.pop_front();
                            if(!args.prismaFiles.empty()) {
                                fread((char*)&prismaMergedKey, 1,
                                      sizeof(agataKey),
                                      args.prismaFiles.front());
                            } else {
                                memset(prismaBuffer, 0, size_buff);
                                // memset(prismaBuffer, 0, sizeof(prismaBuffer));
                            }
                        }

                        prismaBuffer = new char[512 * 512];
                        if(!args.prismaFiles.empty()) {
                            fread(prismaBuffer, 1,
                                  prismaMergedKey.GetBytes()
                                      - sizeof(agataKey),
                                  args.prismaFiles.front());
                        } else {
                            memset(prismaBuffer, 0, size_buff);
                            // memset(prismaBuffer, 0, sizeof(prismaBuffer));
                        }
                        prismaReadRate++;
                    }
                    auto prismaTS = prismaMergedKey.GetTstamp();
                    auto diffBegin
                        = mergedKey.GetTstamp() > prismaTS
                            ? -(int64_t)(mergedKey.GetTstamp() - prismaTS)
                            : (int64_t)(prismaTS - mergedKey.GetTstamp());
                    // Positive number means prisma after
                    auto diffEnd
                        = timeSorted[nb_event - 1].timestamp > prismaTS
                            ? -(int64_t)(timeSorted[nb_event - 1].timestamp
                                         - prismaTS)
                            : (int64_t)(prismaTS
                                        - timeSorted[nb_event - 1]
                                              .timestamp); // Positive number
                    // means prisma after

                    // tsprisma = prismaTS;

                    while(diffBegin
                          < coincWindowAnc) { // Probabily 100, possibly 50
                        if(diffBegin < -coincWindowAnc
                           || (diffBegin >= -coincWindowAnc && diffBegin < 0
                               && diffEnd
                                      < -coincWindowAnc)) { // Prisma before
                            // and alone
                            fwrite((char*)&prismaMergedKey, 1,
                                   sizeof(agataKey),
                                   pFile); // TODO: check if file is finished
                            fwrite(prismaBuffer, 1,
                                   prismaMergedKey.GetBytes()
                                       - sizeof(agataKey),
                                   pFile);
                            prismaWriteRate++;
                            tsprisma = prismaTS;
                            if(rootWrite) coincTree->Fill();
                            tsprisma = 0;
                        } else if(diffBegin < 0
                                  && diffEnd
                                         >= -coincWindowAnc) { // Prisma before
                            // in coinc
                            mergedKey.ts_0 = uint32_t(prismaTS & 0xFFFFFFFF);
                            mergedKey.ts_1
                                = uint32_t((prismaTS >> 32) & 0x0000FFFF);
                            mergedKey.size += prismaMergedKey.GetBytes()
                                            - sizeof(agataKey);
                            prismaToWrite = prismaMergedKey.GetBytes()
                                          - sizeof(agataKey);
                            tsprisma          = prismaTS;
                            prismaCoincidence = true;
                            memcpy(prismaSingleEvent, prismaBuffer,
                                   prismaToWrite);
                        } else { // Prisma after in coinc
                            mergedKey.size += prismaMergedKey.GetBytes()
                                            - sizeof(agataKey);
                            prismaToWrite = prismaMergedKey.GetBytes()
                                          - sizeof(agataKey);
                            tsprisma          = prismaTS;
                            prismaCoincidence = true;
                            memcpy(prismaSingleEvent, prismaBuffer,
                                   prismaToWrite);
                        }
                        if(!fread((char*)&prismaMergedKey, 1, sizeof(agataKey),
                                  args.prismaFiles.front())) {
                            fclose(args.prismaFiles.front());
                            args.prismaFiles.pop_front();
                            std::cout << "End of prisma file "
                                      << prismaFileCounter << std::endl;
                            prismaFileCounter++;
                            if(!args.prismaFiles.empty()) {
                                fread((char*)&prismaMergedKey, 1,
                                      sizeof(agataKey),
                                      args.prismaFiles.front());
                            } else {
                                memset(prismaBuffer, 0, size_buff);
                                // memset(prismaBuffer, 0, sizeof(prismaBuffer));
                            }
                        }
                        if(!fread(prismaBuffer, 1,
                                  prismaMergedKey.GetBytes()
                                      - sizeof(agataKey),
                                  args.prismaFiles.front())) {
                            fclose(args.prismaFiles.front());
                            std::cout << "End of prisma file "
                                      << prismaFileCounter << std::endl;
                            prismaFileCounter++;
                            args.prismaFiles.pop_front();
                            if(!args.prismaFiles.empty()) {
                                fread((char*)&prismaMergedKey, 1,
                                      sizeof(agataKey),
                                      args.prismaFiles.front());
                            } else {
                                memset(prismaBuffer, 0, size_buff);
                                // memset(prismaBuffer, 0, sizeof(prismaBuffer));
                            }
                        };
                        prismaReadRate++;
                        prismaTS = prismaMergedKey.GetTstamp();
                        diffBegin
                            = mergedKey.GetTstamp() > prismaTS
                                ? -(int64_t)(mergedKey.GetTstamp() - prismaTS)
                                : (int64_t)(prismaTS
                                            - mergedKey.GetTstamp()); // Positive
                        diffEnd
                            = timeSorted[nb_event - 1].timestamp > prismaTS
                                ? -(int64_t)(timeSorted[nb_event - 1].timestamp
                                             - prismaTS)
                                : (int64_t)(prismaTS
                                            - timeSorted[nb_event - 1]
                                                  .timestamp); // number
                                                               // means prisma
                                                               // after
                    }
                }

                bool conditionsOK = true;
                for(int j{0}; j < 6; ++j) {
                    // if(timeSorted[0].timestamp==8403257937303) std::cout5A5B
                    // << "slave "<< brdSettings[j].slave << " prisma "<<
                    // prismaCoincidence<< " alone  " << aloneAnc[j] <<  "
                    // minmult  "<< minMultOK[j] <<  " conditionsOK " <<
                    // conditionsOK   << std::endl;

                    if((brdSettings[j].slave && !prismaCoincidence
                        && aloneAnc[j])
                       || !minMultOK[j])
                        conditionsOK = false;

                    // if(timeSorted[0].timestamp==8403257937303) std::cout <<
                    // "cond "<< j << " " << conditionsOK   << std::endl;
                }
                if(!conditionsOK) {
                    if(args.verb)
                        std::cout << "The event has not been accepted"
                                  << std::endl;
                    for(int i{0}; i < nb_event; ++i) timeSorted.pop_front();
                    continue;
                }
                if(args.verb)
                    std::cout << "Writing to the merged key into the adf file"
                              << std::endl;
                fwrite((char*)&mergedKey, sizeof(char), sizeof(agataKey),
                       pFile);
                //		if(1){
                //			std::cout<<"Merged key:
                //\n"<<mergedKey<<std::endl;
                // std::cout<<"PrismaSingleEvent:
                //\n"<<prismaSingleEvent<<std::endl;
                //		}

                if(mergeWithPrisma) {
                    if(prismaToWrite > 0) {
                        if(args.verb)
                            std::cout << "Writing prisma into the adf file"
                                      << std::endl;
                        fwrite(prismaSingleEvent, sizeof(char), prismaToWrite,
                               pFile);
                        prismaWriteRate++;
                        prismaToWrite = 0;
                    }
                }
                uint16_t k = 0;
                if(args.verb)
                    std::cout << "Writing nbevents=" << nb_event
                              << " into the adf file" << std::endl;
                while(k < nb_event) {
                    //	  std::cout << "  k = " << k << " nb_event = " <<
                    // nb_event
                    //		    << " " << timeSorted.size() << std::endl;
                    if(args.verb)
                        std::cout << "Writing subframe nr " << k << std::endl;
                    if(timeSorted.size() != 0) {
                        if(coincData.size() == 0)
                            coincData.push_back(timeSorted[0]);
                        else if(timeSorted[0].timestamp
                                    - coincData[0].timestamp
                                < coincWindowAnc)
                            coincData.push_back(timeSorted[0]);
                        else {
                            fillTree();
                            coincData.clear();
                            coincData.push_back(timeSorted[0]);
                        }
                        if(brdSettings[timeSorted[0].board].isPHA) {
                            caenEventPHA_t frame;
                            frame.theKey.size  = sizeof(caenEventPHA_t);
                            frame.theKey.key   = timeSorted[0].key;
                            frame.theKey.evnum = timeSorted[0].evnum;
                            frame.theKey.ts_0  = uint32_t(
                                 timeSorted[0].timestamp & 0xFFFFFFFF);
                            frame.theKey.ts_1 = uint32_t(
                                (timeSorted[0].timestamp >> 32) & 0x0000FFFF);
                            frame.theData.energy = timeSorted[0].energy;
                            frame.theData.cfd    = (uint16_t)timeSorted[0].cfd;

                            channel_id           = timeSorted[0].channel;
                            board_id             = timeSorted[0].board;
                            tsanc                = timeSorted[0].timestamp;
                            eneanc               = timeSorted[0].energy;
                            mult                 = nb_event;
                            time_anc             = timeSorted[0].timestamp * 10
                                     + timeSorted[0].cfd * 1000;
                            if(rootWrite) coincTree->Fill();
                            fwrite((char*)&frame, sizeof(char),
                                   frame.theKey.size, pFile);

                            channel_id = 0;
                            board_id   = 0;
                            tsanc      = 0;
                            eneanc     = 0;
                            mult       = 0;
                            time_anc   = 0;
                        } else {
                            caenEventPSD_t frame;

                            frame.theKey.size  = sizeof(caenEventPSD_t);
                            frame.theKey.key   = timeSorted[0].key;
                            frame.theKey.evnum = timeSorted[0].evnum;
                            frame.theKey.ts_0  = uint32_t(
                                 timeSorted[0].timestamp & 0xFFFFFFFF);
                            frame.theKey.ts_1 = uint32_t(
                                (timeSorted[0].timestamp >> 32) & 0x0000FFFF);
                            frame.theData.energy = timeSorted[0].energy;
                            frame.theData.qshort = timeSorted[0].qshort;
                            frame.theData.cfd    = (float)timeSorted[0].cfd;

                            channel_id           = timeSorted[0].channel;
                            board_id             = timeSorted[0].board;
                            tsanc                = timeSorted[0].timestamp;
                            mult                 = nb_event;
                            time_anc             = timeSorted[0].timestamp * 10
                                     + timeSorted[0].cfd * 1000;
                            if(rootWrite) coincTree->Fill();

                            fwrite((char*)&frame, sizeof(char),
                                   frame.theKey.size, pFile);

                            channel_id = 0;
                            board_id   = 0;
                            tsanc      = 0;
                            eneanc     = 0;
                            mult       = 0;
                            time_anc   = 0;
                        }

                        writeCountRate[timeSorted[0].board]
                                      [timeSorted[0].channel]++;

                        timeSorted.pop_front();
                    }
                    k++;
                }
                // std::cout << "Time sorted finished  " << timeSorted.size()
                // <<  std::endl;
                tsprisma = 0;
            }
        }
        // timeSorted.clear();
    }
    //  std::cout << "timesort end" << std::endl;
}

void depleteBuffers() {
    /*  for(auto it = eventContainer.begin() ; it!= eventContainer.end() ;
             it++){ std::cout << "Size of "<<it->first<< " "<<
       it->second.size() << std::endl;
             }*/
    // std::cout << "STARTING DEPLETION" << std::endl;

    bool circularBuffersEmpty{false};
    while(true) {
        internalData evtSmallestTS;
        uint16_t*    indexSTS   = nullptr;
        evtSmallestTS.timestamp = std::numeric_limits<uint64_t>::max();

        circularBuffersEmpty    = true;
        for(auto it = eventContainer.begin(); it != eventContainer.end();
            it++) {
            if(!it->second.empty()) circularBuffersEmpty = false;
            if(it->second.size() == 0) continue;
            if(evtSmallestTS.timestamp > it->second.front().timestamp) {
                evtSmallestTS = it->second.front();
                if(indexSTS == nullptr) indexSTS = new uint16_t;
                *indexSTS = it->first;
            }
        }
        if(circularBuffersEmpty) break;
        timeSorted.push_back(evtSmallestTS);
        if(indexSTS != nullptr) eventContainer[*indexSTS].pop_front();
        delete indexSTS;
    }

    while(timeSorted.size() > 3) {
        uint16_t nb_event = 1;
        bool     minMultOK[6];
        std::fill(minMultOK, minMultOK + 6, true);
        if(timeSorted.size() > 1) {
            while(timeSorted[nb_event].timestamp - timeSorted[0].timestamp
                  < coincWindowAnc) {
                if(nb_event >= timeSorted.size()) break;
                nb_event++;
            }
        }
        // std::cout << "keY " << std::hex <<  timeSorted[0].key;
        // std::cout << "     nbevent "<< nb_event << std::endl;

        // mergedKey.size  = timeSorted[0].theKey.size*nb_event+sizeof(agataKey);

        bool aloneAnc[6];
        std::fill(aloneAnc, aloneAnc + 6, false);
        for(int j = 0; j < 6; ++j) {
            //	std::cout << j << "-> "<< minMultOK  << std::endl;
            if(brdSettings[j].slave) { // if labr is requested as slave, checks
                // if there are any DANTE events
                aloneAnc[j] = true;
                for(int i{0}; i < nb_event; ++i) {
                    if(timeSorted[i].board != j) aloneAnc[j] = false;
                }
            }
            if(brdSettings[j].minMult > 1) {
                minMultOK[j]     = false;
                int boardCounter = 0;
                for(int i{0}; i < nb_event; ++i) {
                    if(timeSorted[i].board == j) boardCounter++;
                }

                // std::cout << "minDanteMult "<< minDanteMult  << std::endl;
                if(boardCounter >= brdSettings[j].minMult || boardCounter != 0)
                    minMultOK[j] = true;
            }
        }

        agataKey mergedKey;

        mergedKey.size = sizeof(agataKey);
        for(size_t i{0}; i < nb_event; ++i) {
            if(brdSettings[timeSorted[i].board].isPHA) {
                mergedKey.size += sizeof(caenEventPHA_t);
            } else {
                mergedKey.size += sizeof(caenEventPSD_t);
            }
        }
        mergedKey.key   = 0xCA020100;
        mergedKey.evnum = 0;
        mergedKey.ts_0  = uint32_t(timeSorted[0].timestamp & 0xFFFFFFFF);
        mergedKey.ts_1
            = uint32_t((timeSorted[0].timestamp >> 32) & 0x0000FFFF);
        // std::cout << "Merged ts = " << mergedKey.GetTstamp()  << std::endl;

        bool   prismaCoincidence = false;
        size_t prismaToWrite{0};
        char   prismaSingleEvent[512 * 512];
        if(mergeWithPrisma) {
            std::cout << "Merge with prisma" << std::endl;
            if(prismaBuffer == nullptr) {
                if(!fread((char*)&prismaMergedKey, 1, sizeof(agataKey),
                          args.prismaFiles.front())) {
                    fclose(args.prismaFiles.front());
                    std::cout << "End of prisma file " << prismaFileCounter
                              << std::endl;
                    prismaFileCounter++;
                    args.prismaFiles.pop_front();
                    fread((char*)&prismaMergedKey, 1, sizeof(agataKey),
                          args.prismaFiles.front());
                }
                prismaBuffer = new char[512 * 512];
                if(!fread(prismaBuffer, 1,
                          prismaMergedKey.GetBytes() - sizeof(agataKey),
                          args.prismaFiles.front())) {
                    fclose(args.prismaFiles.front());
                    std::cout << "End of prisma file " << prismaFileCounter
                              << std::endl;
                    prismaFileCounter++;
                    args.prismaFiles.pop_front();
                    fread(prismaBuffer, 1,
                          prismaMergedKey.GetBytes() - sizeof(agataKey),
                          args.prismaFiles.front());
                }
                prismaReadRate++;
            }
            auto prismaTS = prismaMergedKey.GetTstamp();
            auto diffBegin
                = mergedKey.GetTstamp() > prismaTS
                    ? -(int64_t)(mergedKey.GetTstamp() - prismaTS)
                    : (int64_t)(prismaTS
                                - mergedKey.GetTstamp()); // Positive number
            // means prisma after
            auto diffEnd
                = timeSorted[nb_event - 1].timestamp > prismaTS
                    ? -(int64_t)(timeSorted[nb_event - 1].timestamp - prismaTS)
                    : (int64_t)(prismaTS
                                - timeSorted[nb_event - 1]
                                      .timestamp); // Positive number means
            // prisma after

            tsprisma = prismaTS;
            while(diffBegin < coincWindowAnc) { // Probabily 100, possibly 50
                if(diffBegin < -coincWindowAnc
                   || (diffBegin >= -coincWindowAnc && diffBegin < 0
                       && diffEnd
                              < -coincWindowAnc)) { // Prisma before and alone

                    fwrite((char*)&prismaMergedKey, 1, sizeof(agataKey),
                           pFile); // TODO: check if file is finished
                    fwrite(prismaBuffer, 1,
                           prismaMergedKey.GetBytes() - sizeof(agataKey),
                           pFile);
                    prismaWriteRate++;
                    tsprisma = prismaTS;
                    if(rootWrite) coincTree->Fill();
                    tsprisma = 0;

                } else if(diffBegin < 0
                          && diffEnd
                                 >= -coincWindowAnc) { // Prisma before in coinc
                    mergedKey.ts_0 = uint32_t(prismaTS & 0xFFFFFFFF);
                    mergedKey.ts_1 = uint32_t((prismaTS >> 32) & 0x0000FFFF);
                    mergedKey.size
                        += prismaMergedKey.GetBytes() - sizeof(agataKey);
                    prismaToWrite
                        = prismaMergedKey.GetBytes() - sizeof(agataKey);
                    tsprisma          = prismaTS;
                    prismaCoincidence = true;
                    memcpy(prismaSingleEvent, prismaBuffer, prismaToWrite);
                } else { // Prisma after in coinc
                    mergedKey.size
                        += prismaMergedKey.GetBytes() - sizeof(agataKey);
                    prismaToWrite
                        = prismaMergedKey.GetBytes() - sizeof(agataKey);
                    tsprisma          = prismaTS;
                    prismaCoincidence = true;
                    memcpy(prismaSingleEvent, prismaBuffer, prismaToWrite);
                }
                if(!fread((char*)&prismaMergedKey, 1, sizeof(agataKey),
                          args.prismaFiles.front())) {
                    fclose(args.prismaFiles.front());
                    std::cout << "End of prisma file " << prismaFileCounter
                              << std::endl;
                    prismaFileCounter++;
                    args.prismaFiles.pop_front();
                    fread((char*)&prismaMergedKey, 1, sizeof(agataKey),
                          args.prismaFiles.front());
                }
                fread(prismaBuffer, 1,
                      prismaMergedKey.GetBytes() - sizeof(agataKey),
                      args.prismaFiles.front());
                prismaReadRate++;
                prismaTS = prismaMergedKey.GetTstamp();
                diffBegin
                    = mergedKey.GetTstamp() > prismaTS
                        ? -(int64_t)(mergedKey.GetTstamp() - prismaTS)
                        : (int64_t)(prismaTS
                                    - mergedKey.GetTstamp()); // Positive number
                // means prisma after
                diffEnd = timeSorted[nb_event - 1].timestamp > prismaTS
                            ? -(int64_t)(timeSorted[nb_event - 1].timestamp
                                         - prismaTS)
                            : (int64_t)(prismaTS
                                        - timeSorted[nb_event - 1].timestamp);
            }
        }

        bool conditionsOK = true;
        for(int j{0}; j < 6; ++j) {
            if((brdSettings[j].slave && !prismaCoincidence && aloneAnc[j])
               || !minMultOK[j])
                conditionsOK = false;
        }
        if(!conditionsOK) {
            for(int i{0}; i < nb_event; ++i) timeSorted.pop_front();
            continue;
        }

        fwrite((char*)&mergedKey, sizeof(char), sizeof(agataKey), pFile);
        //	if(nbAggr<50){
        //	std::cout<<"Merged key: \n"<<mergedKey<<std::endl;
        //	std::cout<<"PrismaSingleEvent:
        //\n"<<prismaSingleEvent<<std::endl;
        //
        //	}
        if(mergeWithPrisma) {
            if(prismaToWrite > 0) {
                fwrite(prismaSingleEvent, sizeof(char), prismaToWrite, pFile);
                prismaWriteRate++;
                prismaToWrite = 0;
            }
        }
        uint16_t k = 0;
        while(k < nb_event) {
            if(timeSorted.size() < 20)
                std::cout << "  k = " << k << " nb_event = " << nb_event << " "
                          << timeSorted.size() << std::endl;

            if(timeSorted.size() != 0) {
                if(coincData.size() == 0)
                    coincData.push_back(timeSorted[0]);
                else if(timeSorted[0].timestamp - coincData[0].timestamp
                        < coincWindowAnc)
                    coincData.push_back(timeSorted[0]);
                else {
                    fillTree();
                    coincData.clear();
                    coincData.push_back(timeSorted[0]);
                }
                if(brdSettings[timeSorted[0].board].isPHA) {
                    caenEventPHA_t frame;

                    frame.theKey.size  = sizeof(caenEventPHA_t);
                    frame.theKey.key   = timeSorted[0].key;
                    frame.theKey.evnum = timeSorted[0].evnum;
                    frame.theKey.ts_0
                        = uint32_t(timeSorted[0].timestamp & 0xFFFFFFFF);
                    frame.theKey.ts_1 = uint32_t(
                        (timeSorted[0].timestamp >> 32) & 0x0000FFFF);
                    frame.theData.energy = timeSorted[0].energy;
                    frame.theData.cfd    = (uint16_t)timeSorted[0].cfd;

                    channel_id           = timeSorted[0].channel;
                    board_id             = timeSorted[0].board;
                    tsanc                = timeSorted[0].timestamp;
                    eneanc               = timeSorted[0].energy;
                    mult                 = nb_event;
                    time_anc             = timeSorted[0].timestamp * 10
                             + timeSorted[0].cfd * 1000;
                    if(rootWrite) coincTree->Fill();
                    fwrite((char*)&frame, sizeof(char), frame.theKey.size,
                           pFile);
                    channel_id = 0;
                    board_id   = 0;
                    tsanc      = 0;
                    eneanc     = 0;
                    mult       = 0;
                    time_anc   = 0;
                } else {
                    caenEventPSD_t frame;

                    frame.theKey.size  = sizeof(caenEventPSD_t);
                    frame.theKey.key   = timeSorted[0].key;
                    frame.theKey.evnum = timeSorted[0].evnum;
                    frame.theKey.ts_0
                        = uint32_t(timeSorted[0].timestamp & 0xFFFFFFFF);
                    frame.theKey.ts_1 = uint32_t(
                        (timeSorted[0].timestamp >> 32) & 0x0000FFFF);
                    frame.theData.energy = timeSorted[0].energy;
                    frame.theData.qshort = timeSorted[0].qshort;
                    frame.theData.cfd    = (float)timeSorted[0].cfd;

                    channel_id           = timeSorted[0].channel;
                    board_id             = timeSorted[0].board;
                    tsanc                = timeSorted[0].timestamp;
                    eneanc               = timeSorted[0].energy;
                    mult                 = nb_event;
                    time_anc             = timeSorted[0].timestamp * 10
                             + timeSorted[0].cfd * 1000;
                    if(rootWrite) coincTree->Fill();
                    channel_id = 0;
                    board_id   = 0;
                    tsanc      = 0;
                    eneanc     = 0;
                    mult       = 0;
                    time_anc   = 0;

                    fwrite((char*)&frame, sizeof(char), frame.theKey.size,
                           pFile);
                }
                writeCountRate[timeSorted[0].board][timeSorted[0].channel]++;
                timeSorted.pop_front();
            }
            k++;
            if(rootWrite) {
                fTree->Fill();
                resetTreeEvents();
            }
        }
        tsprisma = 0;
    }
    std::cout << "Last TS of caen: "
              << timeSorted[0].timestamp * 10 + timeSorted[0].cfd * 1000
              << std::endl;
    while(!args.prismaFiles.empty()) {
        if(!fread((char*)&prismaMergedKey, 1, sizeof(agataKey),
                  args.prismaFiles.front())) {
            fclose(args.prismaFiles.front());
            args.prismaFiles.pop_front();
            if(args.prismaFiles.empty()) break;
            std::cout << "End of prisma file " << prismaFileCounter
                      << std::endl;
            prismaFileCounter++;
            fread((char*)&prismaMergedKey, 1, sizeof(agataKey),
                  args.prismaFiles.front());
        }
        if(!fread(prismaBuffer, 1,
                  prismaMergedKey.GetBytes() - sizeof(agataKey),
                  args.prismaFiles.front())) {
            fclose(args.prismaFiles.front());
            std::cout << "End of prisma file " << prismaFileCounter
                      << std::endl;
            prismaFileCounter++;
            args.prismaFiles.pop_front();
            if(args.prismaFiles.empty()) break;
        } else {
            fwrite((char*)&prismaMergedKey, 1, sizeof(agataKey), pFile);
            fwrite(prismaBuffer, 1,
                   prismaMergedKey.GetBytes() - sizeof(agataKey), pFile);
            prismaWriteRate++;
            if(rootWrite) coincTree->Fill();
        }
        prismaReadRate++;
    }
    // timeSorted.clear();
    std::cout << "Last TS of PRISMA: " << prismaMergedKey.GetTstamp()
              << std::endl;
}

// ******************************************************
// ******************************************************
// DECODE OF CAEN V1 DIGITIZER DATA (PHA)
// ******************************************************
// ******************************************************

void decode_PHA_v1(uint32_t* inpBuffer) {
    // this is directly taken from the LocalWorkerPHA used in
    // the XDAQ application for the LocalFilter
    // just copy the function LocalWorkerPHA::ProcessBlock
    int             error_code = 0;
    uint32_t        posBuffer  = 0;
    std::bitset<32> decodeField;
    int             countEvents = 0;
    // Aggregate buffer has a header containing different information
    uint32_t        aggLength   = inpBuffer[posBuffer++] & 0x0FFFFFFF;
    decodeField                 = inpBuffer[posBuffer++];
    uint32_t       board = project_range<27, 31>(decodeField).to_ulong();
    bool           boardFailFlag = decodeField.test(26);
    std::bitset<8> channelMask   = project_range<0, 7>(decodeField).to_ulong();
    uint32_t       aggregateCounter = inpBuffer[posBuffer++] & 0x7FFFFF;
    uint32_t       aggregateTimeTag = inpBuffer[posBuffer++];
    std::array<uint16_t, 8> couples;
    uint16_t                index = 0;
    std::bitset<16>         extraBit;
    for(uint16_t bit = 0; bit < 8; bit++) {
        if(channelMask.test(bit)) {
            couples[index] = bit;
            index++;
        }
    }
    uint32_t coupleAggregateSize = 0;
    uint64_t tstamp_10ns         = 0;
    uint64_t tstamp              = 0;
    uint32_t fineTS              = 0;
    int64_t  offsetTS            = 0;
    uint32_t flags               = 0;
    bool     pur                 = false;
    bool     satu                = false;
    bool     lost                = false;
    bool     trap_sat            = false;
    uint32_t chanNum             = 0;
    float    cfd                 = 0.;
    float    offlineCFD          = 0.;
    bool     calcCFD             = false;
    float*   aTrace              = nullptr;
    float*   wspace              = nullptr;

    for(uint8_t cpl = 0; cpl < channelMask.count(); cpl++) {
        coupleAggregateSize = inpBuffer[posBuffer++] & 0x3FFFFF;
        dataFormPHA.SetDataFormat(inpBuffer[posBuffer++]);
        if(args.verb)
            std::cout << "--------------- start decode ---------------"
                      << std::endl;

        if(args.verb) dataFormPHA.printDataFormat();
        const uint16_t evtSize = dataFormPHA.evtSize();
        if(evtSize == 0) {
            std::cout << __func__ << ": at line " << __LINE__ << std::endl;
            dataFormPHA.printDataFormat();
            break;
        }
        for(uint16_t evt = 0; evt < (coupleAggregateSize - 2) / evtSize;
            evt++) {
            float trig  = 0.;
            cfd         = 0.;
            offlineCFD  = 0.;
            calcCFD     = false;
            decodeField = inpBuffer[posBuffer++];
            tstamp      = project_range<0, 30>(decodeField).to_ulong();
            chanNum     = static_cast<uint32_t>(
                project_range<31, 31>(decodeField).to_ulong()
                + couples[cpl] * 2);
            if(args.verb)
                std::cout << "---------------this data was from board: "
                          << board << " channel " << chanNum
                          << " ---------------" << std::endl;
            if(dataFormPHA.enaTrace()) {
                std::valarray<float> samples;
                int                  numSamples = (dataFormPHA.enaDT())
                                                    ? dataFormPHA.numSamples() / 2
                                                    : dataFormPHA.numSamples();
                int                  trigSamp   = 0;
                samples.resize(numSamples);
                for(uint16_t s = 0; s < dataFormPHA.numSamples() / 2; ++s) {
                    decodeField = inpBuffer[posBuffer++];
                    if(dataFormPHA.enaAnalog()) {
                        if(!dataFormPHA.enaDT()) {
                            samples[s * 2]
                                = project_range<0, 13>(decodeField).to_ulong()
                                + (rand() % 1000) / 1000. - 0.5;
                            samples[s * 2 + 1]
                                = project_range<16, 29>(decodeField).to_ulong()
                                + (rand() % 1000) / 1000. - 0.5;
                            if(decodeField.test(15))
                                trigSamp = s * 2;
                            else if(decodeField.test(31))
                                trigSamp = s * 2 + 1;
                        } else {
                            if(dataFormPHA.confAnaProbe1() == 0) {
                                if(decodeField.test(31)) trigSamp = s;
                                samples[s] = project_range<16, 29>(decodeField)
                                                 .to_ulong()
                                           + (rand() % 1000) / 1000. - 0.5;
                            } else if(dataFormPHA.confAnaProbe2() == 0) {
                                if(decodeField.test(15)) trigSamp = s;
                                samples[s] = project_range<0, 13>(decodeField)
                                                 .to_ulong()
                                           + (rand() % 1000) / 1000. - 0.5;
                            } else
                                continue;
                        }
                    }
                }
                if(args.write_traces) {
                    std::fill(intEvt.samples.begin(), intEvt.samples.end(), 0);
                }
                if(dataFormPHA.enaAnalog()) {
                    auto&                wa = waveAnalyzer[board][chanNum];
                    std::valarray<float> baseSamples
                        = samples[std::slice(0, wa.nDcwin, 1)];
                    float baseline = baseSamples.sum() / wa.nDcwin;
                    samples -= baseline;
                    // printf("Baseline = %.3f\n",baseline);
                    if(samples.sum() < 0) samples *= -1;
                    if(wa.GetNSamples() != samples.size())
                        wa.SetNSamples(samples.size());
                    if(aTrace == nullptr) {
                        aTrace = new float[samples.size()];
                        // wa.nDcwin = nbSampBase; // the first 20% --> check
                        // if this is valid
                    }
                    if(wspace == nullptr)
                        wspace = new float[3 * samples.size()];

                    std::copy(begin(samples), end(samples), aTrace);

                    if(args.plot.first == board
                       && args.plot.second == chanNum) {
                        std::cout << "Drawing canvas" << std::endl;
                        TCanvas            c;
                        std::vector<float> xvals(args.n_samples);
                        std::iota(xvals.begin(), xvals.end(), 0);
                        TGraph g1(args.n_samples, xvals.data(), aTrace);
                        TGraph g2(args.n_samples, xvals.data(), wspace);

                        intEvt.imax
                            = wa.TriangAmpli(aTrace, -1, wspace, false);
                        TGraph g3(args.n_samples, xvals.data(), wspace);

                        g1.SetLineColor(kRed + 1);
                        g2.SetLineColor(kBlue + 1);
                        g1.SetLineWidth(3);
                        g2.SetLineWidth(3);
                        g1.SetMarkerColor(kRed + 2);
                        g2.SetMarkerColor(kBlue + 2);
                        g1.SetMarkerStyle(8);
                        g2.SetMarkerStyle(8);
                        g1.Draw("apl");
                        g2.Draw("pl");
                        g3.Draw("pl");
                        c.Update();
                        c.WaitPrimitive();
                    }

                    if(args.write_traces) {
                        intEvt.samples.resize(args.n_samples);
                        unsigned int n_copy = samples.size() <= args.n_samples
                                                ? samples.size()
                                                : args.n_samples;
                        std::copy_n(begin(samples), n_copy,
                                    begin(intEvt.samples));
                    }
                    std::copy(begin(samples), end(samples), aTrace);
                    trig       = wa.TriggerCFD(aTrace, 50, wspace, offlineCFD);
                    offlineCFD = (trigSamp - offlineCFD)
                               * brdSettings[board].fNsPerSample
                               * ((dataFormPHA.enaDT()) ? 2 : 1);
                    calcCFD = true;
                    // printf("trigSamp = %i offlineCFD = %.3f for channel"
                    //        "%i %i\n",trigSamp,offlineCFD,chanNum,board);
                }
            }
            if(dataFormPHA.enaExtras2()) { // decoding of EXTRAS2
                decodeField = inpBuffer[posBuffer++];
                switch(dataFormPHA.confExtras()) {
                case 0: // Extended Time Stamp [31:16] ; baseline*4 [15:0]
                    tstamp
                        = (static_cast<uint64_t>(
                               project_range<16, 31>(decodeField).to_ulong())
                           << 31)
                        | (uint64_t)tstamp;
                    break;
                case 1: // Reserved??
                    break;
                case 2: // Extended Time stamp [31:16] ; Reserved [15:10] ;
                    // Fine Time Stamp [9:0]
                    fineTS = static_cast<uint16_t>(
                        project_range<0, 9>(decodeField).to_ulong());
                    cfd = brdSettings[board].fNsPerTStamp / 1.024
                        * fineTS; // to have it in ps units
                    tstamp
                        = (static_cast<uint64_t>(
                               project_range<16, 31>(decodeField).to_ulong())
                           << 31)
                        | (uint64_t)tstamp;
                    break;
                case 3: // Reserved
                    break;
                case 4: // Lost Trigger Counter [31:16] ; Total Trigger [15:0]
                    break;
                case 5: // Positive zero crossing [31:16] ; Negative zero
                    // crossing [15:0]
                    break;
                case 7: // Reserved
                    break;
                default: break;
                }
            }
            decodeField = inpBuffer[posBuffer++];
            // the last word contains energy[0-14], PU [15] and extra[16:25]
            satu        = decodeField.test(4 + 16);
            lost        = decodeField.test(5 + 16);
            pur         = decodeField.test(9 + 16);
            trap_sat    = decodeField.test(10 + 16);

            //	statsData.time[cpl] = tstamp*2/pow( 10, 9 );   // must be in ns

            offsetTS
                = static_cast<uint64_t>(brdSettings[board].tsOffset[chanNum]);

            tstamp_10ns = tstamp * brdSettings[board].fNsPerTStamp / 10;
            cfd += (tstamp * brdSettings[board].fNsPerTStamp
                    - tstamp_10ns * 10)
                 * 1000; // in ps no to loose precision when using uint16_t
            if(calcCFD)
                offlineCFD = offlineCFD * 1000
                           + (tstamp * brdSettings[board].fNsPerTStamp
                              - tstamp_10ns * 10)
                                 * 1000; // in ps no to loose precision when
            // using uint16_t
            tstamp_10ns += offsetTS + fGlobalOffset;
            if(!calcCFD)
                intEvt.cfd = static_cast<uint32_t>(std::floor(cfd));
            else
                intEvt.cfd = static_cast<uint32_t>(std::floor(offlineCFD));

            if(intEvt.cfd > 10000) {
                intEvt.cfd -= 10000;
                tstamp_10ns += 1;
            }

            // Adding PUR to the flag
            intEvt.energy = static_cast<uint16_t>(
                project_range<0, 15>(decodeField).to_ulong());
            decodeField = (board << 8 | chanNum);

            if(pur) { decodeField.set(16); }
            if(trap_sat) { decodeField.set(17); }
            if(satu) { decodeField.set(18); }
            if(lost) { decodeField.set(19); }
            if(boardFailFlag) { decodeField.set(20); }

            // N is set from bits[17:16] of register 0x1nA0 (default value
            // 1024) Adding flags to the evt num
            intEvt.key       = brdSettings[board].key;

            intEvt.board     = board;
            intEvt.channel   = chanNum;
            intEvt.timestamp = tstamp_10ns;
            intEvt.qshort    = 0;

            intEvt.evnum     = decodeField.to_ulong();
            //      printf("%08X\n",intEvt.evnum);
            readCountRate[board][chanNum]++;
            if(intEvt.energy < 5) continue;
            timeSortEvent(intEvt);

            if(rootWrite) {
                hitpatternPHA->Fill(board * maxchannels + chanNum,
                                    intEvt.energy);
                fOutput->cd();
                // fTree->Fill();
            }
        }
    }
    if(aTrace != nullptr) delete aTrace;
    if(wspace != nullptr) delete wspace;
}

// ******************************************************
// ******************************************************
// DECODE OF CAEN V1 DIGITIZER DATA (PSD)
// ******************************************************
// ******************************************************

void decode_PSD_v1(uint32_t* inpBuffer) {
    // this is directly taken from the LocalWorkerPSD used in
    // the XDAQ application for the LocalFilter
    // just copy the function LocalWorkerPSD::ProcessBlock

    uint32_t        posBuffer = 0;
    std::bitset<32> decodeField;
    // Aggregate buffer has a header containing different information
    uint32_t        aggLength = inpBuffer[posBuffer++] & 0x0FFFFFFF;
    decodeField               = inpBuffer[posBuffer++];
    uint32_t       board      = project_range<27, 31>(decodeField).to_ulong();
    bool           boardFailFlag = decodeField.test(26);
    std::bitset<8> channelMask   = project_range<0, 7>(decodeField).to_ulong();
    uint32_t       aggregateCounter = inpBuffer[posBuffer++] & 0x7FFFFF;
    uint32_t       aggregateTimeTag = inpBuffer[posBuffer++];

    uint8_t        couples[8]       = {0, 0, 0, 0, 0, 0, 0, 0};
    // we need to reconstruct the channel number from the couple id for the
    // 16 channels boards
    int            index            = 0;
    for(uint16_t bit = 0; bit < 8; bit++) {
        if(channelMask.test(bit)) { couples[index++] = bit; }
    }

    uint32_t coupleAggregateSize = 0;
    uint64_t tstamp_10ns         = 0;
    uint64_t tstamp              = 0;
    uint32_t fineTS              = 0;
    int64_t  offsetTS            = 0;
    uint32_t flags               = 0;
    bool     pur                 = false;
    bool     ovr                 = false;
    bool     lost                = false;
    uint32_t chanNum             = 0;
    float*   aTrace              = nullptr;
    float*   wspace              = nullptr;

    // here starts the real decoding of the data for the channels
    for(uint8_t cpl = 0; cpl < channelMask.count(); cpl++) {
        coupleAggregateSize = inpBuffer[posBuffer++] & 0x3FFFFF;
        dataFormPSD.SetDataFormat(inpBuffer[posBuffer++]);
        if(args.verb) dataFormPSD.printDataFormat();
        const uint16_t evtSize = dataFormPSD.evtSize();
        if(evtSize == 0) {
            std::cout << __func__ << ": at line " << __LINE__ << std::endl;
            dataFormPSD.printDataFormat();
        }

        // Depacking the dual channel aggregate

        for(uint16_t evt = 0; evt < (coupleAggregateSize - 2) / evtSize;
            evt++) {
            intEvt.cfd  = 0;
            flags       = 0;
            decodeField = inpBuffer[posBuffer++];
            tstamp      = project_range<0, 30>(decodeField).to_ulong();
            chanNum     = static_cast<uint32_t>(
                project_range<31, 31>(decodeField).to_ulong()
                + couples[cpl] * 2);
            float trig = 0;
            // if the trace are present we should decode them or at least jump
            // the samples
            if(args.write_traces) {
                std::fill(intEvt.samples.begin(), intEvt.samples.end(), 0);
            }
            if(dataFormPSD.enaTrace()) {
                std::valarray<float> samples;
                samples.resize(dataFormPSD.numSamples());
                for(uint16_t s = 0; s < dataFormPSD.numSamples() / 2; ++s) {
                    decodeField = inpBuffer[posBuffer++];
                    samples[s * 2]
                        = project_range<0, 13>(decodeField).to_ulong()
                        + (rand() % 1000) / 1000. - 0.5;
                    samples[s * 2 + 1]
                        = project_range<16, 29>(decodeField).to_ulong()
                        + (rand() % 1000) / 1000. - 0.5;
                }
                auto& wa       = waveAnalyzer[board][channel_id];
                float baseline = static_cast<std::valarray<float>>(
                                     samples[std::slice(0, wa.nDcwin, 1)])
                                     .sum()
                               / wa.nDcwin;
                samples -= baseline;
                if(samples.sum() < 0) samples *= -1;
                if(aTrace == nullptr) aTrace = new float[samples.size()];
                if(wspace == nullptr) wspace = new float[3 * samples.size()];
                std::copy(begin(samples), end(samples), aTrace);
                if(args.write_traces) {
                    intEvt.samples.resize(args.n_samples);
                    unsigned int n_copy = samples.size() <= args.n_samples
                                            ? samples.size()
                                            : args.n_samples;
                    std::copy_n(begin(samples), n_copy, begin(intEvt.samples));
                }
                float otrig = 0;

                trig        = wa.TriggerCFD(aTrace, 5, wspace, otrig);
                // trig = waveAnalyzer[board][channel_id].TriggerCFD(aTrace, 5,
                // wspace, otrig); 	std::cout << "trig = " << trig << "
                // otrig =
                //" << otrig
                //<< std::endl;

                // samples.clear();
            }
            // After the trace we have the extra if the extra is present
            if(dataFormPSD.enaExtras()) {
                decodeField = inpBuffer[posBuffer++];
                switch(dataFormPSD.confExtras()) {
                case 0: // Extended Time Stamp [31:16] ; baseline*4 [15:0]
                    tstamp
                        = (static_cast<uint64_t>(
                               project_range<16, 31>(decodeField).to_ulong())
                           << 31)
                        | tstamp;
                    break;
                case 1: // Extended Time stamp [31:16] ; flags [15:0]
                    // bit[15] Tigger lost 0x40000000
                    // bit[14] Over-range 0x20000000
                    // bit[13] event counter every 1024 events bit up
                    // 0x10000000 bit[12] N lost trigger counted 0x08000000
                    lost = decodeField.test(15);
                    ovr  = decodeField.test(14);
                    tstamp
                        = (static_cast<uint64_t>(
                               project_range<16, 31>(decodeField).to_ulong())
                           << 31)
                        | tstamp;
                    break;
                case 2: // Extended Time stamp [31:16] ; Flags [15:10] ; Fine
                    // Time Stamp [9:0]
                    // bit[15] Tigger lost 0x40000000
                    // bit[14] Over-range 0x20000000
                    // bit[13] event counter every 1024 events bit up
                    // 0x10000000 bit[12] N lost trigger counted 0x08000000
                    lost   = decodeField.test(15);
                    ovr    = decodeField.test(14);
                    fineTS = static_cast<uint16_t>(
                        project_range<0, 9>(decodeField).to_ulong());
                    intEvt.cfd
                        = (brdSettings[board].fNsPerTStamp / 1024. * fineTS)
                        / 10; // to have it in 10 ns units
                    tstamp
                        = (static_cast<uint64_t>(
                               project_range<16, 31>(decodeField).to_ulong())
                           << 31)
                        | tstamp;
                    break;
                case 4: // Lost Trigger Counter [31:16] ; Total Trigger [15:0]
                    break;
                case 5: // Positive zero crossing [31:16] ; Negative zero
                    // crossing [15:0]
                    break;
                case 7: // Debug fixed value;
                    break;
                default: break;
                }
            }
            //	qshort  =
            // static_cast<uint16_t>((inpBuffer[posBuffer+evtSize-1+evtSize*evt+2])&0x7FFF);
            // last word of the buffer has the energies
            decodeField   = inpBuffer[posBuffer++];
            pur           = decodeField.test(15);
            intEvt.qshort = static_cast<uint16_t>(
                project_range<0, 14>(decodeField).to_ulong());
            intEvt.energy = static_cast<uint16_t>(
                project_range<16, 31>(decodeField).to_ulong());

            offsetTS    = brdSettings[board].tsOffset[chanNum];

            tstamp_10ns = tstamp * brdSettings[board].fNsPerTStamp / 10;
            intEvt.cfd += (float)(tstamp * brdSettings[board].fNsPerTStamp
                                  - tstamp_10ns * 10)
                        / 10.;
            tstamp_10ns += offsetTS + fGlobalOffset;
            // Adding PUR to the flags

            flags = ovr << 18 | pur << 16;
            readCountRate[board][chanNum]++;

            intEvt.key       = brdSettings[board].key;
            intEvt.board     = board;
            intEvt.channel   = chanNum;
            intEvt.timestamp = tstamp_10ns;

            intEvt.evnum
                = /*extraBit[0] << 19 | isat << 18 | tsat << 17 |*/ pur << 16
                | board << 8 | chanNum;
            // printf("boardid %i, channel %i, timestamp %u, energy
            // %i\n", boardId, channelId, tstamp, energy );
            if(intEvt.qshort < 5) continue;
            timeSortEvent(intEvt);

            if(rootWrite) {
                hitpatternPSD->Fill(board * 16 + chanNum, intEvt.energy);
                fOutput->cd();
                // fTree->Fill();
            }
        }
    }
    if(aTrace != nullptr) delete aTrace;
    if(wspace != nullptr) delete wspace;
}

// ******************************************************
// ******************************************************
// DECODE OF CAEN V2 DIGITIZER DATA (PHA)
// ******************************************************
// ******************************************************
void decode_PHA_v2(uint32_t* inpBuff) {
    //  std::cout << __func__ << ": at line " << __LINE__ << std::endl;

    uint64_t*       inpBuffer = reinterpret_cast<uint64_t*>(inpBuff);
    uint32_t        posBuffer = 0;
    std::bitset<64> decodeField;
    std::bitset<64> sample[2];
    uint64_t        tstamp_10ns = 0;
    uint64_t        tstamp;
    int64_t         offsetTS = 0;
    decodeField              = inpBuffer[posBuffer++];
    uint16_t board           = project_range<59, 63>(decodeField).to_ulong();
    uint16_t boardID; // to match the 16 channels for the BU
    uint16_t channel;
    uint16_t chanID; // again to match the max 16 channels / board ... useful?
                     // no but don't want to modify the BU
    uint16_t energy;
    uint16_t fineTS;
    float    cfd;
    float    fineTS_2_GTS = 8 / 1024.;
    float*   aTrace       = nullptr;
    float*   wspace       = nullptr;

    uint32_t inpBuffSize  = project_range<0, 26>(decodeField).to_ulong() / 2;
    while(posBuffer < inpBuffSize) {
        decodeField      = swapLong(((uint64_t*)buffer)[posBuffer]);
        uint32_t n_words = project_range<0, 31>(decodeField).to_ulong();
        uint32_t headKey = project_range<60, 63>(decodeField).to_ulong();
        uint32_t aggrNb  = project_range<32, 55>(decodeField).to_ulong();
        //    printf("headKey = %i, n_words = %i, aggrNb = %i, boardNb =
        //    %i\n",headKey,n_words,aggrNb,board);
        if(n_words == 4 && headKey == 0x3) { // START BLOCK
            // This is the start run frame we do not really care about it
            posBuffer += n_words;
            //    printf("Start header posBuffer = %i while input buffer size =
            //    %i\n",posBuffer,inpBuffSize);
            continue;
        }
        if(n_words == 3 && headKey == 0x3) { // STOP BLOCK
            // This is the start run frame we do not really care about it
            posBuffer += n_words;
            //      printf("Stop block detected ... we should have all the
            //      data\n");
            continue;
        }
        //        printf("Number of words in the aggregate = %i (size buffer =
        //        %i) at pos %i for board %i\n",
        // 	   n_words,inpBuffSize,posBuffer,board);

        // So now we should have aggregate events.
        // Since we could maybe have more than 1 aggregate / buffer we loop
        // again.
        //    std::cout << "posBuffer = " << posBuffer << " and input buffer
        //    size = " << inpBuffSize << std::endl;
        uint32_t posEvent = posBuffer++;
        while(posEvent < posBuffer + n_words) {
            if(posEvent == inpBuffSize) break;
            uint32_t flags = 0;
            // if(posAggregate == 0 ) posAggregate++;
            // std::cout << "posEvents = " << posEvent << " inpSize = " <<
            // inpBuffSize << std::endl;
            decodeField    = swapLong(inpBuffer[++posEvent]);
            channel        = project_range<56, 62>(decodeField).to_ulong();
            tstamp         = project_range<0, 47>(decodeField).to_ulong();
            intEvt.channel = channel % 16;
            intEvt.board   = board + channel / 16;

            //   printf("posEvent=%5i - %llx - swapped %llx Channel = %3i,
            //   tstamp =
            //   %llu\n",posEvent,inpBuffer[posEvent],decodeField.to_ulong(),channel,tstamp);
            //       if(decodeField.test(55)) std::cout << "Event with
            //       statistic only" << std::endl;
            // std::cout << " --> Event from channel " << channel << " with
            // TStamp = " << tstamp <<  std::endl;
            decodeField    = swapLong(inpBuffer[++posEvent]);
            energy         = project_range<0, 15>(decodeField).to_ulong();
            fineTS         = project_range<16, 25>(decodeField).to_ulong();
            cfd            = fineTS * fineTS_2_GTS;
            tstamp_10ns    = (tstamp * 8) / 10;
            cfd += (tstamp * 8 - tstamp_10ns * 10);
            // bits 42 to 61 are flags (42-49: high priority, 50-61 low
            // priority)
            // page 51 of rev6
            bool pur     = decodeField.test(42);
            bool purGard = decodeField.test(43);
            bool inpSat  = decodeField.test(44);
            bool trpSat  = decodeField.test(46);
            bool sftTrig = decodeField.test(55);
            bool glbTrig = decodeField.test(54);
            bool extTrig = decodeField.test(53);
            bool slfTrig = decodeField.test(56);
            flags        = decodeField[44] << 18 | decodeField[46] << 17
                  | decodeField[42] << 16;
            if(!slfTrig) { // considered as an idle event for us
                //	std::cout << "Global trigger event for board " << board
                //<< 	  std::endl;
                flags |= 0x80000000;
                // posEvent++;
                //	posBuffer += (posEvent - posBuffer);
                //	continue;
                // std::cout << decodeField.to_string() << std::endl;
            }
            bool wavePresent = decodeField.test(62);
            //      std::cout << "Wave are present? " <<((wavePresent)? "yes" :
            //      "no")
            //		<< " i am idle? "<< ((!slfTrig)? "yes":"no") <<
            // std::endl;
            //      std::cout << std::dec << "Energy = " << energy << " fineTS
            //            = " << fineTS << std::endl;
            // extra
            if(args.write_traces) {
                std::fill(intEvt.samples.begin(), intEvt.samples.end(), 0);
            }
            if(wavePresent) {
                bool     lastExtra = decodeField.test(63);
                uint16_t extraType = 0;
                uint16_t ap1_Type, ap1_Mult, ap2_Type, ap2_Mult;
                bool     ap1_Signed, ap2_Signed;
                uint16_t dp_Type[4];
                uint16_t triggerThres;
                uint16_t downSamp;
                while(!lastExtra) {
                    decodeField = swapLong(inpBuffer[++posEvent]);
                    // printf("decodeField extra = %llx\n",decodeField);
                    lastExtra   = decodeField.test(63);
                    extraType = project_range<60, 62>(decodeField).to_ulong();
                    // std::cout << "extra type = " << extraType << " at
                    // posEvent = " << posEvent << std::endl;
                    switch(extraType) {
                    case 0: // waveform configuration
                        ap1_Type = project_range<0, 2>(decodeField).to_ulong();
                        ap1_Signed = decodeField.test(3);
                        ap1_Mult = project_range<4, 5>(decodeField).to_ulong();
                        ap2_Type = project_range<6, 8>(decodeField).to_ulong();
                        ap2_Signed = decodeField.test(9);
                        ap2_Mult
                            = project_range<10, 11>(decodeField).to_ulong();
                        dp_Type[0]
                            = project_range<12, 15>(decodeField).to_ulong();
                        dp_Type[1]
                            = project_range<16, 19>(decodeField).to_ulong();
                        dp_Type[2]
                            = project_range<20, 23>(decodeField).to_ulong();
                        dp_Type[3]
                            = project_range<24, 27>(decodeField).to_ulong();
                        triggerThres
                            = project_range<28, 43>(decodeField).to_ulong();
                        downSamp
                            = project_range<44, 45>(decodeField).to_ulong();
                        //	    printf("Analog probe 1: %i is signed %i
                        // mult %i\n",ap1_Type,ap1_Signed,ap1_Mult);
                        // printf("Analog probe 2: %i is signed %i mult
                        //%i\n",ap2_Type,ap2_Signed,ap2_Mult); printf("Digital
                        // probe 0: %i, 1: %i, 2: %i, 3:
                        //%i\n",dp_Type[0],dp_Type[1],dp_Type[2],dp_Type[3]);
                        break;
                    case 1: // dead time
                        // uint64_t deadTime =
                        // project_range<0,47>(decodeField).to_ulong();
                        break;
                    case 2: // counter
                        //	    uint32_t savedCount = project_range<
                        // 0,23>(decodeField).to_ulong(); 	    uint32_t
                        // triggCount =
                        // project_range<24,47>(decodeField).to_ulong();
                        break;
                    default:
                        std::cerr << __LINE__ << ": Type not implemented yet: "
                                  << extraType << std::endl;
                    }
                    // posBuff++;
                }
                decodeField = swapLong(inpBuffer[++posEvent]);
                // This is the number of 64 bit words ... the trace will be
                // twice as long;
                uint16_t nbSamples_word
                    = project_range<0, 11>(decodeField).to_ulong();
                // std::cout << "Samples to be read: " << nbSamples_word*2 <<
                // std::endl;
                std::valarray<float> probe1;
                probe1.resize(nbSamples_word * 2);
                int sampTrig = 0;
                for(uint16_t samp = 0; samp < nbSamples_word; ++samp) {
                    decodeField = swapLong(inpBuffer[++posEvent]);
                    sample[0]   = project_range<0, 31>(decodeField).to_ulong();
                    sample[1] = project_range<32, 63>(decodeField).to_ulong();
                    if(ap1_Type == 0x0) {
                        // Should take what was done for GALTRACE in the
                        // LocalFilter of GALILEO
                        for(short sub = 0; sub < 2; ++sub) {
                            if(sample[sub].test(14)) sampTrig = samp * 2 + sub;
                            probe1[samp * 2 + sub]
                                = project_range<0, 13>(sample[sub]).to_ulong()
                                + (rand() % 1000) / 1000. - 0.5;
                        }
                    } else if(ap2_Type == 0x0) {
                        for(short sub = 0; sub < 2; ++sub) {
                            probe1[samp * 2 + sub]
                                = project_range<0, 13>(sample[sub]).to_ulong()
                                + (rand() % 1000) / 1000. - 0.5;
                        }
                    }
                }
                if(probe1.sum() < 0) probe1 *= -1;
                auto& wa       = waveAnalyzer[intEvt.board][intEvt.channel];
                float baseline = static_cast<std::valarray<float>>(
                                     probe1[std::slice(0, wa.nDcwin, 1)])
                                     .sum()
                               / wa.nDcwin;
                int nbSamp = probe1.size() - 1;
                probe1 -= baseline;
                float emax = 0.;
                if(nbSamp > 50)
                    emax = static_cast<std::valarray<float>>(
                               probe1[std::slice(nbSamp - 10, nbSamp, 1)])
                               .sum()
                         / 10.;

                if(aTrace == nullptr) {
                    aTrace = new float[probe1.size()];
                    if(wa.GetNSamples() != probe1.size())
                        wa.SetNSamples(probe1.size());
                    // wa.nDcwin = probe1.size() / 5; // the first 20% -->
                    // check if this is valid
                }
                if(wspace == nullptr) wspace = new float[3 * probe1.size()];
                std::copy(begin(probe1), end(probe1), aTrace);
                if(args.write_traces) {
                    intEvt.samples.resize(args.n_samples);
                    unsigned int n_copy = probe1.size() <= args.n_samples
                                            ? probe1.size()
                                            : args.n_samples;
                    std::copy_n(begin(probe1), n_copy, begin(intEvt.samples));
                }
                // CFD
                float otrig = 0;
                float trig  = wa.TriggerCFD(aTrace, 50, wspace, otrig);
                //	std::cout << "otrig = " << otrig << " trig: " <<trig
                //		  <<" sampleTrig = " << sampTrig << std::endl;
                //		if(outDataKey == keyDataSauron){
                // outEvtSauron->theData.off_cfd = otrig;
                //                	}
                // Rise time
                if(1) {
                    // outEvtSauron->theData.off_cfd = otrig;
                    // float yMax1 =
                    // waveAnalyzer[board][channel]->TriangAmpli(aTrace, 4,
                    // wspace);    // shaping 1 float yMax2 =
                    // waveAnalyzer[board][channel]->TriangAmpli(aTrace,  10,
                    // wspace);    // shaping 2
                    intEvt.imax = wa.TriangAmpli(aTrace, -1, wspace, true);
                    if(args.plot.first == board
                       && args.plot.second == channel) {
                        std::cout << "Drawing canvas" << std::endl;
                        TCanvas            c;
                        std::vector<float> xvals(args.n_samples);
                        std::iota(xvals.begin(), xvals.end(), 0);
                        TGraph g1(args.n_samples, xvals.data(), aTrace);
                        TGraph g2(args.n_samples, xvals.data(), wspace);

                        intEvt.imax
                            = wa.TriangAmpli(aTrace, -1, wspace, false);
                        TGraph g3(args.n_samples, xvals.data(), wspace);

                        g1.SetLineColor(kRed + 1);
                        g2.SetLineColor(kBlue + 1);
                        g1.SetLineWidth(3);
                        g2.SetLineWidth(3);
                        g1.SetMarkerColor(kRed + 2);
                        g2.SetMarkerColor(kBlue + 2);
                        g1.SetMarkerStyle(8);
                        g2.SetMarkerStyle(8);
                        g1.Draw("apl");
                        g2.Draw("pl");
                        g3.Draw("pl");
                        c.Update();
                        c.WaitPrimitive();
                    }
                    intEvt.psd = emax;
                } else {
                    intEvt.imax = 0.;
                    intEvt.psd  = 0.;
                }

            } // end wave present
            // Filling the output event, here for the BU to work without
            // modification we modify
            //  channel and board to match the 16 channel structure.
            offsetTS = brdSettings[intEvt.board].tsOffset[intEvt.channel];
            int outEvtSize = 0;

            tstamp_10ns += offsetTS + fGlobalOffset;
            intEvt.key
                = brdSettings[intEvt.board].key; // default is data:ranc1
            intEvt.timestamp = tstamp_10ns;

            intEvt.evnum     = (intEvt.board << 8 | intEvt.channel);
            intEvt.evnum |= flags;
            //      std::cout << "board = " << board << " boardID = " <<
            //	intEvt.board  << " (channel = " << channel << " and channel "
            //<< 	intEvt.channel << ")\n";
            readCountRate[intEvt.board][intEvt.channel]++;
            if(slfTrig) {
                intEvt.energy = energy;
                intEvt.cfd    = static_cast<uint16_t>(std::floor(cfd));
            } else {
                intEvt.energy = 0;
                intEvt.cfd    = 0;
            }

            if(intEvt.energy < 5) continue;
            timeSortEvent(intEvt);

            if(rootWrite) {
                hitpatternPHA->Fill(intEvt.board * 16 + intEvt.channel,
                                    intEvt.energy);
            }
        }

        posBuffer += (posEvent - posBuffer);
    }

    if(aTrace != nullptr) delete aTrace;
    if(wspace != nullptr) delete wspace;
}

void decode_PSD_v2(uint32_t* inpBuff) {}

void fillTree() {
    if(!rootWrite) return;
    for(auto it = coincData.begin(); it != coincData.end(); ++it) {
        if(brdSettings[it->board].nbChannels > 16) {
            it->board = it->board + it->channel / 16;
            it->channel -= it->channel / 16 * 16;
        }

        switch(it->key) {
        case keyDataSpider:
            SPIDERChannelId[nbSPIDER] = it->channel;
            SPIDERBoardId[nbSPIDER]   = it->board;
            SPIDEREnergy[nbSPIDER]    = it->energy;
            SPIDERTime[nbSPIDER]      = it->cfd;
            TSSPIDERHit[nbSPIDER]     = it->timestamp;
            if(nbSPIDER == 0) TSSPIDER = it->timestamp;
            if(args.write_traces) {
                std::copy(begin(it->samples), end(it->samples),
                          SPIDERTrace[nbSPIDER]);
            }
            nbSPIDER++;
            break;
        case keyDataDante:

            DANTEChannelId[nbDANTE] = it->channel;

            DANTEBoardId[nbDANTE]   = it->board;
            DANTEEnergy[nbDANTE]    = it->energy;
            DANTETime[nbDANTE]      = it->cfd;
            TSDANTEHit[nbDANTE]     = it->timestamp;
            if(nbDANTE == 0) TSDANTE = it->timestamp;
            if(args.write_traces) {
                std::copy(begin(it->samples), end(it->samples),
                          DANTETrace[nbDANTE]);
            }
            nbDANTE++;
            break;
        case keyDataEuclides: break;
        case keyDataLaBr:
            LABRChannelId[nbLABR] = it->channel;
            LABRBoardId[nbLABR]   = it->board;
            LABREnergy[nbLABR]    = it->energy;
            LABRQshort[nbLABR]    = it->qshort;
            LABRTime[nbLABR]      = it->cfd;
            TSLABRHit[nbLABR]     = it->timestamp;
            if(nbLABR == 0) TSLABR = it->timestamp;
            if(args.write_traces) {
                std::copy(begin(it->samples), end(it->samples),
                          LABRTrace[nbLABR]);
            }
            nbLABR++;

            break;
        case keyDataSauron:
            SAURONChannelId[nbSAURON] = it->channel;
            SAURONBoardId[nbSAURON]   = it->board;
            SAURONEnergy[nbSAURON]    = it->energy + rand() / RAND_MAX - 0.5;
            SAURONTime[nbSAURON]      = it->cfd;
            SAURONCfd[nbSAURON]       = it->cfd_off;
            SAURONImax[nbSAURON]      = it->imax;
            SAURONPid[nbSAURON]       = it->psd;
            TSSAURONHit[nbSAURON]     = it->timestamp;
            if(nbSAURON == 0) TSSAURON = it->timestamp;
            if(args.write_traces) {
                std::copy(begin(it->samples), end(it->samples),
                          SAURONTrace[nbSAURON]);
            }
            nbSAURON++;
            break;
        case keyDataOscar: break;
        default:
            std::cerr << "Warning: Unknown data key! 0x" << std::hex << it->key
                      << std::dec << std::endl;
        }
    }
    fTree->Fill();
    resetTreeEvents();
}

void resetTreeEvents() {
    nbSPIDER   = 0;
    nbDANTE    = 0;
    nbEUCLIDES = 0;
    nbLABR     = 0;
    nbSAURON   = 0;
    nbOSCAR    = 0;
}
