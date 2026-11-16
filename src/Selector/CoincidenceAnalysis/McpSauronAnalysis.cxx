
#include "McpSauronAnalysis.h"
#include "TVector3.h"
#include "TLorentzVector.h"

McpSauronAnalysis::McpSauronAnalysis(DetectorAnalysis::Inputs& in,
                                         Mcp& a, Sauron& s, SauronConf& sc,
                                         McpSauronConf&              c,
                                         McpAnalysis::McpReader& ra,
                                         SauronAnalysis::SauronReader& rs,
                                         SauronMcp&                 sm)
    : CoincidenceAnalysis(in, a, ConfManager::getInstance().mcp,
                                  s, sc, c, ra, rs, mcpSauronHistograms),
      mcp(a), sauron(s), mcpSauronConf(c), mcpReader(ra), sauronMcp(sm), sauronReader(rs), sauronConf(sc) {
        (void)mcp;
        (void)sauron;
        (void)mcpSauronConf;
        (void)mcpReader;
        (void)sauronMcp;
        (void)sauronReader;
        (void)sauronConf;
    }

McpSauronFwAnalysis::McpSauronFwAnalysis(DetectorAnalysis::Inputs& in,
                                             Mcp& a, Sauron& s,
                                             McpAnalysis::McpReader&   ra,
                                             SauronAnalysis::SauronReader& rs,
                                             SauronMcp&                 sm)
    : McpSauronAnalysis(in, a, s, ConfManager::getInstance().sauronFw,
                          ConfManager::getInstance().mcpSauronFw, ra, rs, sm) {
    if(coincidenceConf.enable_treeFill && input.type == Inputs::Type::SIGNAL && input.tree)
        input.tree->Branch("McpSauronFw");
}


McpSauronBwAnalysis::McpSauronBwAnalysis(DetectorAnalysis::Inputs& in,
                                             Mcp& a, Sauron& s,
                                             McpAnalysis::McpReader&   ra,
                                             SauronAnalysis::SauronReader& rs,
                                             SauronMcp&                 sm)
    : McpSauronAnalysis(in, a, s, ConfManager::getInstance().sauronBw,
                          ConfManager::getInstance().mcpSauronBw, ra, rs, sm) {
    if(coincidenceConf.enable_treeFill)
        input.tree->Branch("McpSauronBw");
}

bool McpSauronFwAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    McpSauronHistograms& h = mcpSauronHistograms;
    h.dir
        = input.outFile.mkdir("McpSauronFw", "Mcp-SauronFw coincidences");
    h.dir->cd();
    if(!McpSauronAnalysis::AllocateHistos()) return false;
    return true;
}

bool McpSauronBwAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    McpSauronHistograms& h = mcpSauronHistograms;
    h.dir
        = input.outFile.mkdir("McpSauronBw", "Mcp-SauronBw coincidences");
    h.dir->cd();
    if(!McpSauronAnalysis::AllocateHistos()) return false;
    return true;
}

bool McpSauronAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    if(!CoincidenceAnalysis::AllocateHistos()) return false;
    McpSauronHistograms& h = mcpSauronHistograms;
    
    Initialize<TH1D>(
		     h.h_Ex,
		     new TH1D(
			      "h_Ex",
			      "SAURON excitation energy;Excitation Energy [MeV];Counts/0.1 MeV",
			      static_cast<int>(sauronConf.sauronExmax / UNITS::MeV
					       / sauronConf.sauronExbinWidth)
			      + static_cast<int>(10 / sauronConf.sauronExbinWidth),
            -10, sauronConf.sauronExmax / UNITS::MeV),
		     h.ptrs);
    
    int   nbins{(mcpSauronConf.windowRight - mcpSauronConf.windowLeft + 600)*2};
    int   xmin{mcpSauronConf.windowLeft - 300};
    int   xmax{mcpSauronConf.windowRight + 300};
    
    
    Initialize<TH2D>(
		     h.m_Ex_TSmTS,
		     new TH2D("m_Ex_TSmTS",
			      "SAURON Ex vs Time difference with MCP;TS [10 ns]; Excitation Energy [MeV]", nbins,
			      xmin, xmax,
			      static_cast<int>(sauronConf.sauronExmax / UNITS::MeV
					       / sauronConf.sauronExbinWidth),
			      -10, sauronConf.sauronExmax / UNITS::MeV),
		     h.ptrs);

        Initialize<TH2D>(
		     h.m_Ex_posX,
		     new TH2D("m_Ex_posX",
			      "SAURON Ex vs position on target X;X (<-AGATA|PRISMA->) [mm]; Excitation Energy [MeV]", 1000,
			      -100, 100,
			      static_cast<int>(sauronConf.sauronExmax / UNITS::MeV
					       / sauronConf.sauronExbinWidth),
			      -10, sauronConf.sauronExmax / UNITS::MeV),
		     h.ptrs);

	Initialize<TH2D>(
			 h.m_Ex_posY,
			 new TH2D("m_Ex_posY",
				  "SAURON Ex vs position on target Y;Y [mm]; Excitation Energy [MeV]", 1000,
				  -100, 100,
				  static_cast<int>(sauronConf.sauronExmax / UNITS::MeV
						   / sauronConf.sauronExbinWidth),
				  -10, sauronConf.sauronExmax / UNITS::MeV),
			 h.ptrs);
	
	
    
    for(const auto& reaction : input.reactions) {
        McpSauronHistograms::ReactionHistograms& reactionHistos
            = h.reactionHistos[{reaction.first}];
        std::string histoName = "_A" + std::to_string(reaction.first.A) + "Z"
                              + std::to_string(reaction.first.Z);
        std::string histoTitle = " A=" + std::to_string(reaction.first.A)
                               + " Z=" + std::to_string(reaction.first.Z);

        reactionHistos.dir = h.dir->mkdir(
            Form("Reaction%s", histoName.c_str()),
            Form("Spectra with cuts on the identified ion: %s",
                 histoTitle.c_str()));
        reactionHistos.dir->cd();

        Initialize<TH1D>(
            reactionHistos.h_Ex,
            new TH1D(Form("h_Ex%s",histoName.c_str()),
                     "SAURON excitation energy;Excitation Energy "
                     "[MeV];Counts/0.1 MeV",
                     static_cast<int>(sauronConf.sauronExmax / UNITS::MeV
                                      / sauronConf.sauronExbinWidth)
                         + static_cast<int>(10 / sauronConf.sauronExbinWidth),
                     -10, sauronConf.sauronExmax / UNITS::MeV),
            reactionHistos.ptrs);

        Initialize<TH2D>(
			 reactionHistos.m_Ex_Theta,
			 new TH2D(Form("m_Ex_Theta%s",histoName.c_str()),
				  "SAURON theta vs excitation energy;Theta [deg]; Excitation Energy [MeV]", 180,
				  0, 180,
				  static_cast<int>(sauronConf.sauronExmax / UNITS::MeV
						   / sauronConf.sauronExbinWidth),
				  0, sauronConf.sauronExmax / UNITS::MeV),
			 reactionHistos.ptrs);
	
	
    Initialize<TH2D>(
		     reactionHistos.m_Ex_TSmTS,
		     new TH2D(Form("m_Ex_TSmTS%s",histoName.c_str()),
			      "SAURON Ex vs Time difference with MCP;TS [10 ns]; Excitation Energy [MeV]", 180,
			      0, 180,
			      static_cast<int>(sauronConf.sauronExmax / UNITS::MeV
					       / sauronConf.sauronExbinWidth),
			      0, sauronConf.sauronExmax / UNITS::MeV),
		     reactionHistos.ptrs);
    }
    return true;
}


void McpSauronAnalysis::Analyze()
{

    if(settings.verbosity_level > 1) {
        std::cout << "\n------------------------------------------------------"
                     "-------------------------------\n";
        std::cout << "NEW MCP-SAURON EVENT\n " << std::endl;
    }
    // Run the generic coincidence machinery first
    CoincidenceAnalysis::Analyze();

    
    McpSauronHistograms& h = mcpSauronHistograms;

    sauronMcp.sauronMcpGood = false;
    
    // Prepare output container to mirror the coincidence structure (if needed)
    sauronMcp.resize(coincMatrix.size());
    if(settings.verbosity_level>4)
      std::cout<<"Coincidence matrix size: "<<coincMatrix.size()<<std::endl;
    sauronMcp.p3=sauron.p3; //copy SAURON p3 to SauronMcp p3

    TVector3 beamDir = mcp.p1.Vect();
    if(settings.verbosity_level>1){
    std::cout<<"Initial BeamDir (x,y,z) = "<<beamDir.x()<<" "<<beamDir.y()<<" "<<beamDir.z()<<" mag2 "<<beamDir.Mag2()<<std::endl;    
    std::cout<<"MCP p1 (x,y,z) = "<<mcp.p1.Vect().x()<<" "<<mcp.p1.Vect().y()<<" "<<mcp.p1.Vect().z()<<std::endl;
    std::cout<<"both MCP "<<mcp.bothMCP<<std::endl;
    }
    if (beamDir.Mag2() == 0.) beamDir = TVector3(0., 0., 1.);
    beamDir = beamDir.Unit();
    if(settings.verbosity_level>1) std::cout<<"BeamDirO (x,y,z) = "<<beamDir.x()<<" "<<beamDir.y()<<" "<<beamDir.z()<<std::endl;

    // Loop over SAURON hits that are in coincidence with MCP and recompute Ex
    for (size_t i = 0; i < coincMatrix.size(); ++i) {
    if(settings.verbosity_level>5){
        std::cout<<"Coincidence matrix size "<<coincMatrix.size()<<" index "<<i<<std::endl;  
        for(size_t j = 0; j < coincMatrix.at(i).size(); ++j){
            std::cout<<coincMatrix.at(i).at(j)<<"\t";
        }
        std::cout<<std::endl;
    }
        // Require both MCPs to be present    
        if(!mcp.bothMCP){
	        if(settings.verbosity_level>1)std::cout<<"MCP not goot"<<std::endl;
	        continue;
        } else{
            if(settings.verbosity_level>1){
                std::cout<<"MCP good"<<std::endl;
                std::cout<<"mcp.p1 (x,y,z) = "<<mcp.p1.Vect().x()<<" "<<mcp.p1.Vect().y()<<" "<<mcp.p1.Vect().z()<<std::endl;
            }
        }
        if(!sauron.sauronGood){
      	    if(settings.verbosity_level>1) std::cout<<"Sauron empty..."<<std::endl;
      	    continue;
        }
        //      std::cout<<"Sauron p3 "<<sauron.p3.at(i).Vect().x()<<" "<<sauron.p3.at(i).Vect().y()<<" "<<sauron.p3.at(i).Vect().z()<<std::endl;
        if(settings.verbosity_level>1)
	        std::cout<<"\n\n----------------------Size SAURON-MCP "<<coincMatrix.size()<<" "<<i<<"\n\n\n"<<std::endl;

        
            // Skip if there is no valid coincidence on this row
        if(coincMatrix.at(i).size() == 0) {
            if(settings.verbosity_level>1) std::cout<<"No coinc found for this index "<<i<<std::endl;
            continue;
        }
        if(!coincMatrix.at(i).at(0)) continue; //maybe at(0).at(i) ?
        if(settings.verbosity_level>1) std::cout<<"in coinc "<<coincMatrix.at(i).at(0)<<std::endl;
        if(settings.verbosity_level>1) std::cout<<"BeamDir (x,y,z) = "<<beamDir.x()<<" "<<beamDir.y()<<" "<<beamDir.z()<<std::endl;


        // Bounds/validity guards against partially-filled vectors
        if (i >= sauron.E.size() || i >= sauronMcp.p3.size()
	    || i >= sauron.identified.size()) continue; 

        // Require an identified ion so we can pick the correct reaction
        ReactionFragment::Ion ion;
        if (sauron.identified.at(i)){
	    // Build the ion key to fetch the corresponding reaction
	    ion = ReactionFragment::Ion{ static_cast<unsigned int>(sauron.A.at(i)),
				     static_cast<unsigned int>(sauron.Z.at(i)) };
        }else{	
	        ion = sauronConf.ion;
        }
        auto reactionsIt = input.reactions.find(ion);
        auto reactionCopy = reactionsIt->second;
        if (reactionsIt == input.reactions.end()) {
	      // No matching reaction configured for this ion; skip
	      continue;
        }

        // Direction of the detected ion in lab coordinates (unit vector)
        TVector3 ionDirLab = sauronMcp.p3.at(i).Vect();
        if (ionDirLab.Mag2() == 0.) continue;
        ionDirLab = ionDirLab.Unit();

        // Express the ion direction in the *beam frame* where Z is aligned to the MCP beam
        TVector3 ionDirBeamFrame = ionDirLab;
        ionDirBeamFrame.RotateUz(beamDir);

        sauronMcp.sauronMcpGood = true;
        if(settings.verbosity_level>1){
            std::cout<<"sauronMcopGood "<<sauronMcp.sauronMcpGood<<std::endl;
            std::cout<<"ionDirLab (x,y,z) = "<<ionDirLab.x()<<" "<<ionDirLab.y()<<" "<<ionDirLab.z()<<std::endl;
            
        }

        sauronMcp.Time.at(i) = sauron.Time.at(i);     
        //std::cout<<"SAURON time "<<sauron.Time.at(i)<<" SAURON TS "<<sauron.TS<<" SAURON MCP time "<<sauronMcp.Time.at(i)<<std::endl;
        sauronMcp.TS = sauron.TS;
        // Angles *relative to the measured beam axis*
        const double thetaB = ionDirBeamFrame.Theta();
        const double phiB   = ionDirBeamFrame.Phi();



        // Use the already energy-loss corrected kinetic energy from SAURON
        const double Ek = sauron.E.at(i);


        // Recompute the excitation energy from (Ek, thetaB, phiB)
        // Set_Ek_Theta_Phi updates the internal kinematics and returns Ex
        double ex = static_cast<double>(reactionCopy.Set_Ek_Theta_Phi(Ek, thetaB, phiB));
        //double ex2 = static_cast<double>(reactionsIt->second.Set_Ek_Theta_Phi(Ek, 0.732815,1.37445));
        if(settings.verbosity_level>1){
          std::cout<<"thetaB "<<thetaB<<std::endl;
          std::cout<<"phiB "<<phiB<<std::endl;
          std::cout<<"Ek "<<Ek<<std::endl;
          std::cout<<"ex "<<ex<<std::endl;
        }
        if(mcp.bline1 && mcp.bline2 && settings.verbosity_level>4)
	          std::cout<<"Inside BLINE"<<std::endl;

	//std::cout<<"i "<<i <<" size mcp.Time "<<mcp.Time.size()<<" size sauron Time"<<sauron.Time.size()<<std::endl;
        // Overwrite the SAURON excitation energy with the beam-aligned value
        sauronMcp.Ex.at(i) = ex;

        // Also update the binary partner 4-vector from the reaction model for consistency
        sauronMcp.p4.at(i) = reactionsIt->second.GetFreeFragment().Get_P();

        /* *************************** */
        // HISTOS

        Fill(h.h_Ex,sauronMcp.Ex.at(i));

	// TIME DIFERENCE
	
	auto diff = unsignedDiff<double, unsigned long long> (sauron.TS,mcp.TS)
	  + (sauron.Time.at(i)-mcp.Time.at(0))/ (10* UNITS::ns);
	if(settings.verbosity_level>4)
	  std::cout<<"Time difference MCP-SAURON "<<diff<<" /10 ns"<<std::endl;
	Fill(h.m_Ex_TSmTS,diff,sauronMcp.Ex.at(i) );
	

	// Ex vs POSITIONS

	Fill(h.m_Ex_posX,-mcp.postarget.x(),sauronMcp.Ex.at(i));
	Fill(h.m_Ex_posY,mcp.postarget.y(),sauronMcp.Ex.at(i));
	
        // Particle identification histograms
        std::vector<ReactionFragment::Ion> reaction;
        for(unsigned int e{0}; e < sauron.E.size(); ++e) {
	  if(!sauron.identified.at(e)) { continue; }
	  
	  ReactionFragment::Ion idIon{sauron.A.at(e), sauron.Z.at(e)};
	  reaction.emplace_back(idIon);
	}
	std::sort(reaction.begin(), reaction.end());
	
	auto reactionHist = h.reactionHistos.find(reaction);
	if(reactionHist != h.reactionHistos.end()) {
	  for(unsigned int s{0}; s < sauron.E.size(); ++s) {
	    Fill(reactionHist->second.h_Ex, sauronMcp.Ex.at(s));
	    Fill(reactionHist->second.m_Ex_sauronId, sauron.IdPside[s], sauronMcp.Ex[s]);
	    Fill(reactionHist->second.m_Ex_sauronId, sauron.IdNside[s], sauronMcp.Ex[s]);
	    Fill(reactionHist->second.m_Ex_Theta,
		 sauronMcp.p3[s].Theta() / UNITS::deg, sauronMcp.Ex[s]);
	  }
	}
        
	if(settings.verbosity_level>0){
	  std::cout<<"SAURON TS "<<sauron.TS<<std::endl;
	  std::cout<<"MCP TS "<<mcp.TS<<std::endl;  
	  std::cout<<"sauron.p3 "<<sauron.p3.at(i).Vect().x()<<" "<<sauron.p3.at(i).Vect().y()<<std::endl;
	  std::cout<<"sauronMcp.p3 "<<sauronMcp.p3.at(i).Vect().x()<<" "<<sauronMcp.p3.at(i).Vect().y()<<std::endl;
	}
	
    }

    //    }
    if(settings.verbosity_level>1)
      std::cout<<"----------------------------------------"<<std::endl;
}

void McpSauronAnalysis::Finalize() {
    CoincidenceAnalysis::Finalize();
}
