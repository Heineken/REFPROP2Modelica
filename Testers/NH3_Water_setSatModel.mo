within REFPROP2Modelica.Testers;
model NH3_Water_setSatModel

package Medium = REFPROP2Modelica.Media.NH3_Water;

  Medium.ThermodynamicState dewstate;
  Medium.ThermodynamicState bubstate;

  Medium.SaturationProperties sat;
  Medium.SaturationProperties sat2;

  Medium.AbsolutePressure p;
  Medium.MassFraction X[2];

equation
  p=50e5;
  X={0.5,0.5};

  sat = Medium.setSat_pX(p,X);
  sat2 = Medium.setSat_TX(450,X);

  dewstate = Medium.setDewState(sat);
  bubstate = Medium.setBubbleState(sat);

end NH3_Water_setSatModel;
