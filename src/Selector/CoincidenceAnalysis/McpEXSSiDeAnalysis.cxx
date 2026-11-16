#include "McpEXSSiDeAnalysis.h"

McpEXSSiDeAnalysis::McpEXSSiDeAnalysis(Inputs& in, Mcp& m, EXSSiDe& ex,
                                       McpAnalysis::McpReader&         mr,
                                       EXSSiDeAnalysis::EXSSiDeReader& exr)
    : CoincidenceAnalysis(in, m, ConfManager::getInstance().mcp, ex,
                          ConfManager::getInstance().exsside,
                          ConfManager::getInstance().mcpExsside, mr, exr,
                          mcpExssideHistograms),
      mcp(m), exsside(ex),
      mcpEXSSiDeConf(ConfManager::getInstance().mcpExsside), mcpReader(mr),
      exssideReader(exr) {
    (void)mcpReader;
}

bool McpEXSSiDeAnalysis::AllocateHistos() {
    if(!coincidenceConf.enable_histos) return false;
    CoincidenceAnalysis::AllocateHistos();
    McpEXSSiDeHistograms& h = mcpExssideHistograms;
    h.dir
        = input.outFile.mkdir("McpEXSSiDe", "EXOTIC_MCP-EXSSiDe coincidences");
    h.dir->cd();

    Initialize<TH3D>(
        h.m_layout_XYZ,
        new TH3D("m_layout_XYZ",
                 "MCP + EXSSiDe X vs Y vs Z;AGATA<-(-X)[mm];Y[mm];Z[mm]", 200,
                 -199.5, 200.5, 100, -199.5, 200.5, 150, -3140, -2800),
        h.ptrs);

    Initialize<TH3D>(h.m_ionTrack,
                     new TH3D("m_ionTrack",
                              "Ion trajectories (with condition on "
                              "targetPos);AGATA<-(-X)[mm];Y[mm];Z[mm]",
                              100, -49, 51, 100, -49, 51, 400, -3200, -2800),
                     h.ptrs);
    Initialize<TH3D>(h.m_ionTrackSource,
                     new TH3D("m_ionTrackSource",
                              "Ion trajectories (with condition on "
                              "targetPos);AGATA<-(-X)[mm];Y[mm];Z[mm]",
                              100, -49, 51, 100, -49, 51, 400, -3200, -2800),
                     h.ptrs);

    Initialize<TH2D>(h.m_XY_gatedSi,
                     new TH2D("m_XY_gatedSi",
                              "MCP XY gated Si; AGATA<-(-X)(mm); Y(mm)", 1000,
                              -100, 100, 1000, -100, 100),
                     h.ptrs);

    Initialize<TH2D>(h.m_XY_gatedMCP,
                     new TH2D("m_XY_gatedMCP",
                              "Si XY gated MCP; AGATA<-(-X)(mm); Y(mm)", 100,
                              -99.5, 100.5, 100, -99.5, 100.5),
                     h.ptrs);

    if(mcpEXSSiDeConf.debug) {
        Initialize<TH2D>(h.m_expectedXY_gatedMCP,
                         new TH2D("m_expectedXY_gatedMCP",
                                  "Expected Si XY gated MCP based on given "
                                  "source position; AGATA<-(-X)(mm); Y(mm)",
                                  100, -99.5, 100.5, 100, -99.5, 100.5),
                         h.ptrs);

        Initialize<TH2D>(h.m_obsX_expectedX,
                         new TH2D("m_obsX_expectedX",
                                  "Observed X vs expected Si X gated MCP; "
                                  "X_{obs}(mm); X_{exp}(mm)",
                                  100, -99.5, 100.5, 100, -99.5, 100.5),
                         h.ptrs);

        Initialize<TH2D>(h.m_obsY_expectedY,
                         new TH2D("m_obsY_expectedY",
                                  "Observed Y vs expected Si Y gated MCP; "
                                  "Y_{obs}(mm); Y_{exp}(mm)",
                                  100, -99.5, 100.5, 100, -99.5, 100.5),
                         h.ptrs);

        Initialize<TH2D>(h.m_obsX_expectedY,
                         new TH2D("m_obsX_expectedY",
                                  "Observed X vs expected Si Y gated MCP; "
                                  "X_{obs}(mm); Y_{exp}(mm)",
                                  100, -99.5, 100.5, 100, -99.5, 100.5),
                         h.ptrs);

        Initialize<TH2D>(h.m_obsY_expectedX,
                         new TH2D("m_obsY_expectedX",
                                  "Observed Y vs expected Si X gated MCP; "
                                  "Y_{obs}(mm); X_{exp}(mm)",
                                  100, -99.5, 100.5, 100, -99.5, 100.5),
                         h.ptrs);
    }

    Initialize<TH2D>(h.m_XY_Si,
                     new TH2D("m_XY_Si", "m_XY_Si; (-X)[mm];Y[mm]", 1000, -200,
                              200, 1000, -200, 200),
                     h.ptrs);

    Initialize<TH2D>(h.m_XY_target,
                     new TH2D("m_XY_target", "m_XY_target; (-X)[mm];Y[mm]",
                              1000, -2001, 1999, 1000, -2001, 1999),
                     h.ptrs);

    Initialize<TH2D>(
        h.m_chan_TS,
        new TH2D("m_chan_TS",
                 "TS diff between MCP and Si; #Delta T[10 ns];Channel", 1000,
                 -100, 100, 64, 0, 64),
        h.ptrs);

    Initialize<TH1D>(
        h.h_cTime_gated,
        new TH1D("h_cTime_gated", "h_cTime_gated",
                 (mcpEXSSiDeConf.windowRight - mcpEXSSiDeConf.windowLeft + 200)
                     * 10,
                 mcpEXSSiDeConf.windowLeft - 100,
                 mcpEXSSiDeConf.windowRight + 100),
        h.ptrs);

    // for(const auto& id : conf.mcp.getAllMcpId()) {
    //     auto brdchanX   = conf.mcp.getBrdChanFromId(id, McpLut::X);
    //     auto brdchanY   = conf.mcp.getBrdChanFromId(id, McpLut::Y);
    //     auto xThr       = conf.mcp.getThr(brdchanX.first, brdchanX.second);
    //     auto yThr       = conf.mcp.getThr(brdchanY.first, brdchanY.second);
    // }

    return true;
}

void McpEXSSiDeAnalysis::Analyze() {
    CoincidenceAnalysis::Analyze();

    if(settings.verbosity_level > 8) {
        std::cout << "\n------------------------------------------------------"
                     "-------------------------------\n";
        std::cout << "NEW MCP-EXSSiDe EVENT\n " << std::endl;
    }
    McpEXSSiDeHistograms& h = mcpExssideHistograms;

    for(unsigned int i{0}; i < exssideReader.EXSSiDeEnergy->GetSize(); ++i) {
        unsigned int brd  = exssideReader.EXSSiDeBoard->At(i);
        unsigned int chan = exssideReader.EXSSiDeChannel->At(i);

        if(!conf.exsside.isPresent(brd, chan)) continue;

        if(mcp.histopos) {
            auto diff = ((exssideReader.TSEXSSiDe_Hit->At(i)
                          + conf.exsside.getTime(
                              exssideReader.EXSSiDeTime->At(i), brd, chan))
                             * conf.exsside.time_unit
                         - mcp.TS * conf.mcp.time_unit)
                      / (10 * UNITS::nanosecond);
            Fill(h.m_chan_TS, diff, chan);
        }
    }

    // If there is only one MCP we fill it as the first MCP
    if(exsside.good and mcp.histopos)
        for(unsigned int i{0}; i < mcp.pos.size(); i++) {
            if(conf.mcp.nDetectors == 1 and (i == 1)) break;
            // TS dtof histos
            auto dtof
                //= (mcp.Time.at(i) - exsside.Time.at(0)) * (10. * UNITS::ns);
                = (exsside.TS * conf.exsside.time_unit
                   - mcp.TS * conf.mcp.time_unit)
                / (10 * UNITS::nanosecond);
            Fill(h.h_cTime_gated, dtof);

            TVector3 dir;

            // for(unsigned int j{0}; j<exsside.Pos.size(); j++)
            if(dtof > mcpEXSSiDeConf.windowLeft
               and dtof < mcpEXSSiDeConf.windowRight) {
                if(mcp.bothMCP)
                    Fill(h.m_XY_Si, -exsside.Pos.x(), exsside.Pos.y());
                // to be compared with the m_XY_target from the McpAnalysis.cxx

                // if(mcp.pos.at(0).x()>-6.5 and mcp.pos.at(0).x()<5 and
                // mcp.pos.at(0).y()>-42 and mcp.pos.at(0).y()<-18)
                // if((mcp.pos.at(0).x()>=-15 and mcp.pos.at(0).x()<=-4) or
                // (mcp.pos.at(0).x()<=12 and mcp.pos.at(0).x()>=3.6))
                {
                    if(i == 0)
                        Fill(h.m_XY_gatedSi, -mcp.pos.at(i).x(),
                             mcp.pos.at(i).y());
                    Fill(h.m_XY_gatedMCP, -exsside.Pos.x(), exsside.Pos.y());
                    // TRAJECTORY RECONSTRUCTION with MCP1 and Si
                    if(conf.mcp.nDetectors == 1) {
                        //   const ReactionFragment::Ion& ion{conf.mcp.ion};
                        //    auto reactionsIt = input.reactions.find(ion);
                        //    if(reactionsIt != input.reactions.end()) {
                        //        auto beam =
                        //        reactionsIt->second.GetReactionFragment(1);
                        //        TVector3
                        //        beta(dir.X()/sqrt(dtof),dir.Y()/sqrt(dtof),dir.Z()/sqrt(dtof));
                        //        beam.Set_Beta(beta);
                        //        beam.Set_Theta_Phi(dir.Theta(), dir.Phi());
                        //        mcp.p1 = beam.Get_P();
                        //        EvalInTarget(mcp.p1, ion);
                        //    }

                        // TEST
                        // mcp.pos.at(0).SetXYZ(-10.546447,0.,-6989.4536);
                        // exsside.Pos.SetXYZ(-22.747,0.,-6850);

                        dir = -mcp.pos.at(0) + exsside.Pos;

                        TVector3 center(0, 0, 0); // position of the target
                        // plane of the target (x*conf.target.targetNorm.x() +
                        // y*conf.target.targetNorm.y() +
                        // z*conf.target.targetNorm.z() = 0)
                        if(dir.x() != 0 and dir.y() != 0 and dir.z() != 0) {
                            double denom = (conf.target.targetNorm.x()
                                            * dir.x() / dir.z())
                                         + (conf.target.targetNorm.y()
                                            * dir.y() / dir.z())
                                         + conf.target.targetNorm.z();
                            mcp.postarget.SetZ(
                                ((mcp.pos[0].z() / dir.z())
                                     * (conf.target.targetNorm.x() * dir.x()
                                        + conf.target.targetNorm.y() * dir.y())
                                 - (mcp.pos[0].x() * conf.target.targetNorm.x()
                                    + mcp.pos[0].y()
                                          * conf.target.targetNorm.y()))
                                / denom);
                            double zshift
                                = (mcp.postarget.z() - mcp.pos[0].z())
                                / dir.z();
                            mcp.postarget.SetX(zshift * dir.x()
                                               + mcp.pos[0].x());
                            mcp.postarget.SetY(zshift * dir.y()
                                               + mcp.pos[0].y());
                        }
                        if(dir.x() == 0 and (dir.y() != 0 and dir.z() != 0)) {
                            mcp.postarget.SetX(mcp.pos[0].x());
                            double denom = dir.y() * conf.target.targetNorm.y()
                                             / dir.z()
                                         + conf.target.targetNorm.z();
                            mcp.postarget.SetZ(
                                (-mcp.pos[0].x() * conf.target.targetNorm.x()
                                 + (mcp.pos[0].z() * dir.y()
                                    * conf.target.targetNorm.y() / dir.z())
                                 - mcp.pos[0].y() * conf.target.targetNorm.y())
                                / denom);
                            double zshift
                                = (mcp.postarget.z() - mcp.pos[0].z())
                                / dir.z();
                            mcp.postarget.SetY(zshift * dir.y()
                                               + mcp.pos[0].y());
                        }
                        if(dir.y() == 0 and (dir.x() != 0 and dir.z() != 0)) {
                            mcp.postarget.SetY(mcp.pos[0].y());
                            double denom = dir.x() * conf.target.targetNorm.x()
                                             / dir.z()
                                         + conf.target.targetNorm.z();
                            mcp.postarget.SetZ(
                                (-mcp.pos[0].y() * conf.target.targetNorm.y()
                                 + (mcp.pos[0].z() * dir.x()
                                    * conf.target.targetNorm.x() / dir.z())
                                 - mcp.pos[0].x() * conf.target.targetNorm.x())
                                / denom);
                            double zshift
                                = (mcp.postarget.z() - mcp.pos[0].z())
                                / dir.z();
                            mcp.postarget.SetX(zshift * dir.x()
                                               + mcp.pos[0].x());
                        }
                        if(dir.z() == 0 and (dir.x() != 0 and dir.y() != 0)) {
                            mcp.postarget.SetZ(mcp.pos[0].z());
                            double denom = dir.y() * conf.target.targetNorm.y()
                                             / dir.x()
                                         + conf.target.targetNorm.x();
                            mcp.postarget.SetX(
                                (-mcp.pos[0].z() * conf.target.targetNorm.z()
                                 + (mcp.pos[0].x() * dir.y()
                                    * conf.target.targetNorm.y() / dir.x())
                                 - mcp.pos[0].y() * conf.target.targetNorm.y())
                                / denom);
                            double zshift
                                = (mcp.postarget.x() - mcp.pos[0].x())
                                / dir.x();
                            mcp.postarget.SetY(zshift * dir.y()
                                               + mcp.pos[0].y());
                        }
                        if(dir.y() == 0 and dir.x() == 0 and dir.z() != 0) {
                            mcp.postarget.SetX(mcp.pos[0].x());
                            mcp.postarget.SetY(mcp.pos[0].y());
                            mcp.postarget.SetZ(
                                (-mcp.pos[0].x() * conf.target.targetNorm.x()
                                 - mcp.pos[0].y() * conf.target.targetNorm.y())
                                / conf.target.targetNorm.z());
                        }
                        if(dir.y() == 0 and dir.z() == 0 and dir.x() != 0) {
                            mcp.postarget.SetZ(mcp.pos[0].z());
                            mcp.postarget.SetY(mcp.pos[0].y());
                            mcp.postarget.SetX(
                                (-mcp.pos[0].y() * conf.target.targetNorm.y()
                                 - mcp.pos[0].z() * conf.target.targetNorm.z())
                                / conf.target.targetNorm.x());
                        }
                        if(dir.z() == 0 and dir.x() == 0 and dir.y() != 0) {
                            mcp.postarget.SetZ(mcp.pos[0].z());
                            mcp.postarget.SetX(mcp.pos[0].x());
                            mcp.postarget.SetY(
                                (-mcp.pos[0].x() * conf.target.targetNorm.x()
                                 - mcp.pos[0].z() * conf.target.targetNorm.z())
                                / conf.target.targetNorm.y());
                        }
                        if(dir.y() == 0 and dir.x() == 0 and dir.z() == 0) {
                            mcp.postarget.SetX(mcp.pos[0].x());
                            mcp.postarget.SetY(mcp.pos[0].y());
                            mcp.postarget.SetZ(mcp.pos[0].z());
                        }
                        TVector3 dispersion = mcp.postarget - center;
                        TVector3 track      = mcp.postarget - mcp.pos[0];

                        if(fabs(mcp.postarget.x() - center.x())
                               < conf.mcp.tolerance
                           and fabs(mcp.postarget.y() - center.y())
                                   < conf.mcp.tolerance) {
                            mcp.good = true;
                            int nstep{500};
                            for(int jj{-100}; jj < nstep + 1; jj++) {
                               
                                Fill(mcpExssideHistograms.m_ionTrack,
                                     -(mcp.pos[0].x()
                                       + (track.x()) * (jj) / nstep),
                                     (mcp.pos[0].y()
                                      + (track.y()) * (jj) / nstep),
                                     (mcp.pos[0].z()
                                      + (track.z()) * (jj) / nstep));
                                Fill(h.m_XY_target, -mcp.postarget.x(),
                                     mcp.postarget.y());
                            }
                        }

                        // SOURCE POSITION checking reconstruction
                        if(mcpEXSSiDeConf.debug) {
                            TVector3 source(mcpEXSSiDeConf.sourcex,
                                            mcpEXSSiDeConf.sourcey,
                                            mcpEXSSiDeConf.sourcez);

                            dir = -source + mcp.pos.at(0);

                            if(dir.x() != 0 and dir.y() != 0
                               and dir.z() != 0) {
                                double denom = (conf.target.targetNorm.x()
                                                * dir.x() / dir.z())
                                             + (conf.target.targetNorm.y()
                                                * dir.y() / dir.z())
                                             + conf.target.targetNorm.z();
                                mcp.postarget.SetZ(
                                    ((mcp.pos[0].z() / dir.z())
                                         * (conf.target.targetNorm.x() * dir.x()
                                            + conf.target.targetNorm.y()
                                                  * dir.y())
                                     - (mcp.pos[0].x()
                                            * conf.target.targetNorm.x()
                                        + mcp.pos[0].y()
                                              * conf.target.targetNorm.y()))
                                    / denom);
                                double zshift
                                    = (mcp.postarget.z() - mcp.pos[0].z())
                                    / dir.z();
                                mcp.postarget.SetX(zshift * dir.x()
                                                   + mcp.pos[0].x());
                                mcp.postarget.SetY(zshift * dir.y()
                                                   + mcp.pos[0].y());
                            }
                            if(dir.x() == 0
                               and (dir.y() != 0 and dir.z() != 0)) {
                                mcp.postarget.SetX(mcp.pos[0].x());
                                double denom = dir.y()
                                                 * conf.target.targetNorm.y()
                                                 / dir.z()
                                             + conf.target.targetNorm.z();
                                mcp.postarget.SetZ(
                                    (-mcp.pos[0].x()
                                         * conf.target.targetNorm.x()
                                     + (mcp.pos[0].z() * dir.y()
                                        * conf.target.targetNorm.y() / dir.z())
                                     - mcp.pos[0].y()
                                           * conf.target.targetNorm.y())
                                    / denom);
                                double zshift
                                    = (mcp.postarget.z() - mcp.pos[0].z())
                                    / dir.z();
                                mcp.postarget.SetY(zshift * dir.y()
                                                   + mcp.pos[0].y());
                            }
                            if(dir.y() == 0
                               and (dir.x() != 0 and dir.z() != 0)) {
                                mcp.postarget.SetY(mcp.pos[0].y());
                                double denom = dir.x()
                                                 * conf.target.targetNorm.x()
                                                 / dir.z()
                                             + conf.target.targetNorm.z();
                                mcp.postarget.SetZ(
                                    (-mcp.pos[0].y()
                                         * conf.target.targetNorm.y()
                                     + (mcp.pos[0].z() * dir.x()
                                        * conf.target.targetNorm.x() / dir.z())
                                     - mcp.pos[0].x()
                                           * conf.target.targetNorm.x())
                                    / denom);
                                double zshift
                                    = (mcp.postarget.z() - mcp.pos[0].z())
                                    / dir.z();
                                mcp.postarget.SetX(zshift * dir.x()
                                                   + mcp.pos[0].x());
                            }
                            if(dir.z() == 0
                               and (dir.x() != 0 and dir.y() != 0)) {
                                mcp.postarget.SetZ(mcp.pos[0].z());
                                double denom = dir.y()
                                                 * conf.target.targetNorm.y()
                                                 / dir.x()
                                             + conf.target.targetNorm.x();
                                mcp.postarget.SetX(
                                    (-mcp.pos[0].z()
                                         * conf.target.targetNorm.z()
                                     + (mcp.pos[0].x() * dir.y()
                                        * conf.target.targetNorm.y() / dir.x())
                                     - mcp.pos[0].y()
                                           * conf.target.targetNorm.y())
                                    / denom);
                                double zshift
                                    = (mcp.postarget.x() - mcp.pos[0].x())
                                    / dir.x();
                                mcp.postarget.SetY(zshift * dir.y()
                                                   + mcp.pos[0].y());
                            }
                            if(dir.y() == 0 and dir.x() == 0
                               and dir.z() != 0) {
                                mcp.postarget.SetX(mcp.pos[0].x());
                                mcp.postarget.SetY(mcp.pos[0].y());
                                mcp.postarget.SetZ(
                                    (-mcp.pos[0].x()
                                         * conf.target.targetNorm.x()
                                     - mcp.pos[0].y()
                                           * conf.target.targetNorm.y())
                                    / conf.target.targetNorm.z());
                            }
                            if(dir.y() == 0 and dir.z() == 0
                               and dir.x() != 0) {
                                mcp.postarget.SetZ(mcp.pos[0].z());
                                mcp.postarget.SetY(mcp.pos[0].y());
                                mcp.postarget.SetX(
                                    (-mcp.pos[0].y()
                                         * conf.target.targetNorm.y()
                                     - mcp.pos[0].z()
                                           * conf.target.targetNorm.z())
                                    / conf.target.targetNorm.x());
                            }
                            if(dir.z() == 0 and dir.x() == 0
                               and dir.y() != 0) {
                                mcp.postarget.SetZ(mcp.pos[0].z());
                                mcp.postarget.SetX(mcp.pos[0].x());
                                mcp.postarget.SetY(
                                    (-mcp.pos[0].x()
                                         * conf.target.targetNorm.x()
                                     - mcp.pos[0].z()
                                           * conf.target.targetNorm.z())
                                    / conf.target.targetNorm.y());
                            }
                            if(dir.y() == 0 and dir.x() == 0
                               and dir.z() == 0) {
                                mcp.postarget.SetX(mcp.pos[0].x());
                                mcp.postarget.SetY(mcp.pos[0].y());
                                mcp.postarget.SetZ(mcp.pos[0].z());
                            }

                            track = mcp.postarget - mcp.pos[0];
                            if(fabs(mcp.postarget.x() - center.x())
                                   < conf.mcp.tolerance
                               and fabs(mcp.postarget.y() - center.y())
                                       < conf.mcp.tolerance) {
                                int nstep{500};
                                for(int jj{-100}; jj < nstep + 1; jj++) {
                                    // if(mcp.pos[0].z() +
                                    // (track.z())*(jj)/nstep < -6889)
                                    //     if((mcp.pos[0].x() +
                                    //     (track.x())*(jj)/nstep >
                                    //     exsside.Pos.x())){
                                    //         std::cout<<"problem"<<std::endl;
                                    //         std::cout<<jj<<" x
                                    //         "<<-(mcp.pos[0].x() +
                                    //         (track.x())*(jj)/nstep)<<"\ty
                                    //         "<<mcp.pos[0].y() +
                                    //         (track.y())*(jj)/nstep
                                    //         <<"\tz "<<mcp.pos[0].z() +
                                    //         (track.z())*(jj)/nstep<<std::endl;
                                    //         std::cout<<"mcp
                                    //         "<<-mcp.pos[0].x()<<"\tex
                                    //         "<<-exsside.Pos.x()<<std::endl;
                                    //     }
                                    Fill(mcpExssideHistograms.m_ionTrackSource,
                                         -(mcp.pos[0].x()
                                           + (track.x()) * (jj) / nstep),
                                         (mcp.pos[0].y()
                                          + (track.y()) * (jj) / nstep),
                                         (mcp.pos[0].z()
                                          + (track.z()) * (jj) / nstep));
                                }
                            }
                            double zval = mcp.pos.at(0).z() - exsside.Pos.z();
                            double scaleF = -zval / track.z();
                            // std::cout<<"scale "<<scaleF<<std::endl;
                            // std::cout<<"Expected EXSSIDE pos"<<"\tx
                            // "<<(mcp.pos[0].x() + track.x()*scaleF)<<"\ty
                            // "<<(mcp.pos[0].y() + track.y()*scaleF)
                            //     <<"\tz "<<-(mcp.pos[0].z() +
                            //     track.z()*scaleF)<<"\n"<<std::endl;

                            Fill(mcpExssideHistograms.m_expectedXY_gatedMCP,
                                 -(mcp.pos[0].x() + track.x() * scaleF),
                                 (mcp.pos[0].y() + track.y() * scaleF));
                            Fill(mcpExssideHistograms.m_obsX_expectedX,
                                 exsside.Pos.x(),
                                 (mcp.pos[0].x() + track.x() * scaleF));
                            Fill(mcpExssideHistograms.m_obsY_expectedY,
                                 exsside.Pos.y(),
                                 (mcp.pos[0].y() + track.y() * scaleF));
                            Fill(mcpExssideHistograms.m_obsX_expectedY,
                                 exsside.Pos.x(),
                                 (mcp.pos[0].y() + track.y() * scaleF));
                            Fill(mcpExssideHistograms.m_obsY_expectedX,
                                 exsside.Pos.y(),
                                 (mcp.pos[0].x() + track.x() * scaleF));
                        }
                        // SOURCE POSITION checking reconstruction
                    }

                    if(i == 0) {
                        Fill(h.m_layout_XYZ, -mcp.pos.at(i).x(),
                             mcp.pos.at(i).y(), mcp.pos.at(i).z());
                        Fill(h.m_layout_XYZ, -exsside.Pos.x(), exsside.Pos.y(),
                             exsside.Pos.z());
                        if(conf.mcp.nDetectors == 1) {
                            Fill(h.m_ionTrack, -exsside.Pos.x(),
                                 exsside.Pos.y(), exsside.Pos.z());
                            Fill(h.m_ionTrackSource, -exsside.Pos.x(),
                                 exsside.Pos.y(), exsside.Pos.z());
                        }
                    }

                } // mcp.pos
            }
        }
}

bool McpEXSSiDeAnalysis::FillHistos() {
    if(!CoincidenceAnalysis::FillHistos()) return false;

    // Filling histograms
    // McpEXSSiDeHistograms& h = mcpExssideHistograms;

    return true;
}

void McpEXSSiDeAnalysis::Finalize() { CoincidenceAnalysis::Finalize(); }
