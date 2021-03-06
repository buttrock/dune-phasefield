#ifndef DUNE_PHASEFIELD_GRADIENTDISCRETEMODEL_HH
#define DUNE_PHASEFIELD_GRADIENTDISCRETEMODEL_HH

// Dune includes
#include <dune/fem/gridpart/common/gridpart.hh>
#include <dune/fem/gridpart/adaptiveleafgridpart.hh>

// Dune-Fem includes
#include <dune/fem/space/discontinuousgalerkin.hh>
#include <dune/fem/pass/localdg/discretemodel.hh>
#include <dune/fem/function/adaptivefunction.hh>
#include <dune/fem/quadrature/cachingquadrature.hh>
#include <dune/fem/misc/boundaryidentifier.hh>
#include <dune/fem/misc/fmatrixconverter.hh>

// local includes
#include "projectiondiscretemodelcommon.hh"
#include <dune/fem/fluxes/ldgfluxwellbalanced.hh>

//*************************************************************
namespace Dune {

  // GradientModel
  //--------------
  
  template <class Model, class NumFlux, int polOrd, int passUId> 
  class GradientModel;


  // GradientTraits
  //---------------  
  template <class Model, class NumFlux, int polOrd, int passUId >
  struct GradientTraits
  {
    typedef typename Model :: Traits                                 ModelTraits;
    typedef typename ModelTraits :: GridType                         GridType;

    enum { dimRange  = ModelTraits::dimGradRange };
    enum { dimDomain = ModelTraits::dimDomain };

    typedef MyPassTraits< Model, dimRange, polOrd >                    Traits;
    typedef typename Traits :: FunctionSpaceType                     FunctionSpaceType;

    typedef typename Traits :: VolumeQuadratureType                  VolumeQuadratureType;
    typedef typename Traits :: FaceQuadratureType                    FaceQuadratureType;
    typedef typename Traits :: GridPartType                          GridPartType;

    typedef typename Traits :: DiscreteFunctionSpaceType             DiscreteFunctionSpaceType;
    typedef typename Traits :: DestinationType                       DestinationType;
    typedef DestinationType                                          DiscreteFunctionType;

    typedef typename ModelTraits :: DomainType                       DomainType;
    typedef typename DestinationType :: RangeType                    RangeType;
		typedef typename DestinationType :: JacobianRangeType            JacobianRangeType;

    typedef GradientModel< Model, NumFlux, polOrd, passUId >         DGDiscreteModelType;
  };


  // GradientModel
  //--------------

  template <class Model, class NumFlux, int polOrd, int passUId> 
  class GradientModel :
    public Fem::DGDiscreteModelDefaultWithInsideOutside
		<GradientTraits<Model, NumFlux, polOrd, passUId>, passUId >
  {
    typedef Fem::DGDiscreteModelDefaultWithInsideOutside
		<GradientTraits< Model, NumFlux, polOrd, passUId >,passUId > BaseType;

    using BaseType :: inside;
    using BaseType :: outside;

    integral_constant< int, passUId > uVar;

  public:
    typedef GradientTraits< Model, NumFlux, polOrd, passUId >           Traits;

    typedef FieldVector< double, Traits :: dimDomain >                  DomainType;
    typedef FieldVector< double, Traits :: dimDomain-1 >                FaceDomainType;
    typedef typename Traits :: RangeType                                RangeType;
    typedef typename Traits :: GridType                                 GridType;
    typedef typename Traits :: JacobianRangeType                        JacobianRangeType;
    typedef typename Traits :: GridPartType:: IntersectionIteratorType  IntersectionIterator;
    typedef typename IntersectionIterator :: Intersection               Intersection;
    typedef typename GridType :: template Codim< 0 > :: Entity          EntityType;

    enum { evaluateJacobian = NumFlux :: evaluateJacobian };

    // necessary for TESTOPERATOR
    // not sure how it works for dual operators!
    static const bool ApplyInverseMassOperator = true;

  public:
    /**
     * @brief constructor
     */
    GradientModel(const Model& mod,        /*@LST1S@*/
                  const NumFlux& numf) :
      model_( mod ),
      gradientFlux_( numf ),
      cflDiffinv_( 2.0 * ( polOrd + 1) )
    {
    }
    void setTime(double time) {}
    bool hasSource() const { return false; }
    bool hasFlux() const { return true; }  /*@LST1E@*/

    template< class ArgumentTuple, class JacobianTuple > 
    inline double source( const EntityType& en,
                          const double time, 
                          const DomainType& x,
                          const ArgumentTuple& u, 
                          const JacobianTuple& jac, 
                          RangeType& s ) const
    {
      return 0.;
    }

    template <class QuadratureImp, class ArgumentTupleVector > 
    void initializeIntersection(const Intersection& it,
                                const double time,
                                const QuadratureImp& quadInner, 
                                const QuadratureImp& quadOuter,
                                const ArgumentTupleVector& uLeftVec,
                                const ArgumentTupleVector& uRightVec) 
    {
    }

    template <class QuadratureImp, class ArgumentTupleVector > 
    void initializeBoundary(const Intersection& it,
                            const double time,
                            const QuadratureImp& quadInner, 
                            const ArgumentTupleVector& uLeftVec)
    {
    }

    //! dummy method 
    void switchUpwind() const {}

    /**
     * @brief flux function on interfaces between cells for advectionand diffusion
     *
     * @param[in] it intersection
     * @param[in] time current time given by TimeProvider
     * @param[in] x coordinate of required evaluation local to \c it
     * @param[in] uLeft DOF evaluation on this side of \c it
     * @param[in] uRight DOF evaluation on the other side of \c it
     * @param[out] gLeft num. flux projected on normal on this side
     *             of \c it for multiplication with \f$ \phi \f$
     * @param[out] gRight advection flux projected on normal for the other side 
     *             of \c it for multiplication with \f$ \phi \f$
     * @param[out] gDiffLeft num. flux projected on normal on this side
     *             of \c it for multiplication with \f$ \nabla\phi \f$
     * @param[out] gDiffRight advection flux projected on normal for the other side 
     *             of \c it for multiplication with \f$ \nabla\phi \f$
     *
     * @note For dual operators we have \c gDiffLeft = 0 and \c gDiffRight = 0.
     *
     * @return wave speed estimate (multiplied with the integration element of the intersection),
     *              to estimate the time step |T|/wave.
		 */
    template <class QuadratureImp,
              class ArgumentTuple, 
              class JacobianTuple >          /*@LST0S@*/
    double numericalFlux(const Intersection& it,
                         const double time,
                         const QuadratureImp& faceQuadInner,
                         const QuadratureImp& faceQuadOuter,
                         const int quadPoint, 
                         const ArgumentTuple& uLeft,
                         const ArgumentTuple& uRight,
                         const JacobianTuple& jacLeft,
                         const JacobianTuple& jacRight,
                         RangeType& gLeft,
                         RangeType& gRight,
                         JacobianRangeType& gDiffLeft,
                         JacobianRangeType& gDiffRight ) const
    {
      
       return gradientFlux_.gradientNumericalFlux(it, inside(), outside(), time,
																									faceQuadInner, faceQuadOuter, quadPoint,
																									uLeft[ uVar ], uRight[ uVar ], 
																									gLeft, gRight, gDiffLeft, gDiffRight);
    }

    /**
     * @brief same as numericalFlux() but for the boundary
     */
    template <class QuadratureImp, 
              class ArgumentTuple, class JacobianTuple>
    double boundaryFlux(const Intersection& it,
                        double time, 
                        const QuadratureImp& faceQuadInner,
                        const int quadPoint,
                        const ArgumentTuple& uLeft,
                        const JacobianTuple& jacLeft,
                        RangeType& gLeft,
                        JacobianRangeType& gDiffLeft ) const   
    {

      RangeType uRight;
   
			{
				//uRight = uLeft[ uVar ];
			}
      
      return gradientFlux_.gradientBoundaryFlux(it, inside(),
																								time, faceQuadInner, quadPoint,
																								uLeft[ uVar ],
																								uLeft[ uVar], 
																								gLeft,
																								gDiffLeft );
    }

    /**
     * @brief method required by LocalDGPass
     */
    template <class ArgumentTuple, class JacobianTuple> 
    void analyticalFlux(const EntityType& en,
                        const double time, const DomainType& x,
                        const ArgumentTuple& u, 
                        const JacobianTuple& jac,
                        JacobianRangeType& f)
    {
      model_.jacobian(en, time, x, u[ uVar ], f);
    }                              

  private:
    const Model& model_;
    const NumFlux& gradientFlux_;
    const double cflDiffinv_;
  };



}
#include "thetadiscretemodel.hh"
#endif
