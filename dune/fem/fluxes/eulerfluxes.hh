#ifndef DUNE_FEM_DG_EULERFLUXES_HH
#define DUNE_FEM_DG_EULERFLUXES_HH
  
// system includes
#include <string>
#include <cmath>

#include <dune/common/fvector.hh>





////////////////////////////////////////////////////////
//
// Dune interface for Euler fluxes 
//
////////////////////////////////////////////////////////
// LLFFlux
//--------

template< class Model >
class LLFFlux
{
public:
  typedef Model                                       ModelType;
  enum { dimDomain = Model::dimDomain };
  enum { dimRange = Model::dimRange };
  typedef typename Model::Traits                      Traits;
  typedef typename Traits::GridType                   GridType;
  typedef typename GridType::ctype                    ctype;
  typedef typename Traits::EntityType                 EntityType;
  typedef typename Traits::EntityPointerType          EntityPointerType;

  typedef typename Traits::DomainType                 DomainType;
  typedef typename Traits::FaceDomainType             FaceDomainType;
  typedef typename Traits::RangeType RangeType;
  typedef typename Traits::GradientRangeType          GradientType;
  typedef typename Traits::FluxRangeType              FluxRangeType;

  LLFFlux( const Model& mod )
    : model_(mod),
      visc_(Dune::Fem::Parameter::getValue<double>("phasefield.addvisc",0))
  {}

  static std::string name () { return "LLF"; }

  const Model& model() const { return model_; }

  // Return value: maximum wavespeed*length of integrationOuterNormal
  // gLeft,gRight are fluxed * length of integrationOuterNormal
  template< class Intersection, class QuadratureImp >
  inline double 
  numericalFlux( const Intersection& intersection,
                 const EntityType& inside,
                 const EntityType& outside,
                 const double time, 
                 const QuadratureImp& faceQuadInner,
                 const QuadratureImp& faceQuadOuter,
                 const int quadPoint,
                 const RangeType& uLeft, 
                 const RangeType& uRight,
								 RangeType& gLeft,
                 RangeType& gRight) const 
  {
   
    const FaceDomainType& x = faceQuadInner.localPoint( quadPoint );
    DomainType normal = intersection.integrationOuterNormal(x);  
    const double len = normal.two_norm();
    normal *= 1./len;
    
    RangeType visc;
    FluxRangeType anaflux;
   
    model_.advection( inside, time, faceQuadInner.point( quadPoint ),
                     uLeft, anaflux );
    // set gLeft 
    anaflux.mv( normal, gLeft );

    // if there's neighbor, update the value of anaflux
    //if ( intersection.neighbor() )
    model_.advection( outside, time, faceQuadOuter.point( quadPoint ),
											uRight, anaflux );
   
    anaflux.umv( normal, gLeft );

    double maxspeedl, maxspeedr, maxspeed;
    double viscparal, viscparar, viscpara;
    
    const DomainType xGlobal = intersection.geometry().global(x);

    model_.maxSpeed( normal, time, xGlobal, 
                     uLeft, viscparal, maxspeedl );
    model_.maxSpeed( normal, time, xGlobal,
                     uRight, viscparar, maxspeedr );


    maxspeed = (maxspeedl > maxspeedr) ? maxspeedl : maxspeedr;
    viscpara = (viscparal > viscparar) ? viscparal : viscparar;
 

    viscpara*=visc_;
    visc = uRight;
    visc -= uLeft;
    visc *= viscpara;
    gLeft -= visc;
    gLeft *= 0.5*len;
    gRight = gLeft;


    return maxspeed * len;
  }

 protected:
  const Model& model_;
  const double visc_;
};


// Wellbalanced Flux
//--------

template< class Model >
class WBFlux
{
public:
  typedef Model                                       ModelType;
  enum { dimDomain = Model::dimDomain };
  enum { dimRange = Model::dimRange };
  typedef typename Model::Traits                      Traits;
  typedef typename Traits::GridType                   GridType;
  typedef typename GridType::ctype                    ctype;
  typedef typename Traits::EntityType                 EntityType;
  typedef typename Traits::EntityPointerType          EntityPointerType;

  typedef typename Traits::DomainType                 DomainType;
  typedef typename Traits::FaceDomainType             FaceDomainType;
  typedef typename Traits::RangeType                  RangeType;
  typedef typename Traits::GradientRangeType          GradientType;
 
	typedef typename Traits::FluxRangeType              FluxRangeType;
	typedef typename Traits::ThetaRangeType             ThetaRangeType; 
  
	WBFlux( const Model& mod )
    : model_(mod),
      visc_(Dune::Fem::Parameter::getValue<double>("phasefield.addvisc",1)),
			alpha1_(Dune::Fem::Parameter::getValue<double>("phasefield.nonconvisc",0.))
  {
    std::cout<<"Specify alpha="<<alpha1_<<" correctly!\n";
  }

  static std::string name () { return "WB"; }

  const Model& model() const { return model_; }

  // Return value: maximum wavespeed*length of integrationOuterNormal
  // gLeft,gRight are fluxed * length of integrationOuterNormal
  template< class Intersection, class QuadratureImp >
  inline double numericalFlux(const Intersection& intersection,
                              const EntityType& inside,
                              const EntityType& outside,
                              const double time, 
                              const QuadratureImp& faceQuadInner,
                              const QuadratureImp& faceQuadOuter,
                              const int quadPoint,
                              const RangeType& uLeft, 
                              const RangeType& uRight,
								              const ThetaRangeType& thetaLeft,
								              const ThetaRangeType& thetaRight,
								              RangeType& gLeft,
                              RangeType& gRight) const 
  {
    const FaceDomainType& x = faceQuadInner.localPoint( quadPoint );
    DomainType normal = intersection.integrationOuterNormal(x);  
    const double len = normal.two_norm();
    normal *= 1./len;
        
    double rhoLeft  = uLeft[0];
    double rhoRight = uRight[0];
    double phiLeft  = uLeft[dimDomain+1];
    double phiRight = uRight[dimDomain+1];
    phiLeft/=rhoLeft;
    phiRight/=rhoRight;

    double vLeft[dimDomain],vRight[dimDomain];

    for(int i=0; i<dimDomain; i++)
    {
      vLeft[i]  = uLeft[1+i]/rhoLeft;
      vRight[i] = uRight[1+i]/rhoRight;
    }

    

    RangeType visc;
		ThetaRangeType newvisc,thetaFluxLeft,thetaFluxRight;
   	FluxRangeType anaflux;
   
    model_.advection( inside, time, faceQuadInner.point( quadPoint ),
                      uLeft, anaflux );
    // if there's neighbor, update the value of anaflux
    //if ( intersection.neighbor() )
    model_.advection( outside, time, faceQuadOuter.point( quadPoint ),
											uRight, anaflux );
    
    model_.thetaSource( inside, time, faceQuadInner.point( quadPoint ),
                      uLeft, thetaFluxLeft );

    model_.thetaSource( outside, time, faceQuadOuter.point( quadPoint ),
                      uRight,thetaFluxRight );



    // set gLeft 
    anaflux.mv( normal, gLeft );

    //add F(uleft) 
    //if ( intersection.neighbor() )
    anaflux.umv( normal, gLeft );

    double maxspeedl, maxspeedr, maxspeed;
    double viscparal, viscparar, viscpara;
    
    const DomainType xGlobal = intersection.geometry().global(x);
#if  1 
    model_.maxSpeed( normal, time, xGlobal, 
                     uLeft, viscparal, maxspeedl );
    model_.maxSpeed( normal, time, xGlobal,
                     uRight, viscparar, maxspeedr );


    maxspeed = (maxspeedl > maxspeedr) ? maxspeedl : maxspeedr;
    viscpara = (viscparal > viscparar) ? viscparal : viscparar;
    viscpara*=visc_;
    visc = uRight;
   
    visc -= uLeft;

    visc *= viscpara;

    for(int i=1; i<dimDomain;i++)
 		{
			gLeft[i] -= visc[i];
    }

   if( intersection.neighbor() )
   {  
     // \delta\mu  consider sign!!!!!!!!
    newvisc=thetaFluxRight;
    newvisc-=thetaFluxLeft;
      
    // newvisc=.;
     
    gLeft[0]-=newvisc[0];
   	
    for(int i=1; i<dimDomain;i++)
    {
      gLeft[i] -= (vLeft[i]+vRight[i])*0.5*newvisc[0];
    }

    gLeft[dimDomain+1]-=(phiLeft+phiRight)*0.5*newvisc[0];
    
   }  
#endif   
   
   gLeft *= 0.5*len; 
   gRight = gLeft;
 
   RangeType nonConProd(0);

   nonConFlux( normal,
               len,        
               rhoLeft,
               rhoRight, 
               thetaLeft, 
               thetaRight,
               phiLeft,
               phiRight,
               nonConProd);
#if 1          
   gLeft -=nonConProd;
   gRight+=nonConProd;
#endif   
   
   //   std::cout<<"NUmflux out "<<maxspeed << std::endl;
   return maxspeed * len;
  }


  inline void nonConFlux( const DomainType& normal,
                          const double length,  
                          const double rhoLeft,
                           const double rhoRight,
                           const ThetaRangeType& thetaLeft,
                           const ThetaRangeType& thetaRight,
                           const double phiLeft,
                           const double phiRight,
                           RangeType& nonConProd) const
  {
      // {{rho}}
      double averageRho=rhoLeft+rhoRight;
      averageRho*=0.5;
       //[[\mu]]
      double jumpMu=thetaLeft[0]-thetaRight[0];
    

#if USEJACOBIAN
      //{{\tau}}
      double averageTau=thetaLeft[1]-thetaRight[1];
      //[\phi]
      double jumpPhi=phiLeft-phiRight;
#else
      double averageTau=0.;
      double jumpPhi=0.;

#endif
      
      for(int i=0;i<dimDomain;i++)
      {  
        nonConProd[i+1]=normal[i];
        nonConProd[i+1]*=averageRho*jumpMu+averageTau*jumpPhi;        
      }   
 
      //factor comes from the meanvalue of the testfunctions
      nonConProd*=0.5*length;
  }
                           
                           



 protected:
  const Model& model_;
  const double visc_;
	const double alpha1_;
};
#endif // file declaration
