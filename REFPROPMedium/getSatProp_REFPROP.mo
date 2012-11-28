within MediaTwoPhaseMixture.REFPROPMedium;
function getSatProp_REFPROP
  "calls C function with property identifier & returns single property"
   input String what2calc;
   input String statevar;
   input String fluidnames;
   input Real[:] props;
   input Real statevarval;
   input MassFraction X[:] "mass fraction m_NaCl/m_Sol";
   input String errormsg;
   output Real val;
//   input Integer debugmode=1;

 external "C" val = satprops_REFPROP(what2calc, statevar, fluidnames, props, statevarval, X, REFPROP_PATH, errormsg, debugmode);

 annotation (Include="#include <refprop_wrapper.h>", Library="refprop_wrapper");
end getSatProp_REFPROP;
