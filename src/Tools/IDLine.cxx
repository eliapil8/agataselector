#include "IDLine.h"

#include <iostream>
#include <numeric>

#include <TVirtualPad.h>
#include <TVector2.h>
#include <TH2.h>
#include <TH1.h>
#include <TF1.h>
#include <TCutG.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TMarker.h>
#include <TLatex.h>

// Adapted from J. Dudouet

using namespace std;

IDLine::IDLine(Int_t z):
    TGraph(),
    fXProj(0),
    fYProj(0)
{
    SetZ(z);
    SetMarkerStyle(20);
}

IDLine::IDLine(TGraph* gr,Int_t z):
    TGraph(*(TGraph*)gr->Clone()),
    fXProj(0),
    fYProj(0)
{
    SetZ(z);
    SetMarkerStyle(20);
    Npoints = gr->GetN();
}


void IDLine::SetZ(Int_t z)
{
    fZ = z;

    SetName(Form("IDLine_Z=%d",fZ));

    if(fZ%2==0 && fZ>0) {
        SetLineColor(kBlue);
        SetMarkerColor(kBlue);
    }
    else if(fZ%2==1 && fZ>0) {
        SetLineColor(kRed);
        SetMarkerColor(kRed);
    }
    else if(fZ<0) {
        SetLineColor(kMagenta);
        SetMarkerColor(kMagenta);
    }
}


Double_t IDLine::DistanceToLine(Double_t px, Double_t py, Int_t & i_near, Int_t distAxis)
{
    //Compute the closest distance of approach from point px,py to this line.
    //The units of px, py are the same as the coordinates of the graph
    //
    //WARNING: result can be NEGATIVE - see below.
    //
    //The distance from the point to each segment is calculated in turn.
    //
    //If the point lies between the endpoints of at least one segment, the returned
    //distance will be the closest of these segments. In this case i_near gives the
    //index of the first point of the closest segment.
    //
    //If not then the distance is the smallest distance between the endpoint
    //of a segment and the point. In this case i_near gives the index of the
    //closest endpoint.
    //If the closest segment-endpoint is not one of the endpoints of the whole line,
    //the returned distance is positive.
    //On the other hand, if the closest part of the line to the point is one of the
    //two endpoints of the line, the returned distance is NEGATIVE.


    Double_t distance, dist2;
    Double_t DistanceAbs = 1e9;
    Double_t oldDistance = 1e9;
    int n_sequence = 0;
    Int_t i;
    Double_t d;
    distance = dist2 = 9999.;
    Int_t i_nearest_point = 0, inear1 = 0, inear2 = 0;
    Double_t XProj, YProj;
    //distance = closest approach to a segment with point lying between endpoints
    //dist2 = closest approach to a segment with point outside endpoints
    //if distance == 9999 at end, then dist2 is used


    int inPoint = 0;
    int finPoint = GetN() - 1;
    int increment = 1;
    
    //if( px < GetMean(1) ) { 
    //    inPoint = GetN() - 1;
    //    finPoint = 0;
    //    increment = -1;
    //}


    
    for (i = inPoint; i!=finPoint; i+=increment) {
        d = DistanceToLine(px, py, fX[i], fY[i], fX[i + 1], fY[i + 1], i_nearest_point, XProj, YProj, distAxis);
        if(TMath::Abs(d)<DistanceAbs)
        {
            DistanceAbs = TMath::Abs(d);
            fXProj = XProj;
            fYProj = YProj;
        }

        if (d >= 0.) {
            if (d < distance) {
                distance = d;
                inear1 = i;
            }
        } else {
            //point not between endpoints of segment
            if (-d < dist2) {
                dist2 = -d;
                inear2 = i + i_nearest_point;
            }
        }
        if(oldDistance > DistanceAbs) n_sequence++;
        //if(oldDistance < DistanceAbs && n_sequence>5) break;
        oldDistance = DistanceAbs;
    }
    
    

    i_near = inear1;

    if(inear2==0 || inear2==(fNpoints - 1)) return -DistanceAbs;
    else return DistanceAbs;

//    i_near = inear1;
//    if (distance < 9999.)
//        return distance;

//    i_near = inear2;
//    //closest point is an 'internal' point of line - POSITIVE distance
//    if (inear2 > 0 && inear2 < (fNpoints - 1))
//        return dist2;
//    //closest point is one of endpoints of line - NEGATIVE distance
//    return -dist2;
}

Bool_t IDLine::PosRelToLine(Option_t* opt, Double_t px,
                             Double_t py, Double_t xp1,
                             Double_t yp1, Double_t xp2,
                             Double_t yp2, Int_t distAxis)
{
    //Given a line segment defined by endpoints (xp1,yp1) and (xp2,yp2) test the
    //  direction of (px,py) with respect to the line in the (x,y) plane.
    //
    //                                                        M
    //      P1 (xp1,yp1) x---------------------------------------x P2 (xp2,yp2)
    //                                                         |
    //                                                         |
    //                                                         |
    //                                                         |
    //                                                         |
    //                                                         x
    //                                                                P (px,py)
    //This is given by the angle phi between vector MP (component of vector P1->P  (or P2->P)
    //perpendicular to P1->P2) and the +ve x-axis.
    //Point is left of line if 90<phi<270
    //Point is right of line if 270<phi<90
    //Point is above line if 0<phi<180
    //Point is below line if 180<phi<360
    //The option string is "left", "right", "above" or "below", and the result is either kTRUE or kFALSE
    //depending on the relative position of the point and the line.
    //E.g. in the diagram shown above, PosRelToLine("below", ...) would give kTRUE - the point is
    //below the line.


    TVector2 P1(xp1, yp1), P2(xp2, yp2), P(px, py);
    TVector2 P1P2 = P2 - P1;
    TVector2 P1P = P - P1;
    TVector2 MP, M;

    if(distAxis == 0){
        M = P1 + TVector2{(P2.X() - P1.X())/(P2.Y() - P1.Y())*(P.Y() - P1.Y()),(P.Y() - P1.Y())};
        MP = P - M;
    }
    else if(distAxis == 1){
        M = P1 + TVector2{(P.X() - P1.X()),(P2.Y() - P1.Y())/(P2.X() - P1.X()) * (P.X() - P1.X())};
        MP = P - M;
    }
    else if(distAxis == 2){
        MP = P1P.Norm((P1P2));
        M = P-MP;
    }
    
    Double_t phi = MP.Phi() * TMath::RadToDeg();
    //Point is left of line if 90<phi<270
    //Point is right of line if 270<phi<90
    //Point is above line if 0<phi<180
    //Point is below line if 180<phi<360
    Bool_t result = kFALSE;
    if (!strcmp(opt, "left")) {
        result = (phi > 90. && phi < 270.);
    } else if (!strcmp(opt, "right")) {
        result = (phi < 90. || phi > 270.);
    } else if (!strcmp(opt, "above")) {
        result = (phi > 0. && phi < 180.);
    } else if (!strcmp(opt, "below")) {
        result = (phi > 180. && phi < 360.);
    }
    return result;
}

Double_t IDLine::DistanceToLine(Double_t px, Double_t py, Double_t xp1, Double_t yp1, Double_t xp2, Double_t yp2, Int_t & i_nearest_point, Double_t &XProj, Double_t &YProj, Int_t distAxis)
{
    //Given a line segment defined by endpoints (xp1,yp1) and (xp2,yp2) find the
    //  shortest distance between point (px,py) and the line.
    //
    //                                                   M
    //      P1 (xp1,yp1) x---------------------------------------x P2 (xp2,yp2)
    //                                                      |
    //                                                      |
    //                                                      |
    //                                                      |
    //                                                      |
    //                                                      x
    //                                                                P (px,py)
    //This is simply the magnitude of the component of vector P1->P  (or P2->P)
    //perpendicular to P1->P2.
    //
    //If the point is indeed between the two endpoints as shown, then
    //P1M + P2M = P1P2
    //If not, P1M + P2M > P1P2. In this case the closest distance is that to the nearer
    //of the two endpoints, but the value returned is negative; i_nearest_point gives the index (0 or 1)
    //of the nearer endpoint

    TVector2 P1(xp1, yp1), P2(xp2, yp2), P(px, py);
    TVector2 P1P2 = P2 - P1;
    TVector2 P1P = P - P1;
    TVector2 P2P = P - P2;

    TVector2 MP, M;

    if(distAxis == 0){
        M = P1 + TVector2{(P2.X() - P1.X())/(P2.Y() - P1.Y())*(P.Y() - P1.Y()),(P.Y() - P1.Y())};
        MP = P - M;
    }
    else if(distAxis == 1){
        M = P1 + TVector2{(P.X() - P1.X()),(P2.Y() - P1.Y())/(P2.X() - P1.X()) * (P.X() - P1.X())};
        MP = P - M;
    }
    else if(distAxis == 2){
        MP = P1P.Norm((P1P2));
        M = P-MP;
    }

    XProj = M.X();
    YProj = M.Y();



    i_nearest_point = 0;
    //check point lies between endpoints
    Double_t sum = (P1P - MP).Mod() + (P2P - MP).Mod();
    //cout << "sum = "<<sum<<"  P1P2 = " << P1P2.Mod() << " difference = " << sum-P1P2.Mod() << endl;
    if (TMath::Abs(sum - P1P2.Mod()) > 1.e-06) {
        //cout << "take endpoint" << endl;
        if (P1P.Mod() < P2P.Mod()) {
            i_nearest_point = 0;
            XProj = xp1;
            YProj = yp1;
            return -(P1P.Mod());
        }
        i_nearest_point = 1;
        XProj = xp2;
        YProj = yp2;
        return -(P2P.Mod());
    }
    //cout << "point is close to segment" << endl;

    //if(distAxis==1){
    //    TFile *file = TFile::Open("DistanceToLineProjection.root", "update");
    //    TCanvas * c = new TCanvas(Form("d_%.3f_%.3f", px, py), Form("Distance to line projection for point (%.3f, %.3f)", px, py), 800, 600);
    //    TH2F *h = new TH2F(Form("h_%.3f_%.3f", px, py), "Distance to line projection", 100, std::min(xp1, xp2) - 5*TMath::Abs(xp2 - xp1), std::max(xp1, xp2) + 5*TMath::Abs(xp2 - xp1),
    //                       100, std::min(yp1, yp2) - 20*TMath::Abs(yp2 - yp1), std::max(yp1, yp2) + 20*TMath::Abs(yp2 - yp1));
    //    h->Draw();
    //    TLine *line = new TLine(xp1, yp1, xp2, yp2);
    //    line->SetLineColor(kRed);
    //    line->Draw("same");
    //    TLatex *latex = new TLatex();
    //    latex->SetTextSize(0.03);
    //    latex->DrawLatex(xp1, yp1, Form("P1 (%.2f, %.2f)", xp1, yp1));
    //    latex->DrawLatex(xp2, yp2, Form("P2 (%.2f, %.2f)", xp2, yp2));
    //    latex->DrawLatex(px, py, Form("P (%.2f, %.2f)", px, py));
    //    TMarker *point = new TMarker(px, py, kFullCircle);
    //    point->SetMarkerColor(kBlue);
    //    point->SetMarkerSize(1.5);
    //    point->Draw("same");
    //    TMarker *projPoint = new TMarker(XProj, YProj, kFullCircle);
    //    projPoint->SetMarkerColor(kGreen);
    //    projPoint->SetMarkerSize(1.5);
    //    projPoint->Draw("same");
    //    c->Update();   
    //    c->Write();
    //    file->Close();
    //}

    return MP.Mod();
}

Bool_t IDLine::WhereAmI(Double_t px, Double_t py, Double_t &dist, Option_t* opt, Int_t distAxis)
{
    //The relative position of point (px,py) with respect to the line is tested.
    //The option string can be "left", "right", "above" or "below".
    //WhereAmI( px, py, "above") returns kTRUE if the point is above the line etc. etc.
    //
    //First of all, the closest segment/point of the line to the point is found.
    //Then the relative position of the point and this segment/point is tested.


    //Find closest point/segment
    Int_t i_close = 0;
    dist = DistanceToLine(px, py, i_close, distAxis);
    if (dist < 0) {
        //negative distance => closest point
        if (!strcmp(opt, "left")) {
            return (px < fX[i_close]);
        } else if (!strcmp(opt, "right")) {
            return (px > fX[i_close]);
        } else if (!strcmp(opt, "above")) {
            return (py > fY[i_close]);
        } else if (!strcmp(opt, "below")) {
            return (py < fY[i_close]);
        }
        return kFALSE;
    } else {
        //positive distance => closest segment
        return PosRelToLine(opt, px, py, fX[i_close], fY[i_close],
                            fX[i_close + 1], fY[i_close + 1], distAxis);
    }
    return kFALSE;
}

IDGrid::IDGrid(const std::string& folder_name):
    GateSelector<1,1,TGraph>(folder_name), //Imprecise, maybe change
    fNLines(0),
    fClosestLine(0x0),
    fZMin(0),
    fZMax(0),
    fDistToClosestLine(1e9),
    fPID(-1)
{
    if(gates.size() == 0) return;
    if(gates.begin()->first.size() != 1) {
        std::cerr << "IDGrid::IDGrid: The lines should only have 1 entrance index" << std::endl;
    }
    fListOfLines = new TList();
    //fListOfLines->SetOwner();
    std::vector<double> xmean;
    std::vector<double> ymean;    
    for(auto it : gates){
        indices.push_back(it.first[0]);
        if(it.first[0] == 0) continue;
        xmean.push_back(it.second[0].cut->GetMean(1));
        ymean.push_back(it.second[0].cut->GetMean(2));
    }
    std::sort(indices.begin(), indices.end());

    xScale = std::accumulate(xmean.begin(), xmean.end(), 0.0)/xmean.size();
    yScale = std::accumulate(ymean.begin(), ymean.end(), 0.0)/ymean.size();

    for(auto it:gates){
        auto cut = it.second[0].cut;
        for(int i = 0; i < cut->GetN(); i++){
            double x,y;
            cut->GetPoint(i,x,y);
            cut->SetPoint(i,x/xScale,y/yScale);
        }
    }

    for(auto id : indices){
        auto it = gates.find({id});

        if(Settings::getInstance().verbosity_level>2) std::cout << id << " ";

        int z = it->first[0];
        IDLine* l = new IDLine(it->second[0].cut, z);
        

        fListOfLines->Add(l);
        if(z < fZMin) fZMin = z;
        if(z > fZMax) fZMax = z;
        fNLines++;

        if(Settings::getInstance().verbosity_level>2) std::cout << "IDGrid::IDGrid: Added line with id = " << z << std::endl;
        
        easyGrid[z] = std::make_pair(it->second[0].cut->GetMean(1), it->second[0].cut->GetMean(2));

    }
    
    std::cout << std::endl;


}


IDGrid::~IDGrid()
{
    //delete fListOfLines;
}

Double_t IDGrid::Identify(Double_t x, Double_t y, Int_t initZ, Int_t distAxis)
{
    Int_t IBidon=0;

    fPID = -1;

    IDLine* ClosestSecondLine = nullptr;

    ///looking for the closest line
    Double_t BestDist = 1e9;

    //int centerId = 0;
    //
    //for(int id = 1; id < indices.size()-1; id++) {
    //    int z = indices[id];
    //    int next = indices[id+1];
    //    if(y>easyGrid[z].second && y<easyGrid[next].second) {
    //        centerId = z;
    //                
    //        double ratio_x = x/easyGrid[z].first;
    //        if(ratio_x < 0.5 ) {
    //            centerId+=2;
    //        }
    //        else if(ratio_x > 0.5 && ratio_x < 0.75) {
    //            centerId+=1;
    //        }
    //        else if(ratio_x > 1.5 && ratio_x < 2) {
    //            centerId-=1;
    //        }
    //        else if (ratio_x > 2) {
    //            centerId-=2;
    //        }
    //    }
    //    else if (id < 2 && y<easyGrid[z].second) {
    //        centerId = z;
    //    }
    //    else if (id == indices.size()-2 && y>easyGrid[next].second) {
    //        centerId = next;
    //    }
//
    //}
//
//
    ////std::cout << "x y " << x << " " << y << " CenterId: " << centerId << " " << easyGrid[centerId].first << " " << easyGrid[centerId].second << std::endl;
//
//
    //auto foundCenter = std::find(indices.begin(), indices.end(), centerId);
    //int j = foundCenter - indices.begin();
//
    //for(int i = (j-2>1?j-2:1) ; i < (j+2<fNLines-1?j+2:fNLines-1) ; i++) {
    //    
    //    IDLine* l = ((IDLine*)fListOfLines->At(j));
    //    if(l==nullptr){
    //        if(Settings::getInstance().verbosity_level > 2) std::cout << "IDGrid::Identify: Line is null" << std::endl;
    //        continue;
    //    } 
    //    Double_t dist;
    //    Bool_t IsAbove = l->WhereAmI(x,y,dist,"above");
//
    //    if(TMath::Abs(dist)<BestDist) {
    //        BestDist = TMath::Abs(dist);
    //        fClosestLine = l;
    //        fDistToClosestLine = dist;
    //        fIsAbove = IsAbove;
//
    //        if(fIsAbove) ClosestSecondLine = (IDLine*)fListOfLines->At(i+1);
    //        else ClosestSecondLine = (IDLine*)fListOfLines->At(i-1);
    //    }
//
    //    //std::cout<< "Center "<<  centerId << " j " << j << " fClosestLine " << fClosestLine->GetZ() << " fDistToClosestLine " << fDistToClosestLine << " fIsAbove " << fIsAbove << std::endl;
//
    //}
    //int i = 0;
    //IDLine* l = ((IDLine*)fListOfLines->At(i));
    //if(l==nullptr){
    //    if(Settings::getInstance().verbosity_level > 2) std::cout << "IDGrid::Identify: Line is null" << std::endl;
    //}
    //else{
    //    Double_t dist;
    //    Bool_t IsAbove = l->WhereAmI(x,y,dist,"above");
    //    if(TMath::Abs(dist)<BestDist) {
    //        BestDist = TMath::Abs(dist);
    //        fClosestLine = l;
    //        fDistToClosestLine = dist;
    //        fIsAbove = IsAbove;
    //        if(fIsAbove) ClosestSecondLine = (IDLine*)fListOfLines->At(i+1);
    //        
    //    }
    //}
    x /= xScale;
    y /= yScale;

    //std::cout << "x y " << x << " " << y << std::endl;
    //std::cout << "fNLines " << fNLines << std::endl;


    int inId = 0;
    int finId = fNLines - 1;

    if(finId < 0 ) {
        fPID = -1;
        return fPID;   
    }

    if(initZ!=0) {
        auto foundCenter = std::find(indices.begin(), indices.end(), initZ);
        int j = foundCenter - indices.begin();
        inId = (j-1>1?j-1:1);
        finId = (j+1<fNLines-1?j+1:fNLines-1);
    }

    for(int i = inId; i <= finId; i++) {
        //std::cout << "IDLINE i " << i << std::endl;
        IDLine* l = ((IDLine*)fListOfLines->At(i));
        if(l==nullptr){
            if(Settings::getInstance().verbosity_level > 2) std::cout << "IDGrid::Identify: Line is null" << std::endl;
        }
        else{
            Double_t dist;
            Bool_t IsAbove = l->WhereAmI(x,y,dist,"above",distAxis);
            if(TMath::Abs(dist)<BestDist) {
                BestDist = TMath::Abs(dist);
                fClosestLine = l;
                fDistToClosestLine = dist;
                fIsAbove = IsAbove;
                if(fIsAbove) ClosestSecondLine = (IDLine*)fListOfLines->At(i+1);
                else ClosestSecondLine = (IDLine*)fListOfLines->At(i-1);

            }
        }
    }
    

    // Int_t Case=0;

    ///Case 1 : rejection line
    if(fClosestLine->GetZ() <=0 ) {
        fPID = -1;
        return fPID;   
    }

    ///Case 2 : below first line or above last line
    if((fClosestLine->GetZ() == fZMin && !fIsAbove) || (fClosestLine->GetZ() == fZMax && fIsAbove) ) {

        Int_t Idx = fListOfLines->IndexOf(fClosestLine);

        if(fIsAbove) ClosestSecondLine = (IDLine*)fListOfLines->At(Idx-1);
        else ClosestSecondLine = (IDLine*)fListOfLines->At(Idx+1);

        Double_t LinesDist = TMath::Abs(ClosestSecondLine->DistanceToLine(fClosestLine->GetXProj(),fClosestLine->GetYProj(),IBidon,distAxis));

        Double_t DeltaZ = TMath::Abs(fDistToClosestLine)/LinesDist;

        //std::cout << std::setprecision(5) << "DeltaZ " << DeltaZ << " fDistToClosestLine " << fDistToClosestLine << " LinesDist " << LinesDist << std::endl;
        if(!fIsAbove) DeltaZ *= -1.;

        if(TMath::Abs(fDistToClosestLine)/LinesDist>0.5) {
            fPID = -1;
        }
        else {
            Double_t RealZ = fClosestLine->GetZ() + DeltaZ;
            fPID = RealZ;
        }

        if(fDistToClosestLine<0) {
            fPID = -1;
        }
    }
    ///Case 3 : Normal case, between two lines
    else {
        Double_t ClostestSecondLineDistance = ClosestSecondLine->DistanceToLine(x,y,IBidon,distAxis);

        Double_t DeltaZ = TMath::Abs(fDistToClosestLine)/(TMath::Abs(fDistToClosestLine)+TMath::Abs(ClostestSecondLineDistance));
        if(ClosestSecondLine->GetZ()<fClosestLine->GetZ()) DeltaZ*= -1.;

        Double_t RealZ = fClosestLine->GetZ() + DeltaZ;

        fPID = RealZ;

        if(fDistToClosestLine<0) {
            fPID = -1;
        }
    }

    if(x<0 || y<0) {
        fPID = -1;
    } 

    return fPID;
}

IDLine *IDGrid::GetLine(Int_t z)
{
    return (IDLine*)fListOfLines->FindObject(Form("IDLine_Z=%d",z));
}
