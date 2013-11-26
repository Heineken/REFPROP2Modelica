within REFPROP2Modelica.Testers;
model PropsMixtureTwo
package Medium = REFPROP2Modelica.Interfaces.REFPROPMixtureTwoPhaseMedium (
                               final substanceNames={"isobutan","propane"});
  Medium.BaseProperties props;
  Modelica.SIunits.Pressure psat=Medium.saturationPressure(300);
Modelica.SIunits.Pressure p(min=10,max=10e6)=Medium.pressure(props.state);
  Real q = Medium.vapourQuality(props.state);
  Modelica.SIunits.SpecificHeatCapacity cv=Medium.specificHeatCapacityCv(props.state);
  Medium.ThermodynamicState state=Medium.setState_phX(props.p,props.h,props.X);

  Medium.SaturationProperties sat = Medium.setSat_pX(props.p,props.X);
  Medium.SpecificEnthalpy hl = Medium.bubbleEnthalpy(sat);
  Medium.SpecificEnthalpy hv = Medium.dewEnthalpy(sat);
  Medium.Density dl = Medium.bubbleDensity(sat);
  Medium.Density dv = Medium.dewDensity(sat);

  Medium.ThermodynamicState state_l=Medium.setState_pqX(props.p,0,props.X);
  Medium.ThermodynamicState state_v=Medium.setState_pqX(props.p,1,props.X);

equation
    props.p = 1e5;
    props.h = 0+time*8e5;
    props.Xi = {.5};

end PropsMixtureTwo;
