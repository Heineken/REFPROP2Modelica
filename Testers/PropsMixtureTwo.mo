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
equation
    props.p = 1e5;
    props.h = 0+time*8e5;
    props.Xi = {.5};
end PropsMixtureTwo;
