#pragma once

#include <vector>

#include <TLorentzVector.h>
#include <TVector2.h>
#include <TVector3.h>

#include "Container.h"

class Prisma : public Container {
  public:
    TLorentzVector p4;   // 4 momentum of recoil
    TLorentzVector p3;   // 4 momentum of recoil
    TVector3       beta; // beta vector of recoil
    unsigned int   A;
    unsigned int   Z;
    unsigned int   Q;
    double         E;
    double         dE;
    double         TKEL;
    double         TOF;
    double         Length;
    double         mon1; // monitor 1
    double         mon2; // monitor 2
    double         colNr;
    double         X_FP;
    double         MCP_X;
    double         MCP_Y;
    double         Mass;
    double         AoverQ;
    double         RBeta;
    bool           mcp_ok;
    bool           tof_ok;
    bool           side_ok;
    bool           traj_ok;
    bool           ic_ok;
    bool           all_ok;
    void resize(const unsigned int& mult) override { Container::resize(mult); }
    void clear() override {
        Container::clear();
        p4      = TLorentzVector();
        p3      = TLorentzVector();
        beta    = TVector3();
        A       = 0;
        Z       = 0;
        Q       = 0;
        E       = 0;
        dE      = 0;
        TKEL    = 0;
        TOF     = 0;
        Length  = 0;
        mon1    = 0;
        mon2    = 0;
	      X_FP    = 0;
        MCP_X   = 0;
        MCP_Y   = 0;
	      Mass    = 0;
	      AoverQ  = 0;
	      RBeta   = 0;
        mcp_ok  = true;
        tof_ok  = true;
        side_ok = true;
        traj_ok = true;
        ic_ok   = true;
        all_ok  = true;
    }
    ClassDefOverride(Prisma, 5);
};
