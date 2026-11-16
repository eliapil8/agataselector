#include "mwdlib.h"
#include <cstring>
#include <cmath>
#include <algorithm>

//////////////////////////////////////////////
// funzioni per  analisi delle forme d'onda //
//////////////////////////////////////////////

MWD::MWD(unsigned int nn, bool userspace)
{
  fSpace = nullptr;
  Reset(nn, userspace);
}

MWD::~MWD()
{
  if (!uSpace && fSpace) {
    delete[] fSpace;
  }
}

void MWD::Reset(unsigned int nn, bool userspace)
{
  if (fSpace)
    delete[] fSpace;
  fSpace = nullptr;

  // initialize variables to reasonable (??) values
  nDcwin   = 0;             // number of channels used to calculate initial baseline

  nSmooth  = 0;             // timing integration
  nWidthT  = 5;             // timing differentiation

  nDelayCFD  = 10;          // parameters of CFD
  fFractCDF  = 0.25f;
  fThreshTFA = 20.f;
  fThreshCFD = 0;
  nMinWidthP = 3;
  nMinWidthN = 5;

#ifdef MWD_REGIONS
  fTrigAmin  = 0;           // Threshold on CC    --> before applying mwdcal
  fTrigEwin1 = 10.f;        // Energy window on CC: from
  fTrigEwin2 = 1000000.f;   // Energy window on CC: to
  fEminSegs  = 10.f;        // Threshold energy on segment energy spectra

  fOffset = 0;              // Offset applied after MWD | applied only to energy calculations in
  fGain   = 1.f;            // Gain   applied after MWD | MWvalues (not for trigger thresholds)
  fDecay  = 4600.f;         // Decay time (number of samples) for CC

  nT1 = nT2 = 0;            // timing signal region
  nZ1 = nZ2 = 0;            // background   ""
  nE1 = nE2 = 0;            // energy       ""
  nEm = 0;                  // middle of energy region to check flat top
  fT  = 0;                  // position of trigger
  fZ  = 0;                  // level of MWD signal in [nZ1--nZ2]
  fZ1 = fZ2 = 0;            // level of MWD signal in first and seconf half
  fE  = 0;                  // level of MWD signal in [nE1--nE2]
  fE1 = fE2 = 0;            // first and second part of flat top
  fBz   = 0;                // level of background
  fBz1  = fBz2 = 0;         // level of background in first and second half
  fBgLim  = 0;              // max difference between dBz1 and dBz2
  fBFWHM  = 0;              // fwhm of background region
  nTt     = 0;              // time of transient maximum
#endif

  nNsamp  = nn;

  if (userspace) {
    fSpace = 0x0;   // space will be provided by the caller
    uSpace = true;
  }
  else {
    fSpace = new float[3 * nNsamp]; memset(fSpace, 0, 3 * sizeof(float)*nNsamp);
    uSpace = false;
  }
}

// calcola la linea di base iniziale su nDcwin campioni
float MWD::ValStart(const float *ps) const
{
  float vstart = 0.;
  if(nDcwin > 0) {
    for (unsigned int ii = 0; ii < nDcwin; ii++) {
      vstart += ps[ii];
    }
    vstart /= (float)(nDcwin);
  }
  return vstart;
}

// calcola l'ampiezza finale su nDcwin campioni
float MWD::ValEnd(const float *ps) const
{
  float  vstart = 0;
  if(nDcwin > 0) {
    const float *ps1 = ps+nNsamp-nDcwin;
    for (unsigned int ii = 0; ii < nDcwin; ii++) {
      vstart += *ps1++;
    }
    vstart /= (float)(nDcwin);
  }
  else {
    vstart = ps[nNsamp-1];
  }
  return vstart;
}

// moving average
void MWD::MwAverage(const float *ps, float *pd, unsigned int wch)
{
  if(wch <= 1) {
    memcpy(pd, ps, sizeof(float)*nNsamp);
    return;
  }

  float valstart = ValStart(ps);

  // per cominciare assume che tutti i canali precedenti valgano valstart
  float xwch = 1.f/wch;
  float valsum = valstart*xwch;
  float *pdd = pd;
  const float *ps1 = ps;
  const float *ps2 = ps;
  for(unsigned int ii = 0; ii < wch; ii++) {
    valsum += *ps2++ - valstart;
    *pdd++  = valsum*xwch;
  }
  // per il resto dati in somma a destra e in sottrazione a sinistra della MW
  for(unsigned int ii = wch; ii < nNsamp; ii++){
    valsum += *ps2++ - *ps1++;
    *pdd++  = valsum*xwch;
  }
}

// Moving Window Deconvolution a' la GAST
// IEEE Trans.Nucl.Sci. 40(1993)770
// ps     input
// pd     output
// tau    decay time (in channels)
// wch    width of moving window

void MWD::MwDeconv(const float *ps, float *pd, float tau, unsigned int wch)
{
  float  factor = 1.f - (float)exp(-1./tau);

  float valstart = ValStart(ps);

  // per cominciare assume che tutti i canali precedenti valgano valstart
  // e che non ci sia carica accumulata dentro alla MW (valstart e' un valore costante);
  // per il resto dati in somma a destra e in sottrazione a sinistra della MW
  float *pdd = pd;
  const float *ps1 = ps;
  const float *ps2 = ps;
  float valsum = 0.;
  float valnew = 0.;
  for(unsigned int ii = 0; ii < wch; ii++) {
    valnew  = *ps2++  - valstart;
    *pdd++  =  valnew + valsum * factor;
    valsum +=  valnew;
  }
  for(unsigned int ii = wch; ii < nNsamp; ii++) {
    valnew  = *ps2++  - *ps1++;
    *pdd++  =  valnew + valsum * factor;
    valsum +=  valnew;
  }
}

// Jordanov's Delay&Subtract Unit
// NIMA 353(1994)261
// ps     input
// pd     output
// kk     delay

void MWD::MwJDSU(const float *ps, float *pd, unsigned int kk)
{
  if (kk < 1) {
    memcpy(pd, ps, sizeof(float)*nNsamp);
    return;
  }

  float valstart = ValStart(ps);

  // tutti i valori ps[nn-kk < 0] valgono valstart
  float *pdd = pd;
  const float *ps1 = ps;
  const float *ps2 = ps;
  for (unsigned int nn = 0; nn < kk; nn++) {
    *pdd++ = *ps2++ - valstart;
  }
  for (unsigned int nn = kk; nn < nNsamp; nn++) {
    *pdd++ = *ps2++ - *ps1++;
  }
}

// A CFD (i.e. pd[n] = f*ps[n]-ps[n-d]) moving window shaper
// ps     input
// pd     output

void MWD::MwCFShaper(const float *ps, float *pd)
{
  if(nDelayCFD < 1) {               // ??
    memcpy(pd, ps, sizeof(float)*nNsamp);
    return;
  }

  float valstart = ValStart(ps);

  // assume che tutti i canali precedenti valgano valstart
  float *pdd = pd;
  const float *ps1 = ps;
  const float *ps2 = ps;
  for(unsigned int ii = 0; ii < nDelayCFD; ii++) {
    *pdd++  = (*ps2++)*fFractCDF - valstart;
  }
  for(unsigned int ii = nDelayCFD; ii < nNsamp; ii++) {
    *pdd++  = (*ps2++)*fFractCDF - *ps1++;
  }
}

// A simple state machine to implement a Leading Edge Discriminator
// Position of the triggers is on the transition to Above-threshold
// If there is more than one trigger the function returns -(number of triggers)
int MWD::TrigC(const float *pd, float level, unsigned int width)
{
  const float *lpd = pd;
  float flevel = (float)level;
  unsigned int chan = 0, ptrig = 0;
  unsigned int chanL, nset;
  int ntrig = 0;

Below:                                // Below-threshold state
  for(; chan < nNsamp; chan++) {      // loop through the data
    if(*lpd++ > flevel) {             // transition to Above-threshold
      chanL = chan++;                 // record value and increment
      nset  = 1;                      // set width
      goto Above;                     // go to Above-threshold state
    }
  }
  if(ntrig == 1)                      // finish in Below-threshold (i.e. normal) state
    return ptrig;
  else
    return -ntrig;

Above:                                // Above-threshold state
  for(; chan < nNsamp; chan++) {      // loop through the data
    if(*lpd++ > flevel) {             // still above
      nset++;                         // increment width
    }
    else {                            // transition to Below-threshold
      if(nset > width) {              // if width ok
        ntrig++;
        ptrig = chanL;                // store trigger value
      }
      chan++;                         // increment due to break of loop
      goto Below;                     // go to Below-threshold state
    }
  }
  if(nset > width) {                  // finished above threshold
    ntrig++;
    ptrig = chanL;                    // store trigger value
  }
  if(ntrig == 1)                      // finish above threshold
    return ptrig;
  else
    return -ntrig;

}

// A simple state machine to implement a Leading Edge Discriminator
// Returns immediately after finding the first trigger
// Position of the (first) trigger is on the transition to Above-threshold
int MWD::TrigCp(const float *pd, float level, unsigned int width)
{
  const float *lpd = pd;
  float flevel = (float)level;
  unsigned int chan = 0;
  unsigned int chanL, nset;

Below:                                // Below-threshold state
  for(; chan < nNsamp; chan++) {      // loop through the data
    float dd = *lpd++;
    if(dd > flevel) {                 // transition to Above-threshold
      chanL = chan++;                 // record value and increment
      nset  = 1;                      // set width
      goto Above;                     // go to Above-threshold state
    }
  }
  return 0;

Above:                                // Above-threshold state
  for(; chan < nNsamp; chan++) {      // loop through the data
    float dd = *lpd++;
    if(dd > flevel) {                 // still above
      nset++;                         // increment width
    }
    else {                            // transition to Below-threshold
      if(nset > width) {              // if width ok
        return chanL;                 // store trigger value
      }
      chan++;                         // increment due to break of loop
      goto Below;                     // go to Below-threshold state
    }
  }
  if(nset > width) return chanL;      // if width ok store trigger value //FR
  return 0;

}

// A simple state machine to implement a Leading Edge Discriminator
// Returns immediately after finding the first trigger
// Position of the trigger is on the transition to Above-threshold
// Modified for negative signals by changing sign of input (level must still be positive)
int MWD::TrigCn(const float *pd, float level, unsigned int width)
{
  const float *lpd = pd;
  float flevel = (float)level;
  unsigned int chan = 0/*, ptrig = 0*/;
  unsigned int chanL, nset;

Below:                                // Below-threshold state
  for(; chan < nNsamp; chan++) {      // loop through the data
    float dd = *lpd++;
    if(-dd > flevel) {                // transition to Above-threshold
      chanL = chan++;                 // record value and increment
      nset  = 1;                      // set width
      goto Above;                     // go to Above-threshold state
    }
  }
  return 0;

Above:                                // Above-threshold state
  for(; chan < nNsamp; chan++) {      // loop through the data
    float dd = *lpd++;
    if(-dd > flevel) {                // still above
      nset++;                         // increment width
    }
    else {                            // transition to Below-threshold
      if(nset > width) {              // if width ok
        return chanL;                 // store trigger value
      }
      chan++;                         // increment due to break of loop
      goto Below;                     // go to Below-threshold state
    }
  }
  return 0;
}

// A simple state machine to implement a ZC discriminator on a cfd shaped wave
// Position of the trigger is on the transition from the positive to the negative lobe
// Function returns after the first valid trigger
// Looks for POSITIVE_TO_NEGATIVE crossover

float MWD::TrigZC(const float *tfa, const float *cfa, float thrCF, float thrTF)
{
  int state = 0;                      // begin in an indefinite state
  unsigned int widthP = 0, widthN = 0;
  float cfaOld(0), cfaInp(0), tfaInp(0), fract(0);

  for(unsigned int nn = 0; nn < nNsamp; nn++) {
    cfaInp = cfa[nn];
    tfaInp = tfa[nn];
    switch (state) {
    case 0:                           // was below CF threshold in an indefinite state
      if (cfaInp > thrCF) {           // and goes above
        state  = 1;                   // initiate a possible trigger sequence
        widthP = 1;                   // start checking width
      }                               // if it stays below there is nothing to do
      break;
    case 1:                           // was above threshold
      if (cfaInp >= thrCF) {          // and stays above
        widthP++;                     // increment positive width and stay in this state
      }
      else {                          // transition to negative ==> possible ZC
        if(widthP >= nMinWidthP &&    // positive lobe wide enough and
          tfaInp  >  thrTF) {         // tfa signal above threshold?
            state  = 2;              // initiate a negative lobe sequence
            widthN = 1;              // start counting width of negative lobe
            fract  = (cfaOld - thrCF) / (cfaOld - cfaInp);  // interpolate zero crossing
        }
        else
          state = 0;                  // go back to indefinite
      }
      break;
    case 2:                           // below threshold in a pos-triggered state
      if (cfaInp < thrCF) {           // and is still below
        widthN++;                     // increase signal width
        if(widthN == nMinWidthN &&    // negative wide enough (==> triggers only once)
          tfaInp  >  thrTF) {         // and tfa signal still above threshold?
          return (nn-widthN) + fract; // this is a good new trigger
        }
      }
      else {                          // goes positive (with or without trigger)
        state  = 1;                   // start a possible new trigger sequence
        widthP = 1;                   // start checking width
      }
      break;
    }
    cfaOld = cfaInp;
  }
  return 0;
}

// A simple state machine to implement a ZC discriminator on a cfd shaped wave
// At variance of the previous function, the tfa is not requested at nMinWidthN
// Function returns after the first valid trigger
// Looks for POSITIVE_TO_NEGATIVE crossover

float MWD::TrigZCp(const float *cfa, const float *tfa, float thrCF, float thrTF)
{
  int state = 0;                    // begin in an indefinite state
  unsigned int widthP = 0, widthN = 0;
  float cfaOld(0), cfaInp(0), tfaInp(0), fract(0);

  for(unsigned int nn = 0; nn < nNsamp; nn++) {
    cfaInp = cfa[nn];
    tfaInp = tfa[nn];
    switch (state) {
    case 0:                           // was below CF threshold in an indefinite state
      if (cfaInp > thrCF) {           // and goes above
        state  = 1;                   // initiate a possible trigger sequence
        widthP = 1;                   // start checking width
      }                               // if it stays below there is nothing to do
      break;
    case 1:                           // was above threshold
      if (cfaInp >= thrCF) {          // and stays above
        widthP++;                     // increment positive width and stay in this state
      }
      else {                          // transition to negative ==> possible ZC
        if(widthP >= nMinWidthP &&    // positive lobe wide enough and
          tfaInp  >  thrTF) {         // tfa signal above threshold?
            state  = 2;                // initiate a negative lobe sequence
            widthN = 1;               // start counting width of negative lobe
            fract  = (cfaOld - thrCF) / (cfaOld - cfaInp);  // interpolate zero crossing
        }
        else
          state = 0;                  // go back to indefinite
      }
      break;
    case 2:                           // below threshold in a pos-triggered state
      if (cfaInp < thrCF) {       // and is still below
        widthN++;                     // increase signal width
        if(widthN == nMinWidthN) {    // negative wide enough (== triggers only once)
          return (nn-widthN) + fract; // this is a good new trigger
        }
      }
      else {                          // goes positive (with or without trigger)
        state  = 1;                   // start a possible new trigger sequence
        widthP = 1;                   // start checking width
      }
      break;
    }
    cfaOld = cfaInp;
  }
  return 0;
}

// A simple state machine to implement a ZC discriminator on a cfd shaped wave
// At variance of the previous function the value of tfa is not checked at nMinWidthN
// Function returns after the first valid trigger
// Looks for NEGATIVE_TO_POSITIVE crossover

float MWD::TrigZCn(const float *cfa, const float *tfa, float thrCF, float thrTF)
{
  int state = 0;                      // begin in an indefinite state
  unsigned int widthP = 0, widthN = 0;
  float cfaOld(0), cfaInp(0), tfaInp(0), fract(0);

  for(unsigned int nn = 0; nn < nNsamp; nn++) {
    cfaInp = cfa[nn];
    tfaInp = tfa[nn];
    switch (state) {
    case 0:                           // was below CF threshold in an indefinite state
      if (cfaInp <= thrCF) {          // and goes above
        state  = 1;                   // initiate a possible trigger sequence
        widthP = 1;                   // start checking width
      }                               // if it stays below there is nothing to do
      break;
    case 1:                           // was above threshold
      if (cfaInp < thrCF) {           // and stays above
        widthP++;                     // increment positive width and stay in this state
      }
      else {                          // transition to negative ==> possible ZC
        if(widthP >= nMinWidthP &&    // positive lobe wide enough and
          tfaInp <= -thrTF) {         // tfa signal above threshold?
            state  = 2;               // initiate a negative lobe sequence
            widthN = 1;               // start counting width of negative lobe
            fract  = (cfaOld - thrCF) / (cfaOld - cfaInp);  // interpolate zero crossing
        }
        else
          state = 0;                  // go back to indefinite
      }
      break;
    case 2:                           // below threshold in a pos-triggered state
      if (cfaInp >= thrCF) {          // and is still above
        widthN++;                     // increase signal width
        if(widthN == nMinWidthN) {    // negative wide enough (== triggers only once)
          return (nn-widthN) + fract; // this is a good new trigger
        }
      }
      else {                          // goes positive (with or without trigger)
        state  = 1;                   // start a possible new trigger sequence
        widthP = 1;                   // start checking width
      }
      break;
    }
    cfaOld = cfaInp;
  }
  return 0;
}

#ifdef MWD_REGIONS
// Leading Edge discriminator to find first trigger
// Position is transition to Above-threshold
// Signal must stay above threshold for wmin samples
// Function return 0(failure) or 1(success) and sets
// nT1 and nT2 as the Above-threshold region

int MWD::TrigEne(const float *pd, float level, unsigned int wmin)
{
  int Cch1 = 0;                 // kept as variable in case one wants to start later
  int Cch2 = nNsamp;
  const float *pdat = pd + Cch1;
  int ch = Cch1;

Below:  // look for the beginning of a signal
  for(; ch < Cch2; ch++) {
    if(*pdat++ > level) {       // goes above threshold
      nT1 = ch++;
      goto Above;
    }
  }
  return 0;                     // no (sufficiently wide) region found

Above:  // look for the end of the signal
  for(; ch < Cch2; ch++) {
    if(*pdat++ < level) {       // returns below threshold
      nT2 = ch++;
      if(nT2-nT1 >= wmin)
        return 1;
      goto Below;
    }
  }
  nT2 = Cch2-1;                 // signal is still Above-threshold at the end of wave

  return (nT2-nT1 >= wmin) ? 1 : 0;
}

// Sets, in MWD, the regions for
// processing the segments (background and signal)

void MWD::Regions()
{
  nZ1 = 0;
  nZ2 = nT1 - 1*nWidthT;            // questi fattori per la protezione all'inizio
  nZ2 = nZ2 > nZ1+1 ? nZ2 : nZ1+1;  // protezione della zona di fondo

  nE1 = nT1 + 3*nWidthT;
  //nE2 = nT2 - 2*nWidthT;            // inizio e fine fine segnale andrebbero parametrizzati
  nE2 = nT2 - 1;                    // AGATA test Colonia 8-9/2005 (e in generale) con onde troppo corte
  nE2 = nE2 > nE1+2 ? nE2 : nE1+2;
  nEm = (nE1 + nE2)/2;
}

// extract flat top average and the other relevant
// quantities from the (energy) deconvolved waves

float MWD::Values(const float *pdat)
{
  const double SIG2FW = sqrt(8.*log(2.));

  const float *pd = pdat + nZ1;
  fZ = fZ1 = fZ2 = 0.;
  unsigned int izm = (nZ1 + nZ2) / 2;
  for (unsigned int ii = nZ1; ii < izm; ii++) {
    fZ1 += *pd++;
  }
  for (unsigned int ii = izm; ii <= nZ2; ii++) {
    fZ2 += *pd++;
  }

  fZ = (fZ1 + fZ2) / (nZ2 - nZ1 + 1);
  fZ1 /= izm - nZ1;
  fZ2 /= (nZ2 + 1) - izm;

  pd = pdat + nZ1;
  float bval = fZ;
  float tval = 0;
  for (unsigned int ii = nZ1; ii <= nZ2; ii++) {
    float val = *pd++ - bval;
    tval += val*val;
  }
  fBFWHM = float(SIG2FW*sqrt(tval / (nZ2 - nZ1 + 1)));

  fE1 = 0.;
  pd = pdat + nE1;
  for (unsigned int ii = nE1; ii < nEm; ii++) {
    fE1 += *pd++;
  }

  fE2 = 0.;
  pd = pdat+nEm;
  for (unsigned int ii = nEm; ii <= nE2; ii++) {
    fE2 += *pd++;
  }

  fE  =  fE1 + fE2;
  fE /=  nE2 - nE1 + 1;

  fE1 /= nEm - nE1;
  fE2 /= nE2 - nEm + 1;

  fE1 = fOffset + (fE1 - fZ) * fGain;
  fE2 = fOffset + (fE2 - fZ) * fGain;

  fBz    = fOffset + fGain * fZ;
  fBz1   = fOffset + fGain * fZ1;
  fBz2   = fOffset + fGain * fZ2;
  fBFWHM =           fGain * fBFWHM;

  return fOffset + (fE - fZ) * fGain;
}

// determine amplitude of transients
// this assumes that the baseline is at ~zero
float MWD::TransAmpl(const float *pdat, unsigned int ch1)
{
  float vpos = pdat[ch1];
  float vneg = pdat[ch1];
  unsigned int tpos = ch1;
  unsigned int tneg = ch1;

  for(unsigned int ch = ch1+1; ch < nNsamp; ch++) {
    float vv = pdat[ch];
    if(vv > vpos) {
      vpos = vv;
      tpos = ch;
    }
    else if(vv < vneg) {
      vneg = vv;
      tneg = ch;
    }
  }
  if(-vneg > vpos) {
    nTt = tneg;
    return vneg;
  }
  else {
    nTt = tpos;
    return vpos;
  }
}
#endif

// timing filter amplifier followed by a CFD
// the intermediate traces can be retrieved by MWgetShortTFA and MWgetShortCFD

// Per il caso dell'analisi di tracce corte di AGATA, che sono state gia' catturate da un loro
// trigger di tipo LE, l'analisi si puo' semplificare parecchio perche' sappiamo che, se il canale
// ha segnale (cosa che sappiamo dalla misura dell'energia) il punto di trigger esiste, e' quasi
// sempre unico ed e' in una zona che sta nella prima meta' della traccia.
// Dunque:
// 1) non e' necessario differenziare
// 2) lo shaping CFD (1 0 0 .. 0 -g) produce un segnale negativo alla fine della traccia
// 3) si puo' cercare il punto di ZC (da - a +) partendo da sinistra
// 4) lo si puo' interpolare con una parabola (?)
// 5) una volta trovato si potrebbe estrapolare a zero direttamente dal segnale originale (??) (Kalman??)

//#define INTERPOLCFD

float MWD::TriggerCFD(const float * fww, float thr, float *wspace, float &otrig)
{
  if (uSpace) {
    if (!wspace)
      return 0;
    fSpace = wspace;
  }

  float *fw0 = fSpace;
  float *fw1 = fw0 + nNsamp;
  float *fw2 = fw1 + nNsamp;

  // TFA shaping
  MwAverage(fww, fw0, nSmooth);   // integration
  MwJDSU   (fw0, fw1, nWidthT);   // differentiation
 
#ifdef INTERPOLCFD
  // ????
  nDelayCFD += 1;
  MWcfa(fw1, fw2);          // shape CC for cfd trigger
  float fract = 0;
  int tsamp  = MWtrigZC(fw1, fw2, &fract);
  float trig2 = tsamp + fract;
  nDelayCFD -= 1;
#endif

  // CFD shaping
  MwCFShaper(fw1, fw2);  // shape CC for cfd trigger: frac 0.25, delay 6
  float trig1 = TrigZC(fw1, fw2, fThreshCFD, thr);

  otrig = trig1;
  float corr = 0;   // correction to crossover if the signal is a constant slope

#ifdef INTERPOLCFD
  const float tZCmin = 10.f;
  if(trig1>tZCmin) {
    if(trig2>trig1)
      corr = trig1*(nDelayCFD+1)-trig2*nDelayCFD;   // interpolated
    else
      corr = trig1 - nDelayCFD/(1.f-fFractCDF);     // assumed to be a stright line
  }
  else if(trig2>tZCmin) {
     corr = trig2 - (nDelayCFD+1)/(1.f-fFractCDF);  // assumed to be a stright line
  }
#else
  corr = trig1 - nDelayCFD/(1.f-fFractCDF);         // assumed to be a stright line
#endif

  if(nSmooth)               // roughly remove dependency on integration
    corr -= 0.5f*nSmooth;

  if (corr < 1.f)
    corr = 0;
  return corr;
}


// timing filter amplifier followed by a LE
float MWD::TriggerLED(const float *fww, float thr, float *wspace, float& otrig)
{
  if (uSpace) {
    if (!wspace)
      return 0;
    fSpace = wspace;
  }

  float *fw0 = fSpace;
  float *fw1 = fw0 + nNsamp;
  float *fw2 = fw1 + nNsamp;

  // TFA shaping
  MwAverage(fww, fw0, nSmooth);     // integration
  MwJDSU   (fw0, fw1, nWidthT);     // differentiation

  int tsamp = TrigCp(fw1, thr, nMinWidthP);

  otrig = (float)tsamp;
  float corr = float(tsamp) - nSmooth/2;  // correction to crossover if signal is a constant slope (to be improved)
  if (corr < 1.f)
    corr = 0;

  return corr;
}

// determine amplitude after triangular shaping (no deconvolution)
float MWD::TriangAmpli(const float *trace, int shape, float *wspace)
{
  if (uSpace) {
    if (!wspace)
      return 0;
    fSpace = wspace;
  }

  float *fw0 = fSpace;
  float *fw1 = fw0 + nNsamp;
  float *fw2 = fw1 + nNsamp;

  if (shape > 0) {
    // triangular filter
    MwJDSU(trace, fw0, shape);
    MwAverage(fw0, fw1, shape);
  }
  else if (shape == 0) {
    // derivative
    fw1[0] = 0;
    for (unsigned int nn = 1; nn < nNsamp; nn++) {
      fw1[nn] = trace[nn] - trace[nn - 1];
    }
  }
  else {
    // derivative + smoothing
    fw0[0] = 0;
    for (unsigned int nn = 1; nn < nNsamp; nn++) {
      fw0[nn] = trace[nn] - trace[nn - 1];
    }
    MwAverage(fw0, fw1, abs(shape));
  }
 
  float base = ValStart(fw1);
  float mval = 0;
  for (unsigned int nn = nDcwin; nn < nNsamp; nn++)
    mval = std::max(mval, fw1[nn]);
  return mval - base;
}

// remove baseline and adjust vertical gain and scale
void MWD::AdjustTrace(float *fTrace, float fgain)
{
  float yOff = ValStart(fTrace);

  for(unsigned int ii = 0; ii < nNsamp; ii++)
    fTrace[ii] = (fTrace[ii] - yOff)*fgain;
}

void MWD::ShiftWave(float *pwave, int nshift)
{
  if(nshift == 0) {
    return;
  }
  else if(nshift > 0) {
    int ii = int(nNsamp)-1;
    for(; ii >= nshift; ii--)
      pwave[ii] = pwave[ii-nshift];
    for(ii = 1; ii < nshift; ii++)
      pwave[ii] = pwave[0];
  }
  else {
    int ii = 0;
    int jj = int(nNsamp)+nshift;
    for(; ii < jj; ii++)
      pwave[ii] = pwave[ii-nshift];
    ii = int(nNsamp)+nshift;
    jj = int(nNsamp)-1;
    for(; ii < jj; ii++)
      pwave[ii] = pwave[nNsamp-1];
  }
}


