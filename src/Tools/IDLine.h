#pragma once
#include "GateSelector.h"
#include "TMath.h"
#include "TGraph.h"

// Adapted from J. Dudouet

class IDLine;

class IDGrid : public GateSelector< 1, 1, TGraph>{
protected:
    TList *fListOfLines;  
    Int_t fNLines;
    IDLine *fClosestLine;
    Int_t fZMin;
    Int_t fZMax;

    Double_t fDistToClosestLine;
    Bool_t fIsAbove;

    Int_t fZForNewLine{0};

    std::vector<int> indices;
    std::unordered_map<int,std::pair<double,double>> easyGrid;

private:
    Double_t fPID;
    Double_t xScale{1};
    Double_t yScale{1};

public:
    explicit IDGrid(const std::string& folder_name);
    virtual ~IDGrid();

    //void Browse(TBrowser *);
    //virtual void Draw(Option_t *chopt=""); //**MENU**

    Int_t GetZMin() const {return fZMin;}
    Int_t GetZMax() const {return fZMax;}

    Double_t GetPID() const{return fPID;}
    Int_t GetZ() const{return TMath::Nint(fPID);}

    void GetZForNewLine(Int_t Z=0) {fZForNewLine=Z;} //**MENU**
    Double_t Identify(Double_t x, Double_t y, Int_t initZ=0, Int_t distAxis=2);
    IDLine *GetLine(Int_t z);

    //TList *GetListOfZLines(){return fListOfLines;}
    //void Update();
    //void NewLine(Int_t Z=-1); //**MENU**
    //IDLine *AddLine(Int_t Z);
    //void CloneLine(Int_t Ztoclone, Int_t NewZ); //**MENU**
    //void RemoveLine(Int_t Z); //**MENU**
    //void Clean();
    //void TestGrid();


    //ClassDef(IDGrid,2)
};


class IDLine : public TGraph
{
protected:
    Int_t fZ;

    Double_t fXProj;
    Double_t fYProj;

    Int_t Npoints;

public:
    explicit IDLine(Int_t z=0);
    explicit IDLine(TGraph* gr,Int_t z=0);
    ~IDLine() override = default;

    void DrawLine(){};

    void SetZ(Int_t z); //**MENU**
    Int_t GetZ() const{return fZ;}

    Double_t GetXProj() const{return fXProj;}
    Double_t GetYProj() const{return fYProj;}

    //virtual Int_t Compare(const TObject *obj) const;
    Bool_t IsSortable() const override { return kTRUE; }

    //void FitLine(double RangeForFit=5, int NBinsForProj=2){}; //**MENU**
    //void FitPoint(Int_t ipoint, double RangeForFit=5, int NBinsForProj=2){}; //**MENU**

    Double_t DistanceToLine(Double_t px, Double_t py, Int_t & i_near, Int_t distAxis=2);

    Double_t DistanceToLine(Double_t px, Double_t py, Double_t xp1,
                                      Double_t yp1, Double_t xp2, Double_t yp2,
                                      Int_t&, Double_t &XProj, Double_t &YProj, Int_t distAxis=2);

    Bool_t PosRelToLine(Option_t* opt, Double_t px,
                                         Double_t py, Double_t xp1,
                                         Double_t yp1, Double_t xp2,
                                         Double_t yp2, Int_t distAxis=2);

    Bool_t WhereAmI(Double_t px, Double_t py, Double_t &dist, Option_t* opt, Int_t distAxis=2);

    //ClassDef(IDLine,1)
};