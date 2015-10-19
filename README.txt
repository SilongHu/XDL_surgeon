// Module Lever Placer
The XDL_Module.cpp is used for moving all the CLB,DSP,RAMB instances from the specific XDL file.

Using command './XDL_Module <specific XDL file name> <X,Y operation like X+1,Y-1>' to move the module.

//Instance Level Placer
The XDL_Instance.cpp is used for moving the specific instance from its original site to specific site.

Using command './XDL_Instance <XDl file> <Inst_Type> <Old_loc> <New_loc>' to move the specific instance.


If you want to generate the executable file, you should put these c++ file into the TORC environment and follow its tutorial to use 'make' to generate the executable file under LINUX.

Remaining bugs:
1. DSP & BRAM cascaded
2. Carry-Chain overlap.
