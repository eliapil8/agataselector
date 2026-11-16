#include "DanteLut.h"
#include "Units.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// Dante point convention
// 2-----
//|\   |
//| \  |
//|  \ |
// 1----3
//  (x)      //x is beam/target

void DanteLut::ReadLut(const std::string& file_name) {
    std::string   line;
    std::ifstream file(file_name);
    if(!file.is_open()) throw std::runtime_error("Dante Lut not present\n");
    unsigned int board, chan;
    double       thrLow, thrHigh;
    double       p1, p2;
    double       tOffset;
    unsigned int danteNr;
    std::vector<int> nrDantes;
    std::string  chanName;
    while(std::getline(file, line)) {
        if(line.empty() or line.find_first_not_of(" \t") == std::string::npos)
            continue;

        if(line.at(line.find_first_not_of(' ')) == '#') continue;
	    if(settings.verbosity_level>5) std::cout<<line<<std::endl;
	    if(line.find_first_of("nnn") != std::string::npos) continue;
        if(line.at(line.find_first_not_of(' ')) == '-') break;
	
        std::stringstream strline(line);
        strline >> board >> chan >> chanName >> thrLow >> thrHigh;
        strline >> tOffset >> p1 >> p2;
	
        std::transform(chanName.begin(), chanName.end(), chanName.begin(),
                       ::toupper);

	    if(settings.verbosity_level>5){
	      std::cout<<"DANTE ch "<<chanName<<std::endl;
        
	    }
        danteNr = static_cast<unsigned int>(
							 std::stoi(chanName.substr(chanName.find_first_not_of('D'),
										   chanName.find_first_of("XYT")
										   - chanName.find_first_not_of('D'))));
	
        auto& it = prop[danteNr];
        if(nrDantes.size() == 0) {
            nrDantes.push_back(danteNr);
            if(settings.verbosity_level > 0) {
                std::cout << "Dante number " << danteNr
                          << " not in the list, adding it" << std::endl;
            }
        } else {
            if(std::find(nrDantes.begin(), nrDantes.end(), danteNr) == nrDantes.end()) {
                nrDantes.push_back(danteNr);
                if(settings.verbosity_level > 0) {
                    std::cout << "Dante number " << danteNr
                              << " not in the list, adding it" << std::endl;
                }
            }
        }
        
        if(chanName[chanName.size() - 1] == 'X') {
            channelIdtoDanteId[getChannelId(board, chan)] = danteNr;
            danteIdToChannelId[danteNr][X] = getChannelId(board, chan);
            channelIdtoDanteType[getChannelId(board, chan)] = X;
        } else if(chanName[chanName.size() - 1] == 'Y') {

            channelIdtoDanteId[getChannelId(board, chan)] = danteNr;
            danteIdToChannelId[danteNr][Y] = getChannelId(board, chan);
            channelIdtoDanteType[getChannelId(board, chan)] = Y;
        } else if(chanName[chanName.size() - 1] == 'T') {
            channelIdtoDanteId[getChannelId(board, chan)] = danteNr;
            danteIdToChannelId[danteNr][T] = getChannelId(board, chan);
            channelIdtoDanteType[getChannelId(board, chan)] = T;
        } else if(chanName.find("TOFPRISMA") != std::string::npos) {
            it.tofCal[0]                                  = p1;
            it.tofCal[1]                                  = p2;

            it.tofType                                    = Dante::TofType::PRISMA;
            channelIdtoDanteId[getChannelId(board, chan)] = danteNr;
            danteIdToChannelId[danteNr][TOF] = getChannelId(board, chan);
            channelIdtoDanteType[getChannelId(board, chan)] = TOF;
        } else if(chanName.find("TOFDANTE") != std::string::npos) {
            it.tofCal[0]                                  = p1 * UNITS::ns;
            it.tofCal[1]                                  = p2 * UNITS::ns;

            it.tofType                                    = Dante::TofType::DANTE;
            channelIdtoDanteId[getChannelId(board, chan)] = danteNr;
            danteIdToChannelId[danteNr][TOF] = getChannelId(board, chan);
            channelIdtoDanteType[getChannelId(board, chan)] = TOF;
        } else
            throw std::runtime_error("Wrong DANTE name in LUT\n");

        it.thr[getDanteType(board, chan)] = std::make_pair(thrLow, thrHigh);
        timeOffset[getChannelId(board, chan)] = tOffset * 10 * UNITS::ns;
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
            if(points.size() != 5*nrDantes.size()){
                throw std::logic_error("Number of cad and data coordinates are wrong.");
            }
        } catch(const std::exception& e) {
            throw std::runtime_error("You should give 5 points each line:\n xcad1 ycad1 zcad1 xdata1 ydata1\n xcad2 ycad2 zcad2 xdata2 ydata2");
        }

        for(const auto& id : nrDantes) {
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
        if (file.eof() and (( prop[danteNr].pcad.GetNcols() == 0 or prop[danteNr].pcad.GetNrows() == 0) or (prop[danteNr].pdata.GetNcols() == 0 or prop[danteNr].pdata.GetNrows() == 0))) {
        throw std::logic_error( "Reached end of file. And no cad or data (X,Y,Z) points were found in the DANTE LUT" );
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

void DanteLut::SetRotation(const double& rotAngle) {
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

void DanteLut::PrintLut(std::ostream& outstream) {
    static_cast<void>(outstream);
    throw std::runtime_error("Not implemented\n");
}
