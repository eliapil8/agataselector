#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "kdtree.h"

class MappingPoint {
  private:
    double x_mcp;
    double y_mcp;
    double delta;
    double path;
    double x_fp;
    double y_fp;
    double theta_fp;
    double phi_fp;

  public:
    MappingPoint();
    MappingPoint(double x_mcp, double y_mcp, double delta, double path,
                 double x_fp, double y_fp, double theta_fp, double phi_fp)
                : x_mcp(x_mcp), y_mcp(y_mcp), delta(delta), path(path), x_fp(x_fp),
                  y_fp(y_fp), theta_fp(theta_fp), phi_fp(phi_fp){};
    ~MappingPoint();
    double& operator[](int index) {
        switch(index) {
            case 0: return x_mcp;
            case 1: return y_mcp;
            //case 2: return x_fp;
            default: throw std::out_of_range("Index out of range for MappingPoint");
        }
    }
    const double& operator[](int index) const {
        switch(index) {
            case 0: return x_mcp;
            case 1: return y_mcp;
            //case 2: return x_fp;
            default: throw std::out_of_range("Index out of range for MappingPoint");
        }
    }
    static const int DIM = 2;

    double GetXmcp() const { return x_mcp; }
    double GetYmcp() const { return y_mcp; }
    double GetDelta() const { return delta; }
    double GetPath() const { return path; }
    double GetXfp() const { return x_fp; }
    double GetYfp() const { return y_fp; }
    double GetThetafp() const { return theta_fp; }
    double GetPhifp() const { return phi_fp; }
    void SetXmcp(double x) { x_mcp = x; }
    void SetYmcp(double y) { y_mcp = y; }
    void SetDelta(double d) { delta = d; }
    void SetPath(double p) { path = p; }
    void SetXfp(double x) { x_fp = x; }
    void SetYfp(double y) { y_fp = y; }
    void SetThetafp(double t) { theta_fp = t; }
    void SetPhifp(double p) { phi_fp = p; }
    void Print() {
        std::cout << "x_mcp: " << x_mcp << " y_mcp: " << y_mcp << " delta: " << delta
                  << " path: " << path << " x_fp: " << x_fp << " y_fp: " << y_fp
                  << " theta_fp: " << theta_fp << " phi_fp: " << phi_fp << std::endl;
    }
};  

class MappingPrisma {
  public:
    MappingPrisma();
    MappingPrisma(const std::string& filename){ReadMappingFromFile(filename);};
    ~MappingPrisma();
    
    void ReadMappingFromFile(const std::string& filename);
    int FindClosestPoint(const MappingPoint& point);
    MappingPoint GetPoint(int index);

  private:
    std::vector<MappingPoint> MappingPointVector;

    kdt::KDTree<MappingPoint> MappingTree;

};
