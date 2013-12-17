within REFPROP2Modelica.Media;
package NH3_Water "Ammonia and water mixture by REFPROP library"
  extends Interfaces.REFPROPMixtureTwoPhaseMedium(
  final substanceNames={"ammoniaL","water"},
  final debugmode=false,
  final calcTransport=true,
  final calcTwoPhaseNumericalDerivatives=false,
  final calcTwoPhasePsuedoAnalyticalDerivatives=true);
end NH3_Water;
