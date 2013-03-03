within REFPROP2Modelica.Testers;
model PentaneTester "Evaporation of pentane at 1 bar"
  package Medium = REFPROP2Modelica.Media.Pentane (debugmode=true);
  Medium.BaseProperties props(h(start=300e3), d(start=1));
  Real drhodtime_num;
  Real drhodtime_ana;
  Real factor1;
  Real factor2;
  Real check1;
  Real check2;
  Real deltap;
  Real deltah;
  Real ddddp_num;
  Real ddddp_RP;
  Real ddddp_ana;
  Real ddddh_num;
  Real ddddh_RP;
  Real ddddh_ana;
equation
  deltap = 5;
  deltah = 5;
  props.p = 10e5;//1e5+3*time*4e5*(sin(20*time)+1)*0.5;
  props.h = -3e5 + time*7e5;

  // get derivatives for checking
  factor1 = der(props.p);
  factor2 = der(props.h);

  drhodtime_num = ddddp_num*factor1 + ddddh_num*factor2;
  ddddp_num = (Medium.density_phX(
    props.p + 0.5*deltap,
    props.h,
    props.X) - Medium.density_phX(
    props.p - 0.5*deltap,
    props.h,
    props.X))/deltap;
  ddddh_num = (Medium.density_phX(
    props.p,
    props.h + 0.5*deltah,
    props.X) - Medium.density_phX(
    props.p,
    props.h - 0.5*deltah,
    props.X))/deltah;

  drhodtime_ana = ddddp_ana*factor1 + ddddh_ana*factor2;
  -1 = ddddp_ana*1/(props.state.dhdp_rho)*props.state.dhdrho_p;
  ddddh_ana*props.state.dhdrho_p = 1;

  check1 = abs(ddddp_num - props.state.drhodp_h)/abs(ddddp_num)*100
    "difference of derivatives in percent";
  check2 = abs(ddddh_num - props.state.drhodh_p)/abs(ddddh_num)*100
    "difference of derivatives in percent";

  ddddh_RP = props.state.drhodh_p "drho/dh at constant pressure";
  ddddp_RP = props.state.drhodp_h "drho/dp at constant enthalpy";

end PentaneTester;
