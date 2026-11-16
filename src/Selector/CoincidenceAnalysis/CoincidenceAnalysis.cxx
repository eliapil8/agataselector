#include "CoincidenceAnalysis.h"

CoincidenceAnalysis::CoincidenceAnalysis(DetectorAnalysis::Inputs& in,
                                         Container& d1, DetectorConf& c1,
                                         Container& d2, DetectorConf& c2,
                                         CoincidenceConf& cc, Readers& r1,
                                         Readers&               r2,
                                         CoincidenceHistograms& ch)
    : DetectorAnalysis(in, d1, c1, ch), data2(d2), detectorConf2(c2),
      coincidenceConf(cc), coincidenceHistograms(ch), readers1(r1),
      readers2(r2) {
    SetupTimeWindow<CoincidenceConf>(coincidenceConf);
}

bool CoincidenceAnalysis::AllocateHistos() {
    // if(!DetectorAnalysis::AllocateHistos()) return false;
    DetectorAnalysis::AllocateHistos();
    if(!coincidenceConf.enable_histos) return false;
    CoincidenceAnalysis::CoincidenceHistograms& h = coincidenceHistograms;

    {
        int   nbins{(coincidenceConf.windowRight - coincidenceConf.windowLeft + 600)* 10};
        int   xmin{coincidenceConf.windowLeft - 300};
        int   xmax{coincidenceConf.windowRight + 300};
        float binning
            = static_cast<float>(xmax - xmin) / static_cast<float>(nbins);
        Initialize<TH1D>(
            h.h_TSmTS,
            new TH1D(
                "h_TSmTS",
                Form(" Time difference of the two detectors;Time difference "
                     "[10 ns];Counts/%f ns",
                     binning),  
                nbins, xmin, xmax),
            h.ptrs);
        Initialize<TH2D>(h.m_TSmTS_TS,
                         new TH2D("m_TSmTS_TS",
                                  Form(" Time difference of the two detectors "
                                       "vs Time;Time [s];Time difference"
                                       "[10 ns];Counts"),
                                  12 * 360, 0, 48 * 3600, 500, xmin, xmax),
                         h.ptrs);
        Initialize<TH1D>(
            h.h_TSmTSgated,
            new TH1D("h_TSmTSgated",
                     Form("Prisma TS - Gamma TS with gate condition;Time "
                          "difference [10 ns];Counts/%f ns",
                          binning),
                     nbins, xmin, xmax),
            h.ptrs);
    }
    return true;
}

void CoincidenceAnalysis::Analyze() {
    DetectorAnalysis::Analyze();
    CoincidenceAnalysis::CoincidenceHistograms& h = coincidenceHistograms;

    coincMatrix.clear();
    coincMatrix.resize(data2.Time.size(),std::vector<bool>(data.Time.size(),false));
    //    std::cout<<"data time sizes "<<data2.Time.size()<< " " <<data.Time.size()<<std::endl;
    //std::ofstream f("out.txt",std::ios_base::app);
//
    //for(auto v:coincMatrix){
    //    for(auto b:v){
    //        f << b ;
    //    }
    //    f << std::endl;
    //}
    //f << std::endl;
    //f << "----------------" <<  std::endl;

    //in_coinc                                      = false;

    if(data.TS != 0 && data2.TS != 0) {
        if(settings.verbosity_level > 1)
            std::cout << "Processing coincidence\n";


        bool full_coinc = false;
        // Fill time diff histograms
        for(size_t d1{0}; d1 < data.Time.size(); ++d1) {
            auto d1time{data.Time.at(d1)};
            for(size_t d2{0}; d2 < data2.Time.size(); ++d2) {
                auto d2time{data2.Time.at(d2)};

                auto diff = unsignedDiff<double, unsigned long long>(data2.TS,
                                                                     data.TS)
                          + (d2time - d1time) / (10. * UNITS::ns);
                //std::cout<<"d1 TS "<<data.TS<<" d1 time "<<d1time<<std::endl;
                //std::cout<<"d2 TS "<<data2.TS<<" d2 time "<<d2time<<std::endl;
                //std::cout<<"diff "<<diff<<std::endl;
                Fill(h.h_TSmTS, diff);
                if(h.m_TSmTS_TS != nullptr) {
                    Fill(h.m_TSmTS_TS,
                         unsignedDiff<double, unsigned long long>(
                             data.TS, input.firstTS)
                             / 1.e8,
                         diff, 1. / h.m_TSmTS_TS->GetXaxis()->GetBinWidth(1));
                }
		//std::cout<<"wl "<<windowLeft<< " wr "<<windowRight<<std::endl;
                if(diff > windowLeft && diff < windowRight) {
                  // std::cout<<"Coincidence found d1 "<<d1<<" d2 "<<d2<<" diff "<<diff<<std::endl;  
                    if(coincidenceConf.event_by_event){
                        coincMatrix.at(d2).at(d1) = true;
			//	std::cout<<"in Coinc "<<coincMatrix.at(d2).at(d1)<<" "<<d1<<std::endl;
			            in_coinc = true;
                        Fill(h.h_TSmTSgated, diff);
                    }
                    else{
                        full_coinc = true;
                    }
                }
            }
        }
        if(!coincidenceConf.event_by_event&&full_coinc){
            for(size_t d1{0}; d1 < data.Time.size(); ++d1) {
                for(size_t d2{0}; d2 < data2.Time.size(); ++d2) {
                    coincMatrix.at(d2).at(d1) = true;
                    in_coinc = true;

                    auto d1time{data.Time.at(d1)};
                    auto d2time{data2.Time.at(d2)};
                    auto diff = unsignedDiff<double, unsigned long long>(data2.TS, data.TS) + (d2time - d1time) / (10. * UNITS::ns);
                    Fill(h.h_TSmTSgated, diff);
                
                }
            }
        }
    }
}
bool CoincidenceAnalysis::FillHistos() {
    // if(!DetectorAnalysis::FillHistos()) return false;
    //if(!in_coinc) return true;
    DetectorAnalysis::FillHistos();
    if(!coincidenceConf.enable_histos) return false;

    // CoincidenceAnalysis::CoincidenceHistograms& h = coincidenceHistograms;

    return true;
}

void CoincidenceAnalysis::Finalize() {
    DetectorAnalysis::Finalize();
    NormalizeHistos<CoincidenceConf>(coincidenceConf);
}

void CoincidenceAnalysis::Clear() {}
