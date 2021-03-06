#ifndef DUNE_PHASFIELD_FILTER_HH
#define DUNE_PHASFIELD_FILTER_HH

#include<dune/common/fvector.hh>
#include<dune/common/fmatrix.hh>

template<class Range>
struct PhasefieldFilter
{
public:
  enum { dimDomain = (Range::dimension-2)/2 };
  typedef Dune::FieldMatrix<double,Range::dimension,dimDomain> JacobianRangeType;
  typedef typename Range::field_type RangeFieldType;
  typedef Range RangeType;
  typedef Dune::FieldVector<double, 1> ScalarRangeType;
  typedef Dune::FieldVector<double, dimDomain> VeloRangeType;

  static RangeFieldType& rho( RangeType& u)
  {
    return u[0];
  }

  static RangeFieldType& velocity( RangeType &u,int i)
  {
    assert( i<dimDomain);
    return u[i+1];
  }

  
  static RangeFieldType& mu( RangeType& u)
  {
    return u[dimDomain+1];
  }
  
  static RangeFieldType& sigma( RangeType& u, int i)
  {
    assert(i<dimDomain);
    return u[dimDomain+2+i];
  }
  static RangeFieldType& dvelocity( JacobianRangeType &du,int i,int j)
  {
    assert( i<dimDomain&& j<dimDomain);
    return du[i+1][j];
  }
  
  static RangeFieldType& drho( JacobianRangeType& du,int j)
  {
    assert(j <dimDomain);
    return du[0][j];
  }

  
  
  static RangeFieldType& dmu( JacobianRangeType& du, int j)
  {
     assert(j <dimDomain);
     return du[dimDomain+1][j];
  }
  
  static RangeFieldType& dsigma( JacobianRangeType& du, int i, int j)
  {
   
   assert(i<dimDomain && j<dimDomain );
   return du[dimDomain+2+i][j];
  }
   

};




#endif //DUNE_PHASFIELD_FILTER_HH 
