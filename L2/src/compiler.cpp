#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <iostream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <iostream>
#include <assert.h>

#include <parser.h>
#include "liveness_analysis.h"
#include "interference_graph.h"
#include "spill.h"
#include "graph_coloring.h"
#include <code_generator.h>


void print_help (char *progName){
  // std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l] [-i] SOURCE" << std::endl;
  std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l] [-i] [-c] SOURCE" << std::endl;
  // ^ pass the argument c to run the graph coloring.
  return ;
}

int main(
  int argc, 
  char **argv
  ){
  auto enable_code_generator = true;
  auto spill_only = false;
  auto interference_only = false;
  auto liveness_only = false;
  auto run_color = false; // extra debug
  int32_t optLevel = 3;

  /* 
   * Check the compiler arguments.
   */
  auto verbose = false;
  if( argc < 2 ) {
    print_help(argv[0]);
    return 1;
  }
  int32_t opt;
  int64_t functionNumber = -1;
  while ((opt = getopt(argc, argv, "vg:O:slic")) != -1) {
    switch (opt){

      case 'l':
        liveness_only = true;
        break ;

      case 'i':
        interference_only = true;
        break ;

      case 's':
        spill_only = true;
        break ;

      case 'O':
        optLevel = strtoul(optarg, NULL, 0);
        break ;

      case 'g':
        enable_code_generator = (strtoul(optarg, NULL, 0) == 0) ? false : true ;
        break ;

      case 'v':
        verbose = true;
        break ;

      // our extra debug
      case 'c':
        run_color = true;
        break ;

      default:
        print_help(argv[0]);
        return 1;
    }
  }

  /*
   * Parse the input file.
   */
  L2::Program p;
  if (spill_only){

    /* 
     * Parse an L2 function and the spill arguments.
     */
    //p = L2::parse_spill_file(argv[optind]);
 
  } else if (liveness_only){

    /*
     * Parse an L2 function.
     */
    p = L2::parse_function_file(argv[optind]);
  } else if (interference_only || run_color){

    /*
     * Parse an L2 function.
     */
    p = L2::parse_function_file(argv[optind]);

  } else {

    /* 
     * Parse the L2 program.
     */
    p = L2::parse_file(argv[optind]);

  }

  /*
   * Special cases.
   */
  if (spill_only){
    p = L2::parse_spill_file(argv[optind]);
    auto replacementVar = p.variables[p.variables.size() - 2]; 
    bool changed = L2::spillForL2(p,replacementVar);
    L2::generate_code(p,changed);
  }

  /*
   * Liveness test.
   */
  if (liveness_only){
    auto In_Out_sets = L2::liveness_analysis(&p, true);
    
    return 0;
  }

  /*
   * Interference graph test.
   */
  if (interference_only){
    auto liveness = L2::liveness_analysis(&p, false);
    auto g = new L2::Graph();
    auto interference_graph = g->build_graph(p, liveness);
    interference_graph->printGraph();
    return 0;
  }

  /*
  Extra debug graph coloring test.
  - currently it will just run the graph coloring alg once, edit later
  */
  if (run_color){
    auto liveness = L2::liveness_analysis(&p, false);
    auto g = new L2::Graph();
    auto interference_graph = g->build_graph(p, liveness);
    auto colored_graph = L2::color_graph(interference_graph);
    colored_graph->printGraph();
    // colored_graph->printColors();
    return 0;
  }

  /*
   * Generate the target code.
   */
  if (enable_code_generator){
    auto g = new L2:: Graph();
    // bool changed = false;
    do {
      /*
      We want to continue our process of generating liveness, creating the
      interference graph based on it, and coloring that graph until we are able to
      color all variables. At this point, we can replace all the variables with their
      corresponding colors in the final interference graph.
      */
      auto liveness = L2::liveness_analysis(&p, false);
      auto interference_graph = g->build_graph(p, liveness);
      for (auto var_ptr : interference_graph->spilled_vars) {
        // what is 'changed' for?
        bool changed = L2::spillForL2(p, var_ptr);
      }
    } while (false); // we should really do while interference graph produces new spilled variables

    // use the interference graph to replace the variables in the program with registers
    
    // run code gen on the updated program
    L2::generate_code(p,changed); 
    return 0;
  }

  return 0;
}
