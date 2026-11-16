#include "McpMonitorAnalysis.h"

McpMonitorAnalysis::McpMonitorAnalysis(Inputs& in, Mcp& m, Monitor& ex,
                                       McpAnalysis::McpReader&         mr,
                                       MonitorAnalysis::MonitorReader& exr)
    : CoincidenceAnalysis(in, m, ConfManager::getInstance().mcp, ex,
                          ConfManager::getInstance().monitor,
                          ConfManager::getInstance().mcpMonitor, mr, exr,
                          mcpMonitorHistograms),
      mcp(m), monitor(ex),
      mcpMonitorConf(ConfManager::getInstance().mcpMonitor), mcpReader(mr),
      monitorReader(exr),
      ETOFgates(conf.replay.conf_path+"/CUT/MCP","etof") {
    (void)mcpReader;
}

bool McpMonitorAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    CoincidenceAnalysis::AllocateHistos();
    McpMonitorHistograms& h = mcpMonitorHistograms;
    h.dir
        = input.outFile.mkdir("McpMonitor", "EXOTIC_MCP-Monitor coincidences");
    h.dir->cd();

    Initialize<TH3D>(h.m_ionTrack,
                     new TH3D("m_ionTrack",
                              "Ion tracks;(-X)[mm];Y[mm];Z[mm]",
                              100, -49, 51, 100, -49, 51, 600, -3500, 0),
                     h.ptrs);
    
    Initialize<TH3D>(h.m_layout_XYZ,
                     new TH3D("m_layout_XYZ",
                              "MCP X vs Y vs Z;<-AGATA (-X)[mm];Y[mm];Z[mm]", 
                              100, -49, 51, 100, -49, 51, 600, -3500, -2200),
                     h.ptrs);


    Initialize<TH2D>(h.m_TSdiff_monitorID,
                     new TH2D("m_TSdiff_monitorID",
                              "Timestamp difference between Monitor and MCP; TS [10 ns]; ID",100,-50,50,
                 20,0,20),
                    h.ptrs);

    Initialize<TH2D>(h.m_Energy_monitorID,
                     new TH2D("m_Energy_monitorID",
                              "Monitor E gated on MCPs; ID; E [MeV]", 20, 0, 20,
                 static_cast<int>(conf.monitor.monitorEmax / UNITS::MeV
                                  / conf.monitor.monitorbinWidth)
                        + static_cast<int>(10 / conf.monitor.monitorbinWidth),0,
                        conf.monitor.monitorEmax / UNITS::MeV),
                    h.ptrs);

    //SetupLabelsForChannelID(conf.monitor, h.m_Energy_monitorID);

    auto brdchanTOF = conf.mcp.getBrdChanFromId(1, McpLut::TOF12);
    double tofThrlow     = conf.mcp.getTof(conf.mcp.getThr(brdchanTOF.first, brdchanTOF.second).first,brdchanTOF.first, brdchanTOF.second);
    double tofThrhigh    = conf.mcp.getTof(conf.mcp.getThr(brdchanTOF.first, brdchanTOF.second).second,brdchanTOF.first, brdchanTOF.second);
    
    for(const auto& id : conf.monitor.getAllMonitorId()) {

        Initialize<TH2D>(
          h.m_XY_target[id],
          new TH2D(
              Form("m_XY_target_%i",id),
              Form("MONITOR%i gated target hit position XY;(-X)[mm];Y[mm]",id), 1000, -100,
                 100, 1000, -100, 100),
          h.ptrs);

        Initialize<TH2D>(
          h.m_X1X2[id],
          new TH2D(
              Form("m_X1X2_%i",id),
                 Form("MCP1_X vs MCP2_X for MONITOR%i;<-AGATA(-X)[mm];<-AGATA(-X)[mm]",id), 1000, -100,
                 100, 1000, -100, 100),
          h.ptrs);

        Initialize<TH2D>(
          h.m_Y1Y2[id],
          new TH2D(
              Form("m_Y1Y2_%i",id),
                 Form("MCP1_Y vs MCP2_Y for MONITOR%i;(Y)[mm];(Y)[mm]",id), 1000, -100,
                 100, 1000, -100, 100),
          h.ptrs);


        for(const auto& mcpid : conf.mcp.getAllMcpId()){

        Initialize<TH2D>(
          h.m_layout_front[mcpid][id],
          new TH2D(
              Form("m_layout_front_%i_%i",mcpid,id),
              Form("MCP%i+MONITOR%i front (beam) view (for EFFICIENCY X-Y-T);<-AGATA | Prisma->(-X)[mm];Y[mm]",mcpid,id), 1000,
              -100, 100, 1000, -100, 100),
          h.ptrs);

        Initialize<TH2D>(
          h.m_X_Energy[mcpid][id],
          new TH2D(
              Form("m_X_Energy_%i_%i",mcpid,id),
              Form("MCP%i+MONITOR%i X vs Si Energy;<-AGATA | Prisma->(-X)[mm];E [MeV]",mcpid,id), 1000,
              -100, 100, static_cast<int>(conf.monitor.monitorEmax / UNITS::MeV
                                  / conf.monitor.monitorbinWidth)
                        + static_cast<int>(10 / conf.monitor.monitorbinWidth),0,
                        conf.monitor.monitorEmax / UNITS::MeV),
          h.ptrs);



        Initialize<TH2D>(
          h.m_layout_front_gated[mcpid][id],
          new TH2D(
              Form("m_layout_front_gated_%i_%i",mcpid,id),
              Form("MCP%i+MONITOR%i front (beam) view;<-AGATA | Prisma->(-X)[mm];Y[mm]",mcpid,id), 1000,
              -100, 100, 1000, -100, 100),
          h.ptrs);


            unsigned int cnt = 0;
            for(const auto& id2 : conf.mcp.getTofsPresent(mcpid)) {

                if(id2) 
                    Initialize<TH2D>(
                      h.m_Energy_TOF_monitorID[cnt][id],
                      new TH2D(
                          Form("m_Energy_TOF_monitorID_%i_%i",cnt,id),
                          Form("Monitor%i E vs MCP TOF%i; E [MeV]; TOF [ns]",id,cnt), 
                          static_cast<int>(conf.monitor.monitorEmax / UNITS::MeV
                                  / conf.monitor.monitorbinWidth)
                        + static_cast<int>(10 / conf.monitor.monitorbinWidth) , 0,
                        conf.monitor.monitorEmax / UNITS::MeV,1000,
                        tofThrlow,tofThrhigh),
                      h.ptrs);
                cnt++;
            }
        }
    }

    if(!conf.mcpMonitor.cut_etof_ids.empty())
        for(auto& it: conf.mcpMonitor.cut_etof_ids){   

            Initialize<TH3D>(h.m_ionTrack_gatedETOF[it.first][it.second],
                     new TH3D(Form("m_ionTrack_gatedETOF_%i_%i",it.first,it.second),
                              Form("Ion tracks gated on E_TOF cut %i %i;(-X)[mm];Y[mm];Z[mm]",it.first,it.second),
                              100, -49, 51, 100, -49, 51, 600, -3500, -2200),
                     h.ptrs);

        }


    return true;
}

void McpMonitorAnalysis::Analyze() {
    CoincidenceAnalysis::Analyze();

    if(settings.verbosity_level > 2) {
        std::cout << "\n------------------------------------------------------"
                     "-------------------------------\n";
        std::cout << "NEW MCP-Monitor EVENT\n " << std::endl;
    }
    McpMonitorHistograms& h = mcpMonitorHistograms;

    if(settings.verbosity_level>1) 
      std::cout<<" Mon E size "<<monitor.ESi.size()<<std::endl;
    
    //for(unsigned int i{0}; i < monitor.ESi.size(); ++i) {
    for (size_t i = 0; i < coincMatrix.size(); ++i) {   

            auto diff = ( monitor.Time[i] - mcp.TS*conf.mcp.time_unit);
		 
            if(settings.verbosity_level>1) 
                std::cout<<"Monitor TS "<<monitor.Time[i]<<" MCP TS "<<mcp.TS*conf.mcp.time_unit<<" diff "<<diff<<std::endl;
            Fill(h.m_TSdiff_monitorID, diff, monitor.Id.at(i));
        
            
      //      if(diff > conf.mcpMonitor.windowLeft && diff< conf.mcpMonitor.windowRight){ 

                // for EFFICIENCY
                Fill(h.m_Energy_monitorID,monitor.Id.at(i),monitor.ESi.at(i));
                
                
                for(const auto& id : conf.mcp.getAllMcpId()){

		  Fill(h.m_layout_front[id][monitor.Id.at(i)],-mcp.pos.at(id-1).x(),mcp.pos.at(id-1).y());
		  if(mcp.bothMCP){
		    if(settings.verbosity_level>2)
		      std::cout<<"mcp pos with MONITOR "<<std::endl;
		    Fill(h.m_layout_front_gated[id][monitor.Id.at(i)],-mcp.pos.at(id-1).x(),mcp.pos.at(id-1).y());
                    
		    
		    Fill(h.m_layout_XYZ,-mcp.pos.at(id-1).x(),mcp.pos.at(id-1).y(),
			 mcp.pos.at(id-1).z());
		    Fill(h.m_X_Energy[id][monitor.Id.at(i)],-mcp.pos.at(id-1).x(),monitor.ESi.at(i));
		    
		  }
		  unsigned int cnt = 0;
		  for(const auto& id2 : conf.mcp.getTofsPresent(id)){
		    
		    if((id2) && mcp.TOF.at(id-1).at(cnt) > 0){ 
		      auto tof = mcp.TOF.at(id-1).at(cnt);
		      Fill(h.m_Energy_TOF_monitorID[cnt][monitor.Id[i]],monitor.ESi[i],tof);
		      if(mcp.bothMCP && conf.mcpMonitor.enablecutetof)    
			if(!conf.mcpMonitor.cut_etof_ids.empty())
			  for(auto& it: conf.mcpMonitor.cut_etof_ids){    
			    if(monitor.Id.at(i) == it.second){
			      TVector3 track = mcp.postarget - mcp.pos.at(0);
			      auto result = ETOFgates.Id({it.first,it.second},monitor.ESi[i],tof);
			      for(unsigned int k{0}; k<result.second.size(); k++)
				if(result.first && result.second[k] == it.second){
				  int numstep{500}; 
				  for(int jj{-100}; jj < numstep + 1; jj++)
				    Fill(h.m_ionTrack_gatedETOF[it.first][it.second],-(mcp.pos[0].x()
										       + (track.x()) * (jj) / numstep),
					 (mcp.pos[0].y()
					  + (track.y()) * (jj) / numstep),
					 (mcp.pos[0].z()
					  + (track.z()) * (jj) / numstep));                                            
				}
			    }
                                        }
		    }
		    cnt++;
		  }
		  
                }
		
                TVector3 track = mcp.postarget - mcp.pos.at(0);
                int nstep{500};
                if(mcp.bothMCP){
                  for(int jj{-100}; jj < nstep + 1; jj++) {
                          Fill(h.m_ionTrack,
                               -(mcp.pos[0].x()
                                 + (track.x()) * (jj) / nstep),
                               (mcp.pos[0].y()
                                + (track.y()) * (jj) / nstep),
                               (mcp.pos[0].z()
                                + (track.z()) * (jj) / nstep));
                  }   
                    
                    Fill(h.m_XY_target[monitor.Id.at(i)], -mcp.postarget.x(),
                         mcp.postarget.y());
                    Fill(h.m_X1X2[monitor.Id.at(i)], -mcp.pos.at(0).x(), -mcp.pos.at(1).x());
                    Fill(h.m_Y1Y2[monitor.Id.at(i)], mcp.pos.at(0).y(), mcp.pos.at(1).y());
                }

            // } // time difference window
    } // for coincMatrix
                


}
        
    




bool McpMonitorAnalysis::FillHistos() {
    if(!CoincidenceAnalysis::FillHistos()) return false;

    // Filling histograms
    // McpMonitorHistograms& h = mcpMonitorHistograms;

    return true;
}

void McpMonitorAnalysis::Finalize() { CoincidenceAnalysis::Finalize(); }
