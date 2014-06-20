within REFPROP2Modelica.Media;
package NH3_Water "Ammonia and water mixture by REFPROP library"

  extends Interfaces.REFPROPMixtureTwoPhaseMedium(
  final substanceNames={"ammoniaL","water"},
  final debugmode=false);

  replaceable function DynamicViscosityLiquid = dynamicViscosity_LIQconde constrainedby
    partialDynamicViscosityLiquid;
  replaceable function DynamicViscosityVapor = dynamicViscosity_VAPWilke constrainedby
    partialDynamicViscosityVapor;
  replaceable function ThermalConductivityLiquid = thermalConductivity_LIQConde
                                                                                constrainedby
    partialThermalConductivityLiquid;
  replaceable function ThermalConductivityVapor = thermalConductivity_VAPWilke constrainedby
    partialThermalConductivityVapor;

/*
  type ViscLiq = enumeration(
      Conde "Conde 2006",
      ElSayed "El-Sayed 1988",
      HdbKaltetechnik "Handbuch der Kältetechnik",
      SteccoDesideri "Stecco and Desideri 1991",
      TejaRice "Teja and Rice (in Poling 2001)",
      TejaRiceSassen "Teja and Rice using Sassen critical props",
      TejaRiceFit "Teja and Rice (fit others with psi=1.6)") 
    "Liquid viscosity methods";
  constant ViscLiq viscLiq = ViscLiq.Conde annotation(evaluate=true);

  type ViscVap = enumeration(
      Wilke "Wilke (in Poling 2001)",
      Reichenberg "Reichenberg (in Poling 2001)",
      Chung "Chung et al. (in Poling 2001)",
      ChungErrorWeight 
        "Chung et al. incl. mole-fraction avg. error correction (in Poling 2001)")
    "Vapor viscosity methods";
  constant ViscVap viscVap = ViscVap.Wilke;

  type CondLiq = enumeration(
      Conde "Conde 2006",
      ElSayed "El-Sayed 1988",
      Filippov "Filippov (in Poling 2001)",
      Jamieson "(in Poling 2001)") "Liquid conductivity methods";
  constant CondLiq condLiq = CondLiq.Conde;

  type CondVap = enumeration(
      Average "Mole-fraction average (suggested in Poling 2001)",
      MasonSaxena "Mason and Saxena (in Poling 2001)",
      Chung "Chung et al. (in Poling 2001)",
      ChungErrorWeight 
        "Chung et al. incl. mole-fraction avg. error correction (in Poling 2001)")
    "Vapor conductivity methods";
  constant CondVap condVap = CondVap.Average;
*/

  function criticalProperties
    input MassFraction X[:] "mass fraction m_NaCl/m_Sol";
    output Modelica.SIunits.Temperature Tc "critical temperature";
    output Modelica.SIunits.Pressure pc "critical pressure";
    output Modelica.SIunits.Density dc "critical density";
    output MoleFraction[nX] Z "mole fraction";

  protected
    Real[3 + 1*nX] critprops;
    String errormsg=StrJoin(fill("xxxx", 64), "");

  package Internal
  function InternalFunction
    input String fluidnames;
    input Real[:] critprops;
    input MassFraction X[:] "mass fraction m_NaCl/m_Sol";
    input String errormsg;
    output Real val;

  external "C" val=  critprops_REFPROP(
        fluidnames,
        critprops,
        X,
        REFPROP_PATH,
        errormsg,
        debugmode)  annotation (Include="#include <refprop_wrapper.h>", Library="refprop_wrapper");
    annotation(Inline=true);
  end InternalFunction;
  end Internal;

  algorithm
    Internal.InternalFunction(
      fluidnames,
      critprops,
      X,
      errormsg);

      Tc:=critprops[1];
      pc:=critprops[2];
      dc:=critprops[3];
      Z:=critprops[4:4+nX-1];

    annotation(Inline=true,LateInline=true);
  end criticalProperties;

  redeclare function dynamicViscosity
    "Return dynamic viscosity, LIQ=CONDE, VAP=WILKE"
    extends Modelica.Icons.Function;
      input ThermodynamicState state "thermodynamic state record";
      output DynamicViscosity eta "Dynamic viscosity";

  algorithm
    if state.q<=0.01 then // liquid
      eta := DynamicViscosityLiquid(state);
    elseif state.q>=0.99 then // vapor
      eta := DynamicViscosityVapor(state);
    else // twophase
      eta:=1e-6;
    end if;

  end dynamicViscosity;

  redeclare function thermalConductivity
    "Return thermal conductivity, LIQ=Jamison, VAP=MOLE FRACTION AVG"
    extends Modelica.Icons.Function;
    input ThermodynamicState state "thermodynamic state record";
    output ThermalConductivity lambda "thermal conductivity";

  algorithm
    if state.q <= 0.01 then
      // liquid
      lambda :=ThermalConductivityLiquid(state);
    elseif state.q >= 0.99 then
      // vapor
      lambda :=ThermalConductivityVapor(state);
    else
      // twophase
      lambda := 1e-6;
    end if;

  end thermalConductivity;

  function criticalProperties_sassen1990
    input MassFraction X[:] "mass fraction m_NaCl/m_Sol";
    output Modelica.SIunits.Temperature Tc "critical temperature";
    output Modelica.SIunits.Pressure pc "critical pressure";
  //  output Modelica.SIunits.Density dc "critical density";
    output MoleFraction[nX] Z "mole fraction";

  protected
    constant Real Mw=18.0153; // Molar mass of water, g/mol
    constant Real Ma=17.0305; // Molar mass of ammonia, g/mol

  algorithm
    Z[1] :=X[1]/(X[1] + (Ma*(1 - X[1])/Mw));
    Z[2] :=(1 - X[1])/((1 - X[1]) + ((X[1]*Mw)/Ma));

    Tc :=647.14 - 199.822371*Z[1] + 109.035522*Z[1]^2 - 239.626217*Z[1]^3 + 88.689691*Z[1]^4;
    pc :=220.64*1e5 - 37.923795*1e5*Z[1] + 36.424739*1e5*Z[1]^2 -41.851597*1e5*Z[1]^3 -63.805617*1e5*Z[1]^4;

    annotation(Inline=true,LateInline=true);
  end criticalProperties_sassen1990;

  partial function partialDynamicViscosityLiquid
  end partialDynamicViscosityLiquid;

  function dynamicViscosity_LIQconde "Conde 2006"
    extends partialDynamicViscosityLiquid;
    extends Modelica.Icons.Function;
      input ThermodynamicState state "thermodynamic state record";
      output DynamicViscosity eta "Dynamic viscosity";

  protected
    ThermodynamicState state_w;
    ThermodynamicState state_a;

    Modelica.SIunits.Temperature Tc "critical temperature";
    Modelica.SIunits.Pressure pc "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc_a = 405.560;
    constant Modelica.SIunits.Temperature Tc_w = 647.096;
    Modelica.SIunits.Temperature Tcorr_a "Corresponding temp. for ammonia, K";
    Modelica.SIunits.Temperature Tcorr_w "Corresponding temp. for water, K";

    Modelica.SIunits.DynamicViscosity mu_a;
    Modelica.SIunits.DynamicViscosity mu_w;

    Real F1;
    Real F2;
    Real F12;
    Real F21;

    Real F_x;
    Real DELTAeta;

  algorithm
    (Tc,pc,Z) := criticalProperties_sassen1990(state.X);

    Tcorr_a := max(195.5,state.T*Tc_a/Tc) "Corresponding temp. for ammonia, K";
    Tcorr_w := max(273.16,state.T*Tc_w/Tc) "Corresponding temp. for water, K";

    if debugmode then
        Modelica.Utilities.Streams.print("got (Tc,pc,Tcorr_a,Tcorr_w), (" + String(Tc) + "," + String(pc) + "," + String(Tcorr_a) + "," + String(Tcorr_w) + ")");
    end if;

    state_a :=setState_TqX(
      min(Tc_a-1,Tcorr_a),
      0,
      {1,0},
      calcTransport=true);
    state_w :=setState_TqX(
      min(Tcorr_w,Tc_w-1),
      0,
      {0,1},
    calcTransport=true);

    mu_a :=state_a.eta;
    mu_w :=state_w.eta;

    if debugmode then
      Modelica.Utilities.Streams.print("got (mu_a,mu_w,lambda_a,lambda_w), (" + String(mu_a) + "," + String(mu_w) + ")");
    end if;

    mu_a := mu_a*1e6; //convert to (1e-6 Pa.s)
    mu_w := mu_w*1e6; //convert to (1e-6 Pa.s)

    F_x :=6.38*((1 - Z[1])^(1.125*Z[1]))*(1 - exp(-0.585*Z[1]*(1 - Z[1])^0.18))*
      log((mu_a^0.5)*(mu_w^0.5));
    DELTAeta:=(0.534 - 0.815*(state.T/Tc_w))*F_x;
    eta :=exp(Z[1]*log(mu_a) + (1 - Z[1])*log(mu_w) + DELTAeta);
    eta := eta/1e6; // convert to Pa.s

  end dynamicViscosity_LIQconde;

  function dynamicViscosity_LIQelsayed
    "El-Sayed 1988 (correct equations by Thorin 2001)"
    extends partialDynamicViscosityLiquid;
    extends Modelica.Icons.Function;
      input ThermodynamicState state "thermodynamic state record";
      output DynamicViscosity eta "Dynamic viscosity";

  protected
    ThermodynamicState state_w;
    ThermodynamicState state_a;

    Modelica.SIunits.Temperature Tc "critical temperature";
    Modelica.SIunits.Pressure pc "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc_a = 405.560;
    constant Modelica.SIunits.Temperature Tc_w = 647.096;
    Modelica.SIunits.Temperature Tcorr_a "Corresponding temp. for ammonia, K";
    Modelica.SIunits.Temperature Tcorr_w "Corresponding temp. for water, K";

    Modelica.SIunits.DynamicViscosity mu_a;
    Modelica.SIunits.DynamicViscosity mu_w;

    Real F1;
    Real F2;
    Real F12;
    Real F21;

    Real F_x;
    Real F_t;
    Real DELTAeta;

  algorithm
    (Tc,pc,Z) :=criticalProperties_sassen1990(state.X);

    Tcorr_a := max(195.5,state.T*Tc_a/Tc) "Corresponding temp. for ammonia, K";
    Tcorr_w := max(273.16,state.T*Tc_w/Tc) "Corresponding temp. for water, K";

    if debugmode then
        Modelica.Utilities.Streams.print("got (Tc,pc,Tcorr_a,Tcorr_w), (" + String(Tc) + "," + String(pc) + "," + String(Tcorr_a) + "," + String(Tcorr_w) + ")");
    end if;

    state_a :=setState_TqX(
      min(Tc_a-1,Tcorr_a),
      0,
      {1,0},
      calcTransport=true);
    state_w :=setState_TqX(
      min(Tcorr_w,Tc_w-1),
      0,
      {0,1},
      calcTransport=true);

    mu_a :=state_a.eta;
    mu_w :=state_w.eta;

    if debugmode then
      Modelica.Utilities.Streams.print("got (mu_a,mu_w,lambda_a,lambda_w), (" + String(mu_a) + "," + String(mu_w) + ")");
    end if;

    mu_a := mu_a*1e6; //convert to (1e-6 Pa.s)
    mu_w := mu_w*1e6; //convert to (1e-6 Pa.s)

    F_x:=(Z[1]*Z[2] - 0.125*(Z[1]^2)*Z[2])*((log(mu_a*mu_w))^0.5);
    F_t:=4.219 - 3.7996*(state.T*(9/5)/492) + 0.842*(state.T*(9/5)/492)^2;
    F12:=F_t*F_x;
    eta :=(exp(Z[1]*log(mu_a) + Z[2]*log(mu_w) + F12));
    eta := eta/1e6; // convert to Pa.s

  end dynamicViscosity_LIQelsayed;

  function dynamicViscosity_LIQHDK "Handbuch der Kältetechnik"
    extends partialDynamicViscosityLiquid;
      extends Modelica.Icons.Function;
      input ThermodynamicState state "thermodynamic state record";
      output DynamicViscosity eta "Dynamic viscosity";

  algorithm
      eta :=(10^(10^((2000/(500 + state.T - 273.15)) - 4.41 + 0.925*state.X[1] - 1.743*
        state.X[1]^2 + 0.021*state.X[1]^3)) - 1)*1e-3;

  end dynamicViscosity_LIQHDK;

  function dynamicViscosity_LIQTejeRice "Teja and Rice (in Poling 2001)"
    extends partialDynamicViscosityLiquid;
      extends Modelica.Icons.Function;
      input ThermodynamicState state "thermodynamic state record";
      output DynamicViscosity eta "Dynamic viscosity";

  protected
    ThermodynamicState states[2];

    Modelica.SIunits.Temperature Tc_actual "critical temperature";
    Modelica.SIunits.Pressure pc_actual "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc[2] = {405.560,647.096}; // critical T, K
    constant Real MM[2]= {17.0305,18.0153}; // Molar mass, kg/kmol
    constant Real Vc[2] = {75.6889,55.9536}; // critical volume, mol/cm3

    Modelica.SIunits.DynamicViscosity etas[2];

    Real yi,yj,psi_ij,epsilon_i,epsilon_j,Vc_ij,Vcm, Tc_ij, Tcm, T_i, T_j, M_m, epsilon_m;

  algorithm
    (Tc_actual,pc_actual,Z) :=criticalProperties_sassen1990(state.X);

    yi := Z[1];
    yj := Z[2];

    psi_ij := 1;// 8 as suggested by el-sayed???

    epsilon_i := Vc[1]^(2/3)/(Tc[1]*MM[1])^0.5; //               "9-13.18"
    epsilon_j := Vc[2]^(2/3)/(Tc[2]*MM[2])^0.5; //               "9-13.18"

    Vc_ij := (Vc[1]^(1/3) + Vc[2]^(1/3))^3/8;
                                           //               "9-13.23"
    Vcm := yi*yi*Vc[1] + yj*yj*Vc[2] + 2*yi*yj*Vc_ij; //               "9-13.19"

    Tc_ij := psi_ij*(Tc[1]*Tc[2]*Vc[1]*Vc[2])^0.5/Vc_ij; //               "9-13.24"
    Tcm := (yi*yi*Tc[1]*Vc[1] + yj*yj*Tc[2]*Vc[2] + 2*yi*yj*Tc_ij*Vc_ij)/Vcm; //       "9-13.19"

    T_i := max(195.5,Tc[1]/Tcm*state.T);
    T_j := max(273.16,Tc[2]/Tcm*state.T);

    states[1] := setState_TqX(
      min(Tc[1]-1,T_i),
      0,
      {1,0},
      calcTransport=true);
    states[2] := setState_TqX(
      min(T_j,Tc[2]-1),
      0,
      {0,1},
      calcTransport=true);
    etas[1] := states[1].eta;
    etas[2] := states[2].eta;

    M_m :=yi*MM[1] + yj*MM[2];             //   "9-13.21"
    epsilon_m :=Vcm^(2/3)/(Tcm*M_m)^0.5;     //            "9-13.18"

    eta :=exp(yi*log(etas[1]*epsilon_i) + yj*log(etas[2]*epsilon_j))/epsilon_m;       // "9-13.25"

  end dynamicViscosity_LIQTejeRice;

  function dynamicViscosity_LIQTejeRiceSteccoWay "Stecco and Desideri 1991"
    extends partialDynamicViscosityLiquid;
    extends Modelica.Icons.Function;
      input ThermodynamicState state "thermodynamic state record";
      output DynamicViscosity eta "Dynamic viscosity";

  protected
    ThermodynamicState states[2];

    Modelica.SIunits.Temperature Tc_actual "critical temperature";
    Modelica.SIunits.Pressure pc_actual "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc[2] = {405.560,647.096}; // critical T, K
    constant Real MM[2]= {17.0305,18.0153}; // Molar mass, kg/kmol
    constant Real Vc[2] = {75.6889,55.9536}; // critical volume, mol/cm3

    Modelica.SIunits.DynamicViscosity etas[2];

  Real yi,yj,psi_ij,epsilon_i,epsilon_j,Vc_ij,Vcm, Tc_ij, Tcm, T_i, T_j, M_m, epsilon_m;

  algorithm
    (Tc_actual,pc_actual,Z) :=criticalProperties_sassen1990(state.X);

    yi := Z[1];
    yj := Z[2];

    psi_ij := 8;// 8 as suggested by el-sayed???

    epsilon_i := Vc[1]^(2/3)/(Tc[1]*MM[1])^0.5; //               "9-13.18"
    epsilon_j := Vc[2]^(2/3)/(Tc[2]*MM[2])^0.5; //               "9-13.18"

    Vc_ij := (Vc[1]^(1/3) + Vc[2]^(1/3))^3/8;
                                           //               "9-13.23"
    Vcm := yi*yi*Vc[1] + yj*yj*Vc[2] + 2*yi*yj*Vc_ij; //               "9-13.19"

    Tc_ij := psi_ij*(Tc[1]*Tc[2]*Vc[1]*Vc[2])^0.5/Vc_ij; //               "9-13.24"
    Tcm := (yi*yi*Tc[1]*Vc[1] + yj*yj*Tc[2]*Vc[2] + 2*yi*yj*Tc_ij*Vc_ij)/Vcm; //       "9-13.19"

    T_i := max(195.5,state.T);
    T_j := max(273.16,state.T);

    states[1] := setState_TqX(
      min(Tc[1]-1,T_i),
      0,
      {1,0},
      calcTransport=true);
    states[2] := setState_TqX(
      min(T_j,Tc[2]-1),
      0,
      {0,1},
      calcTransport=true);
    etas[1] := states[1].eta;
    etas[2] := states[2].eta;

    M_m :=yi*MM[1] + yj*MM[2];             //   "9-13.21"
    epsilon_m :=Vcm^(2/3)/(Tcm*M_m)^0.5;     //            "9-13.18"

    eta :=exp(yi*log(etas[1]*epsilon_i) + yj*log(etas[2]*epsilon_j))/epsilon_m;       // "9-13.25"

  end dynamicViscosity_LIQTejeRiceSteccoWay;

  function dynamicViscosity_LIQTejeRiceSassensTcrit
    "Teja and Rice using Sassen critical props"
    extends partialDynamicViscosityLiquid;
    extends Modelica.Icons.Function;
      input ThermodynamicState state "thermodynamic state record";
      output DynamicViscosity eta "Dynamic viscosity";
  protected
    ThermodynamicState states[2];

    Modelica.SIunits.Temperature Tc_actual "critical temperature";
    Modelica.SIunits.Pressure pc_actual "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc[2] = {405.560,647.096}; // critical T, K
    constant Real MM[2]= {17.0305,18.0153}; // Molar mass, kg/kmol
    constant Real Vc[2] = {75.6889,55.9536}; // critical volume, mol/cm3

    Modelica.SIunits.DynamicViscosity etas[2];

  Real yi,yj,psi_ij,epsilon_i,epsilon_j,Vc_ij,Vcm, Tc_ij, Tcm, T_i, T_j, M_m, epsilon_m;

  algorithm
    (Tc_actual,pc_actual,Z) :=criticalProperties_sassen1990(state.X);

    yi := Z[1];
    yj := Z[2];

    epsilon_i := Vc[1]^(2/3)/(Tc[1]*MM[1])^0.5; //               "9-13.18"
    epsilon_j := Vc[2]^(2/3)/(Tc[2]*MM[2])^0.5; //               "9-13.18"

    Vc_ij := (Vc[1]^(1/3) + Vc[2]^(1/3))^3/8;                                       //               "9-13.23"
    Vcm := yi*yi*Vc[1] + yj*yj*Vc[2] + 2*yi*yj*Vc_ij; //               "9-13.19"

   Tcm:=Tc_actual;
   T_i := max(195.5,state.T*Tc[1]/Tc_actual)
      "Corresponding temp. for ammonia, K";
   T_j := max(273.16,state.T*Tc[2]/Tc_actual)
      "Corresponding temp. for water, K";

    states[1] := setState_TqX(
      min(Tc[1]-1,T_i),
      0,
      {1,0},
      calcTransport=true);
    states[2] := setState_TqX(
      min(T_j,Tc[2]-1),
      0,
      {0,1},
      calcTransport=true);
    etas[1] := states[1].eta;
    etas[2] := states[2].eta;

    M_m :=yi*MM[1] + yj*MM[2];             //   "9-13.21"
    epsilon_m :=Vcm^(2/3)/(Tcm*M_m)^0.5;     //            "9-13.18"

    eta :=exp(yi*log(etas[1]*epsilon_i) + yj*log(etas[2]*epsilon_j))/epsilon_m;       // "9-13.25"

  end dynamicViscosity_LIQTejeRiceSassensTcrit;

  function dynamicViscosity_LIQTejeRiceFit
    "Teja and Rice (fit others with psi=1.6)"
    extends partialDynamicViscosityLiquid;
    extends Modelica.Icons.Function;
      input ThermodynamicState state "thermodynamic state record";
      output DynamicViscosity eta "Dynamic viscosity";

  protected
    ThermodynamicState states[2];

    Modelica.SIunits.Temperature Tc_actual "critical temperature";
    Modelica.SIunits.Pressure pc_actual "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc[2] = {405.560,647.096}; // critical T, K
    constant Real MM[2]= {17.0305,18.0153}; // Molar mass, kg/kmol
    constant Real Vc[2] = {75.6889,55.9536}; // critical volume, mol/cm3

    Modelica.SIunits.DynamicViscosity etas[2];

  Real yi,yj,psi_ij,epsilon_i,epsilon_j,Vc_ij,Vcm, Tc_ij, Tcm, T_i, T_j, M_m, epsilon_m;

  algorithm
    (Tc_actual,pc_actual,Z) :=criticalProperties_sassen1990(state.X);

    yi := Z[1];
    yj := Z[2];

    psi_ij := 1.6;// 8 as suggested by el-sayed???

    epsilon_i := Vc[1]^(2/3)/(Tc[1]*MM[1])^0.5; //               "9-13.18"
    epsilon_j := Vc[2]^(2/3)/(Tc[2]*MM[2])^0.5; //               "9-13.18"

    Vc_ij := (Vc[1]^(1/3) + Vc[2]^(1/3))^3/8;
                                           //               "9-13.23"
    Vcm := yi*yi*Vc[1] + yj*yj*Vc[2] + 2*yi*yj*Vc_ij; //               "9-13.19"

    Tc_ij := psi_ij*(Tc[1]*Tc[2]*Vc[1]*Vc[2])^0.5/Vc_ij; //               "9-13.24"
    Tcm := (yi*yi*Tc[1]*Vc[1] + yj*yj*Tc[2]*Vc[2] + 2*yi*yj*Tc_ij*Vc_ij)/Vcm; //       "9-13.19"

    T_i := max(195.5,Tc[1]/Tcm*state.T);
    T_j := max(273.16,Tc[2]/Tcm*state.T);

    states[1] := setState_TqX(
      min(Tc[1]-1,T_i),
      0,
      {1,0},
      calcTransport=true);
    states[2] := setState_TqX(
      min(T_j,Tc[2]-1),
      0,
      {0,1},
      calcTransport=true);
    etas[1] := states[1].eta;
    etas[2] := states[2].eta;

    M_m :=yi*MM[1] + yj*MM[2];             //   "9-13.21"
    epsilon_m :=Vcm^(2/3)/(Tcm*M_m)^0.5;     //            "9-13.18"

    eta :=exp(yi*log(etas[1]*epsilon_i) + yj*log(etas[2]*epsilon_j))/epsilon_m;       // "9-13.25"

  end dynamicViscosity_LIQTejeRiceFit;

  partial function partialDynamicViscosityVapor
  end partialDynamicViscosityVapor;

  function dynamicViscosity_VAPReichenberg "Reichenberg (in Poling 2001)"
  extends partialDynamicViscosityVapor;
    extends Modelica.Icons.Function;
      input ThermodynamicState state "thermodynamic state record";
      output DynamicViscosity eta "Dynamic viscosity";

  protected
    ThermodynamicState state_w;
    ThermodynamicState state_a;
    SaturationProperties sat_w;

    Modelica.SIunits.Temperature Tc "critical temperature";
    Modelica.SIunits.Pressure pc "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc_a = 405.560;
    constant Modelica.SIunits.Temperature Tc_w = 647.096;
    constant Modelica.SIunits.Pressure pc_a = 11333000;
    constant Modelica.SIunits.Pressure pc_w = 22064000;
    constant Real Mw=18.0153; // Molar mass of water, g/mol
    constant Real Ma=17.0305; // Molar mass of ammonia, g/mol
    constant Real mu_1 = 1.470; // dipole moment
    constant Real mu_2 = 1.855; // dipole moment

    Modelica.SIunits.DynamicViscosity eta_a;
    Modelica.SIunits.DynamicViscosity eta_w;

    Real Tr_1;
    Real Tr_2;
    Real Tr_12;
    Real mu_r1;
    Real mu_r2;
    Real mu_r12;
    Real U_1;
    Real U_2;
    Real C_1;
    Real C_2;
    Real H_12;
    Real K_1;
    Real K_2;

  algorithm
    (Tc,pc,Z) :=criticalProperties_sassen1990(state.X);

   // it will always be vapor or supercritical!
    state_a :=setState_pTX(
      state.p,
      state.T,
      {1,0},
      calcTransport=true);

    if state.T < Tc_w-1 then //
      sat_w := setSat_TX(state.T, {0,1});
      state_w :=setState_pTX(
      min(state.p,sat_w.psat-1),
      state.T,
      {0,1},
      calcTransport=true);
    else
      state_w :=setState_pTX(
      state.p,
      state.T,
      {0,1},
      calcTransport=true);
    end if;

    eta_a :=state_a.eta;
    eta_w :=state_w.eta;

    eta_a:=eta_a*1e6; //convert to (1e-6 Pa.s)
    eta_w:=eta_w*1e6; //convert to (1e-6 Pa.s)

    Tr_1 := state.T/Tc_a "Reduced temperature of ammonia";
    Tr_2 := state.T/Tc_w "Reduced temperature of water";
    Tr_12 := state.T/(sqrt(Tc_a*Tc_w));
    mu_r1:=52.46*(((mu_1^2)*pc_a)/Tc_a^2);
    mu_r2:=52.46*(((mu_2^2)*pc_w)/Tc_w^2);
    mu_r12:=sqrt(mu_r1*mu_r2);
    U_1:=(((1 + 0.36*Tr_1*(Tr_1 - 1))^(1/6))/sqrt(Tr_1))*(((Tr_1^3.5) + (10*
      mu_r1)^7)/((Tr_1^3.5)*(1 + (10*mu_r1)^7)));
    U_2:=(((1 + 0.36*Tr_2*(Tr_2 - 1))^(1/6))/sqrt(Tr_2))*(((Tr_2^3.5) + (10*
      mu_r2)^7)/((Tr_2^3.5)*(1 + (10*mu_r2)^7)));
    C_1:=(Ma^(1/4))/sqrt(eta_a*U_1);
    C_2:=(Mw^(1/4))/sqrt(eta_w*U_2);
    H_12:= (sqrt((Ma*Mw)/32)/((Ma+Mw)^(3/2)))*(((1+0.36*Tr_12*(Tr_12-1))^(1/6))/sqrt(Tr_12))*((C_1+C_2)^2)*(((Tr_12^3.5)+(10*mu_r12)^7)/((Tr_12^3.5)*(1+(10*mu_r12)^7)));
    K_1:=(Z[1]*eta_a)/(Z[1]+eta_a*(Z[2]*H_12*(3+2*(Mw/Ma))));
    K_2:=(Z[2]*eta_w)/(Z[2]+eta_w*(Z[1]*H_12*(3+(2*Ma/Mw))));
    eta:=(K_1*(1+H_12^2*K_2^2)+K_2*(1+2*H_12*K_1+H_12^2*K_1^2));

    eta := eta/1e6; // convert to Pa.s

  end dynamicViscosity_VAPReichenberg;

  function dynamicViscosity_VAPWilke "Wilke (in Poling 2001)"
    extends partialDynamicViscosityVapor;
    extends Modelica.Icons.Function;
      input ThermodynamicState state "thermodynamic state record";
      output DynamicViscosity eta "Dynamic viscosity";

  protected
    ThermodynamicState state_w;
    ThermodynamicState state_a;
    SaturationProperties sat_w;

    Modelica.SIunits.Temperature Tc "critical temperature";
    Modelica.SIunits.Pressure pc "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc_a = 405.560;
    constant Modelica.SIunits.Temperature Tc_w = 647.096;
    constant Modelica.SIunits.Pressure pc_a = 11333000;
    constant Modelica.SIunits.Pressure pc_w = 22064000;
    constant Real Mw=18.0153; // Molar mass of water, g/mol
    constant Real Ma=17.0305; // Molar mass of ammonia, g/mol

    Real F12,F21;

    Modelica.SIunits.DynamicViscosity mu_a;
    Modelica.SIunits.DynamicViscosity mu_w;

  algorithm
    (Tc,pc,Z) :=criticalProperties_sassen1990(state.X);

   // it will always be vapor or supercritical!
    state_a :=setState_pTX(
      state.p,
      state.T,
      {1,0},
      calcTransport=true);

    if state.T < Tc_w-1 then //
      sat_w := setSat_TX(state.T, {0,1});
      state_w :=setState_pTX(
      min(state.p,sat_w.psat-1),
      state.T,
      {0,1},
      calcTransport=true);
    else
      state_w :=setState_pTX(
      state.p,
      state.T,
      {0,1},
      calcTransport=true);
    end if;

    mu_a :=state_a.eta;
    mu_w :=state_w.eta;

    mu_a:=mu_a*1e6; //convert to (1e-6 Pa.s)
    mu_w:=mu_w*1e6; //convert to (1e-6 Pa.s)

    F12 := ((1 + (sqrt(mu_a/mu_w)*((Mw/Ma)^0.25)))^2)/sqrt(8*(1 + (Ma/Mw)));
    F21 := F12*(mu_w/mu_a)*(Ma/Mw);
    eta:= ((mu_a*Z[1])/(Z[1] + (F12*Z[2]))) + ((mu_w*Z[2])/(Z[2] + (F21*Z[1])));     // Mixture vapour dynamic visc., microPa s, Thorin, 2001
    eta := eta/1e6; // convert to Pa.s

  end dynamicViscosity_VAPWilke;

  function dynamicViscosity_VAPChung "Chung et al. (in Poling 2001)"
    extends partialDynamicViscosityVapor;

    extends Modelica.Icons.Function;
      input ThermodynamicState state "thermodynamic state record";
      output DynamicViscosity eta "Dynamic viscosity";

  protected
    constant Modelica.SIunits.Temperature Tc[2] = {405.560,647.096}; // critical T, K
    constant Real MM[2]= {17.0305,18.0153}; // Molar mass, kg/kmol
  //  constant Real DM[2] = {1.470,1.855}; // dipole moment, debye
    constant Real DM[2] = {1.5,1.8}; // dipole moment, debye
    constant Real omega[2] = {0.2558,0.3443}; // accentric factor
    constant Real Vc[2] = {75.6889,55.9536}; // critical volume, mol/cm3
    constant Real kappa[2] = {0.0,0.076}; // hydrogen bonding or hydrogen association factor

    Modelica.SIunits.Temperature Tc_dum "critical temperature";
    Modelica.SIunits.Pressure pc_dum "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";
    Real yi,yj;

    Real sigma_i,sigma_j,sigma_ij,sigma_m;
    Real epsilon_i_K,epsilon_j_K,epsilon_ij_K,epsilon_m_K;
    Real M_ij,M_m,omega_ij,omega_m;
    Real DM_m, Vcm, Tcm, DM_rm, kappa_m, F_cm, T_star_m, OMEGA_v;
    Real MMm, rho, y, G1, G2, eta_star_star, eta_star;
    Real E[10];

    constant Real a[10] = {6.324,1.210e-3,5.283,6.623,19.745,-1.900,24.275,0.7972,-0.2382,0.06863};
    constant Real b[10] = {50.412,-1.154e-3,254.209,38.096,7.630,-12.537,3.450,1.117,0.06770,0.3479};
    constant Real c[10] = {-51.680,-6.257e-3,-168.48,-8.464,-14.354,4.985,-11.291,0.01235,-0.8163,0.5926};
    constant Real d[10] = {1189.0,0.03728,3898.0,31.42,31.53,-18.15,69.35,-4.117,4.025,-0.727};

  algorithm
    (Tc_dum,pc_dum,Z) :=criticalProperties_sassen1990(state.X);

    yi :=Z[1];
    yj :=Z[2];

    sigma_i:=0.809*Vc[1]^(1/3);                                                  // "9-5.32";
    sigma_j:=0.809*Vc[2]^(1/3);                                                  // "9-5.32"
    sigma_ij:=(sigma_i*sigma_j)^0.5;                                             // "9-5.33"
    sigma_m :=(yi*yi*sigma_i^3 + yj*yj*sigma_j^3 + 2*yi*yj*sigma_ij^3)^(1/3);    // "9-5.24"

    epsilon_i_K :=Tc[1]/1.2593;                                                  // "9-5.34"
    epsilon_j_K :=Tc[2]/1.2593;                                                  // "9-5.34"
    epsilon_ij_K :=(epsilon_i_K*epsilon_j_K)^0.5;                                // "9-5.35"
    epsilon_m_K :=(yi*yi*epsilon_i_K*sigma_i^3 + yj*yj*epsilon_j_K*sigma_j^3 + 2*
      yi*yj*epsilon_ij_K*sigma_ij^3)/sigma_m^3;                                                                                      // "9-5.27"

    M_ij :=2*MM[1]*MM[2]/(MM[1] + MM[2]);                                        // "9-5.40"
    M_m :=((yi*yi*epsilon_i_K*sigma_i^2*MM[1]^0.5 + yj*yj*epsilon_j_K*sigma_j^2*
      MM[2]^0.5 + 2*yi*yj*epsilon_ij_K*sigma_ij^2*M_ij^0.5)/(epsilon_m_K*sigma_m^2))
      ^2;                                                                                                    // "9-5.28"

    omega_ij:=(omega[1] + omega[2])/2;
    omega_m :=(yi*yi*omega[1]*sigma_i^3 + yj*yj*omega[2]*sigma_j^3 + 2*yi*yj*
      omega_ij*sigma_ij^3)/sigma_m^3;                                                                                                // "9-5.29"

    DM_m :=(sigma_m^3*(yi*yi*DM[1]^4/sigma_i^3 + yj*yj*DM[2]^4/sigma_j^3 + 2*yi*
      yj*DM[1]^2*DM[2]^2/sigma_ij^3))^(1/4);                                                                                          // "9-5.30"
    Vcm :=(sigma_m/0.809)^3;                                  // "9-5.44"
    Tcm :=1.2593*epsilon_m_K;                                 // "9-5.42"
    DM_rm :=131.3*DM_m/(Vcm*Tcm)^0.5;                         // "9-5.43"
    kappa_m :=yi*yi*kappa[1] + yj*yj*kappa[2] + 2*yi*yj*(kappa[1]*kappa[2])^0.5;  // "9-5.31"
    F_cm :=1 - 0.275*omega_m + 0.059035*DM_rm^4 + kappa_m;     // "9-5.41"

    T_star_m := state.T/epsilon_m_K;
    OMEGA_v:=1.16145*T_star_m^(-0.14874) + 0.52487*exp(-0.77320*T_star_m) + 2.16178
      *exp(-2.43787*T_star_m);                                                                                        // "9-4.3"

  //  eta :=26.69*F_cm*(M_m*state.T)^0.5/(sigma_m^2*OMEGA_v);  // "1e-6 poise unit = 1-7 "        "9-5.24"
  //  eta := eta/10^7;  // from "mu poise" to "Pa s"

  //  Modelica.Utilities.Streams.print("got OMEGA_v = " + String(OMEGA_v));
  //  Modelica.Utilities.Streams.print("got sigma_m = " + String(sigma_m));
  //  Modelica.Utilities.Streams.print("got epsilon_m_K = " + String(epsilon_m_K));
  //  Modelica.Utilities.Streams.print("got M_m = " + String(M_m));
  //  Modelica.Utilities.Streams.print("got omega_m = " + String(omega_m));
  //  Modelica.Utilities.Streams.print("got DM_rm = " + String(DM_rm));

   // "extention to dense (high pressure)"

    MMm :=yi*MM[1] + yj*MM[2];

  // we always have liquid or vapor, no two-phase

  //   if state.q <= 1 and state.q>=0 then // two-phase state
  //     rho :=state.sat.dv/(MMm*1e-3)/(1e6); // from kg/m3 to mol/cm3
  //   //rho = state.dv / (MMm * 1e-3 [kmol/mol]) / (1e6 [cm^3/m^3])
  //   //{rho=0.000001}
  //   else
      rho:=state.d/(MMm*1e-3)/(1e6);// from kg/m3 to mol/cm3
  //  end if;
    y :=rho*Vcm/6;                                            // "9-6.20"
    G1 :=(1 - 0.5*y)/(1 - y)^3;                               // "9-6.21"
    for i in 1:10 loop
      E[i] :=a[i] + b[i]*omega_m + c[i]*DM_rm^4 + d[i]*kappa_m;
    end for;
    G2 :=(E[1]*((1 - exp(-E[4]*y))/y) + E[2]*G1*exp(E[5]*y) + E[3]*G1)/(E[1]*E[4] +
      E[2] + E[3]);                                                                               // "9-6.22"
    eta_star_star :=E[7]*y^2*G2*exp(E[8] + E[9]*T_star_m^(-1) + E[10]*T_star_m^(-2));             // "9-6.23"
    eta_star :=T_star_m^0.5/OMEGA_v*F_cm*(G2^(-1) + E[6]*y) + eta_star_star;                      // "9-6.19"
    eta := eta_star*36.344*(M_m*Tcm)^0.5/Vcm^(2/3);                                                // "9-6.18"
    eta := eta/10^7;  // from "mu poise" to "Pa s"

  end dynamicViscosity_VAPChung;

  function dynamicViscosity_VAPChungPureErrorWeight
    "Chung et al. incl. mole-fraction avg. error correction (in Poling 2001)"
    extends partialDynamicViscosityVapor;
    extends Modelica.Icons.Function;
      input ThermodynamicState state "thermodynamic state record";
      output DynamicViscosity eta "Dynamic viscosity";

  protected
    constant Modelica.SIunits.Temperature Tc[2] = {405.560,647.096}; // critical T, K
    constant Real MM[2]= {17.0305,18.0153}; // Molar mass, kg/kmol
  //  constant Real DM[2] = {1.470,1.855}; // dipole moment, debye
    constant Real DM[2] = {1.5,1.8}; // dipole moment, debye
    constant Real omega[2] = {0.2558,0.3443}; // accentric factor
    constant Real Vc[2] = {75.6889,55.9536}; // critical volume, mol/cm3
    constant Real kappa[2] = {0.0,0.076}; // hydrogen bonding or hydrogen association factor

    Modelica.SIunits.Temperature Tc_dum "critical temperature";
    Modelica.SIunits.Pressure pc_dum "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";
    Real yi,yj;

    Real sigma_i,sigma_j,sigma_ij,sigma_m;
    Real epsilon_i_K,epsilon_j_K,epsilon_ij_K,epsilon_m_K;
    Real M_ij,M_m,omega_ij,omega_m;
    Real DM_m, Vcm, Tcm, DM_rm, kappa_m, F_cm, T_star_m, OMEGA_v;
    Real MMm, rho, y, G1, G2, eta_star_star, eta_star;
    Real E[10];

    constant Real a[10] = {6.324,1.210e-3,5.283,6.623,19.745,-1.900,24.275,0.7972,-0.2382,0.06863};
    constant Real b[10] = {50.412,-1.154e-3,254.209,38.096,7.630,-12.537,3.450,1.117,0.06770,0.3479};
    constant Real c[10] = {-51.680,-6.257e-3,-168.48,-8.464,-14.354,4.985,-11.291,0.01235,-0.8163,0.5926};
    constant Real d[10] = {1189.0,0.03728,3898.0,31.42,31.53,-18.15,69.35,-4.117,4.025,-0.727};

    ThermodynamicState  state_2,state_1;
    SaturationProperties sat_2;
    Real eta_1,eta_2, eta_1actual,eta_2actual;

  package Pure
  function eta

    input Real DM;
    input Real Vc;
    input Real Tc;
    input Real omega;
    input Real kappa;
    input Real MM;
    input Real T;
    input Real rho;

    output Real eta;

      protected
    Real DM_r,F_c,T_star, y, G1, G2, eta_star_star, eta_star, OMEGA_v;
    Real E[10];

    constant Real a[10] = {6.324,1.210e-3,5.283,6.623,19.745,-1.900,24.275,0.7972,-0.2382,0.06863};
    constant Real b[10] = {50.412,-1.154e-3,254.209,38.096,7.630,-12.537,3.450,1.117,0.06770,0.3479};
    constant Real c[10] = {-51.680,-6.257e-3,-168.48,-8.464,-14.354,4.985,-11.291,0.01235,-0.8163,0.5926};
    constant Real d[10] = {1189.0,0.03728,3898.0,31.42,31.53,-18.15,69.35,-4.117,4.025,-0.727};

  algorithm
    // "! pure values used for error weighting"
    DM_r :=131.3*DM/(Vc*Tc)^0.5;                        // "9-4.12"
    F_c :=1 - 0.2756*omega + 0.059035*DM_r^4 + kappa;   // "9-4.11"
    T_star :=1.2593*T/Tc;         // "9-4.9"
    OMEGA_v:=1.16145*T_star^(-0.14874) + 0.52487*exp(-0.77320*T_star) + 2.16178
      *exp(-2.43787*T_star);                                                                                        // "9-4.3"

    y :=rho*Vc/6;                                       // "9-6.20"
    G1 :=(1 - 0.5*y)/(1 - y)^3;                         // "9-6.21"
      for i in 1:10 loop
        E[i] :=a[i] + b[i]*omega + c[i]*DM_r^4 + d[i]*kappa;
      end for;
    G2 :=(E[1]*((1 - exp(-E[4]*y))/y) + E[2]*G1*exp(E[5]*y) + E[3]*G1)/(E[1]*E[4] +
            E[2] + E[3]);                                                                         //"9-6.22"
    eta_star_star :=E[7]*y^2*G2*exp(E[8] + E[9]*T_star^(-1) + E[10]*T_star^(-2));            // "9-6.23"
    eta_star :=T_star^0.5/OMEGA_v*F_c*(G2^(-1) + E[6]*y) + eta_star_star;                         // "9-6.19"
    eta :=eta_star*36.344*(MM*Tc)^0.5/Vc^(2/3);                                         // "9-6.18"
    eta := eta/10^7;  // from "mu poise" to "Pa s"
  end eta;

  end Pure;

  algorithm
    (Tc_dum,pc_dum,Z) :=criticalProperties_sassen1990(state.X);

    yi :=Z[1];
    yj :=Z[2];

    sigma_i:=0.809*Vc[1]^(1/3);                                                  // "9-5.32";
    sigma_j:=0.809*Vc[2]^(1/3);                                                  // "9-5.32"
    sigma_ij:=(sigma_i*sigma_j)^0.5;                                             // "9-5.33"
    sigma_m :=(yi*yi*sigma_i^3 + yj*yj*sigma_j^3 + 2*yi*yj*sigma_ij^3)^(1/3);    // "9-5.24"

    epsilon_i_K :=Tc[1]/1.2593;                                                  // "9-5.34"
    epsilon_j_K :=Tc[2]/1.2593;                                                  // "9-5.34"
    epsilon_ij_K :=(epsilon_i_K*epsilon_j_K)^0.5;                                // "9-5.35"
    epsilon_m_K :=(yi*yi*epsilon_i_K*sigma_i^3 + yj*yj*epsilon_j_K*sigma_j^3 + 2*
      yi*yj*epsilon_ij_K*sigma_ij^3)/sigma_m^3;                                                                                      // "9-5.27"

    M_ij :=2*MM[1]*MM[2]/(MM[1] + MM[2]);                                        // "9-5.40"
    M_m :=((yi*yi*epsilon_i_K*sigma_i^2*MM[1]^0.5 + yj*yj*epsilon_j_K*sigma_j^2*
      MM[2]^0.5 + 2*yi*yj*epsilon_ij_K*sigma_ij^2*M_ij^0.5)/(epsilon_m_K*sigma_m^2))
      ^2;                                                                                                    // "9-5.28"

    omega_ij:=(omega[1] + omega[2])/2;
    omega_m :=(yi*yi*omega[1]*sigma_i^3 + yj*yj*omega[2]*sigma_j^3 + 2*yi*yj*
      omega_ij*sigma_ij^3)/sigma_m^3;                                                                                                // "9-5.29"

    DM_m :=(sigma_m^3*(yi*yi*DM[1]^4/sigma_i^3 + yj*yj*DM[2]^4/sigma_j^3 + 2*yi*
      yj*DM[1]^2*DM[2]^2/sigma_ij^3))^(1/4);                                                                                          // "9-5.30"
    Vcm :=(sigma_m/0.809)^3;                                  // "9-5.44"
    Tcm :=1.2593*epsilon_m_K;                                 // "9-5.42"
    DM_rm :=131.3*DM_m/(Vcm*Tcm)^0.5;                         // "9-5.43"
    kappa_m :=yi*yi*kappa[1] + yj*yj*kappa[2] + 2*yi*yj*(kappa[1]*kappa[2])^0.5;  // "9-5.31"
    F_cm :=1 - 0.275*omega_m + 0.059035*DM_rm^4 + kappa_m;     // "9-5.41"

    T_star_m := state.T/epsilon_m_K;
    OMEGA_v:=1.16145*T_star_m^(-0.14874) + 0.52487*exp(-0.77320*T_star_m) + 2.16178
      *exp(-2.43787*T_star_m);                                                                                        // "9-4.3"

  //  eta :=26.69*F_cm*(M_m*state.T)^0.5/(sigma_m^2*OMEGA_v);  // "1e-6 poise unit = 1-7 "        "9-5.24"
  //  eta := eta/10^7;  // from "mu poise" to "Pa s"

  //  Modelica.Utilities.Streams.print("got OMEGA_v = " + String(OMEGA_v));
  //  Modelica.Utilities.Streams.print("got sigma_m = " + String(sigma_m));
  //  Modelica.Utilities.Streams.print("got epsilon_m_K = " + String(epsilon_m_K));
  //  Modelica.Utilities.Streams.print("got M_m = " + String(M_m));
  //  Modelica.Utilities.Streams.print("got omega_m = " + String(omega_m));
  //  Modelica.Utilities.Streams.print("got DM_rm = " + String(DM_rm));

   // "extention to dense (high pressure)"

    MMm :=yi*MM[1] + yj*MM[2];

  // we always have liquid or vapor, no two-phase

  //   if state.q <= 1 and state.q>=0 then // two-phase state
  //     rho :=state.sat.dv/(MMm*1e-3)/(1e6); // from kg/m3 to mol/cm3
  //   //rho = state.dv / (MMm * 1e-3 [kmol/mol]) / (1e6 [cm^3/m^3])
  //   //{rho=0.000001}
  //   else
      rho:=state.d/(MMm*1e-3)/(1e6);// from kg/m3 to mol/cm3
  //  end if;
    y :=rho*Vcm/6;                                            // "9-6.20"
    G1 :=(1 - 0.5*y)/(1 - y)^3;                               // "9-6.21"
    for i in 1:10 loop
      E[i] :=a[i] + b[i]*omega_m + c[i]*DM_rm^4 + d[i]*kappa_m;
    end for;
    G2 :=(E[1]*((1 - exp(-E[4]*y))/y) + E[2]*G1*exp(E[5]*y) + E[3]*G1)/(E[1]*E[4] +
      E[2] + E[3]);                                                                               // "9-6.22"
    eta_star_star :=E[7]*y^2*G2*exp(E[8] + E[9]*T_star_m^(-1) + E[10]*T_star_m^(-2));             // "9-6.23"
    eta_star :=T_star_m^0.5/OMEGA_v*F_cm*(G2^(-1) + E[6]*y) + eta_star_star;                      // "9-6.19"
    eta := eta_star*36.344*(M_m*Tcm)^0.5/Vcm^(2/3);                                                // "9-6.18"
    eta := eta/10^7;  // from "mu poise" to "Pa s"

  // error weighting
  // get corresponding values of pure components

    state_1 :=setState_pTX(
      state.p,
      state.T,
      {1,0},
      calcTransport=true);
     if state.T < Tc[2]-1 then //
       sat_2 := setSat_TX(state.T, {0,1});
       state_2 :=setState_pTX(
         min(state.p,sat_2.psat-1),
         state.T,
         {0,1},
         calcTransport=true);
     else
       state_2 :=setState_pTX(
         state.p,
         state.T,
         {0,1},
         calcTransport=true);
     end if;
  //       if state.p < 22064000 then
  //         sat_2 := setSat_pX(state.p, {0,1});
  //         state_2 :=setState_pTX(
  //           state.p,
  //           max(state.T,sat_2.Tsat+0.1),
  //           {0,1},
  //           calcTransport=true);
  //       else
  //         state_2 :=setState_pTX(
  //           state.p,
  //           state.T,
  //           {0,1},
  //           calcTransport=true);
  //       end if;

    eta_1 :=Pure.eta(
      DM[1],
      Vc[1],
      Tc[1],
      omega[1],
      kappa[1],
      MM[1],
      state.T,
      rho); // using mixture rho to do the averaging
    eta_2 :=Pure.eta(
      DM[2],
      Vc[2],
      Tc[2],
      omega[2],
      kappa[2],
      MM[2],
      state.T,
      rho); // using mixture rho to do the averaging

  //   Modelica.Utilities.Streams.print("got eta_1 = " + String(eta_1));
  //   Modelica.Utilities.Streams.print("got state_1.eta = " + String(state_1.eta));
  //   Modelica.Utilities.Streams.print("got eta_2 = " + String(eta_2));
  //   Modelica.Utilities.Streams.print("got state_2.eta = " + String(state_2.eta));

    eta := eta + (state_1.eta-eta_1)*yi + (state_2.eta-eta_2)*yj;

  end dynamicViscosity_VAPChungPureErrorWeight;

  partial function partialThermalConductivityVapor
  end partialThermalConductivityVapor;

  function thermalConductivity_VAPWilke "Mason and Saxena (in Poling 2001)"
    extends partialThermalConductivityVapor;
    extends Modelica.Icons.Function;
    input ThermodynamicState state "thermodynamic state record";
    output ThermalConductivity lambda;

  protected
    SaturationProperties sat_w;
    ThermodynamicState state_w;
    ThermodynamicState state_a;

    Modelica.SIunits.Temperature Tc "critical temperature";
    Modelica.SIunits.Pressure pc "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc_a=405.560;
    constant Modelica.SIunits.Temperature Tc_w=647.096;
    constant Real Mw=18.0153;
    // Molar mass of water, g/mol
    constant Real Ma=17.0305;
    // Molar mass of ammonia, g/mol

    Modelica.SIunits.DynamicViscosity mu_a;
    Modelica.SIunits.ThermalConductivity lambda_a;
    Modelica.SIunits.DynamicViscosity mu_w;
    Modelica.SIunits.ThermalConductivity lambda_w;

    Real F12;
    Real F21;

  algorithm
    (Tc,pc,Z) := criticalProperties_sassen1990(state.X);

    // it will always be vapor or supercritical!
    state_a := setState_pTX(
      state.p,
      state.T,
      {1,0},
      calcTransport=true);

    if state.T < Tc_w - 1 then
      //
      sat_w := setSat_TX(state.T, {0,1});
      state_w := setState_pTX(
        min(state.p, sat_w.psat - 1),
        state.T,
        {0,1},
        calcTransport=true);
    else
      state_w := setState_pTX(
        state.p,
        state.T,
        {0,1},
        calcTransport=true);
    end if;

    mu_a := state_a.eta;
    mu_w := state_w.eta;

    mu_a := mu_a*1e6;
    //convert to (1e-6 Pa.s)
    mu_w := mu_w*1e6;
    //convert to (1e-6 Pa.s)

    F12 := ((1 + (sqrt(mu_a/mu_w)*((Mw/Ma)^0.25)))^2)/sqrt(8*(1 + (Ma/Mw)));
    F21 := F12*(mu_w/mu_a)*(Ma/Mw);

    lambda_a := state_a.lambda;
    lambda_w := state_w.lambda;

    lambda := ((lambda_a*Z[1])/(Z[1] + (F12*Z[2]))) + ((lambda_w*Z[2])/(Z[2] + (
      F21*Z[1])));

    // Mixture vapour th. cond., W/(m K), Thorin, 2001

  end thermalConductivity_VAPWilke;

  function thermalConductivity_VAPLinearMolePoling
    "Mole-fraction average (suggested in Poling 2001)"
    extends partialThermalConductivityVapor;
    extends Modelica.Icons.Function;
    input ThermodynamicState state "thermodynamic state record";
    output ThermalConductivity lambda;

  protected
    SaturationProperties sat_w;
    ThermodynamicState state_w;
    ThermodynamicState state_a;

    Modelica.SIunits.Temperature Tc "critical temperature";
    Modelica.SIunits.Pressure pc "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc_a=405.560;
    constant Modelica.SIunits.Temperature Tc_w=647.096;

    Modelica.SIunits.ThermalConductivity lambda_a;
    Modelica.SIunits.ThermalConductivity lambda_w;

  algorithm
    (Tc,pc,Z) := criticalProperties_sassen1990(state.X);

    // it will always be vapor or supercritical!
    state_a := setState_pTX(
        state.p,
        state.T,
        {1,0},
        calcTransport=true);

    if state.T < Tc_w - 1 then
      //
      sat_w := setSat_TX(state.T, {0,1});
      state_w := setState_pTX(
          min(state.p, sat_w.psat - 1),
          state.T,
          {0,1},
          calcTransport=true);
    else
      state_w := setState_pTX(
          state.p,
          state.T,
          {0,1},
          calcTransport=true);
    end if;

    lambda_a := state_a.lambda;
    lambda_w := state_w.lambda;

    lambda := lambda_a*Z[1] + lambda_w*Z[2];

  end thermalConductivity_VAPLinearMolePoling;

  function thermalConductivity_VAPChung "Chung et al. (in Poling 2001)"
    extends partialThermalConductivityVapor;
    extends Modelica.Icons.Function;
      input ThermodynamicState state "thermodynamic state record";
      output ThermalConductivity lambda;

  protected
    constant Modelica.SIunits.Temperature Tc[2] = {405.560,647.096}; // critical T, K
    constant Real MM[2]= {17.0305,18.0153}; // Molar mass, kg/kmol
  //  constant Real DM[2] = {1.470,1.855}; // dipole moment, debye
    constant Real DM[2] = {1.5,1.8}; // dipole moment, debye
    constant Real omega[2] = {0.2558,0.3443}; // accentric factor
    constant Real Vc[2] = {75.6889,55.9536}; // critical volume, mol/cm3
    constant Real kappa[2] = {0.0,0.076}; // hydrogen bonding or hydrogen association factor
    Real Cv[2]; // ideal gas specfic volume
    constant Real beta[2] = {1.08^(-1),0.78^(-1)}; // hydrogen bonding or hydrogen association factor

    Modelica.SIunits.Temperature Tc_dum "critical temperature";
    Modelica.SIunits.Pressure pc_dum "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";
    Real yi,yj;

    Real sigma_i,sigma_j,sigma_ij,sigma_m;
    Real epsilon_i_K,epsilon_j_K,epsilon_ij_K,epsilon_m_K;
    Real M_ij,M_m,omega_ij,omega_m;
    Real DM_m, Vcm, Tcm, DM_rm, kappa_m, F_cm, T_star_m, OMEGA_v;
    Real MMm, rho, y_m, G1, G2, q, PSI_m, alpha_m, beta_m, Z_m, C_vm;
    Real B[7];
    Real eta_m0, lambda_m0;

    constant Real a[7] = {2.4166,-0.50924,6.6107,14.543,0.79274,-5.8634,91.089};
    constant Real b[7] = {0.74824,-1.5094,5.6207,-8.9139,0.82019,12.801,128.11};
    constant Real c[7] = {-0.91858,-49.991,64.76,-5.6379,-0.69369,9.5893,-54.217};
    constant Real d[7] = {121.72,69.983,27.039,74.344,6.3173,65.529,523.81};

  algorithm
    (Tc_dum,pc_dum,Z) :=criticalProperties_sassen1990(state.X);

    yi :=Z[1];
    yj :=Z[2];

  // first mix visc
    sigma_i:=0.809*Vc[1]^(1/3);                                                  // "9-5.32";
    sigma_j:=0.809*Vc[2]^(1/3);                                                  // "9-5.32"
    sigma_ij:=(sigma_i*sigma_j)^0.5;                                             // "9-5.33"
    sigma_m :=(yi*yi*sigma_i^3 + yj*yj*sigma_j^3 + 2*yi*yj*sigma_ij^3)^(1/3);    // "9-5.24"

    epsilon_i_K :=Tc[1]/1.2593;                                                  // "9-5.34"
    epsilon_j_K :=Tc[2]/1.2593;                                                  // "9-5.34"
    epsilon_ij_K :=(epsilon_i_K*epsilon_j_K)^0.5;                                // "9-5.35"
    epsilon_m_K :=(yi*yi*epsilon_i_K*sigma_i^3 + yj*yj*epsilon_j_K*sigma_j^3 + 2*
      yi*yj*epsilon_ij_K*sigma_ij^3)/sigma_m^3;                                                                                      // "9-5.27"

    M_ij :=2*MM[1]*MM[2]/(MM[1] + MM[2]);                                        // "9-5.40"
    M_m :=((yi*yi*epsilon_i_K*sigma_i^2*MM[1]^0.5 + yj*yj*epsilon_j_K*sigma_j^2*
      MM[2]^0.5 + 2*yi*yj*epsilon_ij_K*sigma_ij^2*M_ij^0.5)/(epsilon_m_K*sigma_m^2))
      ^2;                                                                                                    // "9-5.28"

    omega_ij:=(omega[1] + omega[2])/2;
    omega_m :=(yi*yi*omega[1]*sigma_i^3 + yj*yj*omega[2]*sigma_j^3 + 2*yi*yj*
      omega_ij*sigma_ij^3)/sigma_m^3;                                                                                                // "9-5.29"

    DM_m :=(sigma_m^3*(yi*yi*DM[1]^4/sigma_i^3 + yj*yj*DM[2]^4/sigma_j^3 + 2*yi*
      yj*DM[1]^2*DM[2]^2/sigma_ij^3))^(1/4);                                                                                          // "9-5.30"
    Vcm :=(sigma_m/0.809)^3;                                  // "9-5.44"
    Tcm :=1.2593*epsilon_m_K;                                 // "9-5.42"
    DM_rm :=131.3*DM_m/(Vcm*Tcm)^0.5;                         // "9-5.43"
    kappa_m :=yi*yi*kappa[1] + yj*yj*kappa[2] + 2*yi*yj*(kappa[1]*kappa[2])^0.5;  // "9-5.31"
    F_cm :=1 - 0.275*omega_m + 0.059035*DM_rm^4 + kappa_m;     // "9-5.41"

    T_star_m := state.T/epsilon_m_K;
    OMEGA_v:=1.16145*T_star_m^(-0.14874) + 0.52487*exp(-0.77320*T_star_m) + 2.16178
      *exp(-2.43787*T_star_m);                                                                                        // "9-4.3"

    eta_m0 := 26.69*F_cm*(M_m*state.T)^0.5/(sigma_m^2*OMEGA_v) * 1e-7;  // "1e-6 poise unit = 1-7 "        "9-5.24"

  // second mix cond

    Cv[1] :=(4.238 - 4.215e-3*state.T + 2.041e-5*state.T^2 - 2.126*1e-8*state.T^3 +
      0.761e-11*state.T^4)*8.314 - 8.314;
    Cv[2] :=(4.395 - 4.186e-3*state.T + 1.405e-5*state.T^2 - 1.564*1e-8*state.T^3 +
      0.632e-11*state.T^4)*8.314 - 8.314;
    C_vm :=yi*Cv[1] + yj*Cv[2];    // "10-6.6"
    alpha_m :=C_vm/8.314 - 1.5;// "10-3.14"
    //beta_m = 0.7862 - 0.7109*omega_m + 1.3168*omega_m^2        "10-3.14"
    /*
  For polar materials beta is specific for each compound; Chung, et al. (1984) list values for a few materials. If the compound is polar and 
  is not available,use a default value of (1.32)^(-1)=0.758
  "! how to deal with beta_m for polar mixtures???????"
  "! I would choose similar mixture rules as for sigma, epsilon and kappa how to deal with beta_m for polar mixtures???????"
  */
    beta_m :=yi*yi*beta[1] + yj*yj*beta[2] + 2*yi*yj*(beta[1]*beta[2])^0.5;
    Z_m :=2 + 10.5*(state.T/Tcm)^2;  // "10-3.14"
    PSI_m :=1 + alpha_m*(0.215 + 0.28288*alpha_m - 1.061*beta_m + 0.26665*Z_m)/(0.6366
       + beta_m*Z_m + 1.061*alpha_m*beta_m);                                                                           // "10-3.14"
    lambda_m0 :=3.75*PSI_m/(C_vm/8.314)*eta_m0*C_vm/(M_m*1e-3);  // "10-3.14"

  //  Modelica.Utilities.Streams.print("got OMEGA_v = " + String(OMEGA_v));
  //  Modelica.Utilities.Streams.print("got sigma_m = " + String(sigma_m));
  //  Modelica.Utilities.Streams.print("got epsilon_m_K = " + String(epsilon_m_K));
  //  Modelica.Utilities.Streams.print("got M_m = " + String(M_m));
  //  Modelica.Utilities.Streams.print("got omega_m = " + String(omega_m));
  //  Modelica.Utilities.Streams.print("got DM_rm = " + String(DM_rm));

   // "extention to dense (high pressure)"

    MMm :=yi*MM[1] + yj*MM[2];

  //  //this is just to make sure calculation is done right (is gas) if two-phase..
  //    if state.q <= 1 and state.q>=0 then // two-phase state
  //      rho :=state.sat.dv/(MMm*1e-3)/(1e6); // from kg/m3 to mol/cm3
  //    //rho = state.dv / (MMm * 1e-3 [kmol/mol]) / (1e6 [cm^3/m^3])
  //    //{rho=0.000001}
  //    else
       rho:=state.d/(MMm*1e-3)/(1e6);// from kg/m3 to mol/cm3
  //   end if;

    y_m :=rho*Vcm/6;                                            // "9-6.20"
    G1 :=(1 - 0.5*y_m)/(1 - y_m)^3;                               // "9-6.21"
    for i in 1:7 loop
      B[i] :=a[i] + b[i]*omega_m + c[i]*DM_rm^4 + d[i]*kappa_m;
    end for;
    G2 :=(B[1]*((1 - exp(-B[4]*y_m))/y_m) + B[2]*G1*exp(B[5]*y_m) + B[3]*G1)/(B[1]*B[4] +
      B[2] + B[3]);                                                                               // "9-6.22"

    q :=3.586e-3*(Tcm/(M_m*1e-3))^0.5/Vcm^(2/3);
    lambda :=31.2*eta_m0*PSI_m/(M_m*1e-3)*(G2^(-1) + B[6]*y_m) + q*B[7]*y_m^2*(state.T/
      Tcm)^0.5*G2;                                                                          //        "10-5.5"

  end thermalConductivity_VAPChung;

  function thermalConductivity_VAPChungPureErrorWeight
    "Chung et al. incl. mole-fraction avg. error correction (in Poling 2001)"
    extends partialThermalConductivityVapor;
    extends Modelica.Icons.Function;
      input ThermodynamicState state "thermodynamic state record";
      output ThermalConductivity lambda;

  protected
    constant Modelica.SIunits.Temperature Tc[2] = {405.560,647.096}; // critical T, K
    constant Real MM[2]= {17.0305,18.0153}; // Molar mass, kg/kmol
  //  constant Real DM[2] = {1.470,1.855}; // dipole moment, debye
    constant Real DM[2] = {1.5,1.8}; // dipole moment, debye
    constant Real omega[2] = {0.2558,0.3443}; // accentric factor
    constant Real Vc[2] = {75.6889,55.9536}; // critical volume, mol/cm3
    constant Real kappa[2] = {0.0,0.076}; // hydrogen bonding or hydrogen association factor
    Real Cv[2]; // ideal gas specfic volume
    constant Real beta[2] = {1.08^(-1),0.78^(-1)}; // hydrogen bonding or hydrogen association factor

    Modelica.SIunits.Temperature Tc_dum "critical temperature";
    Modelica.SIunits.Pressure pc_dum "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";
    Real yi,yj;

    Real sigma_i,sigma_j,sigma_ij,sigma_m;
    Real epsilon_i_K,epsilon_j_K,epsilon_ij_K,epsilon_m_K;
    Real M_ij,M_m,omega_ij,omega_m;
    Real DM_m, Vcm, Tcm, DM_rm, kappa_m, F_cm, T_star_m, OMEGA_v;
    Real MMm, rho, y_m, G1, G2, q, PSI_m, alpha_m, beta_m, Z_m, C_vm;
    Real B[7];
    Real eta_m0, lambda_m0;

    constant Real a[7] = {2.4166,-0.50924,6.6107,14.543,0.79274,-5.8634,91.089};
    constant Real b[7] = {0.74824,-1.5094,5.6207,-8.9139,0.82019,12.801,128.11};
    constant Real c[7] = {-0.91858,-49.991,64.76,-5.6379,-0.69369,9.5893,-54.217};
    constant Real d[7] = {121.72,69.983,27.039,74.344,6.3173,65.529,523.81};

  ThermodynamicState state_1,state_2;
  SaturationProperties sat_2;
  Real lambda_1,lambda_2,lambda_1actual,lambda_2actual;

  package Pure
  function lambda

    input Real DM;
    input Real Vc;
    input Real Tc;
    input Real omega;
    input Real kappa;
    input Real MM;
    input Real T;
    input Real rho;
    input Real Cv;
    input Real beta;

    output Real lambda;

      protected
    Real DM_r,F_c,T_star, y, G1, G2, eta_star_star, eta_star, OMEGA_v;
    Real B[7];

    Real eta_0,lambda_0,PSI,Z,alpha,q;

    constant Real a[7] = {2.4166,-0.50924,6.6107,14.543,0.79274,-5.8634,91.089};
    constant Real b[7] = {0.74824,-1.5094,5.6207,-8.9139,0.82019,12.801,128.11};
    constant Real c[7] = {-0.91858,-49.991,64.76,-5.6379,-0.69369,9.5893,-54.217};
    constant Real d[7] = {121.72,69.983,27.039,74.344,6.3173,65.529,523.81};
  algorithm
    // "! pure values used for error weighting"
    DM_r :=131.3*DM/(Vc*Tc)^0.5;                        // "9-4.12"
    F_c :=1 - 0.2756*omega + 0.059035*DM_r^4 + kappa;   // "9-4.11"
    T_star :=1.2593*T/Tc;         // "9-4.9"
    OMEGA_v:=1.16145*T_star^(-0.14874) + 0.52487*exp(-0.77320*T_star) + 2.16178
      *exp(-2.43787*T_star);                                                                                        // "9-4.3"

    eta_0 :=(40.785*F_c*(MM*T)^0.5/(Vc^(2/3)*OMEGA_v))*1e-7; // "1e-6 poise unit = 1-7 "        "9-4.10"

  //"! Then thermal conductivity by Chung"
    alpha :=Cv/8.314 - 1.5;
    Z :=2 + 10.5*(T/Tc)^2;
    PSI:=1 + alpha*(0.215 + 0.28288*alpha - 1.061*beta + 0.26665*Z)/(0.6366 +
          beta*Z + 1.061*alpha*beta);
    lambda_0 :=3.75*PSI/(Cv/8.314)*eta_0*Cv/(MM*1e-3);   // "10-3.14"

  /*
For polar materials beta is specific for each compound; Chung, et al. (1984) list values for a few materials. If the compound is polar and 
is not available,use a default value of (1.32)^(-1)=0.758
*/
    y :=rho*Vc/6;                                       // "9-6.20"
    G1 :=(1 - 0.5*y)/(1 - y)^3;                         // "9-6.21"
      for i in 1:7 loop
        B[i] :=a[i] + b[i]*omega + c[i]*DM_r^4 + d[i]*kappa;
      end for;
    G2 :=(B[1]*((1 - exp(-B[4]*y))/y) + B[2]*G1*exp(B[5]*y) + B[3]*G1)/(B[1]*B[4] +
            B[2] + B[3]);                                                                         //"9-6.22"

    q :=3.586e-3*(Tc/(MM*1e-3))^0.5/Vc^(2/3);
    lambda :=31.2*eta_0*PSI/(MM*1e-3)*(G2^(-1) + B[6]*y) + q*B[7]*y^2*(T/Tc)^0.5*
          G2;                                                                      // "10-5.5"

  end lambda;

  end Pure;

  algorithm
    (Tc_dum,pc_dum,Z) :=criticalProperties_sassen1990(state.X);

    yi :=Z[1];
    yj :=Z[2];

  // first mix visc
    sigma_i:=0.809*Vc[1]^(1/3);                                                  // "9-5.32";
    sigma_j:=0.809*Vc[2]^(1/3);                                                  // "9-5.32"
    sigma_ij:=(sigma_i*sigma_j)^0.5;                                             // "9-5.33"
    sigma_m :=(yi*yi*sigma_i^3 + yj*yj*sigma_j^3 + 2*yi*yj*sigma_ij^3)^(1/3);    // "9-5.24"

    epsilon_i_K :=Tc[1]/1.2593;                                                  // "9-5.34"
    epsilon_j_K :=Tc[2]/1.2593;                                                  // "9-5.34"
    epsilon_ij_K :=(epsilon_i_K*epsilon_j_K)^0.5;                                // "9-5.35"
    epsilon_m_K :=(yi*yi*epsilon_i_K*sigma_i^3 + yj*yj*epsilon_j_K*sigma_j^3 + 2*
      yi*yj*epsilon_ij_K*sigma_ij^3)/sigma_m^3;                                                                                      // "9-5.27"

    M_ij :=2*MM[1]*MM[2]/(MM[1] + MM[2]);                                        // "9-5.40"
    M_m :=((yi*yi*epsilon_i_K*sigma_i^2*MM[1]^0.5 + yj*yj*epsilon_j_K*sigma_j^2*
      MM[2]^0.5 + 2*yi*yj*epsilon_ij_K*sigma_ij^2*M_ij^0.5)/(epsilon_m_K*sigma_m^2))
      ^2;                                                                                                    // "9-5.28"

    omega_ij:=(omega[1] + omega[2])/2;
    omega_m :=(yi*yi*omega[1]*sigma_i^3 + yj*yj*omega[2]*sigma_j^3 + 2*yi*yj*
      omega_ij*sigma_ij^3)/sigma_m^3;                                                                                                // "9-5.29"

    DM_m :=(sigma_m^3*(yi*yi*DM[1]^4/sigma_i^3 + yj*yj*DM[2]^4/sigma_j^3 + 2*yi*
      yj*DM[1]^2*DM[2]^2/sigma_ij^3))^(1/4);                                                                                          // "9-5.30"
    Vcm :=(sigma_m/0.809)^3;                                  // "9-5.44"
    Tcm :=1.2593*epsilon_m_K;                                 // "9-5.42"
    DM_rm :=131.3*DM_m/(Vcm*Tcm)^0.5;                         // "9-5.43"
    kappa_m :=yi*yi*kappa[1] + yj*yj*kappa[2] + 2*yi*yj*(kappa[1]*kappa[2])^0.5;  // "9-5.31"
    F_cm :=1 - 0.275*omega_m + 0.059035*DM_rm^4 + kappa_m;     // "9-5.41"

    T_star_m := state.T/epsilon_m_K;
    OMEGA_v:=1.16145*T_star_m^(-0.14874) + 0.52487*exp(-0.77320*T_star_m) + 2.16178
      *exp(-2.43787*T_star_m);                                                                                        // "9-4.3"

    eta_m0 := 26.69*F_cm*(M_m*state.T)^0.5/(sigma_m^2*OMEGA_v) * 1e-7;  // "1e-6 poise unit = 1-7 "        "9-5.24"

  // second mix cond

    Cv[1] :=(4.238 - 4.215e-3*state.T + 2.041e-5*state.T^2 - 2.126*1e-8*state.T^3 +
      0.761e-11*state.T^4)*8.314 - 8.314;
    Cv[2] :=(4.395 - 4.186e-3*state.T + 1.405e-5*state.T^2 - 1.564*1e-8*state.T^3 +
      0.632e-11*state.T^4)*8.314 - 8.314;
    C_vm :=yi*Cv[1] + yj*Cv[2];    // "10-6.6"
    alpha_m :=C_vm/8.314 - 1.5;// "10-3.14"
    //beta_m = 0.7862 - 0.7109*omega_m + 1.3168*omega_m^2        "10-3.14"
    /*
  For polar materials beta is specific for each compound; Chung, et al. (1984) list values for a few materials. If the compound is polar and 
  is not available,use a default value of (1.32)^(-1)=0.758
  "! how to deal with beta_m for polar mixtures???????"
  "! I would choose similar mixture rules as for sigma, epsilon and kappa how to deal with beta_m for polar mixtures???????"
  */
    beta_m :=yi*yi*beta[1] + yj*yj*beta[2] + 2*yi*yj*(beta[1]*beta[2])^0.5;
    Z_m :=2 + 10.5*(state.T/Tcm)^2;  // "10-3.14"
    PSI_m :=1 + alpha_m*(0.215 + 0.28288*alpha_m - 1.061*beta_m + 0.26665*Z_m)/(0.6366
       + beta_m*Z_m + 1.061*alpha_m*beta_m);                                                                           // "10-3.14"
    lambda_m0 :=3.75*PSI_m/(C_vm/8.314)*eta_m0*C_vm/(M_m*1e-3);  // "10-3.14"

  //  Modelica.Utilities.Streams.print("got OMEGA_v = " + String(OMEGA_v));
  //  Modelica.Utilities.Streams.print("got sigma_m = " + String(sigma_m));
  //  Modelica.Utilities.Streams.print("got epsilon_m_K = " + String(epsilon_m_K));
  //  Modelica.Utilities.Streams.print("got M_m = " + String(M_m));
  //  Modelica.Utilities.Streams.print("got omega_m = " + String(omega_m));
  //  Modelica.Utilities.Streams.print("got DM_rm = " + String(DM_rm));

   // "extention to dense (high pressure)"

    MMm :=yi*MM[1] + yj*MM[2];

  // this is just to make sure calculation is done right (is gas) if two-phase..
  //    if state.q <= 1 and state.q>=0 then // two-phase state
  //      rho :=state.sat.dv/(MMm*1e-3)/(1e6); // from kg/m3 to mol/cm3
  //    //rho = state.dv / (MMm * 1e-3 [kmol/mol]) / (1e6 [cm^3/m^3])
  //    //{rho=0.000001}
  //    else
       rho:=state.d/(MMm*1e-3)/(1e6);// from kg/m3 to mol/cm3
  //   end if;

    y_m :=rho*Vcm/6;                                            // "9-6.20"
    G1 :=(1 - 0.5*y_m)/(1 - y_m)^3;                               // "9-6.21"
    for i in 1:7 loop
      B[i] :=a[i] + b[i]*omega_m + c[i]*DM_rm^4 + d[i]*kappa_m;
    end for;
    G2 :=(B[1]*((1 - exp(-B[4]*y_m))/y_m) + B[2]*G1*exp(B[5]*y_m) + B[3]*G1)/(B[1]*B[4] +
      B[2] + B[3]);                                                                               // "9-6.22"

    q :=3.586e-3*(Tcm/(M_m*1e-3))^0.5/Vcm^(2/3);
    lambda :=31.2*eta_m0*PSI_m/(M_m*1e-3)*(G2^(-1) + B[6]*y_m) + q*B[7]*y_m^2*(state.T/
      Tcm)^0.5*G2;                                                                          //        "10-5.5"

  // error weighting
  // get corresponding values of pure components

    state_1 :=setState_pTX(
      state.p,
      state.T,
      {1,0},
      calcTransport=true);
     if state.T < Tc[2]-1 then //
       sat_2 := setSat_TX(state.T, {0,1});
       state_2 :=setState_pTX(
         min(state.p,sat_2.psat-1),
         state.T,
         {0,1},
         calcTransport=true);
     else
       state_2 :=setState_pTX(
         state.p,
         state.T,
         {0,1},
         calcTransport=true);
     end if;

  lambda_1 :=Pure.lambda(
      DM[1],
      Vc[1],
      Tc[1],
      omega[1],
      kappa[1],
      MM[1],
      state.T,
      rho,
      Cv[1],
      beta[1]);
  lambda_2 :=Pure.lambda(
      DM[2],
      Vc[2],
      Tc[2],
      omega[2],
      kappa[2],
      MM[2],
      state.T,
      rho,
      Cv[2],
      beta[2]);

  //   Modelica.Utilities.Streams.print("got eta_1 = " + String(eta_1));
  //   Modelica.Utilities.Streams.print("got state_1.eta = " + String(state_1.eta));
  //   Modelica.Utilities.Streams.print("got eta_2 = " + String(eta_2));
  //   Modelica.Utilities.Streams.print("got state_2.eta = " + String(state_2.eta));

    lambda_1actual:=state_1.lambda;
    lambda_2actual:=state_2.lambda;

    lambda := lambda + (state_1.lambda-lambda_1)*yi + (state_2.lambda-lambda_2)*yj;

  end thermalConductivity_VAPChungPureErrorWeight;

  partial function partialThermalConductivityLiquid
  end partialThermalConductivityLiquid;

  function thermalConductivity_LIQLinearMoleElsayed "El-Sayed 1988"

    extends partialThermalConductivityLiquid;
    extends Modelica.Icons.Function;
    input ThermodynamicState state "thermodynamic state record";
    output ThermalConductivity lambda;

  protected
    ThermodynamicState state_w;
    ThermodynamicState state_a;

    Modelica.SIunits.Temperature Tc "critical temperature";
    Modelica.SIunits.Pressure pc "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc_a=405.560;
    constant Modelica.SIunits.Temperature Tc_w=647.096;
    Modelica.SIunits.Temperature Tcorr_a "Corresponding temp. for ammonia, K";
    Modelica.SIunits.Temperature Tcorr_w "Corresponding temp. for water, K";

    Modelica.SIunits.ThermalConductivity lambda_a;
    Modelica.SIunits.ThermalConductivity lambda_w;

  algorithm
    (Tc,pc,Z) := criticalProperties_sassen1990(state.X);

    Tcorr_a := state.T*Tc_a/Tc "Corresponding temp. for ammonia, K";
    Tcorr_w := state.T*Tc_w/Tc "Corresponding temp. for water, K";

    if debugmode then
      Modelica.Utilities.Streams.print("got (Tc,pc,Tcorr_a,Tcorr_w), (" +
        String(Tc) + "," + String(pc) + "," + String(Tcorr_a) + "," + String(
        Tcorr_w) + ")");
    end if;

    state_a := setState_TqX(
        min(Tc_a - 1, Tcorr_a),
        0,
        {1,0},
        calcTransport=true);
    state_w := setState_TqX(
        min(Tcorr_w, Tc_w - 1),
        0,
        {0,1},
        calcTransport=true);

    lambda_a := state_a.lambda;
    lambda_w := state_w.lambda;

    lambda := lambda_a*Z[1] + lambda_w*Z[2];

  end thermalConductivity_LIQLinearMoleElsayed;

  function thermalConductivity_LIQConde "Conde 2006"

    extends partialThermalConductivityLiquid;
    extends Modelica.Icons.Function;
    input ThermodynamicState state "thermodynamic state record";
    output ThermalConductivity lambda;

  protected
    ThermodynamicState state_w;
    ThermodynamicState state_a;

    Modelica.SIunits.Temperature Tc "critical temperature";
    Modelica.SIunits.Pressure pc "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc_a=405.560;
    constant Modelica.SIunits.Temperature Tc_w=647.096;
    Modelica.SIunits.Temperature Tcorr_a "Corresponding temp. for ammonia, K";
    Modelica.SIunits.Temperature Tcorr_w "Corresponding temp. for water, K";

    Modelica.SIunits.ThermalConductivity lambda_a;
    Modelica.SIunits.ThermalConductivity lambda_w;

    Real rho_p1;

  algorithm
    (Tc,pc,Z) := criticalProperties_sassen1990(state.X);

    Tcorr_a := state.T*Tc_a/Tc "Corresponding temp. for ammonia, K";
    Tcorr_w := state.T*Tc_w/Tc "Corresponding temp. for water, K";

    if debugmode then
      Modelica.Utilities.Streams.print("got (Tc,pc,Tcorr_a,Tcorr_w), (" +
        String(Tc) + "," + String(pc) + "," + String(Tcorr_a) + "," + String(
        Tcorr_w) + ")");
    end if;

    state_a := setState_TqX(
        min(Tc_a - 1, Tcorr_a),
        0,
        {1,0},
        calcTransport=true);
    state_w := setState_TqX(
        min(Tcorr_w, Tc_w - 1),
        0,
        {0,1},
        calcTransport=true);

    lambda_a := state_a.lambda;
    lambda_w := state_w.lambda;

    //  Modelica.Utilities.Streams.print("got first lambda = " + String(lambda_a));

    rho_p1 := state_a.d*Z[1]^0.425;
    state_a := setState_dqX(
        max(rho_p1, 225),
        0,
        {1,0},
        calcTransport=true);

    lambda_a := state_a.lambda;

    //  Modelica.Utilities.Streams.print("got second lambda = " + String(lambda_a));

    lambda := lambda_a*Z[1] + lambda_w*Z[2];

  end thermalConductivity_LIQConde;

  function thermalConductivity_LIQFilipov "Filippov (in Poling 2001)"

    extends partialThermalConductivityLiquid;
    extends Modelica.Icons.Function;
    input ThermodynamicState state "thermodynamic state record";
    output ThermalConductivity lambda;

  protected
    ThermodynamicState state_w;
    ThermodynamicState state_a;
    SaturationProperties sat_a;

    Modelica.SIunits.Temperature Tc "critical temperature";
    Modelica.SIunits.Pressure pc "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc_a=405.560;
    constant Modelica.SIunits.Temperature Tc_w=647.096;
    constant Modelica.SIunits.Pressure pc_a=11333000;
    constant Modelica.SIunits.Pressure pc_w=22064000;

    Modelica.SIunits.ThermalConductivity lambda_a;
    Modelica.SIunits.ThermalConductivity lambda_w;

  algorithm
    (Tc,pc,Z) := criticalProperties_sassen1990(state.X);

    if state.T < Tc_a - 2 then
      //
      sat_a := setSat_TX(state.T, {1,0});
      state_a := setState_pTX(
          max(state.p, sat_a.psat + 10),
          state.T,
          {1,0},
          calcTransport=true);
    else
      state_a := setState_pTX(
          max(pc_a + 10, state.p),
          Tc_a - 2,
          {1,0},
          calcTransport=true);
    end if;

    // the water will always be liquid
    state_w := setState_pTX(
        state.p,
        min(state.T, Tc_w - 2),
        {0,1},
        calcTransport=true);

    lambda_a := state_a.lambda;
    lambda_w := state_w.lambda;

    lambda := state.X[1]*lambda_a + state.X[2]*lambda_w - 0.72*state.X[1]*state.X[
      2]*max(lambda_a - lambda_w, lambda_w - lambda_a);

  end thermalConductivity_LIQFilipov;

  function thermalConductivity_LIQJamie "Jamieson (in Poling 2001)"

    extends partialThermalConductivityLiquid;
    extends Modelica.Icons.Function;
    input ThermodynamicState state "thermodynamic state record";
    output ThermalConductivity lambda;

  protected
    ThermodynamicState state_w;
    ThermodynamicState state_a;
    SaturationProperties sat_a;

    Modelica.SIunits.Temperature Tc "critical temperature";
    Modelica.SIunits.Pressure pc "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc_a=405.560;
    constant Modelica.SIunits.Temperature Tc_w=647.096;
    constant Modelica.SIunits.Pressure pc_a=11333000;
    constant Modelica.SIunits.Pressure pc_w=22064000;

    Modelica.SIunits.ThermalConductivity lambda_a;
    Modelica.SIunits.ThermalConductivity lambda_w;

  algorithm
    (Tc,pc,Z) := criticalProperties_sassen1990(state.X);

    if state.T < Tc_a - 2 then
      //
      sat_a := setSat_TX(state.T, {1,0});
      state_a := setState_pTX(
          max(state.p, sat_a.psat + 10),
          state.T,
          {1,0},
          calcTransport=true);
    else
      state_a := setState_pTX(
          max(pc_a + 10, state.p),
          Tc_a - 2,
          {1,0},
          calcTransport=true);
    end if;

    // the water will always be liquid
    state_w := setState_pTX(
        state.p,
        min(state.T, Tc_w - 2),
        {0,1},
        calcTransport=true);

    lambda_a := state_a.lambda;
    lambda_w := state_w.lambda;

    if lambda_a > lambda_w then
      lambda := state.X[1]*lambda_a + state.X[2]*lambda_w - (lambda_a -
        lambda_w)*(1 - sqrt(state.X[1]))*state.X[1];
    else
      lambda := state.X[1]*lambda_a + state.X[2]*lambda_w - (lambda_w -
        lambda_a)*(1 - sqrt(state.X[2]))*state.X[2];
    end if;

  end thermalConductivity_LIQJamie;

  function surfaceTensionTX
    "Return surface tension, ideal solution based on corresponding temperature"
    extends Modelica.Icons.Function;
      input Temperature T;
      input MassFraction X[nX]
      "bulk mass fraction to determine critical T and p";
      output SurfaceTension sigma "Dynamic viscosity";

  protected
    SaturationProperties sat_w;
    SaturationProperties sat_a;

    SurfaceTension sigma_w;
    SurfaceTension sigma_a;

    Modelica.SIunits.Temperature Tc "critical temperature";
    Modelica.SIunits.Pressure pc "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc_a = 405.560;
    constant Modelica.SIunits.Temperature Tc_w = 647.096;
   Modelica.SIunits.Temperature Tcorr_a "Corresponding temp. for ammonia, K";
    Modelica.SIunits.Temperature Tcorr_w "Corresponding temp. for water, K";

    constant Modelica.SIunits.Temperature TNH3min=195.5;

  algorithm
    (Tc,pc,Z) := criticalProperties_sassen1990(X);

    Tcorr_a := T*Tc_a/Tc "Corresponding temp. for ammonia, K";
    Tcorr_w := T*Tc_w/Tc "Corresponding temp. for water, K";

    if debugmode then
        Modelica.Utilities.Streams.print("got (Tc,pc,Tcorr_a,Tcorr_w), (" + String(Tc) + "," + String(pc) + "," + String(Tcorr_a) + "," + String(Tcorr_w) + ")");
    end if;

    sat_a := setSat_TX(
      max(TNH3min,Tcorr_a),
      {1,0},
      calcTransport=true);
    sat_w := setSat_TX(
      Tcorr_w,
      {0,1},
      calcTransport=true);

    sigma_a :=sat_a.sigma;
    sigma_w :=sat_w.sigma;

    sigma:= sigma_a*Z[1] + sigma_w*Z[2];

  end surfaceTensionTX;

  redeclare function surfaceTension
    "Return surface tension, ideal solution based on corresponding temperature"
    extends Modelica.Icons.Function;
    input SaturationProperties sat;
    output SurfaceTension sigma "Dynamic viscosity";

  protected
    SaturationProperties sat_w;
    SaturationProperties sat_a;

    SurfaceTension sigma_w;
    SurfaceTension sigma_a;

    Modelica.SIunits.Temperature Tc "critical temperature";
    Modelica.SIunits.Pressure pc "critical pressure";
    Modelica.SIunits.MoleFraction[nX] Z "mole fraction";

    constant Modelica.SIunits.Temperature Tc_a=405.560;
    constant Modelica.SIunits.Temperature Tc_w=647.096;
    Modelica.SIunits.Temperature Tcorr_a "Corresponding temp. for ammonia, K";
    Modelica.SIunits.Temperature Tcorr_w "Corresponding temp. for water, K";

    constant Modelica.SIunits.Temperature TNH3min=195.5;

  algorithm
    (Tc,pc,Z) := criticalProperties_sassen1990(sat.X);

    Tcorr_a := sat.Tsat*Tc_a/Tc "Corresponding temp. for ammonia, K";
    Tcorr_w := sat.Tsat*Tc_w/Tc "Corresponding temp. for water, K";

    if debugmode then
      Modelica.Utilities.Streams.print("got (Tc,pc,Tcorr_a,Tcorr_w), (" +
        String(Tc) + "," + String(pc) + "," + String(Tcorr_a) + "," + String(
        Tcorr_w) + ")");
    end if;

    sat_a := setSat_TX(
        min(Tc_a - 1, max(TNH3min, Tcorr_a)),
        {1,0},
        calcTransport=true);
    sat_w := setSat_TX(
        min(Tcorr_w, Tc_w - 1),
        {0,1},
        calcTransport=true);

    sigma_a := sat_a.sigma;
    sigma_w := sat_w.sigma;

    sigma := sigma_a*Z[1] + sigma_w*Z[2];

  end surfaceTension;



end NH3_Water;
