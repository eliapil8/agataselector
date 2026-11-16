root -l << EOF
.L ../changeNameGates.C
for(int z = 24; z < 31; z++){
convertSpliToTGraph("/home/engy/AGATAAnalysisWorkshop/PrismaConf/cal/xfp.spli",Form("aoverqxfp__%i_0__%i",z,3));
//scaleCutG(Form("charge__0_%i__%i.root",z,q),1./29979.2458/0.699570,1.);
} 
EOF
