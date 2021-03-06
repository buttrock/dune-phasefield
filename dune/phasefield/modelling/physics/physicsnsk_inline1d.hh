#ifndef PHYSICSNSK_INLINE1D_HH
#define PHYSICSNSK_INLINE1D_HH
namespace Dune{
//================================================
//
//
//================================================


template<class Thermodynamics>
class PhasefieldPhysics<1,Thermodynamics>
{
  typedef Thermodynamics ThermodynamicsType;
 
  public:
    enum{dimDomain=1};  
    enum { dimRange = dimDomain+1 };
    enum { dimThetaRange =  1};
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
    thermoDynamics_(thermodyn)
  {
 
  }
   
  inline void conservativeToPrimitive( const RangeType& cons, RangeType& prim ) const;
 
  template< class JacobianRangeImp >
  inline void totalEnergy( const RangeType& cons, 
                           const JacobianRangeImp& grad,
                           double& kin,
                           double& therm,
                           double& surf,
                           double& total ) const;

  inline void chemicalPotential( const RangeType& cons, 
																	double& mu ) const;

	inline void pressure( const RangeType& cons, 
																	 double& p) const;

 
  inline void analyticalFlux( const RangeType& u, JacobianRangeType& f ) const;
  
  inline void jacobian( const RangeType& u, JacobianFluxRangeType& a) const;

  inline double maxSpeed( const DomainType& n, const RangeType& u ) const;
  
  inline double stiffSource(const DomainType& x,
                            const double time,
                            const RangeType& u,
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
	inline void korteweg( const RangeType& u,
												 const JacobianRangeImp& du,
												 ThetaJacobianRangeType& f ) const;
	
  template< class JacobianRangeImp >
	inline void boundarydiffusion( const RangeType& u,
												 const JacobianRangeImp& du,
												 JacobianRangeType& f ) const;
  
  template< class JacobianRangeImp >
	inline void boundarykorteweg( const RangeType& u,
												 const JacobianRangeImp& du,
												 ThetaJacobianRangeType& f ) const;

 
  
public:

	inline double delta()const  { return thermoDynamics_.delta(); }
	inline double deltaInv()const{ return thermoDynamics_.deltaInv(); }
  inline double mu1() const { return thermoDynamics_.mu1();}
 	inline double mu2() const { return thermoDynamics_.mu2();}


};



template< class Thermodynamics >
inline void PhasefieldPhysics< 1, Thermodynamics >
::conservativeToPrimitive( const RangeType& cons, RangeType& prim ) const
 {
  	assert( cons[0] > 0. );
  	double rho,rho_inv;
  	rho=cons[0];
    rho_inv=1./rho;
    
    //velocity 
    prim[0] = cons[1]*rho_inv;
    //pressure  
  	prim[dimDomain] = thermoDynamics_.pressure(rho);
  
  }

 template< class Thermodynamics > 
 template<class JacobianRangeImp>   
 inline void PhasefieldPhysics<1,Thermodynamics >
  :: totalEnergy( const RangeType& cons, 
                  const JacobianRangeImp& grad , 
                  double& kin, 
                  double& therm,
                  double& surf,
                  double& total ) const
  {
	  double rho = cons[0];
	  double rho_inv = 1./rho;
    
    double kineticEnergy,surfaceEnergy;
    kineticEnergy=cons[1]*cons[1];
    double gradrho=grad[0][0];
    surfaceEnergy=gradrho*gradrho;
    
    kineticEnergy*=0.5*rho_inv;
    surfaceEnergy*=delta()*0.5;
   
	  therm = thermoDynamics_.helmholtz( rho );
	  therm += surfaceEnergy;
    kin = kineticEnergy;
    total = therm+kineticEnergy; 
    surf = surfaceEnergy;
  }

  template< class Thermodynamics >
  inline void PhasefieldPhysics<1,Thermodynamics >
  ::chemicalPotential( const RangeType& cons, 
												double& mu ) const
	{
		assert( cons[0] > 1e-20 );

		double rho=cons[0];
   
  	mu=thermoDynamics_.chemicalPotential(rho);
  }

  template< class Thermodynamics >
	inline void PhasefieldPhysics<1,Thermodynamics >
  ::pressure( const RangeType& cons, 
                         double& p ) const
	{
		assert( cons[0] > 1e-20 );
	  
		double rho=cons[0];
		
		p=thermoDynamics_.pressure(rho);
			
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
  }

  template< class Thermodynamics > 
  inline void PhasefieldPhysics< 1, Thermodynamics>
  ::jacobian( const RangeType & u, JacobianFluxRangeType& a) const
  {
    //assert(u[0] > 1e-10);
    double rho=u[0];
    a[0][0] = rho;     //rho
    a[1][0] = u[1]/rho;//(rho v)/rho
  }

	template< class Thermodynamics >
	inline double PhasefieldPhysics< 1, Thermodynamics  >
	::stiffSource(const DomainType& xglobal, //model already gives globla coordinate
                const double time,
                const RangeType& u,
								const GradientRangeType& du,
					  		const ThetaRangeType& theta,
								const ThetaJacobianRangeType& dtheta,
								const JacobianRangeType& jacU,
                RangeType& f) const
	{
    
  
    f[0]=0;
    //-(\rho\nabla\mu) 
    f[1]=-dtheta[0]*u[0];

    return deltaInv(); 
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
  }
  
  template<class Thermodynamics>
  template< class JacobianRangeImp >
  inline void PhasefieldPhysics< 1, Thermodynamics>
  ::korteweg( const RangeType& u,
               const JacobianRangeImp& du,
               ThetaJacobianRangeType& diff ) const
  {
	
    diff[0][0]=-thermoDynamics_.delta()*du[0][0];
  
  }

  template< class Thermodynamics >
  template< class JacobianRangeImp >
  inline void PhasefieldPhysics< 1 ,Thermodynamics >
  ::boundarydiffusion( const RangeType& u,
               const JacobianRangeImp& du,
               JacobianRangeType& diff) const
  {
     diffusion(u,du,diff); 
  }
  
  template<class Thermodynamics>
  template< class JacobianRangeImp >
  inline void PhasefieldPhysics< 1, Thermodynamics>
  ::boundarykorteweg( const RangeType& u,
               const JacobianRangeImp& du,
               ThetaJacobianRangeType& diff ) const
  {
	  diff[0][0]=0.;
  }


 template< class Thermodynamics >
 inline double PhasefieldPhysics< 1, Thermodynamics>
 ::maxSpeed( const DomainType& n, const RangeType& u) const
 {
  assert(u[0] > 1e-20);
  double u_normal=(u[1]*n[0])/u[0];
  double c=thermoDynamics_.a( u[0] );
if(c>0 )
  return std::abs(u_normal)+sqrt(c);
else
  return std::abs(u_normal);
  
 } 

}//end namespace Dune
#endif// PHYSICS_INLINE_HH

