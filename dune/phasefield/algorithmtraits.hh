#ifndef ALGORITHM_TRAITS_HH
#define ALGORITHM_TRAITS_HH

#include <dune/fem/gridpart/adaptiveleafgridpart.hh>
#include <dune/fem/space/dgspace/localrestrictprolong.hh>
#include <dune/fem/solver/odesolver.hh>
#include <dune/fem/operator/wellbalancedoperator.hh>


template <class GridImp,
          class ProblemGeneratorImp, 
          int polOrd>             
struct AlgorithmTraits 
{
  enum { polynomialOrder = polOrd };

  // type of Grid
  typedef GridImp                                  GridType;
	typedef ProblemGeneratorImp                      ProblemGeneratorType;

  // Choose a suitable GridView
  typedef Dune :: DGAdaptiveLeafGridPart< GridType >       GridPartType;

  // problem dependent types 
  typedef typename ProblemGeneratorType :: template Traits< GridPartType > :: InitialDataType  InitialDataType;
  typedef typename ProblemGeneratorType :: template Traits< GridPartType > :: ModelType        ModelType;
  typedef typename ProblemGeneratorType :: template Traits< GridPartType > :: FluxType         FluxType;
	
	//typedef Dune :: WellBalancedPhasefieldOperator< ModelType, FluxType,DiffusionFluxId,  polynomialOrder >    DiscreteOperatorType;
	
	typedef Dune :: DGAdvectionDiffusionOperator< ModelType, FluxType, polynomialOrder >    DiscreteOperatorType;
	


	// Types of all Discretefuntions used in the simulation: Destinations of the passes, Scalarspace for energy calculation
  typedef typename DiscreteOperatorType :: DestinationType                         DiscreteFunctionType;
  
	typedef typename DiscreteOperatorType :: Destination1Type                        DiscreteSigmaType;
	
	typedef typename DiscreteOperatorType :: Destination2Type                        DiscreteThetaType;
	typedef typename DiscreteOperatorType :: DiscreteScalarType                      DiscreteScalarType;

	// ... as well as the Space type
	typedef typename  DiscreteOperatorType :: SpaceType                              DiscreteSpaceType;
  typedef typename  DiscreteOperatorType :: Space1Type                             SigmaDiscreteSpaceType;   
	typedef typename  DiscreteOperatorType :: Space2Type                             ThetaDiscreteSpaceType;
  typedef typename  DiscreteOperatorType :: ScalarDiscreteFunctionSpaceType        ScalarDiscreteSpaceType;
	

	// The ODE Solvers                                                         /*@LST1S@*/
  typedef DuneODE :: OdeSolverInterface< DiscreteFunctionType > OdeSolverType ;
  // type of restriction/prolongation projection for adaptive simulations 
  typedef Dune :: RestrictProlongDefault< DiscreteFunctionType > RestrictionProlongationType;
};




#endif