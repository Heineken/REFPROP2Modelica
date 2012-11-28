within MediaTwoPhaseMixture.REFPROPMedium;
function getProp_REFPROP
  "calls C function with property identifier & returns single property"
    input String what2calc;
    input String statevars;
    input String fluidnames;
    input Real[:] props;
    input Real statevar1;
    input Real statevar2;
    input MassFraction X[:] "mass fraction m_NaCl/m_Sol";
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    input String errormsg;
//    input Integer debug=1;
    output Real val;

   external "C" val = props_REFPROP(what2calc, statevars, fluidnames, props, statevar1, statevar2, X, phase, REFPROP_PATH, errormsg, debugmode);

annotation (Include="#include <refprop_wrapper.h>", Library="refprop_wrapper");
end getProp_REFPROP;
