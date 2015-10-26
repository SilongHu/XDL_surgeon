// Module Lever Placer
The XDL_Module.cpp is used for moving all the CLB,DSP,RAMB instances from the specific XDL file.

Using command './XDL_Module <specific XDL file name> <X,Y operation like X+1,Y-1>' to move the module.

//Instance Level Placer
The XDL_Instance.cpp is used for moving the specific instance from its original site to specific site.

Using command './XDL_Instance <XDl file> <Inst_Type> <Old_loc> <New_loc>' to move the specific instance.

//Module Level Router(MLR)
The Module_Router.cpp contains the functions of Module Level Placer and also enables to change all PIPs(Relating to existing Instance) TILE.

If you want to generate the executable file, you should put these c++ file into the TORC environment and follow its tutorial to use 'make' to generate the executable file under LINUX.

Remaining bugs:
1. DSP & BRAM cascaded
2. Carry-Chain overlap.
3. Some specific TIEOFF sites I do not mention in my Module_Router.cpp(TIEOFF_X10Y0->DSP_R_X9Y0)
4. In Module Level Router, When I input the X-10,Y-10, the program only executes the first step(X-10) and ignore the second step
5. MLR just focus on Y-direction and the CLB modification(Not tried DSP & BRAM yet).
6. MLR X-direction errors.

Now, I can convert my generated XDL to NCD file without error, which just focus on the Y-direction of CLB. As for X-direction, I can change the corresponding tile location in the PIP, however, when it comes to converting XDL to NCD, the error shows: Can't find arc in tile CLBXX with connecting wires XX and XX, which means I should also change the sink and source simultaneously.
