#if defined GRIDDIM
#ifndef CODEDIM
#define CODEDIM GRIDDIM
#endif
#endif

// include host specific macros set by configure script   /*@LST0S@*/
#include <config.h>

// include std libs
#include <iostream>
#include <string>

// Dune includes
#include <dune/fem/misc/threadmanager.hh>
#include <dune/fem/misc/l2error.hh>
#include <dune/fem/operator/projection/l2projection.hh>
#include <dune/fem/gridpart/common/gridpart.hh>
#include <dune/fem/solver/odesolver.hh>

// local includes
#include "phasefield.hh"

#if HAVE_PETSC
#include <petsc.h>
#endif

#if HAVE_SLEPC
#include <slepc.h>
#endif


#ifdef BASEFUNCTIONSET_CODEGEN_GENERATE
#include <dune/fem/space/basefunctions/codegen.hh>

std::string autoFilename(const int dim, const int polynomialOrder ) 
{
  std::stringstream name; 
  name << "autogeneratedcode_" << dim << "d_k" << polynomialOrder << ".hh";
  return name.str();
}

void cleanup(const int k, const std::string& filename ) 
{
  std::cerr << "Code for k="<< k << " generated!! " << std::endl;
  Dune::Fem::CodegenInfo :: instance().dumpInfo();
  Dune::Fem::CodegenInfo :: instance().clear();
  std::remove( filename.c_str() );
}

void codegen()
{
  // only one thread for codegen 
  Dune :: Fem :: ThreadManager :: setMaxNumberThreads( 1 );

  try 
  {
    const int dim = CODEDIM;
    std::cout << "Generating Code \n";
    std::string filename;
#ifdef ONLY_ONE_P
    try {
      filename = autoFilename( dim, POLORDER ) ;
      Dune::Fem::CodegenInfo :: instance().setFileName( filename );
      DG_ONE_P :: simulate();  
    }
    catch (Dune::Fem::CodegenInfoFinished) 
    {
      cleanup( POLORDER , filename );
    }
#endif
  }
  catch (Dune::Fem::CodegenInfoFinished) {}


  //////////////////////////////////////////////////
  //  write include header 
  //////////////////////////////////////////////////

  std::ofstream file( "autogeneratedcode.hh" );

  if( file ) 
  {
    for( int dim=1; dim<4; ++dim ) 
    {
      std::stringstream dimfilename;
      dimfilename << "./autogeneratedcode/autogeneratedcode_" << dim << "d.hh";
      file << "#if CODEDIM == " << dim << std::endl;
      file << "#include \"" << dimfilename.str() << "\"" << std::endl;
      file << "#endif" << std::endl;
      
      std::ofstream dimfile( dimfilename.str().c_str() );
      if( dimfile )  
      {
        const int maxPolOrder = ( dim == 3 ) ? 4 : 8 ;
        // max polorder is 4 in 3d and 8 in 2d at the moment 
        for( int i=0; i <= maxPolOrder; ++ i ) 
        {
          dimfile << "#if POLORDER == " << i << std::endl;
          dimfile << "#include \"" << autoFilename( dim, i ) << "\"" << std::endl;
          dimfile << "#endif" << std::endl;
        }
      }
    }
  }

  // dump all information 
  std::cerr << "All automated code generated, bye, bye !! " << std::endl;
}
#endif

/**
 * @brief main function for the LocalDG Advection-Diffusion application
 *
 * \c main starts the Simulation of an advection-diffusion pde with
 * the localdg method with EOC analysis and visual output to grape, paraview or
 * gnuplot.
 * \attention The localdg implementation uses the \c Dune::Pass
 * concept.
 *
 * @param argc number of arguments from command line
 * @param argv array of arguments from command line
 * @param envp array of environmental variables
 * @return 0 we don't program bugs. :)
 */
int main(int argc, char ** argv, char ** envp) {          /*@LST0S@*/

  /* Initialize MPI (always do this even if you are not using MPI) */
  Dune::MPIManager :: initialize( argc, argv );
  try {

#if HAVE_PETSC
    static char help[] = "Petsc-Slepc init";
#endif

#if HAVE_SLEPC
    SlepcInitialize(&argc,&argv,(char*)0,help);
#elif HAVE_PETSC 
    PetscInitialize(&argc,&argv,(char *)0,help);
#endif

  // *** Initialization
  Dune::Parameter::append(argc,argv);                           /*@\label{dg:param0}@*/
  if (argc == 2) {
    Dune::Parameter::append(argv[1]);
  } else {
    Dune::Parameter::append("parameter");                       /*@\label{dg:paramfile}@*/
  }                                                       /*@\label{dg:param1}@*/

  // get number of desired threads (default is 1)
  int numThreads = Dune :: Parameter::getValue< int >("fem.parallel.numberofthreads", 1);
  Dune :: Fem :: ThreadManager :: setMaxNumberThreads( numThreads );

  int polynomialOrder = 1;
  polynomialOrder = Dune::Parameter :: getValue("femhowto.polynomialOrder", polynomialOrder );

// #ifdef BASEFUNCTIONSET_CODEGEN_GENERATE
//   codegen();
// #else 

  // code for only one polynomial degree 

	simulation :: simulate();  
	// write parameters used 
  Dune::Parameter::write("parameter.log");
  }
  catch (Dune::Exception &e) {                            /*@\label{dg:catch0}@*/
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }                                                      /*@\label{dg:catch1}@*/

  return 0;
}
