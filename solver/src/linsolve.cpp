#include "declare_solver.hpp"
#include "../../lalib/src/declare_lalib.hpp"
#include "../../lalib/src/nonstationarySolvers.hpp"
#include "../../lalib/src/stationarySolvers.hpp"
#include "../../lalib/src/crs/crsMatrix.hpp"


using namespace std;
using namespace lalib;


void solver::solve(string coef_path, string rhs_path, string ret_path,
		   string init_path, string method, string verbosityString) {

  int verbosity = stoi(verbosityString);

  if (verbosity != 0) {
    cout << "\n" << " NOTE: Verbosity not yet supported" << "\n\n";
  }

  // Initialize the matrices

  std::cout << "Forming the coefficient matrix ..." << "\n";
  CRSMatrix A = CRSMatrix(coef_path, 1);

  std::cout << "Forming the right hand side matrix ..." << "\n";
  CRSMatrix b = CRSMatrix(rhs_path, 1);
  CRSMatrix x0;
  
  if (init_path != "") {
    std::cout << "Forming the wanted initial guess ..." << "\n";
    x0 = CRSMatrix(init_path);
  }
  else {
    std::cout << "Forming a 0 matrix as an initial guess ..." << "\n";
    x0 = CRSMatrix(b.nrows(), b.ncols());
  }

  std::cout << "\n" << "Solving a " << x0.nrows() << " dimensional system ..." << "\n";

  CRSMatrix ret;

  clock_t start = clock();
  // Call the solver
  if (method == "" || method == "CG" || method == "cg") {
    std::cout << "Calling the Conjugate Gradient method ..." << "\n";
    ret = cgSolve<CRSMatrix>(A, x0, b);
  }
  else if (method == "Jacobi" || method == "jacobi" || method == "JACOBI") {
    std::cout << "Calling the Jacobi method ..." << "\n";
    ret = jacobiSolve<CRSMatrix>(A, x0, b);
  }
  else if (method == "Gauss-Seidel" || method == "gauss-seidel" || method == "GAUSS-SEIDEL" || method == "GS" || method == "gs"){
    std::cout << "Calling the Gauss-Seidel method ..." << "\n";
    ret = gsSolve<CRSMatrix>(A, x0, b);
  }
  else if (method == "SOR" || method == "sor" || method == "Sor"){
    std::cout << "Calling the SOR method ..." << "\n";
    ret = sorSolve<CRSMatrix>(A, x0, b);
  }
  else {
    throw runtime_error("Improper solver provided!");
  }
  clock_t end = clock();
  
  std::cout << "\n" << "Time taken by the solver: " << ((double)(end - start)) / CLOCKS_PER_SEC << " seconds" << "\n";
  
  std::cout << "Residual norm: " << (A.matmul(ret) - b).norm() << "\n\n";

  std::cout << "Saving the solution as " << ret_path << " ..." << "\n";
  ret.save(ret_path);
}
