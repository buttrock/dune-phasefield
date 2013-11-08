#ifndef DUNE_PHASFIELD_FILTER_HH
#define DUNE_PHASFIELD_FILTER_HH

#include<dune/common/fvector.hh>
#include<dune/common/fmatrix.hh>

template<class Range>
struct PhasefieldFilter
{
public:
  enum {dimDomain = (Range::dimension-4)/2};
  typedef Dune::FieldMatrix<double,Range::Dimension,dimDomain> JacobianRangeType;
  typedef typename Range::field_type RangeFieldType;
  typedef Range RangeType;
  typedef Dune::FieldVector<double, 1> ScalarRangeType;
  typedef Dune::FieldVector<double, dimDomain> VeloRangeType;


  static RangeFieldType& velocity( RangeType &u,int i)
  {
    assert( i<dimDomain);
    return u[i+1];
  }
  static RangeFieldType& rho( RangeType& u)
  {
    return u[0];
  }
  static RangeFieldType& phi(RangeType& u)
  {
    return u[dimDomain+1];
  }
  
   static RangeFieldType& mu( RangeType& u)
  {
    return u[dimDomain+2];
  }
  
  static RangeFieldType& tau( RangeType& u)
  {
    return u[dimDomain+3];
  }
  
  static RangeFieldType& sigma( RangeType& u, int i)
  {
    assert(i<dimDomain);
    return u[dimDomain+4+i];
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
  static RangeFieldType& dphi(JacobianRangeType& du,int j)
  {
     assert(j <dimDomain); 
     return du[dimDomain+1][j];
  }
  
   static RangeFieldType& mu( JacobianRangeType& du, int j)
  {
     assert(j <dimDomain);
     return du[dimDomain+2][j];
  }
  
  static RangeFieldType& tau( JacobianRangeType& du,int j)
  {   
    assert(j <dimDomain);
    return du[dimDomain+3][j];
  }
  
  static RangeFieldType& sigma( JacobianRangeType& du, int i, int j)
  {
   
   assert(i<dimDomain);
   return du[dimDomain+4+i][j];
  }


};




#endif //DUNE_PHASFIELD_FILTER_HH 
