#ifndef SOLVER_HPP
#define SOLVER_HPP


/*
  TODO: DESCRIPTION
*/


#include <cstdio>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <map>
#include <chrono>
#include <float.h>
#include <time.h>
#include <math.h>
#include <omp.h>

#include "../../utils/messaging.hpp"


namespace solver {

  void welcome(bool always_print);
  
  std::map<std::string, std::string> parse_file(const std::string& filepath);
  
  void solve(std::string coef_path, std::string rhs_path, std::string ret_path,
	     std::string init_path, std::string method, std::string verbosityString);
}


#endif


