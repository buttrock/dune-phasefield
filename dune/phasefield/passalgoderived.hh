#ifndef PASSALGORITHM_HH
#define PASSALGORITHM_HH
#include "algorithmtraits.hh"
#include "algorithmbase.hh"

//solver
#include <dune/fem/util/phasefieldodesolver.hh>
//adaptation
#include <dune/fem/adaptation/estimator1.hh>
#if WELLBALANCED
#include <dune/phasefield/util/wb_energyconverter.hh>
#else
#include <dune/phasefield/util/energyconverter.hh>
#endif
namespace Dune{

template< class GridImp, 
          class AlgorithmTraits>
class PassAlgorithm: public PhasefieldAlgorithmBase< GridImp, AlgorithmTraits, PassAlgorithm<GridImp, AlgorithmTraits> >
{
  public:
    typedef PassAlgorithm< GridImp, AlgorithmTraits> ThisType;
    typedef PhasefieldAlgorithmBase< GridImp , AlgorithmTraits , ThisType > BaseType;
    typedef typename BaseType::Traits Traits;
    typedef typename BaseType::GridType GridType;
    typedef typename BaseType::DiscreteFunctionType DiscreteFunctionType;
    typedef typename BaseType::DiscreteScalarType DiscreteScalarType;
    typedef typename BaseType::ModelType ModelType;
    typedef typename BaseType::TimeProviderType TimeProviderType; 
    typedef typename BaseType::AdaptationManagerType AdaptationManagerType;
    //Operator
    typedef typename Traits::FluxType FluxType;
    typedef typename Traits::DiscreteOperatorType DiscreteOperatorType; 
    //Solver
    typedef typename Traits::OdeSolverType OdeSolverType;
    typedef typename OdeSolverType::MonitorType OdeSolverMonitorType;
    //discrete spaces
    typedef typename Traits::SigmaDiscreteSpaceType SigmaDiscreteSpaceType;
    typedef typename Traits::ThetaDiscreteSpaceType ThetaDiscreteSpaceType;
    //discrete functions 
    typedef typename Traits::DiscreteSigmaType DiscreteSigmaType;
    typedef typename Traits::DiscreteThetaType DiscreteThetaType;
    //Adaptation 
    typedef  Estimator1< DiscreteFunctionType > EstimatorType;
    //IOTuple
  	//type of IOTuple 
    typedef typename BaseType::IOTupleType IOTupleType;
 
    using BaseType::grid_;
    using BaseType::gridPart_;
    using BaseType::space_;
    using BaseType::solution_;
    using BaseType::oldsolution_;
    using BaseType::model_;
    using BaseType::overallTimer_;
    using BaseType::tolerance_;
 
  private:
    FluxType                numericalFlux_;
    DiscreteOperatorType    dgOperator_;
    SigmaDiscreteSpaceType  sigmaSpace_;
    ThetaDiscreteSpaceType  thetaSpace_;
    DiscreteSigmaType*      sigma_;
	  DiscreteThetaType*      theta_;
    DiscreteFunctionType*   additionalVariables_;
    OdeSolverType*          odeSolver_;
    OdeSolverMonitorType    odeSolverMonitor_;
    int                     odeSolverType_;
  
  public:
    //Constructor
    PassAlgorithm( GridType& grid):
      BaseType( grid ),
      numericalFlux_( *model_ ),
 	    dgOperator_(grid,numericalFlux_),
      sigmaSpace_(gridPart_ ),
	    thetaSpace_( gridPart_ ),
      sigma_( Fem :: Parameter :: getValue< bool >("phasefield.sigma", false) ? new DiscreteSigmaType("sigma",sigmaspace()) : nullptr),
		  theta_( Fem :: Parameter :: getValue< bool >("phasefield.theta", false) ? new DiscreteThetaType("theta",thetaspace() ) : nullptr),
		  additionalVariables_( Fem::Parameter :: getValue< bool >("phasefield.additionalvariables", false) ? 
													new DiscreteFunctionType("additional", space() ) : 0 ),
		  odeSolver_( 0 )
      {} 
    
    using BaseType::space;
    using BaseType::energyspace;
    using BaseType::gridSize;
    using BaseType::oldsolution;
    using BaseType::solution;
    using BaseType::energy;
    using BaseType::dataPrefix;
    using BaseType::problem;
    using BaseType::model;


 
 //Access Methods
    SigmaDiscreteSpaceType& sigmaspace (){ return sigmaSpace_;}
    ThetaDiscreteSpaceType& thetaspace (){ return thetaSpace_;}
    
    DiscreteFunctionType* additionalVariables (){ return additionalVariables_;}
    DiscreteThetaType*     theta (){ return theta_; }


    IOTupleType getDataTuple()
      {
        return IOTupleType( &solution(),this->additionalVariables(),this->theta(),this->energy());
      }
   
   OdeSolverType* createOdeSolver(TimeProviderType&timeProvider) 
	    {
		    typedef PhaseFieldOdeSolver< DiscreteOperatorType > OdeSolverImpl;
		    return new OdeSolverImpl(timeProvider, dgOperator_ );
	    }


    void initializeSolver( typename BaseType::TimeProviderType& timeProvider)
      { 
        if( odeSolver_ == nullptr ) odeSolver_ = createOdeSolver( timeProvider );    
		    assert( odeSolver_ );
        odeSolver_->initialize( solution());
      }
  
    void step ( TimeProviderType& timeProvider,
		  				  int& newton_iterations,
			  			  int& ils_iterations,
					  	  int& max_newton_iterations,
				  		  int& max_ils_iterations)
	    {
        DiscreteFunctionType& U=solution();
		    // reset overall timer
        overallTimer_.reset(); 
		    assert(odeSolver_);

        odeSolver_->solve( U, odeSolverMonitor_ );
		    newton_iterations     = odeSolverMonitor_.newtonIterations_;
        ils_iterations        = odeSolverMonitor_.linearSolverIterations_;
        max_newton_iterations = odeSolverMonitor_.maxNewtonIterations_ ;
        max_ils_iterations    = odeSolverMonitor_.maxLinearSolverIterations_;
	   }
 
};
}// end namespace Dune

#endif //PASSALGORITHM_HH
