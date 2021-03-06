#ifndef NSK_MIXED_MODEL_HH
#define NSK_MIXED_MODEL_HH

#include<dune/common/fvector.hh>
#include<dune/common/fmatrix.hh>

#include<dune/fem/io/parameter.hh>

#include "../nskfilter.hh"
#define LAPLACE 1 
template<class Grid, class Problem>
class NSKModel
{
  public:
    typedef Problem ProblemType;
    typedef Grid GridType;
    enum{ dimDomain = GridType::dimensionworld };

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
    NSKModel( const ProblemType& problem):
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
                           RangeFieldType& mu) const;

    inline void drhomuSource ( const RangeFieldType rho,
                                const RangeFieldType rhoOld,
                                RangeFieldType& mu) const;
;


    inline double maxSpeed( const DomainType& normal,
                            const RangeType& u) const;
    
    // this gives an estimation of the lipschitz constant of the rhs of the phasefield equation
    inline double lipschitzC() const
    {
      return 1;
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
      return problem_.thermodynamics().pressure( rho  );
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
inline void NSKModel< Grid,Problem>
::totalEnergy ( const DomainType& xgl,
                RangeType& vu,
                double& kin,
                double& therm,
                double& total,
                double& surf ) const
{
  double rho=Filter::rho(vu);
  double kineticEnergy{0.},surfaceEnergy{0.};
  for(int i=0; i<dimDomain; i++)
  {
    kineticEnergy+=Filter::velocity(vu,i)*Filter::velocity(vu,i);
    surfaceEnergy+=Filter::sigma(vu,i)*Filter::sigma(vu,i);
  }
  
  
  kin=rho*0.5*kineticEnergy;
  surfaceEnergy*=0.5;
  surfaceEnergy*=problem_.thermodynamics().delta();

  therm=problem_.thermodynamics().helmholtz(rho);
  therm+=surfaceEnergy;

  total=therm+kin;
  surf=surfaceEnergy;
}

template< class Grid, class Problem>
inline void NSKModel< Grid,Problem>
::systemSource ( const double time,
                 const double deltaT,
                 const RangeType& vu,
                 const DomainType& xgl,
                 RangeType& s ) const
{
  s=0.;
}


template<class Grid, class Problem > 
inline void NSKModel< Grid, Problem >
::muSource ( RangeFieldType rho,
             RangeFieldType rhoOld,
             RangeFieldType& mu) const
{

  mu=problem_.thermodynamics().chemicalPotential(rho,rhoOld);

}
template<class Grid, class Problem > 
inline void NSKModel< Grid, Problem >
::drhomuSource ( RangeFieldType rho,
                 RangeFieldType rhoOld,
                 RangeFieldType& mu ) const

{
  mu=problem_.thermodynamics().drhochemicalPotential(rho,rhoOld);
}


template< class Grid, class Problem >
inline double NSKModel< Grid, Problem>
::maxSpeed( const DomainType& normal,
            const RangeType& u) const
{
  RangeType unitnormal=u;

  double unormal(0.);
  for( int ii = 0 ; ii<dimDomain ; ++ii)
    unormal+=u[1+ii]*normal[ii];

  double c=problem_.thermodynamics().a(u[0])*normal.two_norm2();

  return std::abs(unormal)+std::sqrt(c);
}

template< class Grid, class Problem>
inline void NSKModel< Grid,Problem>
::dirichletValue(const double time, const DomainType& xglobal, RangeType& g) const
{
  problem_.evaluate(time,xglobal,g);
}

template< class Grid, class Problem >
template< class JacobianVector>
inline void NSKModel< Grid, Problem>
::scalar2vectorialDiffusion( const RangeType& vu ,const JacobianVector& dphi,DiffusionTensorType& du) const
{
  double mu1,mu2;
  diffFactors( vu[0], mu1, mu2);
  scalar2vectorialDiffusion( mu1, mu2, dphi, du);
}

template< class Grid, class Problem >
template< class JacobianVector>
inline void NSKModel< Grid, Problem>
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
inline void NSKModel< Grid, Problem>
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
inline void NSKModel< Grid, Problem>
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
inline void NSKModel< Grid, Problem>
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

