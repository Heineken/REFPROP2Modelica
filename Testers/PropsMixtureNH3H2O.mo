within REFPROP2Modelica.Testers;
model PropsMixtureNH3H2O
package Medium = REFPROP2Modelica.Interfaces.REFPROPMixtureTwoPhaseMedium (
                               final substanceNames={"ammonia","water"},
                               inputChoice=REFPROP2Modelica.Interfaces.MixtureInputChoice.dTX);

  Medium.ThermodynamicState state;

  Modelica.SIunits.Density d;
  Modelica.SIunits.Temperature T;
  Real[2] X;

  Modelica.SIunits.AbsolutePressure p;
  Real q;

protected
  Modelica.SIunits.Temperature T_init = 400;
  Real[2] X_init = {0.5,0.5};
  Modelica.SIunits.AbsolutePressure p_init = Medium.pressure_TqX(T_init,0.5,X_init);
initial algorithm
  d := Medium.density_pTX(p_init,T_init,X_init);
equation
  der(d) = 0;
  X[1] = 0.25 + 0.5 * time;
  X[2] = 1 - X[1];
  T = 350 + 100 * time;
  state = Medium.setState_dTX(d,T,X);

  p = Medium.pressure(state);
  q = Medium.vapourQuality(state);

end PropsMixtureNH3H2O;
