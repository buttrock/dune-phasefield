#ifndef JUMPESTIMATOR_HH 
#define JUMPESTIMATOR_HH

//- Dune includes
// #include <dune/grid/common/referenceelements.hh>

#include <cmath>

//- Dune-fem includes 
#include <dune/fem/quadrature/caching/twistutility.hh>
#include <dune/fem/quadrature/cachingquadrature.hh>
#include <dune/fem/operator/common/spaceoperatorif.hh> 
#include <dune/fem/operator/matrix/blockmatrix.hh>
#include <dune/fem/space/discontinuousgalerkin.hh>
#include <dune/fem/quadrature/intersectionquadrature.hh>
#include <dune/fem/misc/h1norm.hh>

#include <dune/fem/operator/assembled/phasefieldfilter.hh>

namespace Dune
{
                 
  template<class UFunction, class Model>
  class JumpEstimator
  {
    typedef JumpEstimator< UFunction, Model> ThisType;

  public:
    typedef UFunction DiscreteFunctionType;
    typedef Model ModelType;
    typedef typename DiscreteFunctionType :: DiscreteFunctionSpaceType DiscreteFunctionSpaceType;
    typedef typename DiscreteFunctionType :: LocalFunctionType LocalFunctionType;
  
    typedef typename DiscreteFunctionSpaceType :: DomainFieldType DomainFieldType; 
    typedef typename DiscreteFunctionSpaceType :: RangeFieldType RangeFieldType; 
    typedef typename DiscreteFunctionSpaceType :: DomainType DomainType; 
    typedef typename DiscreteFunctionSpaceType :: RangeType RangeType; 
    typedef typename DiscreteFunctionSpaceType :: JacobianRangeType JacobianRangeType; 
    typedef typename DiscreteFunctionSpaceType :: GridPartType GridPartType; 
    typedef typename DiscreteFunctionSpaceType :: IteratorType IteratorType;

    typedef typename GridPartType :: GridType GridType;
    typedef typename GridPartType :: IndexSetType IndexSetType;
    typedef typename GridPartType :: IntersectionIteratorType IntersectionIteratorType;

    typedef typename IntersectionIteratorType :: Intersection IntersectionType;

    typedef typename GridType :: template Codim< 0 > :: Entity ElementType;
    typedef typename GridType :: template Codim< 0 > :: EntityPointer ElementPointerType;
    typedef typename ElementType::Geometry GeometryType;

    static const int dimension = GridType :: dimension;

    typedef Fem::CachingQuadrature< GridPartType, 0 > ElementQuadratureType;
    typedef Fem::CachingQuadrature< GridPartType, 1 > FaceQuadratureType;
    typedef std :: vector< double > ErrorIndicatorType;
    
  protected:
    const DiscreteFunctionType &uh_;
    const double &beta_;
    const DiscreteFunctionSpaceType &dfSpace_;
    const GridPartType &gridPart_;
    const IndexSetType &indexSet_;
    GridType &grid_;
    ErrorIndicatorType indicator_;
    mutable ErrorIndicatorType refined_;
   
    const  ModelType& model_;
    double totalIndicator2_,maxIndicator_;
    const double theta_;
    int maxLevel_;
    int minLevel_;
    const double coarsen_;
    const double tolfactor_;
      // const ProblemType &problem_;

  public:
    explicit JumpEstimator (const DiscreteFunctionType &uh , GridType &grid,const  ModelType& model):
      uh_( uh ),
      beta_(1.),
      dfSpace_( uh.space() ),
      gridPart_( dfSpace_.gridPart() ),
	    indexSet_( gridPart_.indexSet() ),
	    grid_( grid ),
	    indicator_( indexSet_.size( 0 )),
	    refined_( indexSet_.size( 0 )), 
      model_(model),
      totalIndicator2_(0),
	    maxIndicator_(0),
	    theta_( Dune::Fem::Parameter::getValue("phasefield.adaptive.theta",0.) ),
	    maxLevel_(Dune::Fem::Parameter::getValue<int>("fem.adaptation.finestLevel")), 
	    minLevel_(Dune::Fem::Parameter::getValue<int>("fem.adaptation.coarsestLevel")),
	    coarsen_(Dune::Fem::Parameter::getValue<double>("fem.adaptation.coarsenPercent",0.1)),
      tolfactor_( Dune::Fem::Parameter::getValue<double>("phasefield.adaptive.tolfactor",1) ) 
      {
        clear();
      }  
    
    // make this class behave as required for a LocalFunctionAdapter
    typedef Fem::FunctionSpace< double, double, dimension, 2 > FunctionSpaceType;
    void init(const ElementType &en)
    {
      enIndex_ = indexSet_.index(en);
      entity_ = &en;
    }
  
    template< class PointType >
    void evaluate(const PointType& x,FieldVector<double,2> &ret)
    {
    
      ret[0] = indicator_[enIndex_];
      ret[1] = refined_[enIndex_];
    }
  private:
    const ElementType *entity_;
    int enIndex_;

  public:
    void clear ()
    {
      indicator_.resize( indexSet_.size( 0 ));
      refined_.resize( indexSet_.size( 0 ));
      std::fill( indicator_.begin(), indicator_.end(), 0.);
      std::fill( refined_.begin(), refined_.end(), std::numeric_limits<int>::min());
    }
    
    double estimate ( )
    {
      clear();
 
      const IteratorType end = dfSpace_.end();
      
      for( IteratorType it = dfSpace_.begin(); it != end; ++it )
      {
      
        const ElementType &entity = *it;
    
 
        estimateLocal(entity);

         
      }
      
      return computeIndicator();
    }
    
    double computeIndicator()
    {
      totalIndicator2_ = 0.0;
  
        for (unsigned int i=0;i<indicator_.size();++i)
        {
          totalIndicator2_ += indicator_[ i ];
          maxIndicator_ = std::max(maxIndicator_,indicator_[i]);
        }
	
	      return sqrt( totalIndicator2_ );
    }


    //! mark all elements due to given tolerance 
    bool mark ( const double tolerance ) const
    {
      int marked = 0;
      if (tolerance < 0)
      {
        const IteratorType end = dfSpace_.end();
        for( IteratorType it = dfSpace_.begin(); it != end; ++it )
        {
          const ElementType &entity = *it;

           grid_.mark( 1, entity );
          ++marked;
      	}
      }
      else
      {
      	// loop over all elements
        const IteratorType end = dfSpace_.end();
        for( IteratorType it = dfSpace_.begin(); it != end; ++it )
        {
      
         const ElementType &entity = *it;
         int index=indexSet_.index(entity);
         if( indicator_[ index ] > tolerance)
	       {
	          if(entity.level()<maxLevel_)
		        {
 
              grid_.mark( 1, entity );
              refined_[ index ] = 1;	
              
              IntersectionIteratorType end = gridPart_.iend( entity );
		          for( IntersectionIteratorType inter = gridPart_.ibegin( entity ); inter != end; ++inter )
		          {
                const IntersectionType &intersection = *inter;
                if( intersection.neighbor() )
			          {
			            const ElementPointerType pOutside = intersection.outside();
			            const ElementType &outside = *pOutside;  
			        
                  if(outside.level()<maxLevel_)
			            {
                    grid_.mark( 1, outside );
			              refined_[indexSet_.index(outside)]=1;
                    ++marked;
			            
                  }
                }
		          }
            ++marked;
		       }
	        }
          else if( indicator_[ index ] < coarsen_*tolerance )
	        {
            if( entity.level()>minLevel_  )
            { 
              
		          grid_.mark(-1,entity);
            }
          }
	        else
          {
            grid_.mark(0,entity);
	        }
	  

	      }
      }
      
      return (marked > 0);
    
    }
    
    bool estimateAndMark(double tolerance)
    {
      double esti=estimate();
      return mark(tolerance);
    }
    

  protected:
    //! caclulate error on element 
    void estimateLocal ( const ElementType &entity )
    {
      
      const int insideIndex = indexSet_.index( entity );    
 
      const typename ElementType :: Geometry &geometry = entity.geometry();
      const Dune::ReferenceElement< double, dimension > &refElement =
      Dune::ReferenceElements< double, dimension >::general( entity.type() );
      const LocalFunctionType uLocal = uh_.localFunction( entity );

      const double volume = geometry.volume();
      double h2 = (dimension == 2 ? volume : std :: pow( volume, 2.0 / (double)dimension ));  
      //double h2=std::sqrt(volume);
      const int index = indexSet_.index( entity );
     
      ElementQuadratureType quad( entity, 2*(dfSpace_.order() )+1 );
      const int numQuadraturePoints = quad.nop();
      double sigmasquared=0.;
      double maxsigma=0; 
      RangeType range( 0. );
      RangeType rangeNb( 0. );

      double indLocMax=-1E100;
      double indLocMin= 1E100;
      
      for( int qp = 0; qp < numQuadraturePoints; ++qp )
        {
	        DomainType xEn=quad.point( qp );
          uLocal.evaluate(quad[qp],range);
          const double rhoqp=range[dimension+1];
          indLocMax=std::max( indLocMax , rhoqp );
          indLocMin=std::min( indLocMin , rhoqp );
	    
        }

      const IntersectionIteratorType iend = gridPart_.iend( entity );
      for ( IntersectionIteratorType nb = gridPart_.ibegin( entity);nb!=iend; ++nb)
      {
        const IntersectionType& intersection = *nb;
        if( intersection.neighbor() )
        {
          ElementPointerType outp = intersection.outside();
          const ElementType& outside = *outp;
          const int outsideIndex = indexSet_.index( outside );
 
          if ( outside.partitionType() == Dune::GhostEntity ||
                entity.level() > outside.level() ||
              (entity.level() == outside.level() && index < outsideIndex ) )
          {  
            const LocalFunctionType uNbLocal = uh_.localFunction( outside );
            ElementQuadratureType quadNb( entity, 2*(dfSpace_.order() )+1);
            const int numQuadraturePointsNb = quadNb.nop();

            double indLocNbMax=-1E100;
            double indLocNbMin= 1E100;
            
            for( int qpNb=0; qpNb<numQuadraturePointsNb; ++qpNb)
            {
              DomainType xNb=quadNb.point( qpNb );
              uNbLocal.evaluate( xNb, rangeNb );
              const double rhoNb=rangeNb[dimension+1];
              indLocNbMax=std::max( indLocNbMax , rhoNb );
              indLocNbMin=std::min( indLocNbMin , rhoNb );
            }
        
            double indLocalDiff = std::abs( indLocMax - indLocNbMin );
            indLocalDiff = std::max( indLocalDiff, std::abs( indLocMin -indLocNbMax) );
            indicator_[ insideIndex ]  = std::max( indicator_[ insideIndex ]  , indLocalDiff );
            indicator_[ outsideIndex ] = std::max( indicator_[ outsideIndex ] , indLocalDiff );
          } 
        }
      }
    }

    
    
    
  
  };
}
#endif
