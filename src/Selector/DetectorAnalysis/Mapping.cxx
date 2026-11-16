#include "Mapping.h"

#include <fstream>
#include <sstream>
#include "Units.h"


MappingPoint::MappingPoint() {
    x_mcp = 0;
    y_mcp = 0;
    delta = 0;
    path = 0;
    x_fp = 0;
    y_fp = 0;
    theta_fp = 0;
    phi_fp = 0;
}

MappingPoint::~MappingPoint() {}

MappingPrisma::MappingPrisma(){}

MappingPrisma::~MappingPrisma(){}

void MappingPrisma::ReadMappingFromFile(const std::string& filename){
    std::ifstream file(filename);
    std::cout<< "Reading mapping file: " << filename << std::endl;
    if(!file.is_open())
        throw std::runtime_error(
            "Mapping file is not present or could not be opened\n");
    std::string line;
    while(std::getline(file, line)) {
        std::stringstream lineStream;
        lineStream.clear();
        lineStream.str(line);
        double x_mcp, y_mcp, delta, path, x_fp, y_fp, theta_fp, phi_fp;
        lineStream >> x_mcp >> y_mcp >> delta >> path >> x_fp >> y_fp >> theta_fp >> phi_fp;
        MappingPoint point(x_mcp*UNITS::mm, y_mcp*UNITS::mm, delta, path*UNITS::mm, x_fp*UNITS::mm, y_fp*UNITS::mm, theta_fp*UNITS::rad, phi_fp*UNITS::rad);
        //point.Print();
        MappingPointVector.push_back(point);
    }

}

int MappingPrisma::FindClosestPoint(const MappingPoint& point) {
    return MappingTree.nnSearch(point);
}

MappingPoint MappingPrisma::GetPoint(int index) {
    return MappingPointVector[index];
}


