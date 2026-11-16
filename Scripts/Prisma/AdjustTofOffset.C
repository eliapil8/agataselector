// This script is used to adjust the TOF offsets for the A/Q calibration
// Open a root file with the AnalyzedTree and call the script with
// .x AdjustTofOffset.C
void AdjustTofOffset(){

	double tofOffsets[10];
	tofOffsets[0] = 0.0;
	tofOffsets[1] = 0.0;
	tofOffsets[2] = 0.0;
	tofOffsets[3] = 0.0;
	tofOffsets[4] = 0.0;
	tofOffsets[5] = 0.0;
	tofOffsets[6] = 0.0;
	tofOffsets[7] = 0.0;
	tofOffsets[8] = 0.0;
	tofOffsets[9] = 0.0;

	double globalTofOffset = -2.0;


	AnalyzedTree->SetAlias("TOF2",Form(" (tof*1e9+%f)*1e-9",globalTofOffset));
	//AnalyzedTree->SetAlias("TOF3"," ( ((col_nbr==0)*(TOF2*1.e9 + 0.09)) + ((col_nbr==1)*(TOF2*1.e9 + 0.07))+ ((col_nbr==2)*(TOF2*1.e9 + 0.05 ))+ ((col_nbr==3)*(TOF2*1.e9 + 0.02))+ ((col_nbr==4)*(TOF2*1.e9 + 0.01))+ ((col_nbr==5)*(TOF2*1.e9 + 0.0))+ ((col_nbr==6)*(TOF2*1.e9 - 0.02))+ ((col_nbr==7)*(TOF2*1.e9 - 0.04))+ ((col_nbr==8)*(TOF2*1.e9 - 0.025))+ ((col_nbr==9)*(TOF2*1.e9 - 0.1)) )*1.e-9");
	AnalyzedTree->SetAlias("TOF3",Form(" ( ((col_nbr==0)*(TOF2*1.e9 + %f)) + ((col_nbr==1)*(TOF2*1.e9 + %f))+ ((col_nbr==2)*(TOF2*1.e9 + %f ))+ ((col_nbr==3)*(TOF2*1.e9 + %f))+ ((col_nbr==4)*(TOF2*1.e9 + %f))+ ((col_nbr==5)*(TOF2*1.e9 + %f))+ ((col_nbr==6)*(TOF2*1.e9 + %f))+ ((col_nbr==7)*(TOF2*1.e9 + %f))+ ((col_nbr==8)*(TOF2*1.e9 + %f))+ ((col_nbr==9)*(TOF2*1.e9 + %f)) )*1.e-9",tofOffsets[0],tofOffsets[1],tofOffsets[2],tofOffsets[3],tofOffsets[4],tofOffsets[5],tofOffsets[6],tofOffsets[7],tofOffsets[8],tofOffsets[9]));

	AnalyzedTree->SetAlias("newBeta","beta*tof/TOF3");
	AnalyzedTree->SetAlias("newGamma","1/sqrt(1-newBeta*newBeta)");
	AnalyzedTree->SetAlias("newBetaGamma","newBeta*newGamma");

	AnalyzedTree->SetAlias("BetaGamma","beta*gamma");

	AnalyzedTree->SetAlias("newAoq","a_over_q*BetaGamma/newBetaGamma");
}