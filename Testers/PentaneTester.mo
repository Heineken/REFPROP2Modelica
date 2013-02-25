within REFPROP2Modelica.Testers;
model PentaneTester "Evaporation of pentane at 1 bar"
package Medium = REFPROP2Modelica.Media.Pentane (debugmode=true);
// ",final explicitVars = "pd"";
//package Medium = REFPROP2Modelica.REFPROPMedium(final substanceNames={"CO2","water"});
  Medium.BaseProperties props(h(start=300e3),d(start=1));
//  Modelica.SIunits.Pressure psat=Medium.saturationPressure(300);
/*  Modelica.SIunits.SpecificEnthalpy h=Medium.specificEnthalpy_pTX(1e5,293,{.5,.5});
  Modelica.SIunits.Density d;
  Modelica.SIunits.SpecificEntropy s;
  Modelica.SIunits.Temperature Tsat;
  Modelica.SIunits.Pressure psat;
  */
//Modelica.SIunits.Pressure p=Medium.pressure(props.state);
//  Modelica.SIunits.MolarMass MM;
/*Modelica.SIunits.DynamicViscosity eta = Medium.dynamicViscosity(props.state);

Modelica.SIunits.DynamicViscosity eta_l = Medium.dynamicViscosity_liq(props.state);
Modelica.SIunits.DynamicViscosity eta_g = Medium.dynamicViscosity_gas(props.state);
*/
//  Real q = Medium.vapourQuality(props.state);
//  Modelica.SIunits.SpecificHeatCapacity cv=Medium.specificHeatCapacityCv(props.state);
//  Medium.ThermodynamicState state=Medium.setState_phX(props.p,props.h,props.X);
Real drhodtime_num;
Real drhodtime_ana;
  Real factor1;
  Real factor2;
  Real check1;
  Real check2;
  Real deltap;
  Real deltah;
  Real ddddp_num;
  Real ddddp_num_RP;
  Real ddddp_ana;
  Real ddddh_num;
  Real ddddh_num_RP;
  Real ddddh_ana;
equation
  deltap = 5;
  deltah = 5;
  props.p = 10e5;//1e5+3*time*4e5*(sin(20*time)+1)*0.5;
  props.h = -3e5+time*7e5;

  // get derivatives for checking
  factor1 = der(props.p);
  factor2 = der(props.h);

  drhodtime_num =  ddddp_num * factor1 + ddddh_num * factor2;
  ddddp_num = (Medium.density_phX(props.p+0.5*deltap,props.h,props.X)-Medium.density_phX(props.p-0.5*deltap,props.h,props.X)) / deltap;
  ddddh_num = (Medium.density_phX(props.p,props.h+0.5*deltah,props.X)-Medium.density_phX(props.p,props.h-0.5*deltah,props.X)) / deltah;

  drhodtime_ana =  ddddp_ana * factor1 + ddddh_ana * factor2;
  -1 = ddddp_ana * 1/(props.state.dhdp_rho) * props.state.dhdrho_p;
  ddddh_ana * props.state.dhdrho_p= 1;

  check1 = ddddp_num - props.state.drhodp_h;
  check2 = ddddh_num - props.state.drhodh_p;

  ddddh_num_RP = props.state.drhodh_p "drho/dh at constant pressure";
  ddddp_num_RP = props.state.drhodp_h "drho/dp at constant enthalpy";

  //  props.s = 5.88105;
//   props.T = 400;
//   props.state.x = 0.5;
//    props.Xi = {.5};
//    props.X = {.1,.9};
//    props.Xi = {.5};
  //  d = props.d;
  //h = props.h;
  //h = Medium.dewEnthalpy(props.sat);
  //d = Medium.density(props.state);
  //s = specificEntropy(props.state);
  //s = props.state.s;
//  MM = Medium.molarMass(props.state);
/*  
  Tsat = Medium.saturationTemperature(props.p,props.X);
//  Tsat = Medium.temperature_pqX(props.p,0.5,props.X);
  psat = Medium.saturationPressure(props.T,props.X);
//  psat = Medium.pressure_TqX(props.T,0.5,props.X);
//  h = Medium.dewEnthalpy(props.sat);
//  s = Medium.dewEntropy(props.sat);
  s = Medium.bubbleEntropy(props.sat);
//  d = Medium.dewDensity(props.sat);
  d = Medium.bubbleDensity(props.sat);
*/
end PentaneTester;
