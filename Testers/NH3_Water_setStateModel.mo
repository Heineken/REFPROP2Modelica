within REFPROP2Modelica.Testers;
model NH3_Water_setStateModel

package Medium = REFPROP2Modelica.Media.NH3_Water;

  Medium.ThermodynamicState state;
//  Medium.SaturationProperties sat;

 Medium.AbsolutePressure p;
 Medium.SpecificEnthalpy h;
 Medium.MassFraction X[2];
 Medium.Density d;

Medium.MassFraction Xdef[Medium.nX]=Medium.X_default;

Real q;

Real dddh_pX;
Real dddp_hX;
Real dddh_pX_num;
Real dddp_hX_num;

equation
  p=50e5;
  h=5e5 + time*25e5;
  X={0.2,0.8};

  state = Medium.setState_phX(p,h,X);
  q = Medium.vapourQuality(state);

  d = Medium.density(state);

  dddh_pX = Medium.density_derh_p(state);
  dddh_pX_num = Medium.density(Medium.setState_phX(p,h+1,X))-d;

  dddp_hX = Medium.density_derp_h(state);
  dddp_hX_num = Medium.density(Medium.setState_phX(p+1,h,X))-d;

end NH3_Water_setStateModel;
