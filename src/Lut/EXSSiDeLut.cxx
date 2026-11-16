#include "EXSSiDeLut.h"

#include <sstream>

/*Naming scheme for channels:
        -2mm-           +y (ceiling)
    |   |   |   |   |
    -----------------
    |   |   |   |   |
    -----------------
    |   |   |   |   |
    -----------------   beam +z
    |   |   |   |   |
    -----------------
    |   |   |   |   |
    -----------------
    |   |   |   |   |
`
  NSIDECH00 -> N side, Channel 0-31
  PSIDECH32 -> P side, Channel 32-63

*/

EXSSiDeLut::EXSSiDeLut() : settings(Settings::getInstance()) {}

void EXSSiDeLut::ReadLut(const std::string& file_name) {
    fLut_file_fullname = file_name;
    prop.clear();
    unsigned int  mapNumber;
    std::string   line;
    std::ifstream file(file_name);
    if(!file.is_open()) throw std::runtime_error("EXSSiDe Lut not present\n");
    unsigned int board, chan, nCal;
    double       thrLow, thrHigh;
    double       wid, t0, coeff;
    std::string  chanName;
    while(std::getline(file, line)) {
        if(line.empty() or line.find_first_not_of(" \t") == std::string::npos)
            continue;

        if(line.at(line.find_first_not_of(' ')) == '#') continue;
        if(line.at(line.find_first_not_of(' ')) == '-') break;

        std::stringstream strline(line);
        strline >> board >> chan >> mapNumber >> chanName >> thrLow >> thrHigh
            >> wid >> t0 >> nCal;

        switch( mapNumber / 100) {
        case 0: prop[chan].side = EXSSiDeSide::NSIDE; break;
        case 1: prop[chan].side = EXSSiDeSide::PSIDE; break;
        default:
            throw std::runtime_error("In EXSSiDe Lut: map number first digit "
                                     "(side) is not correct\n");
            break;
        }
        prop[chan].brd        = board;
        prop[chan].strip      = mapNumber % 100;
        prop[chan].name       = chanName;
        prop[chan].thr        = std::make_pair(thrLow, thrHigh);
        prop[chan].width      = wid ;
        prop[chan].timeOffset = t0;
        prop[chan].map        = mapNumber;
        
        //std::cout<<prop[chan].brd<<"\t"<<prop[chan].strip<<"\t"<<prop[chan].name<<"\t"
        //        <<prop[chan].map<<"\t"<<std::endl;
        for(unsigned int i{0}; i < nCal; ++i) {
            strline >> coeff;
            prop[chan].cal.push_back(coeff);
        }
    }

    // P O S I T I O N  I N  S I L I C O N 
    // in the LUT is given the position of some strips in space
    // knowing the channel and the width of each strip we can calibrate in position

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
            if(points.size() != 5){
            throw std::logic_error("Number of cad and data coordinates are wrong.");
            }
        } catch(const std::exception& e) {
          std::cerr <<"\033[1;31m"<< "Error: " << e.what();
          std::cerr << "\033[0m"<<std::endl;
          std::cerr << "You should give 4 points each line:\n xcad1 ycad1 zcad1 nstripHoriz nstripVert"<<std::endl;  
            return;
        }
        
        pcad.ResizeTo(4,npoints);
        pdata.ResizeTo(3,npoints);

        for(int i{0}; i< 3; i++)
            pcad(i,npoints-1) = points.at(i);
        
        
        unsigned int psde,nsde;
        try{
            switch(static_cast<int>(points.at(3)) / 100 ? 1 : 0){ 
                case 0 : {
                    nsde = static_cast<int>(points.at(3)) % 100; 
                    break;
                }
                case 1 :{
                    psde = static_cast<int>(points.at(3)) % 100; 
                    break;
                }
                default: throw std::runtime_error("Error! One strip of the NSIDE and the other of the PSIDE\n");
                break;
            }
            switch(static_cast<int>(points.at(4)) / 100 ? 1 : 0){ 
                case 0 : {
                    nsde = static_cast<int>(points.at(4)) % 100;
                    break;
                }
                case 1 :{
                    psde = static_cast<int>(points.at(4)) % 100; 
                    break;
                }
                default: throw std::runtime_error("Error! One strip of the NSIDE and the other of the PSIDE\n");
                break;
            }
        }catch(...){
            return;
        }
        try{
            if((points.at(3) / 100) == (points.at(4) / 100)){
                throw std::logic_error("Error! One strip of the NSIDE and the other of the PSIDE\n");
            }
        } catch(...){
            return;
        }
        pdata(0,npoints-1) = mapChanIdtoPos(nsde,psde).second;
        pdata(1,npoints-1) = mapChanIdtoPos(nsde,psde).first;

        if(settings.verbosity_level>2)
            std::cout<<mapChanIdtoPos(nsde,psde).second<<" "<< mapChanIdtoPos(nsde,psde).first<<std::endl;
        pcad(3,npoints-1) = 1;
        pdata(2,npoints-1) = 1;
        

        points.clear();

    }

    try{
        if (file.eof() and (( pcad.GetNcols() == 0 or pcad.GetNrows() == 0) or (pdata.GetNcols() == 0 or pdata.GetNrows() == 0))) {
        throw std::logic_error( "Reached end of file. And no cad or data (X,Y,Z) points were found in the MCP LUT" );
        }
    } catch(const std::exception& e) {
          std::cerr <<"\033[1;31m"<< "Error: " << e.what();
          std::cerr << "\033[0m"<<std::endl;
          return;
    }
     
    // Calculate vector differences
    // Make cross product to get a lin indip dimension
    TMatrixD dt = pdata;
    dt.Transpose(dt);
    TMatrixD ddtinv = (pdata * dt).Invert();

    tsf.ResizeTo(4,3); 
    tsf   = (pcad * dt) * ddtinv;
    if(settings.verbosity_level > 0) {
        std::cout << "Cad " << std::endl;
        pcad.Print();
        std::cout << "Data " <<  std::endl;
        pdata.Print();
        std::cout << "Transform " << std::endl;
        tsf.Print();
    }
    

    if(settings.verbosity_level > 2) {
        std::cout << "Printing EXSSiDe lut" << std::endl;
        for(const auto& it : prop) {
                std::cout << "Board: " << it.second.brd << " channel: " << it.first
                          << " has name: " << it.second.name << std::endl;
            
        }
    }
}

void EXSSiDeLut::PrintLut(std::ostream& outstream) {
    assert(!fLut_file_fullname.empty());
    std::ifstream old_lut_file(fLut_file_fullname.c_str());

    if(!old_lut_file.is_open())
        throw std::runtime_error("EXSSiDe Lut " + fLut_file_fullname
                                 + " not present\n");
    unsigned int board, chan;
    std::string  chanName;
    std::string  line;

    while(std::getline(old_lut_file, line)) {
        if(line.empty()
           or line.find_first_not_of(" \t") == std::string::npos) {
            outstream << line << '\n';
            continue;
        }
        if(line.at(line.find_first_not_of(' ')) == '#') {
            outstream << line << '\n';
            continue;
        }
        std::vector<std::string> spaces;
        std::vector<std::string> words;

        Lut::SplitStringIntoWordsAndSpaces(line, words, spaces);

        std::stringstream strline(line);

        strline >> board >> chan;
        auto         params = prop[chan];

        // print until theta
        unsigned int iter   = 0;

        // sanity check
        assert(spaces.size() >= 10 + params.cal.size()
               && "not enough spaces in the line to print the lut");

        outstream << spaces[iter] << board;
        ++iter;
        outstream << spaces[iter] << chan;
        ++iter;
        outstream << spaces[iter] << params.map;
        ++iter;
        outstream << spaces[iter] << params.name;
        ++iter;
        outstream << spaces[iter] << params.thr.first;
        ++iter;
        outstream << spaces[iter] << params.thr.second;
        ++iter;
        outstream << spaces[iter] << params.width / UNITS::mm;
        ++iter;
        outstream << spaces[iter] << params.timeOffset / (10 * UNITS::ns);
        ++iter;
        outstream << spaces[iter] << params.cal.size();
        ++iter;
        for(auto& cal : params.cal) {
            outstream << spaces[iter] << cal;
            ++iter;
        }

        // now we should have "iter" number of spaces and "iter" number of
        // words printed, but there might be a comment at the end of the line
        for(; iter < spaces.size(); iter++) {
            outstream << spaces[iter];
            if(iter < words.size()) { outstream << words[iter]; }
        }
        outstream << '\n';
    }
}
