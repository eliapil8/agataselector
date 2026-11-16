#ifndef MWDLIB_H_INCLUDED
#define MWDLIB_H_INCLUDED

// Encapsulates some of the moving window (MW) processing components
// Contains quite a lot no-more (or not-yet) used stuff
// Should be simplified
// Should write the Set&Get methods and make the internal variables private
// Should be completed (e.g. adding deconvolution of multiple poles)

// #define MWD_REGIONS     // uncomment to have processing of AGATA-like regions

class MWD {
  private:
    unsigned int nNsamp; // lenght of wave
    float*       fSpace; // 3*[nNsamp] internal working buffers
    bool         uSpace; // if not provided by user.
    bool         uDiscr; // uDiscr ? CFD : LET

  public:
    unsigned int
        nDcwin; // number of channels used to calculate initial baseline

    unsigned int nSmooth; // timing integration
    unsigned int nWidthT; // timing differentiation

    unsigned int nDelayCFD; // parameters of CFD
    float        fFractCDF;
    float        fThreshTFA;
    float        fThreshCFD;
    unsigned int nMinWidthP;
    unsigned int nMinWidthN;

#ifdef MWD_REGIONS
    float        fTrigAmin;  // Threshold on CC    --> before applying mwdcal
    float        fTrigEwin1; // Energy window on CC: from
    float        fTrigEwin2; // Energy window on CC: to
    float        fEminSegs;  // Threshold energy on segment energy spectra

    float        fOffset; // Offset applied after MWD | applied only to energy
                          // calculations in
    float        fGain; // Gain   applied after MWD | MWvalues (not for trigger
                        // thresholds)
    float        fDecay; // Decay time (number of samples) for CC

    unsigned int nT1, nT2;   // timing signal region
    unsigned int nZ1, nZ2;   // background   ""
    unsigned int nE1, nE2;   // energy       ""
    unsigned int nEm;        // middle of energy region to check flat top
    float        fT;         // position of trigger
    float        fZ;         // level of MWD signal in [nZ1--nZ2]
    float        fZ1, fZ2;   // level of MWD signal in first and seconf half
    float        fE;         // level of MWD signal in [nE1--nE2]
    float        fE1, fE2;   // first and second part of flat top
    float        fBz;        // level of background
    float        fBz1, fBz2; // level of background in first and second half
    float        fBgLim;     // max difference between dBz1 and dBz2
    float        fBFWHM;     // fwhm of background region
    unsigned int nTt;        // time of transient maximum
#endif

    MWD(unsigned int nn = 100,
        bool         userspace
        = false); // defaults to internal space (not thread safe)
    virtual ~MWD();

    void  Reset(unsigned int nn, bool userspace);

    float ValStart(const float*) const; // average of first MWD.nDcwin samples
    float ValEnd(const float*) const;   // average of last  MWD.nDcwin samples
    void  MwAverage(const float*, float*, unsigned int); // moving average
    void  MwDeconv(const float*, float*, float, unsigned int); // Gast's MWD
    void  MwJDSU(const float*, float*, unsigned int);          // Jordanov DSU
    void  MwCFShaper(const float*, float*);                    // CFD shaping
    int   TrigC(const float*, float,
                unsigned int); // find trigger_position or -trigger_count
    int   TrigCp(const float*, float,
                 unsigned int); // find position of first n2p trigger
    int
    TrigCn(const float*, float, unsigned int); // find position of first p2n ""
    float TrigZC(const float*, const float*, float thrCF,
                 float thrTF); // CFD trigger (standard p2n)
    float TrigZCp(const float*, const float*, float thrCF,
                  float thrTF); // CFD trigger (p2n less requests to tfa)
    float TrigZCn(const float*, const float*, float thrCF,
                  float thrTF); // CFD trigger (n2p less requests to tfa)
#ifdef MWD_REGIONS
    int TrigEne(
        const float*, float,
        unsigned int); // find trigger and set above-threshold markers nT[12]
    void  Regions();   // set working regions
    float Values(const float*); // extract energies from working regions
    float
    TransAmpl(const float*, unsigned int); // determine amplitude of transients
#endif
    void  AdjustTrace(float*, float); // remove offset and scale amplitude
    void  ShiftWave(float* pW, int);  // shift wave left or right

    void  TrigMode(bool mode) { uDiscr = mode; }
    bool  TrigMode() { return uDiscr; }

    // once debugged allow wspace to default to 0
    float TriggerCFD(const float* ww, float thr, float* wspace /* = 0x0*/,
                     float& otrig); // CFD with internal shaping
    float TriggerLED(const float* ww, float thr, float* wspace /* = 0x0*/,
                     float& otrig); // LED with internal shaping //FR

    float TriangAmpli(
        const float* ww, int shape,
        float* wspace /* = 0x0*/, bool derSecondOrder=false); // determine amplitude after triangular
                                   // shaping (no deconvolution)
    void SetNSamples(unsigned int ns);
 
    unsigned int GetNSamples(){ return nNsamp; }
};

#endif // MWDLIB_H_INCLUDED
