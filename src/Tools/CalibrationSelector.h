#pragma once

#include "Settings.h"

#include <array>
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <iostream> 
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unordered_map>
#include <utility>
#include <vector>

template <int N> class CalibrationSelector {
private:
    struct ArrayHasher {
        std::size_t operator()(const std::array<int, N>& arr) const {
            std::hash<int> hasher;
            std::size_t    hash = 0;
            for(const auto& i : arr) {
                hash ^= hasher(i) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            return hash;
        }
    };
public:

    struct Calibration {
        std::vector<double> cal;
        std::pair<double, double> thr;
    };
    std::unordered_map<std::array<int, N>,
                       Calibration,
                       ArrayHasher> calibrations;
    explicit CalibrationSelector(const std::string& filePath) {
        //struct stat info {};

        size_t startOfLastComponent = filePath.find_last_of('/');
        std::string folderPath = filePath.substr(0, startOfLastComponent);
        std::string fileName = filePath.substr(startOfLastComponent+1);

         // Function to create missing directories
        auto createDirectories = [](const std::string& path) {
            size_t pos = 0;
            do {
                pos = path.find_first_of('/', pos + 1);
                std::string subPath = path.substr(0, pos);

                 struct stat sb;
                if(stat(subPath.c_str(), &sb) != 0) {  // Check if the directory exists
                    if(mkdir(subPath.c_str(), 0755) != 0) {
                        std::cerr << "Error: Could not create directory " << subPath << std::endl;
                        return false;
                    }
                }
            } while(pos != std::string::npos);
            return true;
        };

        // Attempt to create any missing directories in folderPath
        if(!createDirectories(folderPath)) {
            throw std::runtime_error("CalibrationSelector: Could not create directory structure for path " + folderPath);
        }

        if(Settings::getInstance().verbosity_level > 6) {
            std::cout << "CalibrationSelector: Loading calibration file " << filePath << std::endl;
        }
        
        // Check if file already exists or create it if missing
        DIR* dir = opendir(folderPath.c_str());
        if(dir != nullptr) {
            closedir(dir);
            std::ifstream           file(filePath);
            std::string             line;
            if(file.is_open()) {
                while(std::getline(file, line)) {
                    if(line[0] == '#' || line.empty()) continue;

                    std::istringstream sstr(line);
                    std::string token;
                    std::array<std::vector<double>, 3> parameters;
                    int parIndex = -1;
                    while(sstr >> token) {
                        if(token == "id"){
                            parIndex = 0;
                        } else if (token == "cal")
                        {
                            parIndex = 1;
                        } else if (token == "thr"){
                            parIndex = 2;
                        } else if (parIndex != -1){
                            parameters[parIndex].push_back(std::stod(token));
                        }
                    }
                    if(parameters[0].size() != N) {
                        throw std::runtime_error("CalibrationSelector: id size does not match N");
                    }
                    std::array<int, N> tmpIdx;
                    for(int i = 0; i < N; i++) {
                        tmpIdx[i] = static_cast<int>(parameters[0].at(i));
                    }
                    if(parameters[2].size() != 2 && parameters[2].size() != 0){
                        throw std::runtime_error("CalibrationSelector: thr size does not match 2");
                    }
                    std::pair<double, double> tmpThr;
                    if(parameters[2].size() == 0){
                        tmpThr = std::make_pair(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
                    }else if (parameters[2].size() == 2){
                        tmpThr = std::make_pair(parameters[2].at(0), parameters[2].at(1));
                    } else {
                        throw std::runtime_error("CalibrationSelector: thr size does not match 2");
                    }

                    Calibration tmpCal{parameters[1], tmpThr};
                    calibrations.emplace(tmpIdx, tmpCal);
                }
            }
            else {
                std::ofstream ofile(filePath);
                ofile << "#Calibration file" << std::endl;
                ofile << "id ";
                for(int i = 0; i < N; i++) ofile << "0 ";
                ofile << " cal 0 1 thr 0 4096" << std::endl;
            }

        }
        else {
            std::cerr << "CalibrationSelector: Could not open directory " << folderPath << std::endl;
            }
    }

    std::pair<bool, double> GetCalibration(const std::array<int, N>& key, const double& value) {
        auto it = calibrations.find(key);
	
        if(it == calibrations.end()) {
            return {false, value};
        }else if(value < it->second.thr.first || value > it->second.thr.second){
            return {false, value};
        }else{
            double result{0};
	 
            for(int i = 0; i < (int)it->second.cal.size(); i++) {
                result += it->second.cal.at(i) * pow(value, i);
            }
            return {true, result};
        }
    }
    bool InsideThreshold(const std::array<int, N>& key, const double& value) {
        auto it = calibrations.find(key);
	
        if(it == calibrations.end()) {
            return false;
        }
        return value > it->second.thr.first && value < it->second.thr.second;
    }
    std::pair<bool, std::pair<double,double>> GetMatrixCalibration(const std::array<int, N>& key, std::pair<double,double> value){
            auto it = calibrations.find(key);
	
            if(it == calibrations.end()) {
                return {false, value};
            }else if(value.first < it->second.thr.first || value.first > it->second.thr.second || value.second < it->second.thr.first || value.second > it->second.thr.second){
                return {false, value};
            }else{

                if(it->second.cal.size() != 4) 
                    throw std::runtime_error("CalibrationMatrixSelector: cal size does not match 4");
                std::pair <double,double> result = std::make_pair(0,0);
    
                result.first = it->second.cal.at(0)*value.first + it->second.cal.at(1)*value.second;
                result.second = it->second.cal.at(2)*value.first + it->second.cal.at(3)*value.second;

                return {true, result};
            }
    }

    bool ValidCalibration(const std::array<int, N>& key) {
        return calibrations.find(key) != calibrations.end();
    }

    bool ValidThreshold(const std::array<int, N>& key) {
        auto it = calibrations.find(key);
        if(it == calibrations.end()) {
            return false;
        }
        return it->second.thr.first != std::numeric_limits<double>::min() && it->second.thr.second != std::numeric_limits<double>::max();
    }
};
