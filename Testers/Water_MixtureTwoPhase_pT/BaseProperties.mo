within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
model extends BaseProperties "Base properties of medium"
  Real GVF = q * d / d_g "gas void fraction";
  Modelica.SIunits.Density d_l = Modelica.Media.Water.IF97_Utilities.rhol_T(T);
  Modelica.SIunits.Density d_g = Modelica.Media.Water.IF97_Utilities.rhov_T(T);
  /*  Modelica.SIunits.Density d_l = Modelica.Media.Water.IF97_Utilities.rhol_p(p);
  Modelica.SIunits.Density d_g = Modelica.Media.Water.IF97_Utilities.rhov_p(p);*/
  /*  Modelica.SIunits.Density d_l = Modelica.Media.Water.IF97_Utilities.BaseIF97.Regions.rhol_p(p);
  Modelica.SIunits.Density d_g = Modelica.Media.Water.IF97_Utilities.BaseIF97.Regions.rhov_p(p);*/
  Modelica.SIunits.SpecificEnthalpy h_l = bubbleEnthalpy(sat);
  Modelica.SIunits.SpecificEnthalpy h_g = dewEnthalpy(sat);
  Real q = min(max((h - h_l) / (h_g - h_l + 1e-18), 0), 1)
    "(min=0,max=1) gas phase mass fraction";
  //  Integer phase_out "calculated phase";
  //END no gas case
equation
  u = h - p / d;
  MM = M_H2O;
  R = Modelica.Constants.R / MM;
  //End GVF
  //DENSITY
  //  q = vapourQuality(state);
  d = Modelica.Media.Water.WaterIF97_base.density_ph(p, h);
  //  d = d_l/(1-q*(1-d_l/d_g));
  //End DENSITY
  //ENTHALPY
  h = specificEnthalpy_pTX(p, T, X);
  /*
      if (p_H2O>p) then
    h_H2O_g = Modelica.Media.Water.WaterIF97_base.specificEnthalpy_pT(p,T,1);
  else
    h_H2O_g = Modelica.Media.Water.WaterIF97_base.dewEnthalpy(Modelica.Media.Water.WaterIF97_base.setSat_p(p));
  end if;
  h_gas_dissolved = 0;
  Delta_h_solution = solutionEnthalpy(T) 
      "TODO: gilt nur bei ges�ttigter L�sung";
*/
  //assert(abs(((1-q)*h_l + q*h_g-h)/h) < 1e-3,"Enthalpie stimmt nicht! h_calc="+String((1-q)*h_l + q*h_g)+"<>h="+String(h));
  //End ENTHALPY
  s = 0 "TODO";
  state = ThermodynamicState(p=  p, T=  T, X=  X, X_l=  X, h=  h, GVF=  GVF, q=  q, s=  0, d_g=  d_g, d_l=  d_l, d=  d, phase=  0)
    "phase_out";
  sat.psat = p "TODO";
  sat.Tsat = T "saturationTemperature(p) TODO";
  sat.X = X;
  annotation(Documentation(info = "<html></html>"), Documentation(revisions = "<html>

</html>"));
end BaseProperties;

