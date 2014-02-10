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

Real dddX_ph_num;

equation
  p=50e5;
  h=3e5 + time*25e5;
  X={0.5,0.5};

  state = Medium.setState_phX(p,h,X,calcTransport=false,partialDersInputChoice=3);
  q = Medium.vapourQuality(state);
  d = Medium.density(state);

  dddh_pX = Medium.density_derh_p(state);
  dddh_pX_num = Medium.density(Medium.setState_phX(p,h+1,X))-d;

  dddp_hX = Medium.density_derp_h(state);
  dddp_hX_num = Medium.density(Medium.setState_phX(p+1,h,X))-d;

  dddX_ph_num = (Medium.density(Medium.setState_phX(p,h,cat(1,{X[1]+0.0001},{X[2]-0.0001})))-d)/0.0001;

end NH3_Water_setStateModel;
