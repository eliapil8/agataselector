#include "McpLut.h"
#include "Units.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// Mcp point convention

void McpLut::ReadLut(const std::string& file_name) {
    std::string   line;
    std::ifstream file(file_name);
    if(!file.is_open()) throw std::runtime_error("Mcp Lut not present\n");
    unsigned int board, chan;
    double       thrLow, thrHigh;
    double       tOffset;
    double       p1{0},p2{0};
    unsigned int mcpNr;
    std::string  chanName;

    while(std::getline(file, line)) {
        if(line.empty() or line.find_first_not_of(" \t") == std::string::npos)
            continue;
        if(line.at(line.find_first_not_of(' ')) == '#') continue;
        if(line.find_first_of("nnn") != std::string::npos) continue;
        if(line.at(line.find_first_not_of(' ')) == '-') break;
        

        std::stringstream strline(line);
        strline >> board >> chan >> chanName >> thrLow >> thrHigh;
        strline >> tOffset >> p1 >> p2;
    
        std::transform(chanName.begin(), chanName.end(), chanName.begin(),
                       ::toupper);

        // this define the detector number
        // returns the first number in the string D1[X,Y,T]1
        
        mcpNr = static_cast<unsigned int>(
                std::stoi(chanName.substr(chanName.find_first_not_of('D'),
                                      chanName.find_first_of("XYT")
                                      - chanName.find_first_not_of('D'))));
        mcpIdSet.insert(mcpNr);
        
        std::string eventType = chanName.substr(chanName.find_first_not_of("D")+1,1);
        
        bool hasC = (chanName.find('C') != std::string::npos);    

        channelIdtoMcpId[getChannelId(board, chan)] = mcpNr;
        
        if(eventType == "X"){
            if(!hasC){
            mcpIdToChannelId[mcpNr][McpType::X] = getChannelId(board, chan);
            channelIdtoMcpType[getChannelId(board, chan)] = McpType::X;
	    prop[mcpNr].tofsPresent[McpType::X] = false;
           // std::cout<<"not cond X ch "<<chan<<std::endl;
            }else{
            mcpIdToChannelId[mcpNr][McpType::XC] = getChannelId(board, chan);
            channelIdtoMcpType[getChannelId(board, chan)] = McpType::XC;
	    prop[mcpNr].tofsPresent[McpType::XC] = false;
            //std::cout<<"cond X ch "<<chan<<std::endl;
            }   
        }
        if(eventType == "Y"){
            if(!hasC){
            mcpIdToChannelId[mcpNr][McpType::Y] = getChannelId(board, chan);
            channelIdtoMcpType[getChannelId(board, chan)] = McpType::Y;
	    prop[mcpNr].tofsPresent[McpType::Y] = false;
           // std::cout<<"not cond Y ch "<<chan<<std::endl;
            }
            else{
            mcpIdToChannelId[mcpNr][McpType::YC] = getChannelId(board, chan);
            channelIdtoMcpType[getChannelId(board, chan)] = McpType::YC;
	    prop[mcpNr].tofsPresent[McpType::YC] = false;
            //std::cout<<"cond Y ch "<<chan<<std::endl;            
            }   
        }

        if(eventType == "T"){
            if(chanName == "D1T2"){
                mcpIdToChannelId[mcpNr][McpType::TOF12] = getChannelId(board, chan);
                channelIdtoMcpType[getChannelId(board, chan)] = McpType::TOF12;
                prop[mcpNr].tofsPresent[McpType::TOF12] = true;
              //  std::cout<<"not cond T ch "<<chan<<std::endl; 
                
            } else if(chanName == "D1T2C"){
                mcpIdToChannelId[mcpNr][McpType::TOF12C] = getChannelId(board, chan);
                channelIdtoMcpType[getChannelId(board, chan)] = McpType::TOF12C;
                prop[mcpNr].tofsPresent[McpType::TOF12C] = true;
               // std::cout<<"cond T ch "<<chan<<std::endl; 

            } else if (chanName[3] == 'D'){
                if (chanName[1] == '1'){
                    mcpIdToChannelId[mcpNr][McpType::TOF1DANTE] = getChannelId(board, chan);
                    channelIdtoMcpType[getChannelId(board, chan)] = McpType::TOF1DANTE;
                    prop[mcpNr].tofsPresent[McpType::TOF1DANTE] = true;
                }
                else if (chanName[1] == '2'){
                    mcpIdToChannelId[mcpNr][McpType::TOF2DANTE] = getChannelId(board, chan);
                    channelIdtoMcpType[getChannelId(board, chan)] = McpType::TOF2DANTE;
                    prop[mcpNr].tofsPresent[McpType::TOF2DANTE] = true;
                }
                else throw std::runtime_error("TOF channel not recognized");
            } else throw std::runtime_error("TOF channel not recognized"); 
        }

        thr[getChannelId(board, chan)]    = std::make_pair(thrLow, thrHigh);
        tofCal[getChannelId(board, chan)] = {p1, p2};
        timeOffset[getChannelId(board, chan)] = tOffset ;
    }

    file.clear();
    file.seekg(0,file.cur);

    int npoints = 0;
    while(std::getline(file, line)) {
        if(line.empty() or line.find_first_not_of(" \t") == std::string::npos)
            continue;
        if(line.at(line.find_first_not_of(' ')) == '#') continue;

        std::stringstream strline(line);
        std::vector<double> points;
        double val;

        npoints++;

        while(strline >> val)
            points.push_back(val);
        
        try{
            if(points.size() != 10){
            throw std::logic_error("Number of cad and data coordinates are wrong.");
            }
        } catch(const std::exception& e) {
            throw std::runtime_error("You should give 5 points each line:\n xcad1 ycad1 zcad1 xdata1 ydata1\n xcad2 ycad2 zcad2 xdata2 ydata2");
        }

        for(const auto& id : mcpIdSet) {
            auto& it = prop[id];
            it.pcad.ResizeTo(4,npoints);
            it.pdata.ResizeTo(3,npoints);

            for(int i{0}; i< 3; i++)
                it.pcad(i,npoints-1) = points.at(i+5*(id-1));
            for(int i{0}; i< 2; i++)
                it.pdata(i,npoints-1) = points.at(i+3+5*(id-1));

            it.pcad(3,npoints-1) = 1;
            it.pdata(2,npoints-1) = 1;
        }

        points.clear();
        
    }

    try{
        if (file.eof() and (( prop[mcpNr].pcad.GetNcols() == 0 or prop[mcpNr].pcad.GetNrows() == 0) or (prop[mcpNr].pdata.GetNcols() == 0 or prop[mcpNr].pdata.GetNrows() == 0))) {
        throw std::logic_error( "Reached end of file. And no cad or data (X,Y,Z) points were found in the MCP LUT" );
        }
    } catch(const std::exception& e) {
          std::cerr <<"\033[1;31m"<< "Error: " << e.what();
          std::cerr << "\033[0m"<<std::endl;
          return;
    }
     
    
    // Calculate vector differences
    for(auto& it : prop) { 
        // Make cross product to get a lin indip dimension
        TMatrixD dt = it.second.pdata;
        dt.Transpose(dt);
        TMatrixD ddtinv = (it.second.pdata * dt).Invert();
        it.second.tsf   = (it.second.pcad * dt) * ddtinv;

        if(settings.verbosity_level > 0) {
            std::cout << "Cad id " << it.first << std::endl;
            it.second.pcad.Print();
            std::cout << "Data id " << it.first << std::endl;
            it.second.pdata.Print();
            std::cout << "Transform id " << it.first << std::endl;
            it.second.tsf.Print();
        }
    }
}

void McpLut::SetRotation(const double& rotAngle) {
    for(auto& it : prop) {
        
        int npoints = it.second.pcad.GetNcols();
        for(signed int i{0}; i < npoints; ++i) {
            TVector3 pVec(it.second.pcad[0][i], it.second.pcad[1][i],
                          it.second.pcad[2][i]);
            pVec.RotateY(rotAngle);

            it.second.pcad[0][i] = pVec.X();
            it.second.pcad[1][i] = pVec.Y();
            it.second.pcad[2][i] = pVec.Z();
        }
        TMatrixD dt = it.second.pdata;
        dt.Transpose(dt);
        TMatrixD ddtinv = (it.second.pdata * dt).Invert();
        it.second.tsf   = (it.second.pcad * dt) * ddtinv;
    }
}

void McpLut::PrintLut(std::ostream& outstream) {
    static_cast<void>(outstream);
    throw std::runtime_error("Not implemented\n");
}
