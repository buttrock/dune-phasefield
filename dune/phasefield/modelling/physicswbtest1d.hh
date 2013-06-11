#ifndef PHYSICSWB_INLINE1D_HH
#define PHYSICSWB_INLINE1D_HH
namespace Dune{
//================================================
//
//for dim=1
//
//================================================


template<class Thermodynamics>
class PhasefieldPhysics<1,Thermodynamics>
{
  typedef Thermodynamics ThermodynamicsType;
 
  public:
    enum{dimDomain=1};  
    enum{ phaseId = dimDomain+1} ;
    enum { dimRange = dimDomain + 2 };
    enum { dimThetaRange =  2 };
    enum { dimThetaGradRange = dimThetaRange*dimDomain };
    enum { dimGradRange = dimRange * dimDomain };
    
    typedef double RangeFieldType;

    typedef FieldVector< double, dimDomain >                  DomainType;
    typedef FieldVector< double, dimDomain - 1 >              FaceDomainType;
    typedef FieldVector< double, dimRange >                   RangeType;
    typedef FieldVector< double, dimThetaRange >              ThetaRangeType;
    typedef FieldVector< double, dimGradRange >               GradientType;
    typedef FieldVector< double, dimThetaGradRange >          ThetaGradientRangeType;
    typedef FieldMatrix< double, dimRange, dimDomain >        JacobianRangeType;                          
    typedef FieldMatrix< double, dimRange, dimDomain >        FluxRangeType;
    typedef FieldVector< double, dimGradRange >               GradientRangeType;

   typedef FieldMatrix< double, dimThetaRange, dimDomain >    ThetaJacobianRangeType;
   typedef FieldMatrix< double, dimGradRange, dimDomain >    JacobianFluxRangeType;
  protected:
    const ThermodynamicsType thermoDynamics_;
  public:
  PhasefieldPhysics(const ThermodynamicsType& thermodyn):
    thermoDynamics_(thermodyn),
    delta_(Dune::Fem::Parameter::getValue<double>("phasefield.delta")),
    delta_inv_(1./delta_)
  {
 
  }
   
  inline void conservativeToPrimitive( const RangeType& cons, RangeType& prim ) const;
 
  template< class JacobianRangeImp >
  inline void totalEnergy( const RangeType& cons, 
                           const JacobianRangeImp& grad,
                           double& res ) const;

  inline void chemPotAndReaction( const RangeType& cons, 
																	double& mu,
																	double& reaction ) const;

	inline void pressureAndReaction( const RangeType& cons, 
																	 double& p,
																	 double& reaction ) const;
  inline void nonConProduct(const RangeType & uL, 
														const RangeType & uR,
														const ThetaRangeType& thetaL,
														const ThetaRangeType& thetaR,
														RangeType& ret) const;

 
  inline void analyticalFlux( const RangeType& u, JacobianRangeType& f ) const;
  
  inline void jacobian( const RangeType& u, JacobianFluxRangeType& a) const;

  inline double maxSpeed( const DomainType& n, const RangeType& u ) const;
  
  inline double stiffSource(const RangeType& u,
								            const GradientRangeType& du,
								            const ThetaRangeType& theta,
								            const ThetaJacobianRangeType& dtheta,
								            const JacobianRangeType& uJac,
                            RangeType& f) const;
 
  template< class JacobianRangeImp >
	inline void diffusion( const RangeType& u,
												 const JacobianRangeImp& du,
												 JacobianRangeType& f ) const;
  
  template< class JacobianRangeImp >
	inline void allenCahn( const RangeType& u,
												 const JacobianRangeImp& du,
												 ThetaJacobianRangeType& f ) const;
	
   template< class JacobianRangeImp >
	inline void boundarydiffusion( const RangeType& u,
												 const JacobianRangeImp& du,
												 JacobianRangeType& f ) const;
  
  template< class JacobianRangeImp >
	inline void boundaryallenCahn( const RangeType& u,
												 const JacobianRangeImp& du,
												 ThetaJacobianRangeType& f ) const;
	
  
public:

	inline double delta()const {return delta_;}
	inline double delta_inv()const{return delta_inv_;}
  inline double mu1() const {return thermoDynamics_.mu1();}
 	inline double mu2() const {abort();return 0.;}


protected:
  const double delta_; 
	double delta_inv_;
 };



template< class Thermodynamics >
inline void PhasefieldPhysics< 1, Thermodynamics >
::conservativeToPrimitive( const RangeType& cons, RangeType& prim ) const
 {
  	assert( cons[0] > 0. );
  
  	double rho,rho_inv,phi;
  	rho=cons[0];
    rho_inv=1./rho;
    phi=cons[phaseId];
    phi*=rho_inv;
 
    //velocity 
    prim[0] = cons[1]*rho_inv;
    //pressure  
  	prim[phaseId-1] = thermoDynamics_.pressure(rho,phi);
    //phasefield
    prim[phaseId] = phi;
  
  }

 template< class Thermodynamics > 
 template<class JacobianRangeImp>   
 inline void PhasefieldPhysics<1,Thermodynamics >
  :: totalEnergy( const RangeType& cons, 
                  const JacobianRangeImp& grad , 
                  double& res ) const
  {
    assert( cons[0] > 0. );
	  double rho = cons[0];
	  double rho_inv = 1. /rho;
	  double phi = cons[phaseId];
    phi*=rho_inv;
    
    double kineticEnergy,surfaceEnergy;
    kineticEnergy=cons[1]*cons[1];
    double gradphi=grad[2][0];
    surfaceEnergy=gradphi*gradphi;
 
    kineticEnergy*=0.5*rho_inv;
    surfaceEnergy*=delta_*0.5;
 
	  double freeEnergy = thermoDynamics_.helmholtz( rho, phi );
//    res = kineticEnergy;
    res = surfaceEnergy+freeEnergy+kineticEnergy; 
  }

  template< class Thermodynamics >
  inline void PhasefieldPhysics<1,Thermodynamics >
  ::chemPotAndReaction( const RangeType& cons, 
												double& mu,
												double& reaction ) const
	{
		assert( cons[0] > 1e-20 );
		
		double rho=cons[0];
		double phi=cons[phaseId];
		phi/=rho;
		
		mu=thermoDynamics_.chemicalPotential(rho,phi);
		reaction=thermoDynamics_.reactionSource(rho,phi); 
	}

  template< class Thermodynamics >
	inline void PhasefieldPhysics<1,Thermodynamics >
  ::pressureAndReaction( const RangeType& cons, 
                         double& p,
												 double& reaction ) const
	{
		assert( cons[0] > 1e-20 );
	  
		double rho=cons[0];
		double phi=cons[phaseId];
		phi/=rho;
		
		p=thermoDynamics_.pressure(rho,phi);
		reaction=thermoDynamics_.reactionSource(rho,phi); 
			
		assert( p > 1e-20 );
	}



  template< typename Thermodynamics >
  inline void  PhasefieldPhysics< 1, Thermodynamics>
  ::analyticalFlux( const RangeType& u, JacobianRangeType& f ) const
  {
		assert( u[0] > 1e-10 );
		double rho_inv = 1. / u[0];
		const double v = u[1]*rho_inv;
   
 		f[0][0] = u[1];
		f[1][0] = v*u[1];
		f[2][0] = u[2]*v;
  }

  template< class Thermodynamics > 
  inline void PhasefieldPhysics< 1, Thermodynamics>
  ::jacobian( const RangeType & u, JacobianFluxRangeType& a) const
  {
    //assert(u[0] > 1e-10);

    a[0][0] = u[0]; //rho
    a[1][0] = u[1]/u[0];//(rho v)/rho
    a[2][0] = u[2]/u[0];//(rho phi)/rho
  }

	template< class Thermodynamics >
 	inline void   PhasefieldPhysics< 1, Thermodynamics >
	::nonConProduct(const RangeType & uL, 
									const RangeType & uR,
									const ThetaRangeType& thetaL,
									const ThetaRangeType& thetaR,
									RangeType& ret) const
	{  abort();
	  ret[0]=0;
    ret[1]=0.5*(uL[0]+uR[0])*(thetaL[0]-thetaR[0]);
 	  ret[2]=0;
  }
	
	template< class Thermodynamics >
	inline double PhasefieldPhysics< 1, Thermodynamics  >
	::stiffSource(const RangeType& u,
								const GradientRangeType& du,
					  		const ThetaRangeType& theta,
								const ThetaJacobianRangeType& dtheta,
	              const JaconianRangeType,  
                RangeType& f) const
	{
  	f[0]=0;
		f[1]=-dtheta[0]*u[0]-du[2]*theta[1];
		f[2]=-theta[1];
	  return delta_; 
  }
  template< class Thermodynamics >
  template< class JacobianRangeImp >
  inline void PhasefieldPhysics< 1 ,Thermodynamics >
  ::diffusion( const RangeType& u,
               const JacobianRangeImp& du,
               JacobianRangeType& diff) const
  {
    assert( u[0] > 1e-10 );
		const double muLoc = mu1();
    const double dxv   = du[1][0]; //dv/dx
		diff[0][0]=0.;
		diff[1][0]=muLoc*dxv;
  	diff[2][0]=0.;
 
  }
  template<class Thermodynamics>
  template< class JacobianRangeImp >
  inline void PhasefieldPhysics< 1, Thermodynamics>
  ::allenCahn( const RangeType& u,
               const JacobianRangeImp& du,
               ThetaJacobianRangeType& diff ) const
  {
   assert( u[0] > 1e-10 );
	
	 diff[0][0]=0.;
	 diff[1][0]=-delta_*du[2][0];

  }
  template< class Thermodynamics >
  template< class JacobianRangeImp >
  inline void PhasefieldPhysics< 1 ,Thermodynamics >
  ::boundarydiffusion( const RangeType& u,
               const JacobianRangeImp& du,
               JacobianRangeType& diff) const
  {
    assert( u[0] > 1e-10 );
		const double muLoc = mu1();
    const double dxv   = du[1][0]; //dv/dx
		diff[0][0]=0.;
		diff[1][0]=muLoc*dxv;
  	diff[2][0]=0.;
 
  }
  template<class Thermodynamics>
  template< class JacobianRangeImp >
  inline void PhasefieldPhysics< 1, Thermodynamics>
  ::boundaryallenCahn( const RangeType& u,
               const JacobianRangeImp& du,
               ThetaJacobianRangeType& diff ) const
  {
   assert( u[0] > 1e-10 );
	
	 diff[0][0]=0.;
	 diff[1][0]=0.;

  }


 template< class Thermodynamics >
 inline double PhasefieldPhysics< 1, Thermodynamics>
 ::maxSpeed( const DomainType& n, const RangeType& u) const
 {
  assert(u[0] > MIN_RHO);
  double u_normal=u[1]*n[0]/u[0];
  double c=thermoDynamics_.a(u[0],u[2]);
  return std::abs(u_normal)+sqrt(c);

 } 



}//end namespace Dune
#endif// PHYSICS_INLINE_HH
