# Usage

Add here TGraph to correct the aberrations of PRISMA as a function of the xmcp.
Any file with the ".root" extension will be considered.
Generate iteratively the cuts with the --set_gates option. This will create a cuts.root file in this folder.
The file will be updated each time --set_gates is run (and not deleted).
If multiple gates are set with the same name, the selector will use the latest one.