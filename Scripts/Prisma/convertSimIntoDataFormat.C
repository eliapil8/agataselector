#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>
#include <TMath.h>

void convertSimIntoDataFormat(const std::string& simfile,const std::string& outTreeFile) {
    TFile* simFile = new TFile(simfile.c_str(),"read");
    if (simFile==nullptr) {
        std::cerr << "Failed to open the input file." << std::endl;
        return;
    }
    TTree * simTree = (TTree*) simFile->Get("PRISMA");

    TFile * outFile = new TFile(outTreeFile.c_str(),"recreate");

    TTree * outTree = new TTree("PrismaTree","PrismaTree"); 


    double  x_mcp, 
            y_mcp, 
            mcp_time,
            energy, 
            path, 
            x_fp, 
            y_fp, 
            ppac_mom_x, 
            ppac_mom_y, 
            ppac_mom_z,
            ppac_time,
            radius,
            tof;

    int MCP_hit, 
        PPAC_hit;
    
    bool MCP_ok, TOF_ok;

    simTree->SetBranchAddress("MCP_PosX",&x_mcp); 
    simTree->SetBranchAddress("MCP_PosY",&y_mcp); 
    simTree->SetBranchAddress("Energy",&energy); 
    simTree->SetBranchAddress("Traj",&path); 
    simTree->SetBranchAddress("PPAC_PosX",&x_fp); 
    simTree->SetBranchAddress("PPAC_PosY",&y_fp); 
    simTree->SetBranchAddress("PPAC_MomX",&ppac_mom_x); 
    simTree->SetBranchAddress("PPAC_MomY",&ppac_mom_y); 
    simTree->SetBranchAddress("PPAC_MomZ",&ppac_mom_z); 
    simTree->SetBranchAddress("MCP_hit",&MCP_hit);
    simTree->SetBranchAddress("PPAC_hit",&PPAC_hit);
    simTree->SetBranchAddress("MCP_Time",&mcp_time);
    simTree->SetBranchAddress("PPAC_Time",&ppac_time);
   

    outTree->Branch("MCP_X",&x_mcp);
    outTree->Branch("MCP_Y",&y_mcp);
    outTree->Branch("Radius",&radius);
    outTree->Branch("Length",&path);
    outTree->Branch("X_FP",&x_fp);
    outTree->Branch("Y_FP",&y_fp);
    outTree->Branch("TOF",&tof);
    outTree->Branch("mcp_ok",&MCP_ok);
    outTree->Branch("tof_ok",&TOF_ok);

    for(int i = 0; i < simTree->GetEntries();++i){
        simTree->GetEntry(i);

        if (MCP_hit<1 || PPAC_hit<1) continue;
        double Brho = 2*3.107*sqrt(pow(((energy+3727.3794066)/3727.3794066),2)-1);
        double Brho_ref = 1.22;
        auto ppac_dir = TVector3(ppac_mom_x,ppac_mom_y,ppac_mom_z);
        ppac_dir.RotateY(60*TMath::DegToRad());
        x_mcp = -x_mcp*TMath::Cos(45*TMath::DegToRad());
        y_mcp = y_mcp;
        radius = Brho / 0.869423;
        path = path;
        x_fp = 500-x_fp;
        y_fp = y_fp;
        tof = ppac_time-mcp_time;
        MCP_ok = MCP_hit>0;
        TOF_ok = PPAC_hit>0;        
        
        outTree->Fill();
    }
    outFile->Write();

}