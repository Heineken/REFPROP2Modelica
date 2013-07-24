within REFPROP2Modelica.Interfaces;
partial package PartialMixtureTwoPhaseMedium
  "Template class for two phase medium of a mixture of substances "
  extends Modelica.Media.Interfaces.PartialMixtureMedium(ThermoStates=inputChoice);
  constant Boolean smoothModel = false
    "true if the (derived) model should not generate state events";
  constant Boolean onePhase =    false
    "true if the (derived) model should never be called with two-phase inputs";

   //constant Real minValue = 1e-6;

   constant FluidConstants mixtureFluidConstants(
     iupacName =              "mixture",
     casRegistryNumber =      "mixture",
     chemicalFormula =        "mixture",
     structureFormula =       "mixture",
     molarMass =              0.001,
     criticalTemperature =    0,
     criticalPressure =       0,
     criticalMolarVolume =    0,
     acentricFactor =         0,
     triplePointTemperature = 0,
     triplePointPressure =    0,
     meltingPoint =           0,
     normalBoilingPoint =     0,
     dipoleMoment =           0);
     // The following values are not constant!

  import InputChoice = REFPROP2Modelica.Interfaces.MixtureInputChoice;

constant InputChoice inputChoice=MixtureInputChoice.phX
    "Default choice of input variables for property computations";

  type FixedPhase = Integer(min=0,max=2)
    "phase of the fluid: 1 for 1-phase, 2 for two-phase, 0 for not known, e.g. interactive use";
  record FluidLimits "validity limits for fluid model"
    extends Modelica.Icons.Record;
    Temperature TMIN "minimum temperature";
    Temperature TMAX "maximum temperature";
    Density DMIN "minimum density";
    Density DMAX "maximum density";
    AbsolutePressure PMIN "minimum pressure";
    AbsolutePressure PMAX "maximum pressure";
    SpecificEnthalpy HMIN "minimum enthalpy";
    SpecificEnthalpy HMAX "maximum enthalpy";
    SpecificEntropy SMIN "minimum entropy";
    SpecificEntropy SMAX "maximum entropy";
    annotation(Documentation(
        info="<html>
          <p>The minimum pressure mostly applies to the liquid state only.
          The minimum density is also arbitrary, but is reasonable for techical
          applications to limit iterations in non-linear systems. The limits in
          enthalpy and entropy are used as safeguards in inverse iterations.</p>
          </html>"));
  end FluidLimits;

redeclare replaceable record extends ThermodynamicState
    "Thermodynamic state of two phase medium"
    MolarMass molarMass "Molar mass of bulk mixture";
    FixedPhase phase(min=0, max=2)
      "phase of the fluid: 1 for 1-phase, 2 for two-phase, 0 for not known, e.g. interactive use";
    //PrandtlNumber Pr "prandtl number";
    //Temperature T "temperature";
    VelocityOfSound w(min=1e-8) "velocity of sound";
    //Modelica.SIunits.CubicExpansionCoefficient beta
    //"isobaric expansion coefficient";
    SpecificHeatCapacity cp(min=1e-8) "specific heat capacity cp";
    SpecificHeatCapacity cv(min=1e-8) "specific heat capacity cv";
    Density d(min=1e-8) "density";
    //DynamicViscosity eta "dynamic viscosity";
    SpecificEnthalpy h "specific enthalpy";
    //Modelica.SIunits.Compressibility kappa "compressibility";
    //ThermalConductivity lambda "thermal conductivity";
    //AbsolutePressure p "pressure";
    SpecificEntropy s "specific entropy";
    //MassFraction X[nX] "Mass fraction of components in kg/kg";
    annotation(Documentation(info="<html></html>"));
end ThermodynamicState;

  replaceable record SaturationProperties
    "Saturation properties of two phase medium"
    extends Modelica.Icons.Record;
    Temperature Tsat(min=1e-8) "saturation temperature";
  //   Real dTp "derivative of Ts wrt pressure";
  //   DerDensityByPressure ddldp "derivative of dls wrt pressure";
  //   DerDensityByPressure ddvdp "derivative of dvs wrt pressure";
  //   DerEnthalpyByPressure dhldp "derivative of hls wrt pressure";
  //   DerEnthalpyByPressure dhvdp "derivative of hvs wrt pressure";
  //   Density dl "density at bubble line (for pressure ps)";
  //   Density dv "density at dew line (for pressure ps)";
  //   SpecificEnthalpy hl "specific enthalpy at bubble line (for pressure ps)";
  //   SpecificEnthalpy hv "specific enthalpy at dew line (for pressure ps)";
    AbsolutePressure psat(min=1e-8) "saturation pressure";
  //   SurfaceTension sigma "surface tension";
  //   SpecificEntropy sl "specific entropy at bubble line (for pressure ps)";
  //   SpecificEntropy sv "specific entropy at dew line (for pressure ps)";
    MassFraction X[nX] "Bulk mass fractions";
  //   MassFraction Xl[nX] "Mass fractions of liquid phase";
  //   MassFraction Xv[nX] "Mass fractions of gaseous phase";
  //   annotation(Documentation(info="<html></html>"));
  end SaturationProperties;

redeclare replaceable model extends BaseProperties(
    p(min=1e-8,stateSelect = if preferredMediumStates and
                       (basePropertiesInputChoice == InputChoice.phX or
                        basePropertiesInputChoice == InputChoice.pTX or
                        basePropertiesInputChoice == InputChoice.psX) then
                            StateSelect.prefer else StateSelect.default),
    T(min=1e-8,stateSelect = if preferredMediumStates and
                       (basePropertiesInputChoice == InputChoice.pTX or
                       basePropertiesInputChoice == InputChoice.dTX) then
                         StateSelect.prefer else StateSelect.default),
    h(stateSelect = if preferredMediumStates and
                       basePropertiesInputChoice == InputChoice.phX then
                         StateSelect.prefer else StateSelect.default),
    d(min=1e-8,stateSelect = if preferredMediumStates and
                       basePropertiesInputChoice == InputChoice.dTX then
                         StateSelect.prefer else StateSelect.default))
    import REFPROP2Modelica.Interfaces.MixtureInputChoice;
    parameter MixtureInputChoice basePropertiesInputChoice = inputChoice
      "Choice of input variables for property computations";
    FixedPhase phaseInput
      "Phase input for property computation functions, 2 for two-phase, 1 for one-phase, 0 if not known";
    Integer phaseOutput
      "Phase output for medium, 2 for two-phase, 1 for one-phase";
    SpecificEntropy s(
      stateSelect = if basePropertiesInputChoice == InputChoice.psX then
                       StateSelect.prefer else StateSelect.default)
      "Specific entropy";
    SaturationProperties sat "saturation property record";
equation
    MM = state.molarMass;
    R = Modelica.Constants.R/max(1e-8,MM);
    if (onePhase or (basePropertiesInputChoice == InputChoice.pTX)) then
      phaseInput = 1 "Force one-phase property computation";
    else
      phaseInput = 0 "Unknown phase";
    end if;
    if (basePropertiesInputChoice == InputChoice.phX) then
      // Compute the state record (including the unique ID)
      state =
        setState_phX(
        p,
        h,
        X,
        phaseInput);
      d = density(state);
      s = specificEntropy(state);
      T = temperature(state);
    elseif (basePropertiesInputChoice == InputChoice.dTX) then
      state =
        setState_dTX(
        d,
        T,
        X,
        phaseInput);
      h = specificEnthalpy(state);
      p = pressure(state);
      s = specificEntropy(state);
    elseif (basePropertiesInputChoice == InputChoice.pTX) then
      state =
        setState_pTX(
        p,
        T,
        X,
        phaseInput);
      d = density(state);
      h = specificEnthalpy(state);
      s = specificEntropy(state);
    elseif (basePropertiesInputChoice == InputChoice.psX) then
      state =
        setState_psX(
        p,
        s,
        X,
        phaseInput);
      d = density(state);
      h = specificEnthalpy(state);
      T = temperature(state);
    end if;
    // Compute the internal energy
    u = h - p/max(1e-8,d);
    // Compute the saturation properties record
    sat = setSat_pX(state.p,state.X);
    // Event generation for phase boundary crossing
    if smoothModel then
      // No event generation
      phaseOutput = state.phase;
    else
      // Event generation at phase boundary crossing
      if basePropertiesInputChoice == InputChoice.phX then
        phaseOutput = if ((h > bubbleEnthalpy(sat) and h < dewEnthalpy(sat)) and
                           p < fluidConstants[1].criticalPressure) then 2 else 1;
      elseif basePropertiesInputChoice == InputChoice.dTX then
        phaseOutput = if  ((d < bubbleDensity(sat) and d > dewDensity(sat)) and
                            T < fluidConstants[1].criticalTemperature) then 2 else 1;
      elseif basePropertiesInputChoice == InputChoice.psX then
        phaseOutput = if ((s > bubbleEntropy(sat) and s < dewEntropy(sat)) and
                           p < fluidConstants[1].criticalPressure) then 2 else 1;
      else
        // basePropertiesInputChoice == pTX
        phaseOutput = 1;
      end if;
    end if;
end BaseProperties;

//   redeclare replaceable partial model extends BaseProperties
//     "Base properties (p, d, T, h, s, u, R, MM, sat) of two phase medium"
//   //  Temperature T(start=300);
//     Modelica.SIunits.SpecificEntropy s;
//     SaturationProperties sat "Saturation properties at the medium pressure";
//     annotation(Documentation(info="<html></html>"));
//   end BaseProperties;

  replaceable partial function getMolarMass
   extends Modelica.Icons.Function;
   output MolarMass MM "Molar mass of the mixture";
    annotation (Documentation(info="<html></html>"));
  end getMolarMass;

  replaceable partial function getCriticalTemperature
    extends Modelica.Icons.Function;
    output Temperature Tcrit "Molar mass of the mixture";
    annotation (Documentation(info="<html></html>"));
  end getCriticalTemperature;

  replaceable partial function getCriticalPressure
   extends Modelica.Icons.Function;
   output AbsolutePressure Pcrit "Molar mass of the mixture";
    annotation (Documentation(info="<html></html>"));
  end getCriticalPressure;

  replaceable partial function getCriticalMolarVolume
    extends Modelica.Icons.Function;
    output MolarVolume v "Molar mass of the mixture";
    annotation (Documentation(info="<html></html>"));
  end getCriticalMolarVolume;

  replaceable partial function setDewState
    "Return the thermodynamic state on the dew line"
    extends Modelica.Icons.Function;
    input SaturationProperties sat "saturation point";
    input FixedPhase phase(min = 1, max = 2) =  1 "phase: default is one phase";
    output ThermodynamicState state "complete thermodynamic state info";
    annotation(Documentation(info="<html></html>"));
  end setDewState;

  replaceable partial function setBubbleState
    "Return the thermodynamic state on the bubble line"
    extends Modelica.Icons.Function;
    input SaturationProperties sat "saturation point";
    input FixedPhase phase(min = 1, max = 2) =  1 "phase: default is one phase";
    output ThermodynamicState state "complete thermodynamic state info";
    annotation(Documentation(info="<html></html>"));
  end setBubbleState;

  redeclare replaceable partial function extends setState_dTX
    "Return thermodynamic state as function of d, T and composition X or Xi"
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    annotation(Documentation(info="<html></html>"));
  end setState_dTX;

  redeclare replaceable partial function extends setState_phX
    "Return thermodynamic state as function of p, h and composition X or Xi"
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    annotation(Documentation(info="<html></html>"));
  end setState_phX;

  redeclare replaceable partial function extends setState_psX
    "Return thermodynamic state as function of p, s and composition X or Xi"
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    annotation(Documentation(info="<html></html>"));
  end setState_psX;

  redeclare replaceable partial function extends setState_pTX
    "Return thermodynamic state as function of p, T and composition X or Xi"
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    annotation(Documentation(info="<html></html>"));
  end setState_pTX;

  replaceable function setSat_TX
    "Return saturation property record from temperature"
    extends Modelica.Icons.Function;
    input Temperature T "temperature";
    input MassFraction X[nX] "Mass fractions";
    output SaturationProperties sat "saturation property record";
  algorithm
    sat.Tsat := T;
    sat.psat := saturationPressure(T,X);
    sat.X := X;
    annotation(Documentation(info="<html></html>"));
  end setSat_TX;

  replaceable function setSat_pX
    "Return saturation property record from pressure"
    extends Modelica.Icons.Function;
    input AbsolutePressure p "pressure";
    input MassFraction X[nX] "Mass fractions";
    output SaturationProperties sat "saturation property record";
  algorithm
    sat.psat := p;
    sat.Tsat := saturationTemperature(p,X);
    sat.X := X;
    annotation(Documentation(info="<html></html>"));
  end setSat_pX;

/*
Functions to obtain fluid properties from the currently active state.
*/

  replaceable partial function bubbleEnthalpy
    "Return bubble point specific enthalpy"
      extends Modelica.Icons.Function;
      input SaturationProperties sat "saturation property record";
      output SpecificEnthalpy hl "boiling curve specific enthalpy";
    annotation(Documentation(info="<html></html>"));
  end bubbleEnthalpy;

  replaceable partial function dewEnthalpy "Return dew point specific enthalpy"
      extends Modelica.Icons.Function;
      input SaturationProperties sat "saturation property record";
      output SpecificEnthalpy hv "dew curve specific enthalpy";
    annotation(Documentation(info="<html></html>"));
  end dewEnthalpy;

  replaceable partial function bubbleEntropy
    "Return bubble point specific entropy"
    extends Modelica.Icons.Function;
    input SaturationProperties sat "saturation property record";
    output SpecificEntropy sl "boiling curve specific entropy";
    annotation(Documentation(info="<html></html>"));
  end bubbleEntropy;

  replaceable partial function dewEntropy "Return dew point specific entropy"
    extends Modelica.Icons.Function;
    input SaturationProperties sat "saturation property record";
    output SpecificEntropy sv "dew curve specific entropy";
    annotation(Documentation(info="<html></html>"));
  end dewEntropy;

  replaceable partial function bubbleDensity "Return bubble point density"
      extends Modelica.Icons.Function;
      input SaturationProperties sat "saturation property record";
      output Density dl "boiling curve density";
    annotation(Documentation(info="<html></html>"));
  end bubbleDensity;

  replaceable partial function dewDensity "Return dew point density"
      extends Modelica.Icons.Function;
      input SaturationProperties sat "saturation property record";
      output Density dv "dew curve density";
    annotation(Documentation(info="<html></html>"));
  end dewDensity;

  replaceable partial function saturationPressure "Return saturation pressure"
      extends Modelica.Icons.Function;
      input Temperature T "temperature";
      input MassFraction X[:]={1} "fluid composition as mass fractions";
      output AbsolutePressure p "saturation pressure";
    annotation(Documentation(info="<html></html>"));
  end saturationPressure;

  function saturationPressure_der "Return saturation pressure time derivative"
    extends Modelica.Icons.Function;
    input Temperature T "temperature";
    input MassFraction X[:]={1} "fluid composition as mass fractions";
    input Real T_der "Temperature derivative";
    output Real p_der "saturation pressure derivative";
    // Standard definition
  algorithm
    p_der :=T_der/saturationTemperature_derp_sat(setSat_TX(T,X));
    annotation(Inline = true);
  end saturationPressure_der;

  replaceable function saturationPressure_sat "Return saturation temperature"
      extends Modelica.Icons.Function;
      input SaturationProperties sat "saturation property record";
      output AbsolutePressure p "saturation pressure";
  algorithm
      p := sat.psat;
    annotation(Documentation(info="<html></html>"));
  end saturationPressure_sat;

  replaceable partial function saturationTemperature
    "Return saturation temperature"
      extends Modelica.Icons.Function;
      input AbsolutePressure p "pressure";
      input MassFraction X[:]={1} "fluid composition as mass fractions";
      output Temperature T "saturation temperature";
    annotation(Documentation(info="<html></html>"));
  end saturationTemperature;

  replaceable function saturationTemperature_sat
    "Return saturation temperature"
      extends Modelica.Icons.Function;
      input SaturationProperties sat "saturation property record";
      output Temperature T "saturation temperature";
  algorithm
      T := sat.Tsat;
    annotation(Documentation(info="<html></html>"));
  end saturationTemperature_sat;

  replaceable partial function saturationTemperature_derp
    "Return derivative of saturation temperature w.r.t. pressure"
      extends Modelica.Icons.Function;
      input AbsolutePressure p "pressure";
      output Real dTp "derivative of saturation temperature w.r.t. pressure";
    annotation(Documentation(info="<html></html>"));
  end saturationTemperature_derp;

  replaceable function saturationTemperature_derp_sat
    "Return derivative of saturation temperature w.r.t. pressure"
      extends Modelica.Icons.Function;
      input SaturationProperties sat "saturation property record";
      output Real dTp "derivative of saturation temperature w.r.t. pressure";
  algorithm
      dTp := saturationTemperature_derp(sat.psat);
    annotation(Documentation(info="<html></html>"));
  end saturationTemperature_derp_sat;

  /*  redeclare replaceable partial function extends molarMass 
    "Return the molar mass of the medium"
    algorithm 
      MM := fluidConstants[1].molarMass;
    end molarMass;*/

  replaceable partial function dBubbleDensity_dPressure
    "Return bubble point density derivative"
      extends Modelica.Icons.Function;
      input SaturationProperties sat "saturation property record";
      output DerDensityByPressure ddldp "boiling curve density derivative";
    annotation(Documentation(info="<html></html>"));
  end dBubbleDensity_dPressure;

  replaceable partial function dDewDensity_dPressure
    "Return dew point density derivative"
      extends Modelica.Icons.Function;
      input SaturationProperties sat "saturation property record";
      output DerDensityByPressure ddvdp "saturated steam density derivative";
    annotation(Documentation(info="<html></html>"));
  end dDewDensity_dPressure;

  replaceable partial function dBubbleEnthalpy_dPressure
    "Return bubble point specific enthalpy derivative"
      extends Modelica.Icons.Function;
      input SaturationProperties sat "saturation property record";
      output DerEnthalpyByPressure dhldp
      "boiling curve specific enthalpy derivative";
    annotation(Documentation(info="<html></html>"));
  end dBubbleEnthalpy_dPressure;

  replaceable partial function dDewEnthalpy_dPressure
    "Return dew point specific enthalpy derivative"
      extends Modelica.Icons.Function;
      input SaturationProperties sat "saturation property record";
      output DerEnthalpyByPressure dhvdp
      "saturated steam specific enthalpy derivative";
    annotation(Documentation(info="<html></html>"));
  end dDewEnthalpy_dPressure;

   redeclare replaceable function density_phX
    "Return density from p, h, and X or Xi"
       extends Modelica.Icons.Function;
       input AbsolutePressure p "Pressure";
       input SpecificEnthalpy h "Specific enthalpy";
       input MassFraction X[nX] "Mass fractions";
       input FixedPhase phase=0
      "2 for two-phase, 1 for one-phase, 0 if not known";
       output Density d "density";
   algorithm
     d := density(
       setState_phX(
         p,
         h,
         X,
         phase));
     annotation(Documentation(info="<html></html>"));
   end density_phX;

  redeclare replaceable function density_psX
    "Return density from p, s, and X or Xi"
      extends Modelica.Icons.Function;
      input AbsolutePressure p "Pressure";
      input SpecificEntropy s "Specific entropy";
      input MassFraction X[nX] "Mass fractions";
      input FixedPhase phase=0
      "2 for two-phase, 1 for one-phase, 0 if not known";
      output Density d "Density";
  algorithm
    d := density(
      setState_psX(
        p,
        s,
        X,
        phase));
    annotation(Documentation(info="<html></html>"));
  end density_psX;

  redeclare replaceable function density_pTX
    "Return density from p, T, and X or Xi"
      extends Modelica.Icons.Function;
      input AbsolutePressure p "Pressure";
      input Temperature T "Temperature";
      input MassFraction X[nX] "Mass fractions";
      input FixedPhase phase=0
      "2 for two-phase, 1 for one-phase, 0 if not known";
      output Density d "Density";
  algorithm
    d := density(
      setState_pTX(
        p,
        T,
        X,
        phase));
    annotation(Documentation(info="<html></html>"));
  end density_pTX;

replaceable function specificEnthalpy_dTX
    "Return specific enthalpy from d, T, and X or Xi"
  extends Modelica.Icons.Function;
  input Density d "Pressure";
  input Temperature T "Specific entropy";
  input MassFraction X[nX] "Mass fractions";
  input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
  output SpecificEnthalpy h "specific enthalpy";
algorithm
    h := specificEnthalpy(
      setState_dTX(
      d,
      T,
      X,
      phase));
annotation(Documentation(info="<html></html>"));
end specificEnthalpy_dTX;

  redeclare replaceable function specificEnthalpy_psX
    "Return specific enthalpy from p, s, and X or Xi"
      extends Modelica.Icons.Function;
      input AbsolutePressure p "Pressure";
      input SpecificEntropy s "Specific entropy";
      input MassFraction X[nX] "Mass fractions";
      input FixedPhase phase=0
      "2 for two-phase, 1 for one-phase, 0 if not known";
      output SpecificEnthalpy h "specific enthalpy";
  algorithm
    h := specificEnthalpy(
      setState_psX(
        p,
        s,
        X,
        phase));
    annotation(Documentation(info="<html></html>"));
  end specificEnthalpy_psX;

  redeclare replaceable function specificEnthalpy_pTX
    "Return specific enthalpy from pressure, temperature and mass fraction"
      extends Modelica.Icons.Function;
      input AbsolutePressure p "Pressure";
      input Temperature T "Temperature";
      input MassFraction X[nX] "Mass fractions";
      input FixedPhase phase=0
      "2 for two-phase, 1 for one-phase, 0 if not known";
      output SpecificEnthalpy h "Specific enthalpy at p, T, X";
  algorithm
    h := specificEnthalpy(
      setState_pTX(
        p,
        T,
        X,
        phase));
    annotation(Documentation(info="<html></html>"));
  end specificEnthalpy_pTX;

replaceable function specificEntropy_phX
    "Return specific entropy from p, h, and X or Xi"
  extends Modelica.Icons.Function;
  input AbsolutePressure p "Pressure";
  input SpecificEnthalpy h "Specific enthalpy";
  input MassFraction X[nX] "Mass fractions";
  input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
  output SpecificEntropy s "specific enthalpy";
algorithm
    s := specificEntropy(
      setState_phX(
      p,
      h,
      X,
      phase));
annotation(Documentation(info="<html></html>"));
end specificEntropy_phX;

  redeclare replaceable function temperature_phX
    "Return temperature from p, h, and X or Xi"
      extends Modelica.Icons.Function;
      input AbsolutePressure p "Pressure";
      input SpecificEnthalpy h "Specific enthalpy";
      input MassFraction X[nX] "Mass fractions";
      input FixedPhase phase=0
      "2 for two-phase, 1 for one-phase, 0 if not known";
      output Temperature T "Temperature";
  algorithm
    T := temperature(
      setState_phX(
        p,
        h,
        X,
        phase));
    annotation(Documentation(info="<html></html>"));
  end temperature_phX;

  redeclare replaceable function temperature_psX
    "Return temperature from p, s, and X or Xi"
      extends Modelica.Icons.Function;
      input AbsolutePressure p "Pressure";
      input SpecificEntropy s "Specific entropy";
      input MassFraction X[nX] "Mass fractions";
      input FixedPhase phase=0
      "2 for two-phase, 1 for one-phase, 0 if not known";
      output Temperature T "Temperature";
  algorithm
    T := temperature(
      setState_psX(
        p,
        s,
        X,
        phase));
    annotation(Documentation(info="<html></html>"));
  end temperature_psX;

  replaceable function setState_dT "Return thermodynamic state from d and T"
    extends Modelica.Icons.Function;
    input Density d "density";
    input Temperature T "Temperature";
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    output ThermodynamicState state "thermodynamic state record";
  algorithm
    assert(nX==1,"This function is not allowed for mixtures.");
    state :=
     setState_dTX(
        d,
        T,
        fill(0, 0),
        phase);
    annotation(Documentation(info="<html></html>"));
  end setState_dT;

  replaceable function setState_ph "Return thermodynamic state from p and h"
    extends Modelica.Icons.Function;
    input AbsolutePressure p "Pressure";
    input SpecificEnthalpy h "Specific enthalpy";
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    output ThermodynamicState state "thermodynamic state record";
  algorithm
    assert(nX==1,"This function is not allowed for mixtures.");
    state :=
      setState_phX(
        p,
        h,
        fill(0, 0),
        phase);
    annotation(Documentation(info="<html></html>"));
  end setState_ph;

  replaceable function setState_ps "Return thermodynamic state from p and s"
    extends Modelica.Icons.Function;
    input AbsolutePressure p "Pressure";
    input SpecificEntropy s "Specific entropy";
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    output ThermodynamicState state "thermodynamic state record";
  algorithm
    assert(nX==1,"This function is not allowed for mixtures.");
    state :=
      setState_psX(
        p,
        s,
        fill(0, 0),
        phase);
    annotation(Documentation(info="<html></html>"));
  end setState_ps;

  replaceable function setState_pT "Return thermodynamic state from p and T"
    extends Modelica.Icons.Function;
    input AbsolutePressure p "Pressure";
    input Temperature T "Temperature";
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    output ThermodynamicState state "thermodynamic state record";
  algorithm
    assert(nX==1,"This function is not allowed for mixtures.");
    state :=
      setState_pTX(
        p,
        T,
        fill(0, 0),
        phase);
    annotation(Documentation(info="<html></html>"));
  end setState_pT;

  replaceable function setState_px
    "Return thermodynamic state from pressure and vapour quality"
    input AbsolutePressure p "Pressure";
    input MassFraction x "Vapour quality";
    output ThermodynamicState state "Thermodynamic state record";
  algorithm
    assert(nX==1,"This function is not allowed for mixtures.");
    state := setState_ph(
        p,
        (1 - x)*bubbleEnthalpy(
        setSat_pX(p,{1})) +
        x*dewEnthalpy(
        setSat_pX(p,{1})),
        2);
    annotation(Documentation(info="<html></html>"));
  end setState_px;

  replaceable function setState_Tx
    "Return thermodynamic state from temperature and vapour quality"
    input Temperature T "Temperature";
    input MassFraction x "Vapour quality";
    output ThermodynamicState state "thermodynamic state record";
  algorithm
    assert(nX==1,"This function is not allowed for mixtures.");
    state := setState_ph(
        saturationPressure_sat(
        setSat_TX(T,{1})),
        (1 - x)*bubbleEnthalpy(
        setSat_TX(T,{1})) +
        x*dewEnthalpy(
        setSat_TX(T,{1})),
        2);
    annotation(Documentation(info="<html></html>"));
  end setState_Tx;

  replaceable function density_ph "Return density from p and h"
    extends Modelica.Icons.Function;
    input AbsolutePressure p "Pressure";
    input SpecificEnthalpy h "Specific enthalpy";
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    output Density d "Density";
  algorithm
    assert(nX==1,"This function is not allowed for mixtures. Use density_phX() instead!");
    d := density_phX(p, h, fill(0,0), phase);
    annotation(Documentation(info="<html></html>"));
  end density_ph;

  replaceable function specificEnthalpy_dT
    "Return specific enthalpy from d and T"
    extends Modelica.Icons.Function;
    input Density d "Density";
    input Temperature T "Temperature";
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    output SpecificEnthalpy h "specific enthalpy";
  algorithm
    assert(nX==1,"This function is not allowed for mixtures. Use specificEnthalpy_dX() instead!");
    h := specificEnthalpy(setState_dTX(
        d,
        T,
        fill(0, 0),
        phase));
    annotation(Documentation(info="<html></html>"));
  end specificEnthalpy_dT;

  replaceable function temperature_ph "Return temperature from p and h"
    extends Modelica.Icons.Function;
    input AbsolutePressure p "Pressure";
    input SpecificEnthalpy h "Specific enthalpy";
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    output Temperature T "Temperature";
  algorithm
    assert(nX==1,"This function is not allowed for mixtures. Use temperature_phX() instead!");
    T := temperature_phX(p, h, fill(0,0),phase);
    annotation(Documentation(info="<html></html>"));
  end temperature_ph;

  replaceable function pressure_dT "Return pressure from d and T"
    extends Modelica.Icons.Function;
    input Density d "Density";
    input Temperature T "Temperature";
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    output AbsolutePressure p "Pressure";
  algorithm
    assert(nX==1,"This function is not allowed for mixtures. Use pressure_dTX() instead!");
    p := pressure(setState_dTX(
        d,
        T,
        fill(0, 0),
        phase));
    annotation(Documentation(info="<html></html>"));
  end pressure_dT;

  replaceable function specificEnthalpy_ps
    "Return specific enthalpy from p and s"
    extends Modelica.Icons.Function;
    input AbsolutePressure p "Pressure";
    input SpecificEntropy s "Specific entropy";
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    output SpecificEnthalpy h "specific enthalpy";
  algorithm
    assert(nX==1,"This function is not allowed for mixtures. Use specificEnthalpy_psX() instead!");
    h := specificEnthalpy_psX(p,s,reference_X);
    annotation(Documentation(info="<html></html>"));
  end specificEnthalpy_ps;

  replaceable function temperature_ps "Return temperature from p and s"
    extends Modelica.Icons.Function;
    input AbsolutePressure p "Pressure";
    input SpecificEntropy s "Specific entropy";
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    output Temperature T "Temperature";
  algorithm
    assert(nX==1,"This function is not allowed for mixtures. Use temperature_psX() instead!");
    T := temperature_psX(p,s,fill(0,0),phase);
    annotation(Documentation(info="<html></html>"));
  end temperature_ps;

  replaceable function density_ps "Return density from p and s"
    extends Modelica.Icons.Function;
    input AbsolutePressure p "Pressure";
    input SpecificEntropy s "Specific entropy";
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    output Density d "Density";
  algorithm
    assert(nX==1,"This function is not allowed for mixtures. Use density_psX() instead!");
    d := density_psX(p, s, fill(0,0), phase);
    annotation(Documentation(info="<html></html>"));
  end density_ps;

  replaceable function specificEnthalpy_pT
    "Return specific enthalpy from p and T"
    extends Modelica.Icons.Function;
    input AbsolutePressure p "Pressure";
    input Temperature T "Temperature";
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    output SpecificEnthalpy h "specific enthalpy";
  algorithm
    assert(nX==1,"This function is not allowed for mixtures. Use specificEnthalpy_pTx() instead!");
    h := specificEnthalpy_pTX(p, T, fill(0,0),phase);
    annotation(Documentation(info="<html></html>"));
  end specificEnthalpy_pT;

  replaceable function density_pT "Return density from p and T"
    extends Modelica.Icons.Function;
    input AbsolutePressure p "Pressure";
    input Temperature T "Temperature";
    input FixedPhase phase=0 "2 for two-phase, 1 for one-phase, 0 if not known";
    output Density d "Density";
  algorithm
    d := density(
      setState_pTX(
        p,
        T,
        fill(0, 0),
        phase));
    annotation(Documentation(info="<html></html>"));
  end density_pT;

redeclare replaceable function density
  input ThermodynamicState state "Thermodynamic state record";
  output Density d;
algorithm
  d:=state.d;
end density;

redeclare replaceable function pressure
  input ThermodynamicState state "Thermodynamic state record";
  output AbsolutePressure p;
algorithm
  p:=state.p;
end pressure;

redeclare replaceable function specificEnthalpy
  input ThermodynamicState state "Thermodynamic state record";
  output SpecificEnthalpy h;
algorithm
  h:=state.h;
end specificEnthalpy;

redeclare replaceable function specificEntropy
  input ThermodynamicState state "Thermodynamic state record";
  output SpecificEntropy s;
algorithm
  s:=state.s;
end specificEntropy;

redeclare replaceable function temperature
  input ThermodynamicState state "Thermodynamic state record";
  output Temperature T;
algorithm
  T:=state.T;
end temperature;

  replaceable function vapourQuality "Return vapour quality"
    input ThermodynamicState state "Thermodynamic state record";
    output MassFraction x "Vapour quality";
  protected
    constant SpecificEnthalpy eps = 1e-8;
  algorithm
    x := min(max((specificEnthalpy(state) - bubbleEnthalpy(
      setSat_pX(
      pressure(state), state.X)))/(dewEnthalpy(
      setSat_pX(
      pressure(state), state.X)) - bubbleEnthalpy(
      setSat_pX(
      pressure(state), state.X)) + eps), 0), 1);
    annotation(Documentation(info="<html></html>"));
  end vapourQuality;

  replaceable partial function surfaceTension
    "Return surface tension sigma in the two phase region"
    extends Modelica.Icons.Function;
    input SaturationProperties sat "saturation property record";
    output SurfaceTension sigma "Surface tension sigma in the two phase region";
    annotation(Documentation(info="<html></html>"));
  end surfaceTension;

type DerPressureByDensity = Real (unit="m2/s2");
type DerDerPressureByDensityByDensity = Real (unit="(m5)/(kg.s2)");
type DerPressureByTemperature = Real (unit="kg/(K.m.s2)");
//type DerDensityByTemperature = Real (unit="kg/(m3.K)");
//type DerDensityByPressure = Real (unit="s2/m2");
type DerDerPressureByTemperatureByTemperature = Real (unit="kg/(m.s2.K2)");
type DerDerPressureByTemperatureByDensity = Real (unit="(m2)/(s2.K)");

type DerEnthalpyByDensity = Real (unit="J.m3/kg");
//type DerEnthalpyByPressure = Real (unit="J.m.s2/kg");
type DerEnthalpyByTemperature = Real (unit="J/K");

    annotation(Documentation(info="<html>
  <h1>PartialMixtureTwoPhaseMedium</h1>
  </html>
"));
end PartialMixtureTwoPhaseMedium;
