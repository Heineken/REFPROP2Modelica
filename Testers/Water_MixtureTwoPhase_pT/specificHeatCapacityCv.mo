within REFPROP2Modelica.Testers.Water_MixtureTwoPhase_pT;
function extends specificHeatCapacityCv
  "specific heat capacity at constant pressure of water"
algorithm
  if Modelica.Media.Water.WaterIF97_base.dT_explicit then
    cv:=Modelica.Media.Water.IF97_Utilities.cv_dT(state.d, state.T, state.phase);
  elseif Modelica.Media.Water.WaterIF97_base.pT_explicit then
    cv:=Modelica.Media.Water.IF97_Utilities.cv_pT(state.p, state.T);
  else
    cv:=Modelica.Media.Water.IF97_Utilities.cv_ph(state.p, state.h, state.phase);
  end if;
  annotation(Documentation(info = "<html>
                                <p>In the two phase region this function returns the interpolated heat capacity between the
                                liquid and vapour state heat capacities.</p>
                                </html>"));
end specificHeatCapacityCv;

