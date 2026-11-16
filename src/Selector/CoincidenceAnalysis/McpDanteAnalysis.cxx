#include "McpDanteAnalysis.h"

McpDanteAnalysis::McpDanteAnalysis(Inputs& in, Mcp& m, Dante& ex,
                                       McpAnalysis::McpReader&         mr,
                                       DanteAnalysis::DanteReader& exr)
    : CoincidenceAnalysis(in, m, ConfManager::getInstance().mcp, ex,
                          ConfManager::getInstance().dante,
                          ConfManager::getInstance().mcpDante, mr, exr,
                          mcpDanteHistograms),
      mcp(m), dante(ex),
      mcpDanteConf(ConfManager::getInstance().mcpDante), mcpReader(mr),
      danteReader(exr){
    (void)mcpReader;
}

bool McpDanteAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    CoincidenceAnalysis::AllocateHistos();
    McpDanteHistograms& h = mcpDanteHistograms;
    h.dir
        = input.outFile.mkdir("McpDante", "EXOTIC_MCP-Dante coincidences");
    h.dir->cd();

    Initialize<TH3D>(h.m_ionTrack,
                     new TH3D("m_ionTrack",
                              "Ion tracks;(-X)[mm];Y[mm];Z[mm]",
                              100, -49, 51, 100, -49, 51, 600, -3500, 0),
                     h.ptrs);
    
    Initialize<TH3D>(h.m_layout_XYZ,
                     new TH3D("m_layout_XYZ",
                              "MCP X vs Y vs Z;<-AGATA (-X)[mm];Y[mm];Z[mm]", 
                              100, -49, 51, 100, -49, 51, 600, -3500, 0),
                     h.ptrs);


    Initialize<TH2D>(h.m_TSdiff_danteID,
                     new TH2D("m_TSdiff_danteID",
                              "Timestamp difference between Dante and MCP; TS [10 ns]; ID",100,-50,50,
                 20,0,20),
                    h.ptrs);

   

    //SetupLabelsForChannelID(conf.dante, h.m_Energy_danteID);

    auto brdchanTOF = conf.mcp.getBrdChanFromId(1, McpLut::TOF12);
    double tofThrlow     = conf.mcp.getTof(conf.mcp.getThr(brdchanTOF.first, brdchanTOF.second).first,brdchanTOF.first, brdchanTOF.second);
    double tofThrhigh    = conf.mcp.getTof(conf.mcp.getThr(brdchanTOF.first, brdchanTOF.second).second,brdchanTOF.first, brdchanTOF.second);
    
    for(const auto& id : conf.dante.getAllDanteId()) {
        auto brdchanX = conf.dante.getBrdChanFromId(id, DanteLut::X);
        auto brdchanY = conf.dante.getBrdChanFromId(id, DanteLut::Y);
        auto xThr     = conf.dante.getThr(brdchanX.first, brdchanX.second);
        auto yThr     = conf.dante.getThr(brdchanY.first, brdchanY.second);
        Initialize<TH2D>(
            h.m_dante[id],
            new TH2D(Form("m_dante_%i", id),
                     Form("Total counts for Dante detector %i;X;Y;Counts", id),
                     1000, -100,100,1000,-100,100),
            h.ptrs);


        Initialize<TH2D>(
          h.m_XY_target[id],
          new TH2D(
              Form("m_XY_target_%i",id),
              Form("DANTE%i gated target hit position XY;(-X)[mm];Y[mm]",id), 1000, -100,
                 100, 1000, -100, 100),
          h.ptrs);

        Initialize<TH2D>(
          h.m_X1X2[id],
          new TH2D(
              Form("m_X1X2_%i",id),
                 Form("MCP1_X vs MCP2_X for DANTE%i;<-AGATA(-X)[mm];<-AGATA(-X)[mm]",id), 1000, -100,
                 100, 1000, -100, 100),
          h.ptrs);

        Initialize<TH2D>(
          h.m_Y1Y2[id],
          new TH2D(
              Form("m_Y1Y2_%i",id),
                 Form("MCP1_Y vs MCP2_Y for DANTE%i;(Y)[mm];(Y)[mm]",id), 1000, -100,
                 100, 1000, -100, 100),
          h.ptrs);


        for(const auto& mcpid : conf.mcp.getAllMcpId()){

        Initialize<TH2D>(
          h.m_layout_front[mcpid][id],
          new TH2D(
              Form("m_layout_front_%i_%i",mcpid,id),
              Form("MCP%i+DANTE%i front (beam) view (for EFFICIENCY X-Y-T);<-AGATA | Prisma->(-X)[mm];Y[mm]",mcpid,id), 1000,
              -100, 100, 1000, -100, 100),
          h.ptrs);

        Initialize<TH2D>(
          h.m_layout_front_gated[mcpid][id],
          new TH2D(
              Form("m_layout_front_gated_%i_%i",mcpid,id),
              Form("MCP%i+DANTE%i front (beam) view (for EFFICIENCY X-Y-T);<-AGATA | Prisma->(-X)[mm];Y[mm]",mcpid,id), 1000,
              -100, 100, 1000, -100, 100),
          h.ptrs);




        }
    }


    return true;
}

void McpDanteAnalysis::Analyze() {
    CoincidenceAnalysis::Analyze();

    if(settings.verbosity_level > 8) {
        std::cout << "\n------------------------------------------------------"
                     "-------------------------------\n";
        std::cout << "NEW MCP-Dante EVENT\n " << std::endl;
    }
    McpDanteHistograms& h = mcpDanteHistograms;

    for(unsigned int i{0}; i < dante.Pos.size(); ++i) {
        
        if(!mcp.histopos) continue;
        auto diff = ( dante.TS*conf.dante.time_unit - mcp.TS*conf.mcp.time_unit)
                    / (10 * UNITS::nanosecond);
		 //	    auto diff = ( dante.Time[i] - mcp.Time[0])
		 //   / (10 * UNITS::nanosecond);
        if(settings.verbosity_level>1) 
            std::cout<<"Dante TS "<<dante.Time[i]<<" MCP TS "<<mcp.Time[0]<<" diff "<<diff<<std::endl;
        Fill(h.m_TSdiff_danteID, diff, dante.Id.at(i));
        
            
        if(diff > conf.mcpDante.windowLeft && diff< conf.mcpDante.windowRight ){
            
            Fill(h.m_dante[dante.Id.at(i)], -dante.Pos.at(i).x(),dante.Pos.at(i).y());
            Fill(h.m_layout_XYZ,-dante.Pos.at(i).x(),dante.Pos.at(i).y(),
                 dante.Pos.at(i).z());
            for(const auto& id : conf.mcp.getAllMcpId()){

		        Fill(h.m_layout_front[id][dante.Id.at(i)],-mcp.pos.at(id-1).x(),mcp.pos.at(id-1).y());
		        if(mcp.bothMCP){
		            if(settings.verbosity_level>2)
		                std::cout<<"mcp.histpos with DANTE "<<std::endl;
		            Fill(h.m_layout_front_gated[id][dante.Id.at(i)],-mcp.pos.at(id-1).x(),mcp.pos.at(id-1).y());


		            Fill(h.m_layout_XYZ,-mcp.pos.at(id-1).x(),mcp.pos.at(id-1).y(),
			         mcp.pos.at(id-1).z());
                
		        }
		        unsigned int cnt = 0;
		        //for(const auto& id2 : conf.mcp.getTofsPresent(id)){
                
		            //if((id2) && mcp.TOF.at(id-1).at(cnt) > 0){ 
		            //    auto tof = mcp.TOF.at(id-1).at(cnt);
		            //    Fill(h.m_X_TOF_danteID[cnt][dante.Id[i]],dante.ESi[i],tof);
		            //    if(mcp.bothMCP && conf.mcpDante.enablecutetof)    
			        //        
			        //}
                    //                        }
                
		        //    cnt++;
		        //}
		  
            }
		
            TVector3 track = mcp.postarget - mcp.pos.at(0);
            int nstep{500};
            for(int jj{-100}; jj < nstep + 1; jj++) {
                if(mcp.bothMCP && mcp.bline1 && mcp.bline2)
                    Fill(h.m_ionTrack,
                         -(mcp.pos[0].x()
                           + (track.x()) * (jj) / nstep),
                         (mcp.pos[0].y()
                          + (track.y()) * (jj) / nstep),
                         (mcp.pos[0].z()
                          + (track.z()) * (jj) / nstep));
            }   
            if(mcp.histopos){
                if(mcp.bline1 && mcp.bline2)
                  Fill(h.m_XY_target[dante.Id.at(i)], -mcp.postarget.x(),
                     mcp.postarget.y());
                Fill(h.m_X1X2[dante.Id.at(i)], -mcp.pos.at(0).x(), -mcp.pos.at(1).x());
                Fill(h.m_Y1Y2[dante.Id.at(i)], mcp.pos.at(0).y(), mcp.pos.at(1).y());
            }

        } // diff coinc.left right
    } // for dante.Pos.size
                


}
        
    




bool McpDanteAnalysis::FillHistos() {
    if(!CoincidenceAnalysis::FillHistos()) return false;

    // Filling histograms
    // McpDanteHistograms& h = mcpDanteHistograms;

    return true;
}

void McpDanteAnalysis::Finalize() { CoincidenceAnalysis::Finalize(); }
