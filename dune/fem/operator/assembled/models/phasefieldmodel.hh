#ifndef PHASEFIELD_MIXED_MODEL_HH
#define PHASEFIELD_MIXED_MODEL_HH

#include<dune/common/fvector.hh>
#include<dune/common/fmatrix.hh>

#include<dune/fem/io/parameter.hh>

#include "../phasefieldfilter.hh"
#define LAPLACE 1 
template<class Grid, class Problem>
class PhasefieldModel
{
  public:
    typedef Problem ProblemType;
    typedef Grid GridType;
    enum{ dimDomain = GridType::dimensionworld };
    enum{ dimAcRange = ProblemType::dimRange/2};
    enum{ dimNvStRange=ProblemType::dimRange/2};
    enum{ dimRange = ProblemType::dimRange };

    typedef typename ProblemType::ThermodynamicsType ThermodynamicsType;
    typedef double RangeFieldType; 
    typedef typename Dune::FieldVector<RangeFieldType,dimRange> RangeType;
    typedef typename Dune::FieldVector<RangeFieldType,dimDomain> DomainType; 
    typedef typename Dune::FieldMatrix<RangeFieldType,dimRange,dimDomain> JacobianRangeType;
    typedef typename Dune::FieldMatrix<RangeFieldType, dimDomain , dimDomain> ComponentDiffusionType;
    typedef typename std::array< ComponentDiffusionType,dimDomain> DiffusionTensorType;
    typedef PhasefieldFilter<RangeType> Filter;


    //contructor
  public:
    PhasefieldModel( const ProblemType& problem):
      problem_(problem),
      force_(Dune::Fem::Parameter::getValue<double>("phasefield.gravity",0)),
      diffquotthresh_(Dune::Fem::Parameter::getValue<double>("phasefield.diffquotthresh"))
  {}


    inline void totalEnergy ( const DomainType& xgl,
                              RangeType& vu,
                              double& kin,
                              double& therm,
                              double& total,
                              double& surf ) const;

    // additional Source for the whole syten eg. for 
    // generatring exact solutions
    inline void systemSource ( const double time,
                               const double deltaT,
                               const RangeType& vu,
                               const DomainType& xgl,
                               RangeType& s) const;


    inline void muSource ( const RangeFieldType rho,
                           const RangeFieldType rhoOld,
                           const RangeFieldType phi,
                           RangeFieldType& mu) const;

    inline void drhomuSource ( const RangeFieldType rho,
                                const RangeFieldType rhoOld,
                                const RangeFieldType phi,
                                RangeFieldType& mu) const;

    inline void dphimuSource ( const RangeFieldType rho,
                                const RangeFieldType rhoOld,
                                const RangeFieldType phi,
                                RangeFieldType& mu) const;

    inline void tauSource ( const RangeFieldType phi,
                            const RangeFieldType phiOld,
                            const RangeFieldType rho,
                            RangeFieldType& tau) const;

    inline void dphitauSource ( const RangeFieldType phi,
                                const RangeFieldType phiOld,
                                const RangeFieldType rho,
                                RangeFieldType& tau) const;

    inline void drhotauSource ( const RangeFieldType phi,
                                const RangeFieldType phiOld,
                                const RangeFieldType rho,
                                RangeFieldType& tau) const;


    inline double maxSpeed( const DomainType& normal,
                            const RangeType& u) const;
    
    // this gives an estimation of the lipschitz constant of the rhs of the phasefield equation
    inline double lipschitzC() const
    {
      return problem_.thermodynamics().lipschitzC();
    }

    inline void dirichletValue ( const double time,
                                 const DomainType& xglobal,
                                 RangeType& g) const;


    template< class JacobianVector>
    inline void scalar2vectorialDiffusion ( const RangeType& vu,
                                            const JacobianVector& dphi,
                                            DiffusionTensorType& diffusion ) const;

    template< class JacobianRange >
    inline void diffusion ( const RangeType& vu,
                            const JacobianRange& dvu,
                            JacobianRange& diffusion) const;

    template< class JacobianRange >
    inline void diffusionprime ( const RangeType& vu,
                                 const JacobianRange& dvu,
                                 JacobianRange& diffusion) const;




    inline RangeFieldType pressure (double rho , double phi) const
    {
      return problem_.thermodynamics().pressure( rho , phi );
    }

    inline RangeFieldType h2 ( double rho ) const
    {
      return  problem_.thermodynamics().h2( rho );
    }

    inline RangeFieldType h2prime ( double rho ) const
    {
      return problem_.thermodynamics().h2prime( rho );
    }

    inline double reactionFactor () const
    { 
      return problem_.thermodynamics().reactionFactor();
    }

    inline double delta () const
    {
      return problem_.thermodynamics().delta();
    }

    inline double deltaInv () const
    {
      return problem_.thermodynamics().deltaInv();
    }


  private:
   inline void diffFactors (const RangeFieldType phi, double& mu1, double& mu2 ) const
    {
      mu2 = phi*problem_.thermodynamics().mu2Liq()+(1-phi)*problem_.thermodynamics().mu2Vap();
      mu1 = phi*problem_.thermodynamics().mu1Liq()+(1-phi)*problem_.thermodynamics().mu1Vap();
    }

    inline void diffFactorsPrime (const RangeFieldType phi, double& mu1, double& mu2 ) const
    {
      mu2 = problem_.thermodynamics().mu2Liq()-problem_.thermodynamics().mu2Vap();
      mu1 = problem_.thermodynamics().mu1Liq()-problem_.thermodynamics().mu1Vap();
    }

    template< class JacobianRange >
    inline void diffusion ( const RangeFieldType& mu1,
                            const RangeFieldType& mu2,
                            const RangeType& vu,
                            const JacobianRange& dvu,
                            JacobianRange& diffusion) const;

    template< class JacobianVector>
    inline void scalar2vectorialDiffusion ( const RangeFieldType& mu1,
                                            const RangeFieldType& mu2,
                                            const JacobianVector& dphi,
                                            DiffusionTensorType& diffusion ) const;
 
    const ProblemType& problem_;
    const double force_;
    const double diffquotthresh_;
};

template< class Grid, class Problem>
inline void PhasefieldModel< Grid,Problem>
::totalEnergy ( const DomainType& xgl,
                RangeType& vu,
                double& kin,
                double& therm,
                double& total,
                double& surf ) const
{
  double rho=Filter::rho(vu);
  double phi=Filter::phi(vu);
  double kineticEnergy{0.},surfaceEnergy{0.};
  for(int i=0; i<dimDomain; i++)
  {
    kineticEnergy+=Filter::velocity(vu,i)*Filter::velocity(vu,i);
    surfaceEnergy+=Filter::sigma(vu,i)*Filter::sigma(vu,i);
  }
  
  surfaceEnergy*=h2(rho);
  
  kin=rho*0.5*kineticEnergy;
  surfaceEnergy*=0.5;
  surfaceEnergy*=problem_.thermodynamics().delta();

  therm=problem_.thermodynamics().helmholtz(rho,phi);

  total=therm+kin+surfaceEnergy;
  surf=surfaceEnergy;
}

template< class Grid, class Problem>
inline void PhasefieldModel< Grid,Problem>
::systemSource ( const double time,
                 const double deltaT,
                 const RangeType& vu,
                 const DomainType& xgl,
                 RangeType& s ) const
{
  s=0.;
#if PROBLEM==1
  double x=xgl[0];
  double tn=time+diffquotthresh_*deltaT;
#if GRIDDIM==1
  double y=1.;
  s[0]=0.5*(problem_.thermodynamics().rhsRho(time,x,y)+problem_.thermodynamics().rhsRho(tn,x,y));
  s[1]=0.5*(problem_.thermodynamics().rhsV1(time,x,y)+problem_.thermodynamics().rhsV1(tn,x,y));
  s[2]=0.5*(problem_.thermodynamics().rhsPhi(time,x,y)+problem_.thermodynamics().rhsPhi(tn,x,y));
#else
  double y=xgl[1];
  s[0]=0.5*(problem_.thermodynamics().rhsRho(time,x,y) + problem_.thermodynamics().rhsRho(tn,x,y));
  s[1]=0.5*(problem_.thermodynamics().rhsV1(time,x,y)  + problem_.thermodynamics().rhsV1(tn,x,y) );
  s[2]=0.5*(problem_.thermodynamics().rhsV2(time,x,y)  + problem_.thermodynamics().rhsV2(tn,x,y) );
  s[3]=0.5*(problem_.thermodynamics().rhsPhi(time,x,y) + problem_.thermodynamics().rhsPhi(tn,x,y));
#endif

# else
 //Gravity
  s[dimDomain]=vu[0]*force_;
#endif
}


template<class Grid, class Problem > 
inline void PhasefieldModel< Grid, Problem >
::muSource ( RangeFieldType rho,
             RangeFieldType rhoOld,
             RangeFieldType phi,
             RangeFieldType& mu) const
{
#if TAYLOR
  mu=problem_.thermodynamics().chemicalPotential(rho,phi,rhoOld);
#elif DIFFQUOT
  double diffrho=rho-rhoOld;
  if( std::abs(diffrho)<diffquotthresh_)
    mu=problem_.thermodynamics().chemicalPotential(rhoOld,phi);
  else
    {
      double fnew,fold;
      fnew=problem_.thermodynamics().helmholtz(rho,phi);
      fold=problem_.thermodynamics().helmholtz(rhoOld,phi);
      mu=(fnew-fold)/(rho-rhoOld);
    }
#else
    mu=problem_.thermodynamics().chemicalPotential(rhoOld,phi);
#endif

}
template<class Grid, class Problem > 
inline void PhasefieldModel< Grid, Problem >
::drhomuSource ( RangeFieldType rho,
                 RangeFieldType rhoOld,
                 RangeFieldType phi,
                 RangeFieldType& mu ) const

{
#if TAYLOR
  mu=problem_.thermodynamics().drhochemicalPotential(rho,phi,rhoOld);
#else
  mu=problem_.thermodynamics().drhochemicalPotential(rhoOld,phi);
#endif
}
template<class Grid, class Problem > 
inline void PhasefieldModel< Grid, Problem >
::dphimuSource ( RangeFieldType rho,
                 RangeFieldType rhoOld,
                 RangeFieldType phi,
                 RangeFieldType& mu ) const
{
#if TAYLOR
  mu=problem_.thermodynamics().dphichemicalPotential(rho,phi,rhoOld);
#else
  mu=problem_.thermodynamics().dphichemicalPotential(rhoOld,phi);
#endif
} 


template< class Grid, class Problem > 
inline void PhasefieldModel< Grid, Problem>
::tauSource ( RangeFieldType phi,
              RangeFieldType phiOld,
              RangeFieldType rhoOld,
              RangeFieldType& tau) const
{
#if TAYLOR
  tau=problem_.thermodynamics().reactionSource(rhoOld,phi,phiOld);
#elif DIFFQUOT
  double diffphi=phi-phiOld;
  if( std::abs(diffphi)<diffquotthresh_)
   tau=problem_.thermodynamics().reactionSource(rhoOld,phiOld);
 else
  {
    double fnew,fold;
    fnew=problem_.thermodynamics().helmholtz(rhoOld,phi);
    fold=problem_.thermodynamics().helmholtz(rhoOld,phiOld);
    tau=(fnew-fold)/(phi-phiOld);
  }
#else
   tau=problem_.thermodynamics().reactionSource(rhoOld,phiOld);
#endif 
}

template< class Grid, class Problem > 
inline void PhasefieldModel< Grid, Problem>
::dphitauSource ( RangeFieldType phi,
                  RangeFieldType phiOld,
                  RangeFieldType rho,
                  RangeFieldType& tau) const
{
#if TAYLOR
  tau=problem_.thermodynamics().dphireactionSource(rho,phi,phiOld);
#else
  tau=problem_.thermodynamics().dphireactionSource(rho,phiOld);
#endif
}

template< class Grid, class Problem >
inline void PhasefieldModel< Grid, Problem>
::drhotauSource ( RangeFieldType phi,
                  RangeFieldType phiOld,
                  RangeFieldType rho,
                  RangeFieldType& tau) const
{
#if TAYLOR
  tau=problem_.thermodynamics().drhoreactionSource(rho,phi,phiOld);
#else
  tau=problem_.thermodynamics().drhoreactionSource(rho,phiOld);
#endif
}

template< class Grid, class Problem >
inline double PhasefieldModel< Grid, Problem>
::maxSpeed( const DomainType& normal,
            const RangeType& u) const
{
  RangeType unitnormal=u;

  double unormal(0.);
  for( int ii = 0 ; ii<dimDomain ; ++ii)
    unormal+=u[1+ii]*normal[ii];

  double c=problem_.thermodynamics().a(u[0],u[dimDomain+1])*normal.two_norm2();

  return std::abs(unormal)+std::sqrt(c);
}

template< class Grid, class Problem>
inline void PhasefieldModel< Grid,Problem>
::dirichletValue(const double time, const DomainType& xglobal, RangeType& g) const
{
  problem_.evaluate(time,xglobal,g);
}

template< class Grid, class Problem >
template< class JacobianVector>
inline void PhasefieldModel< Grid, Problem>
::scalar2vectorialDiffusion( const RangeType& vu ,const JacobianVector& dphi,DiffusionTensorType& du) const
{
  double mu1,mu2;
  diffFactors( vu[dimDomain+1], mu1, mu2);
  scalar2vectorialDiffusion( mu1, mu2, dphi, du);
}

template< class Grid, class Problem >
template< class JacobianVector>
inline void PhasefieldModel< Grid, Problem>
::scalar2vectorialDiffusion ( const RangeFieldType& mu1,
                              const RangeFieldType& mu2,
                              const JacobianVector& dphi,
                              DiffusionTensorType& du) const
{
  //du[i][j][k]
  
#if  LAPLACE
  for( int ii=0 ; ii < dimDomain  ; ++ii)
    {
      for(int jj=0 ; jj < dimDomain ; ++jj )
        du[ ii ][ ii ][ jj ] = mu1*dphi[ 0 ][ jj ];
    }
#else
  for( int ii=0 ; ii < dimDomain  ; ++ii)
    {
      for( int jj = 0; jj < dimDomain; ++ jj )
        {
          du[ ii ][ jj ][ ii ]=0.5*mu1*dphi[ 0 ][ jj ];
          du[ ii ][ ii ][ jj ]=0.5*mu1*dphi[ 0 ][ jj ];
        }
      for( int jj=0 ; jj < dimDomain ; ++jj ) 
        {
          du[ ii ][ jj ][ jj ]+=mu2*dphi[ 0 ][ ii ];
        }
     du[ ii ][ ii ][ ii ]+=0.5*mu1*dphi[ 0 ] [ ii ];
    }
#endif
} 

template< class Grid, class Problem > 
template< class JacobianRange >
inline void PhasefieldModel< Grid, Problem>
::diffusion ( const RangeType& vu,
              const JacobianRange& dvu,
              JacobianRange& diff) const
{
  double mu1(0.),mu2(0.);
  diffFactors( vu[dimDomain+1], mu1, mu2);
  diffusion( mu1 , mu2 , vu , dvu , diff );
}

template< class Grid, class Problem >
template< class JacobianRange >
inline void PhasefieldModel< Grid, Problem>
::diffusionprime ( const RangeType& vu,
                   const JacobianRange& dvu,
                   JacobianRange& diff) const
{
  double mu1(0.),mu2(0.);
  diffFactorsPrime( vu[dimDomain+1] , mu1 , mu2 );
  diffusion( mu1 , mu2 , vu , dvu , diff );
}


template< class Grid, class Problem >
template< class JacobianRange >
inline void PhasefieldModel< Grid, Problem>
::diffusion ( const RangeFieldType& mu1,
              const RangeFieldType& mu2,
              const RangeType& vu,
              const JacobianRange& dvu,
              JacobianRange& diffusion) const
{
  diffusion=0;
#if LAPLACE 
   for(int ii = 0 ; ii < dimDomain ; ++ii )
    for(int jj = 0; jj < dimDomain ; ++ jj)
      diffusion[1+ii][jj]=mu1*dvu[1+ii][jj];
      
#else
 
  for(int ii = 0 ; ii < dimDomain ; ++ii )
    {
      for(int jj = 0; jj < dimDomain ; ++ jj)
        {
          diffusion[ 1 +ii ][ ii ]+=mu2*dvu[ 1+jj ][ jj ];
        }

      for(int jj=0; jj<dimDomain ; ++jj )
        {
         diffusion[ 1+ii][ jj ]+=mu1*0.5*(dvu[ ii+1 ][ jj ]+dvu[ 1+jj ][ ii ]);
        } 
      }
#endif
}

#endif

