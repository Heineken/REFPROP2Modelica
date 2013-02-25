within REFPROP2Modelica.Testers;
model R410mixTester "Density of saturated R410 vapour"
package Medium = REFPROP2Modelica.Media.R410mix(debugmode=true);
  Medium.BaseProperties props;
  Medium.Density d;
  Medium.SpecificEnthalpy h(start=300e3);
  Medium.AbsolutePressure p = Medium.pressure(props.state);
  Medium.SpecificHeatCapacity cv = Medium.specificHeatCapacityCv(props.state);
equation
    props.p = 101325;
    h =  Medium.dewEnthalpy(props.sat);
    props.h = h+0.2*h*0.5*(sin(5*time)+1);
    props.d = d;
    props.Xi = {0.697615};
end R410mixTester;
