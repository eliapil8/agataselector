#pragma once

#include <array>
#include <dirent.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <TFile.h>
#include <TH2.h>
#include <TH2D.h>
#include <TKey.h>
#include <TMethod.h>
#include <TROOT.h>
#include <TText.h>
#include <TCutG.h>
#include <TCanvas.h>

#include "Settings.h"

#include <condition_variable>
#include <mutex>

template <int N, int M, class C=TCutG> class GateSelector {
  protected:
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
    struct Properties{
        C* cut;
        std::array<int, M> ids;
        double meanX;
        double meanY;
    };
    std::unordered_map<std::array<int, N>,
                       std::vector<Properties>,
                       ArrayHasher>
                               gates;
  private:
    static std::string keepOnlyDigits(const std::string& str) {
        std::string result;
        for(const auto& c : str) {
            if(std::isdigit(c)) { result += c; }
        }
        return result;
    }

    static std::vector<std::string> split(const std::string& s, const std::string& delimiter) {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = s.find(delimiter);
        while (end != std::string::npos) {
            tokens.push_back(s.substr(start, end - start));
            start = end + delimiter.length();
            end = s.find(delimiter, start);
        }
        tokens.push_back(s.substr(start));
        return tokens;
    }

    static std::unordered_map<std::array<int, N>, std::vector<Properties>, ArrayHasher> 
    FindCuts(const std::string& inFilePath, const std::string& cutNamePattern) {
        std::unordered_map<std::array<int, N>, std::vector<Properties>, ArrayHasher> foundGates;
        struct stat info {};
        if(stat(inFilePath.c_str(), &info) != 0) return foundGates;
        DIR*           dir;
        struct dirent* ent;

        if((dir = opendir(inFilePath.c_str())) != nullptr) {
            /* print all the files and directories within directory */
            while((ent = readdir(dir)) != nullptr) {
                // printf ("%s\n", ent->d_name);
                std::string tmpName = ent->d_name;
                if(tmpName.substr(tmpName.find_last_of('.') + 1) == "root") {
                    TFile    cutsFile((inFilePath +"/" + tmpName).c_str(), "read");
                    TIter    contents(cutsFile.GetListOfKeys());
                    TKey*    key;
                    TObject* obj;
                    while((key = dynamic_cast<TKey*>(contents()))) {
                        obj = cutsFile.Get(key->GetName());
                        if(C::Class() == obj->IsA()){
                            auto*       cut = dynamic_cast<C*>(obj);
                            std::string cutName = cut->GetName();
                            if(!cutNamePattern.empty() && cutName.find(cutNamePattern) == std::string::npos) continue;
                            if(Settings::getInstance().verbosity_level > 3)
                                std::cout << "Found cut: " << cutName
                                          << " in file: " << tmpName
                                          << std::endl;

                            auto tokens = split(cutName, "__");
                            if(tokens.size() != 3) throw std::runtime_error("Invalid cut name: " + cutName);
                            auto cutType = tokens[0];
                            auto idsIn = split(tokens[1], "_");
                            auto idsOut = split(tokens[2], "_");

                            std::array<int, N> detectorId{-1};
                            if(idsIn.size() != N) throw std::runtime_error("Invalid number of ids in cut name: " + cutName);

                            if(Settings::getInstance().verbosity_level > 15)
                                	std::cout << " input indexes: ";
                            for(int i = 0; i < N; i++){
                                detectorId[i] = std::stoi(keepOnlyDigits(idsIn[i]));
                            	if(Settings::getInstance().verbosity_level > 15)
                                	std::cout << detectorId[i] << " ";
			                }

                            if(Settings::getInstance().verbosity_level > 15)
                                	std::cout << "\n output  indexes: ";
                            std::array<int, M> idsOutArr{-1};
                            if(idsOut.size() != M) throw std::runtime_error("Invalid number of ids in cut name: " + cutName);
                            for(int i = 0; i < M; i++){
                                idsOutArr[i] = std::stoi(keepOnlyDigits(idsOut[i]));
                            	if(Settings::getInstance().verbosity_level > 15)
                                	std::cout << idsOutArr[i] << " ";
			                }
                            if(Settings::getInstance().verbosity_level > 15)
                                	std::cout << std::endl;
                            
                            Properties tmpProp;
                            tmpProp.cut = cut;
                            tmpProp.ids = idsOutArr;
                            tmpProp.meanX = cut->GetMean(1);
                            tmpProp.meanY = cut->GetMean(2);

                            foundGates[detectorId].emplace_back(tmpProp);
                        }
                    }
                }
            }
            closedir(dir);
        } else {
            throw std::runtime_error("Could not open cuts directory :"
                                                           + inFilePath);
        }
        return foundGates;
    }

  public:
    GateSelector(const std::string& folder_name,
                 const std::string& cut_name="") {
        gates = FindCuts(folder_name, cut_name);
    }
    //GateSelector(const std::string&                folder_name) {
    //    gates = FindCuts(folder_name);
    //}
    //~GateSelector() {
    //    for(auto& it : gates) {
    //        for(auto& prop : it.second) {
    //            delete prop.cut;
    //        }
    //    }
    //}    

    //Returns the ids of the gates that contain the point (x,y)
    std::pair<bool, std::array<int, M>>
    Id(const std::array<int, N>& id, const double& x, const double& y) const {
        auto it = gates.find(id);
        if(it != gates.end()) {
            for(const auto& prop : it->second) {
                if(prop.cut->IsInside(x, y)) { return {true, prop.ids}; }
            }
        }
        return {false, {}};
    }


  std::pair<bool,std::vector<std::array<int, M>>> AllId(const std::array<int, N>& id, const double &x, const double& y) const {
        auto it = gates.find(id);
        if(it != gates.end()) {
            std::vector<std::array<int, M>> ids;
            for(const auto& prop : it->second) {
	      if(prop.cut->IsInside(x, y)) {ids.emplace_back(prop.ids);}
            }
            return {true, ids};
        }
        return {false, {}};
    }



  
    std::vector<std::array<int, M>> getListOfIds(const std::array<int, N>& id) const {
        auto it = gates.find(id);
        if(it != gates.end()) {
            std::vector<std::array<int, M>> ids;
            for(const auto& prop : it->second) {
                ids.emplace_back(prop.ids);
            }
            return ids;
        }
        return {};
    }



    void Print() {
        for(auto it : gates){
            std::cout << "Cut ";
            for(auto i: it.first)   std::cout << i << " ";
            for(const auto& prop : it.second) {
                for(auto i: prop.ids)   std::cout << i << " ";
            //    std::cout << "MeanX: " << prop.meanX << " MeanY: " << prop.meanY << std::endl;
            }
            std::cout << std::endl;
        }
    }

    size_t size() const{ return gates.size();}

    std::pair<double, std::array<int, M>> Eval(const std::array<int, N>& id, const double& x) const {
        auto it = gates.find(id);
        double result = x;
        std::array<int, M> idsOut{-1};
        if(it != gates.end()) {
            double closestAvg = std::numeric_limits<double>::max(); 
            for(const auto& prop : it->second) {
                if(prop.meanY < closestAvg){
                    closestAvg = prop.meanY;
                    idsOut = prop.ids;
                    result = prop.cut->Eval(x);
                }
            }
        }
        if(Settings::getInstance().verbosity_level > 20){

            std::cout << "IDLine - Eval: ";
            for (int i = 0; i < N; ++i ) std::cout << id[i] << " "; 
            std::cout << x << " " << idsOut[0] << " " << result << std::endl;
        }
            

        return {result, idsOut};
    }

    //C* GetCut(const std::array<int, N>& id) const {
    //    auto it = gates.find(id);
    //    if(it != gates.end()) {
    //        return it->second[0].cut;

    //    }
    //    return nullptr;
    //}
    
    //std::pair<std::pair<double, double>, std::pair<std::array<int, M>, std::array<int, M>> 
    //    EvalNeighbours(const std::array<int, N>& id, const double& x, const double& y) const {
    //    
    //    auto it = gates.find(id);
    //    double resultMin = x;
    //    double resultMax = x;
    //    std::array<int, M> idsOutMin{-1};
    //    std::array<int, M> idsOutMax{-1};
    //    if(it != gates.end()) {
    //        double closestMinVal = std::numeric_limits<double>::min(); 
    //        double closestMaxVal = std::numeric_limits<double>::max(); 
    //        for(const auto& prop : it->second) {
//
//
//
    //                closestAvg = prop.meanY;
    //                idsOut = prop.ids;
    //                result = prop.cut->Eval(x);
    //            
    //        }
    //    }
    //    return {result, idsOut};
    //}


    static void
    //Sets the gates for the matrices in the file inFileName
    SetGates(const std::string& inFileName, const std::string& pathToMatrices,
             const std::string& matrixPattern, const std::string& outFilePath,
             const std::string& cutsType) {

        std::cout << "Set gates " << cutsType << "? (y/n)" << std::endl;
        std::string answer;
        std::cin >> answer;
        if(answer.compare("y") != 0) return;
        
        TFile matricesFile(inFileName.c_str(), "read");
        if(!matricesFile.IsOpen()) throw std::runtime_error("Cannot open gate file");
        auto* directory = static_cast<TDirectoryFile*>(
            matricesFile.Get(pathToMatrices.c_str()));
        if(directory == nullptr) throw std::runtime_error("folder "+pathToMatrices+" does not exist in gate file");
        TIter    contents(directory->GetListOfKeys());
        TKey*    key;
        TObject* obj;

        auto foundGates = FindCuts(outFilePath, cutsType);

        std::vector<C*> cuts;
        while((key = dynamic_cast<TKey*>(contents()))) {
            obj = directory->Get(key->GetName());
            if(obj->InheritsFrom("TH2")) {
                std::string objName = obj->GetName();
                if(objName.find(matrixPattern) == std::string::npos) continue;

                //Find the last M digits separated by "_"
                std::string  delimiter = "_";
                size_t       pos = 0;
                std::string  token;

                std::array<int, N> ids{-1};

                int i = 0;
                bool validCut{true};
                std::string ogObjectName = objName;
                while((pos = objName.rfind(delimiter)) != std::string::npos) {
                    token = objName.substr(pos + 1);
                    try{
                        ids[N - i - 1] = std::stoi(keepOnlyDigits(token));
                    }catch (std::invalid_argument& e){
                        std::cout << "Invalid argument: " << token << std::endl;
                        validCut = false;
                        break;
                    }
                    objName.erase(pos, objName.length());
                    i++;
                    if (i == N) break;
                }
                if(!validCut) continue;

                TCanvas cv("gateSetter", "gateSetter", 800, 600);
                cv.ToggleToolBar();
                cv.ToggleEventStatus();
                auto*      matrix = dynamic_cast<TH2*>(obj);
                if(matrix->GetEntries() == 0) {
                    std::cout << "Skipping empty matrix " << objName << std::endl;
                    continue;
                }
                matrix->Draw("colz");

                for(auto& gate: foundGates[ids]){
                    gate.cut->SetLineColor(kBlue+1);
                    gate.cut->Draw("same LP");
                }

                cv.Update();
                
                bool stop = false;

                // cms edit: this whole section down to the end of the code, Nov 2024
                std::cout << "-------------------------------------------------------\n\nA window with the matrix " << ogObjectName << " has appeared. We require " << M << " IDs to be input for new CUTS. Consult the PRISMA_README.md for info on which ID corresponds to which physical attribute.\n" << std::endl;
                std::cout << "Do you want to change the axes limits? If yes, type new limits x0 x1 y0 y1 and hit Enter, if not type 'n', or type 's' to skip this matrix." << std::endl;

                std::string option;
                double x0, x1, y0, y1;

                // Use std::cin to handle simple choices without needing std::getline
                if (std::cin >> option) {
                    if (option == "s") {
                        continue;  // Skip this matrix if the user typed "s"
                    } else if (option == "n") {
                        std::cout << "Keeping default axis limits." << std::endl;
                    } else {
                        // Try to interpret the input as axis limits
                        std::string ss(option); // Start with the first word and add a space for parsing
                        x0 = stod (ss);
                        
                        // Read the remaining three limits directly from std::cin
                        if (std::cin >> x1 >> y0 >> y1) {
                            matrix->GetXaxis()->SetRangeUser(x0, x1);
                            matrix->GetYaxis()->SetRangeUser(y0, y1);
                            std::cout << "Axes limits set to X:[" << x0 << ", " << x1 << "] and Y:[" << y0 << ", " << y1 << "]." << std::endl;

                            // Force the canvas to update and refresh
                            cv.Modified();
                            cv.Update();
                        } else {
                            std::cout << "Invalid input. Continuing with default axis limits." << std::endl;
                            std::cin.clear(); // Clear error flags from failed reads
                            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Flush the buffer
                        }
                    }
                } else {
                    // Handle any unexpected input
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid input format." << std::endl;
                }


                // Now do the cutting
                while(!stop){
                    C* cut = nullptr;
                    if(C::Class() == TCutG::Class()){
                        std::cout << "Please click on the points you wish to be part of this cut. Double click to finish, or close the window if you don't want to add cuts." << std::endl;
                        cut = static_cast<C*>(cv.WaitPrimitive("CUTG", "CutG"));
                    }else if(C::Class() == TGraph::Class()){
                        std::cout << "Please click Graph in the toolbar, then click on the points you wish to be part of this graph. Double click to finish." << std::endl;
                        cut = static_cast<C*>(cv.WaitPrimitive("Graph", "TGraph"));
                    }else{
                        throw std::runtime_error("Not implemented for this type of cut");
                    }

                    // Check if the user closed the window before finishing the cut
                    if (cut == nullptr) {
                        std::cerr << "-------------------------------------------------------\nCut creation window closed.\n-------------------------------------------------------" << std::endl;
                        stop = true;
                        break;
                        // Optionally: retry or exit gracefully
                        // return or exit to handle gracefully, or prompt the user to retry
                    }

                    
                    std::string name{cutsType+"_"};
                    for(int i = 0; i < N; ++i){
                            name += "_" + std::to_string(ids[i]);
                    }
                    name += "_";
                    for(int i = 0; i < M; i++){
                        std::cout << "The CUT name is currently " << name << std::endl;
                        std::cout << "What is the next ID/label? (or to quit, type q): " << std::endl;

                        std::string id;
                        std::cin >> id;

                        if(id.compare("q") == 0) {
                            stop = true;
                            break;
                        }
                        name += "_" + id;

                    }
                    if(stop) break;
                    cut->SetName(name.c_str());
                    cut->SetLineColor(kRed+1);
                    cut->SetLineWidth(3);
                	cut->SetMarkerStyle(20);
                    cut->Draw("same LP");

                    cv.Update();
                    cuts.push_back(cut);
                    std::cout << "-------------------------------------------------------\nSaved CUT " << name << "\n-------------------------------------------------------" << std::endl;

                }
                // cv.WaitPrimitive();
            }
        }
	struct stat st = {};

	if (stat(outFilePath.c_str(), &st) == -1) {
	    mkdir(outFilePath.c_str(), 0700);
	}
        TFile cutsFile((outFilePath+"cuts.root").c_str(), "update");
        for(auto cut: cuts){
            C* oldCut = static_cast<C*>(cutsFile.Get(cut->GetName()));
            if(oldCut != nullptr){
                std::string newName = oldCut->GetName();
		newName.insert(0, "old");
                oldCut->Write();
	    }
            cut->Write();
        }
        cutsFile.Close();
        std::cout << "-------------------------------------------------------\nCUTS successfully written to: " << outFilePath+"cuts.root" << "\n-------------------------------------------------------" << std::endl;
    }
};
