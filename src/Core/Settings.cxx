#include "Settings.h"

#include <fstream>
#include <sstream>
#include <unistd.h>

std::ostream& Settings::operator<<(std::ostream& os) const {
    os << "Usage:\n";
    os << "./RunSelector [options, ...] [#run, ...]\n";
    os << "\t\t--nrevts                     [#]                      Specify "
          "number of events to analyze, default is all\n";
    os << "\t\t--nrthr                      [#]                      Specify "
          "maximum number of threads, default is 6\n";
    os << "\t\t--conf                       [file]                   Specify "
          "a conf file, default is in ../Conf/\n";
    os << "\t\t--iofile                     [infile, outfile]        Specify "
          "an input and an output file, this will disregard the conf\n";
    os << "\t\t--iolistfile                 [file]                   Specify "
          "a file where to read a list of inputFile outputFile line by line\n";
    os << "\t\t--verb                       [#]                      "
          "Verbosity: 0 (prints progress Sel and nothing else), 1 (disables "
          "progress put prints debugging info)\n";
    os << "\t\t--no_user_sel                                         Runs "
          "standard selector without the analyis contained in "
          "User/UserSelector.cxx\n";
    os << "\t\t--print_conf                 [out_file]               Prints "
          "the default conf file\n";
    os << "\t\t--rm_partial                                          Removes "
          "partial output files\n";
    os << "\t\t--distribute                                          "
          "Distributes the work on different nodes\n";
    os << "\t\t--optimize                                            Run "
          "optimizer\n";
    os << "\t\t--only_enabled_histos                                 Write "
          "histos contained in  Conf/enabled_histo.conf\n";
    os << "\t\t--debug_canvas                                        Shows a "
          "canvas to debug fit results in the optimization procedure\n";
    os << "\t\t--reduction_cond             [reduction condition]    Creates "
          "a new reduced data TTree based on the given reduction condition\n";
    os << "\t\t--set_gates                  [selector file]          Starts "
          "the automatic procedure to set gates on\n";
    os << "\t\t--mem_check                                           Performs "
          "the filling check\n";
    os << "\t\t--no_hadd                                             Does not "
          "run hadd\n";
    os << "\t\t--update_runs                                         Check if "
          "selector was run on a given file, in case skip\n";
    os << "\t\t--update_prisma                                       Creates "
          "new root files with updated prisma branches for specified runs\n";
    os << "\t\t--sum_all                                             Creates "
          "a file called runs-##-##-##.root with the sum of various runs\n";
    os << "\t\t--subtract_bkg                                        "
          "Subtracts "
          "background based on the gates set up in the conf\n";
    os << "\t\t--window_size                                         Window "
          "size used to print progress\n";
    os << "\t\t--spy                        [histogram]              Opens a "
          "canvas to monitor the progress on a single histogram\n";
    return os;
}

void Settings::readListFile(const std::string& listFileName) {
    std::ifstream file(listFileName);
    if(!file.is_open())
        throw std::runtime_error(
            "List file is not present or could not be opened\n");
    std::string inp, outp, line;
    while(std::getline(file, line)) {
        std::stringstream lineStream;
        lineStream << line;
        lineStream >> inp >> outp;
        inFileName.push_back(inp);
        outFileName.push_back(outp);
    }
}

void Settings::parseOptions(int argc, char** argv) {
    for(int i{1}; i < argc; ++i) { options.emplace_back(argv[i]); }
    {
        char tmp[256];
        if (getcwd(tmp, 256) == nullptr) {
            throw std::runtime_error("Could not get current working directory\n");
        }
        cwd = tmp;
    }
    for(auto opt = options.begin(); opt != options.end(); ++opt) {
        if(*opt == "--help" || *opt == "-h") {
            printHelp = true;

        } else if(*opt == "--nrevts") {
            try {
                std::string::size_type sz;
                nrEvts = (long long)std::stod(*++opt,&sz);
            } catch(...) {
                throw std::runtime_error(
                    "nrevts number does not make sense\n");
            };

        } else if(*opt == "--nrthr") {
            try {
                nrThr = std::stoi(*++opt);

            } catch(...) {
                throw std::runtime_error("nrthr number does not make sense\n");
            };

        } else if(*opt == "--conf") {
            try {
                confFileName = *++opt;
            } catch(...) {
                throw std::runtime_error(
                    "Conf file name does not make sense\n");
            };

        } else if(*opt == "--iofile") {
            inFileName.emplace_back(*++opt);
            outFileName.emplace_back(*++opt);
            if(inFileName.back().substr(inFileName.back().length() - 5,
                                        inFileName.back().length())
               != ".root")
                throw std::runtime_error(inFileName.back()
                                         + " is not a root file\n");
            if(outFileName.back().substr(outFileName.back().length() - 5,
                                         outFileName.back().length())
               != ".root")
                throw std::runtime_error(outFileName.back()
                                         + " is not a root file\n");

        } else if(*opt == "--iolistfile") {
            if(opt + 1 == options.end())
                throw std::runtime_error("iolistfile option requires a file\n");
            ioListFile = *++opt;
            readListFile(ioListFile);

        } else if(*opt == "--verb") {
            try {
                verbosity_level = std::stoi(*++opt);
            } catch(...) {
                throw std::runtime_error("Error with provided verb value\n");
            };

        } else if(*opt == "--no_user_sel") {
            userSelector = false;

        } else if(*opt == "--print_conf") {
            if(opt + 1 == options.end())
                throw std::runtime_error("print_conf option requires a file\n");   
            printFileName = *++opt;

        } else if(*opt == "--rm_partial") {
            remove_partial = true;

        } else if(*opt == "--distribute") {
            distribute = true;

        } else if(*opt == "--optimize") {
            optimize = true;

        } else if(*opt == "--only_enabled_histos") {
            only_enabled_histos = true;

        } else if(*opt == "--debug_canvas") {
            debug_canvas = true;

        } else if(*opt == "--update_runs") {
            update_runs = true;

        } else if(*opt == "--update_prisma") {
            update_prisma = true;
            no_hadd       = true;

        } else if(*opt == "--sum_all") {
            sum_all = true;

        } else if(*opt == "--subtract_bkg") {
            subtract_bkg = true;

        } else if(*opt == "--no_hadd") {
            no_hadd = true;

        } else if(*opt == "--reduction_cond") {
            if(opt + 1 == options.end())
                throw std::runtime_error("reduction_cond option requires a "
                                         "reduction condition\n");
            reduction_condition = *++opt;
            no_hadd             = true;
        } else if(*opt == "--set_gates") {
            if(opt + 1 == options.end())
                throw std::runtime_error("set_gates option requires a file\n");
            set_gates = *++opt;

        } else if(*opt == "--mem_check") {
            mem_check = true;

        } else if(*opt == "--spy") {
            if(opt + 1 == options.end())
                throw std::runtime_error("spy option requires a file\n");
            spy = *++opt;

        } else if(*opt == "--window_size") {
            try {
                window_size = std::stoi(*++opt);
            } catch(...) {
                throw std::runtime_error(
                    "window_size number does not make sense\n");
            };
        } else {
            try {
                runNr.push_back(std::stoi(*opt));
            } catch(...) {
                throw std::runtime_error("Option " + (*opt)
                                         + " is not valid\n");
            }
        }
    }
}
