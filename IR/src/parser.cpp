#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include "parser.h"

namespace pegtl = TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace IR {

  /*
  Debug flag.
  */
  const int debug = 1;

  /*
  Forward declarations.
  */

  /*
  Counters for temporary variables, etc.
  */
  int64_t counter = 0;

  /*
  Stack for storing the parsed items.
  */
  std::vector<Item*> parsed_items;


  /*
  Keywords.
  */
  struct str_length : TAO_PEGTL_STRING( "length" ) {};
  struct str_return : TAO_PEGTL_STRING( "return" ) {};
  struct str_define : TAO_PEGTL_STRING( "define" ) {};
  struct str_branch : TAO_PEGTL_STRING( "br" ) {};
  struct str_call : TAO_PEGTL_STRING( "call" ) {};
  struct str_print : TAO_PEGTL_STRING( "print" ) {};
  struct str_allocate : TAO_PEGTL_STRING( "allocate" ) {};
  struct str_input : TAO_PEGTL_STRING( "input" ) {};
  struct str_tuperr : TAO_PEGTL_STRING( "tuple-error" ) {};
  struct str_tenserr : TAO_PEGTL_STRING( "tensor-error" ) {};
  struct str_main : TAO_PEGTL_STRING( "@main" ) {};
  struct str_new : TAO_PEGTL_STRING( "new" ) {};
  struct str_Array : TAO_PEGTL_STRING( "Array" ) {};
  struct str_Tuple : TAO_PEGTL_STRING( "Tuple" ) {};
  struct str_array : TAO_PEGTL_STRING( "array" ) {};
  struct str_tuple : TAO_PEGTL_STRING( "tuple" ) {};
  struct str_code : TAO_PEGTL_STRING( "code" ) {};
  struct str_int64 : TAO_PEGTL_STRING( "int64" ) {};
  struct str_void : TAO_PEGTL_STRING( "void" ) {};

  /*
  Terminal symbols.
  */
  struct str_arrow : TAO_PEGTL_STRING( "<-" ) {};
  struct str_comment : TAO_PEGTL_STRING( "//" ) {};
  struct str_bracks: TAO_PEGTL_STRING( "[]" ) {};
  struct str_lesseq : TAO_PEGTL_STRING( "<=" ) {};
  struct str_less : TAO_PEGTL_STRING( "<" ) {};
  struct str_eq : TAO_PEGTL_STRING( "=" ) {};
  struct str_greatereq : TAO_PEGTL_STRING( ">=" ) {};
  struct str_greater : TAO_PEGTL_STRING( ">" ) {};
  struct str_plus : TAO_PEGTL_STRING( "+" ) {};
  struct str_minus : TAO_PEGTL_STRING( "-" ) {};
  struct str_mult : TAO_PEGTL_STRING( "*" ) {};
  struct str_and : TAO_PEGTL_STRING( "&" ) {};
  struct str_shiftl : TAO_PEGTL_STRING( "<<" ) {};
  struct str_shiftr : TAO_PEGTL_STRING( ">>" ) {};
  struct name:
    pegtl::seq<
      pegtl::plus< 
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >
        >
      >,
      pegtl::star<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >,
          pegtl::digit
        >
      >
    > {};
  struct number:
    pegtl::seq<
      pegtl::opt<
        pegtl::sor<
          str_plus,
          str_minus
        >
      >,
      pegtl::plus< 
        pegtl::digit
      >
    > {};
  struct comment: 
    pegtl::disable<
      str_comment,
      pegtl::until< pegtl::eolf > 
    > {};
  struct spaces :
    pegtl::star< 
      pegtl::sor<
        pegtl::one< ' ' >,
        pegtl::one< '\t'>
      >
    > {};
  struct seps : 
    pegtl::star<
      pegtl::seq<
        spaces,
        pegtl::eol
      >
    > {};
  struct seps_with_comments : 
    pegtl::star< 
      pegtl::seq<
        spaces,
        pegtl::sor<
          pegtl::eol,
          comment
        >
      >
    > {};


  /*
  Terminal Rules.
  */
  struct label_rule:
    pegtl::seq<
      pegtl::one< ':' >,
      name
    > {};
  struct I_rule:
    pegtl::seq<
      pegtl::one< '@' >,
      name
    > {};
  // custom rule
  struct stderr_rule:
    pegtl::sor<
      str_tuperr,
      str_tenserr
    > {};
  struct stdlib_rule:
    pegtl::sor<
      str_print,
      str_input,
    > {};
  struct var_rule:
    pegtl::seq<
      pegtl::one< '%' >,
      name
    > {};
  struct N_rule:
    pegtl::seq<
      number
    > {};
  struct op_rule:
    pegtl::sor<
      str_plus,
      str_minus,
      str_mult,
      str_and,
      str_shiftl,
      str_shiftr,
      str_lesseq,
      str_less,
      str_eq,
      str_greatereq,
      str_greater
    > {};


  /*
  Rules
  */

  /*
  Tokens rules.
  */
  struct u_rule:
    pegtl::sor<
      var_rule,
      I_rule
    > {};
  struct t_rule:
    pegtl::sor<
      var_rule,
      N_rule
    > {};
  struct s_rule:
    pegtl::sor<
      t_rule,
      I_rule
    > {};
  struct type_keywords_rule:
    pegtl::sor<
      str_int64,
      str_tuple,
      str_code,
      str_void
    > {};
  struct type_rule:
    pegtl::seq<
      type_keywords_rule,
      pegtl::star< str_bracks >
    > {};
  struct param_rule:
    pegtl::seq<
      var_rule
    > {};
  // struct type_rule:
  //   pegtl::sor<
  //     pegtl::seq<
  //       str_int64,
  //       pegtl::star< str_bracks >
  //     >,
  //     str_tuple,
  //     str_code,
  //     str_void
  //   > {};
  // struct T_rule:
  //   pegtl::sor<
  //     t_rule,
  //     str_void
  //   > {};
  struct callee_rule:
    pegtl::sor<
      u_rule,
      stdlib_rule
    > {};
  // custom tokens
  struct defined_fname:
    pegtl::seq<
      I_rule
    > {};
  struct array_access:
    // [t]
    pegtl::seq<
      pegtl::one< '[' >,
      // arg_rule,
      t_rule,
      pegtl::one< ']' >
    > {};
  struct args_rule:
    pegtl::seq<
      pegtl::opt< t_rule >,
      spaces,
      pegtl::star<
        pegtl::one< ',' >,
        spaces,
        t_rule
      >
    > {};
  struct full_array_access_rule:
    pegtl::plus<
      pegtl::seq<
        spaces,
        array_access
      >
    > {};

  /*
  Label rules.
  */
  struct Label_label_rule:
    // label
    pegtl::seq<
      label_rule
    > {};

  /*
  Terminator rules.
  */
  struct Terminator_single_branch_rule:
    // br label
    pegtl::seq<
      spaces,
      str_branch,
      spaces,
      label_rule
    > {};
  struct Terminator_double_branch_rule:
    // br t label1 label2
    pegtl::seq<
      spaces,
      str_branch,
      spaces,
      t_rule,
      spaces,
      label_rule,
      spaces,
      label_rule
    > {};
  struct Terminator_return_val_rule:
    // return t
    pegtl::seq<
      spaces,
      str_return,
      spaces,
      t_rule
    > {};
  struct Terminator_return_rule:
    // return
    pegtl::seq<
      spaces,
      str_return
    > {};
  // struct Terminator_rule:
  //   pegtl::sor<
  //     pegtl::seq< pegtl::at< Terminator_return_val_rule >, Terminator_return_val_rule >,
  //     pegtl::seq< pegtl::at< Terminator_return_rule >, Terminator_return_rule >,
  //     pegtl::seq< pegtl::at< Terminator_double_branch_rule >, Terminator_double_branch_rule >,
  //     pegtl::seq< pegtl::at< Terminator_single_branch_rule >, Terminator_single_branch_rule >
  //   > {};

  /*
  Instruction rules.
  */
  struct Instruction_type_declaration_rule:
    // type var
    pegtl::seq<
      spaces,
      type_rule,
      spaces,
      var_rule
    > {};
  struct Instruction_operation_rule:
    // var <- t1 op t2
    pegtl::seq<
      spaces,
      var_rule,
      spaces,
      str_arrow,
      spaces,
      t_rule,
      spaces,
      op_rule,
      spaces,
      t_rule
    > {};
  struct Instruction_assignment_rule:
    // var <- s
    pegtl::seq<
      spaces,
      var_rule,
      spaces,
      str_arrow,
      spaces,
      s_rule
    > {};
  struct Instruction_load_rule:
    // var1 <- var2[t]...
    pegtl::seq<
      spaces,
      var_rule,
      spaces,
      str_arrow,
      spaces,
      var_rule,
      spaces,
      full_array_access_rule
    > {};
  struct Instruction_store_rule:
    // var1[t]... <- s
    pegtl::seq<
      spaces,
      var_rule,
      spaces,
      full_array_access_rule,
      spaces,
      str_arrow,
      spaces,
      s_rule
    > {};
  struct Instruction_dim_length_rule:
    // var1 <- length var2 t
    pegtl::seq<
      spaces,
      var_rule,
      spaces,
      str_arrow,
      spaces,
      str_length,
      spaces,
      var_rule,
      spaces,
      t_rule
    > {};
  struct Instruction_length_rule:
    // var1 <- length var2
    pegtl::seq<
      spaces,
      var_rule,
      spaces,
      str_arrow,
      spaces,
      str_length,
      spaces,
      var_rule
    > {};
  struct Instruction_error_call_rule:
    // call <tuple/tensor>-error ( args? )
    pegtl::seq<
      spaces,
      str_call,
      spaces,
      stderr_rule,
      spaces,
      pegtl::one< '(' >,
      spaces,
      pegtl::opt< args_rule >,
      spaces,
      pegtl::one< ')' >
    > {};
  struct Instruction_call_function_rule:
    // call callee ( args? )
    pegtl::seq<
      spaces,
      str_call,
      spaces,
      callee_rule,
      spaces,
      pegtl::one< '(' >,
      spaces,
      pegtl::opt< args_rule >,
      spaces,
      pegtl::one< ')' >
    > {};
  struct Instruction_call_function_assignment_rule:
    // var <- call callee ( args? )
    pegtl::seq<
      spaces,
      var_rule,
      spaces,
      str_arrow,
      spaces,
      str_call,
      spaces,
      callee_rule,
      spaces,
      pegtl::one< '(' >,
      spaces,
      pegtl::opt< args_rule >,
      spaces,
      pegtl::one< ')' >
    > {};
  struct Instruction_array_initialization_rule:
    // var <- new Array( args )
    pegtl::seq<
      spaces,
      var_rule,
      spaces,
      str_arrow,
      spaces,
      str_new,
      spaces,
      str_Array,
      spaces,
      pegtl::one< '(' >,
      spaces,
      args_rule,
      spaces,
      pegtl::one< ')' >
    > {};
  struct Instruction_tuple_initialization_rule:
    // var <- new Tuple( t )
    pegtl::seq<
      spaces,
      var_rule,
      spaces,
      str_arrow,
      spaces,
      str_new,
      spaces,
      str_Tuple,
      spaces,
      pegtl::one< '(' >,
      spaces,
      t_rule,
      spaces,
      pegtl::one< ')' >
    > {};
  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at< Label_label_rule >, Label_label_rule >,
      pegtl::seq< pegtl::at< Terminator_return_val_rule >, Terminator_return_val_rule >,
      pegtl::seq< pegtl::at< Terminator_return_rule >, Terminator_return_rule >,
      pegtl::seq< pegtl::at< Terminator_double_branch_rule >, Terminator_double_branch_rule >,
      pegtl::seq< pegtl::at< Terminator_single_branch_rule >, Terminator_single_branch_rule >,
      // ^new additions for the "everything is an instruction" approach
      pegtl::seq< pegtl::at< Instruction_type_declaration_rule >, Instruction_type_declaration_rule >,
      pegtl::seq< pegtl::at< Instruction_array_initialization_rule >, Instruction_array_initialization_rule >,
      pegtl::seq< pegtl::at< Instruction_tuple_initialization_rule >, Instruction_tuple_initialization_rule >,
      pegtl::seq< pegtl::at< Instruction_call_function_assignment_rule >, Instruction_call_function_assignment_rule >,
      pegtl::seq< pegtl::at< Instruction_call_function_rule >, Instruction_call_function_rule >,
      pegtl::seq< pegtl::at< Instruction_dim_length_rule >, Instruction_dim_length_rule >,
      pegtl::seq< pegtl::at< Instruction_length_rule >, Instruction_length_rule >,
      pegtl::seq< pegtl::at< Instruction_operation_rule >, Instruction_operation_rule >,
      pegtl::seq< pegtl::at<Instruction_load_rule>, Instruction_load_rule >,
      pegtl::seq< pegtl::at<Instruction_store_rule>, Instruction_store_rule>,
      pegtl::seq< pegtl::at< Instruction_assignment_rule >, Instruction_assignment_rule >
    > {};
  struct Instructions_rule:
    pegtl::star<
      pegtl::seq<
        seps_with_comments,
        // pegtl::bol,
        spaces,
        Instruction_rule,
        seps_with_comments
      >
    > {};
      
  /*
  Basic Block rules.
  */
  // struct BB_rule:
  //   pegtl::seq<
  //     spaces,
  //     // label_rule,
  //     Label_label_rule,
  //     seps_with_comments,
  //     spaces,
  //     Instructions_rule,
  //     seps_with_comments,
  //     spaces,
  //     Terminator_rule
  //   > {};
  // struct BBs_rule:
  //   pegtl::plus<
  //     seps_with_comments,
  //     BB_rule,
  //     seps_with_comments
  //   > {};
  
  /*
  Function rules.
  */

  struct Function_header_rule:
    pegtl::seq<
      spaces,
      str_define,
      spaces,
      type_rule,
      spaces,
      defined_fname,
      spaces,
      pegtl::one< '(' >,
      spaces,
      pegtl::star<
        pegtl::seq<
          spaces,
          type_rule,
          spaces,
          // var_rule,
          param_rule,
          spaces,
          pegtl::opt< 
            pegtl::one< ',' >
          >,
          spaces
        >
      >,
      spaces,
      pegtl::one< ')' >
    > {};
  // struct Function_header_rule:
  //   pegtl::seq<
  //     spaces,
  //     str_define,
  //     spaces,
  //     type_rule,
  //     spaces,
  //     defined_fname,
  //     spaces,
  //     pegtl::one< '(' >,
  //     spaces,
  //     pegtl::seq<
  //       pegtl::opt<
  //         pegtl::seq<
  //           type_rule,
  //           spaces,
  //           var_rule
  //         >
  //       >,
  //       spaces,
  //       pegtl::star<
  //         pegtl::seq<
  //           pegtl::one< ',' >,
  //           spaces,
  //           var_rule
  //         >
  //       >
  //     >,
  //     spaces,
  //     pegtl::one< ')' >
  //   > {};
  struct Function_rule:
    pegtl::seq<
      Function_header_rule,
      seps,
      spaces,
      pegtl::one< '{' >,
      // BBs_rule,
      seps_with_comments,
      Instructions_rule,
      seps_with_comments,
      spaces,
      pegtl::one< '}' >
    > {};
  struct Functions_rule:
    pegtl::plus<
      pegtl::seq<
        seps_with_comments,
        Function_rule,
        seps_with_comments
      >
    > {};

  /*
  Program rules.
  */
  struct entry_point_rule:
    pegtl::seq<
      Functions_rule
    > {};

  /*
  Grammar rules.
  */
  struct full_grammar :
    pegtl::must<
      entry_point_rule
    > {};


  /*
  Actions.
  */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  /*
  Debug actions.
  */
  template<> struct action < str_branch > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized a str_branch rule\n";
    }
  };

  template<> struct action < spaces > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // if (debug) std::cerr << "Recognized a spaces rule\n";
    }
  };

  template<> struct action < seps_with_comments > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // if (debug) std::cerr << "Recognized a seps_with_comments rule\n";
    }
  };


  /*
  Terminal actions.
  */
  template<> struct action < str_bracks > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized a str_bracks rule\n";

      TypeEnum bracks;
      auto it = stringToTypeEnum.find(in.string());
      if (it != stringToTypeEnum.end()) {
        bracks = it->second;
      } else {
        if (debug) std::cerr << "string " << in.string() << " is not a valid type.\n";
      }

      auto type = new Type(bracks);
      parsed_items.push_back(type);
    }
  };

  template<> struct action < type_keywords_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized a type_keywords rule\n";

      TypeEnum type_keyword;
      auto it = stringToTypeEnum.find(in.string());
      if (it != stringToTypeEnum.end()) {
        type_keyword = it->second;
      } else {
        if (debug) std::cerr << "string " << in.string() << " is not a valid type.\n";
      }

      auto type = new Type(type_keyword);
      parsed_items.push_back(type);
    }
  };

  template<> struct action < label_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized a label rule\n";
      auto label = new Label(in.string());
      parsed_items.push_back(label);
    }
  };

  template<> struct action < I_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized an I rule\n";

      auto name = new userFuncName(in.string());
      parsed_items.push_back(name);
      if (debug) std::cerr << "pushed new fname onto, parsed_items now has size " << parsed_items.size() << "\n";
    }
  };

  template<> struct action < stderr_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized an stderr rule\n";

      auto name = new userFuncName(in.string());
      parsed_items.push_back(name);
    }
  };

  template<> struct action < stdlib_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized an stdlib rule\n";

      auto name = new userFuncName(in.string());
      parsed_items.push_back(name);
    }
  };

  template<> struct action < var_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized an var rule for " << in.string() << "\n";

      auto variable = new Variable(in.string());
      parsed_items.push_back(variable);
    }
  };

  template<> struct action < N_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized an N rule\n";

      auto number = new Number(std::stoll(in.string()));
      parsed_items.push_back(number);
    }
  };

  template<> struct action < op_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized an op rule\n";

      OperatorEnum op;
      auto it = stringToOperatorEnum.find(in.string());
      if (it != stringToOperatorEnum.end()) {
        op = it->second;
      } else {
        if (debug) std::cerr << "string " << in.string() << " is not a valid operator.\n";
      }

      auto operator_instance = new Operator(op);
      parsed_items.push_back(operator_instance);
    }
  };
  
  // // custom, not in explicit grammar
  // template<> struct action < arg_rule > {
  //   template< typename Input >
  //   static void apply( const Input & in, Program & p) {
  //     if (debug) std::cerr << "Recognized an arg rule\n";

  //     auto arg = parsed_items.back();
  //     parsed_items.pop_back();
  //     parsed_args.push(arg);
  //   }
  // };

  template<> struct action < type_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized a type rule\n";

      if (debug) std::cerr << "parsed_items has size " << parsed_items.size() << "\n";

      auto type_temp = parsed_items.front();
      parsed_items.erase(parsed_items.begin());
      if (debug) std::cerr << "made it past grabbing the type " << type_temp->print() << "\n";
      if (debug) std::cerr << "parsed_items now has size " << parsed_items.size() << "\n";

      int64_t dims = 0;
      while (!parsed_items.empty()) {
        // if theres anything on the stack, i think it can be assumed that they're brackets. 
        parsed_items.pop_back();
        dims++;
      }

      auto type = dynamic_cast<Type*>(type_temp);
      if (!type) {
        if (debug) std::cerr << "Program is incorrect, " << type->print() << " should be of Type type.\n";
      }

      type->dims = dims;
      parsed_items.push_back(type);
      if (debug) std::cerr << "pushed onto, parsed_items now has size " << parsed_items.size() << "\n";
    }
  };

  template<> struct action < param_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized a param rule for " << in.string() << "\n";

      auto f = p.functions.back();
      auto param_temp = parsed_items.back();
      parsed_items.pop_back();
      auto type_temp = parsed_items.back();
      parsed_items.pop_back();

      auto param = dynamic_cast<Variable*>(param_temp);
      if (!param) {
        if (debug) std::cerr << "Program is incorrect, " << param->print() << " should be of variable type.\n";
      }

      auto type = dynamic_cast<Type*>(type_temp);
      if (!type) {
        if (debug) std::cerr << "Program is incorrect, " << type->print() << " should be of Type type.\n";
      }

      /*
      Associate the variable within the containing function's maps.
      */
      std::string param_name = param->print();
      f->variableNameToPointer[param_name] = param;
      // f->variableToTypeMapping[var] = type;
      f->variableNameToType[param_name] = type;

      f->parameters.push_back(param);
    }
  };

  // custom, not in explicit grammar ()
  template<> struct action < defined_fname > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized a defined_fname rule for function " << in.string() << "\n";

      auto fname_temp = parsed_items.back();
      parsed_items.pop_back();
      auto type_temp = parsed_items.back();
      parsed_items.pop_back();

      auto fname = dynamic_cast<userFuncName*>(fname_temp);
      if (!fname) {
        if (debug) std::cerr << "Program is incorrect, " << fname->print() << " should be of userFuncName type.\n";
      }

      auto type = dynamic_cast<Type*>(type_temp);
      if (!type) {
        if (debug) std::cerr << "Program is incorrect, " << type->print() << " should be of Type type.\n";
      }

      if (debug) std::cerr << "grabbed the type and fname, parsed_items now has size " << parsed_items.size() << "\n";
      if (debug) std::cerr << "p.functions has size " << p.functions.size() << "\n";

      auto function = new Function(fname, type);
      p.functions.push_back(function);

      if (debug) std::cerr << "created a function and added it, p.functions now has size " << p.functions.size() << "\n";
    }
  };


  /*
  Group actions.
  */
  // for matching variables in a function definition.
  // template<> struct action < params_rule > {
  //   template< typename Input >
  //   static void apply( const Input & in, Program & p) {
  //     if (debug) std::cerr << "Recognized an params rule\n";

  //     std::vector<Item*> args_vec;

  //     /*
  //     Populate the new vector of args, avoid making a copy just in case.
  //     */
  //     while (!parsed_items.empty()) {
  //       auto arg = parsed_items.front();
  //       parsed_items.erase(parsed_items.begin());
  //       args_vec.push_back(arg);
  //     }

  //     auto args = new varArguments(args_vec);
  //   }
  // };

  // // for matching variables in a function call.
  // template<> struct action < args_rule > {
  //   template< typename Input >
  //   static void apply( const Input & in, Program & p) {
  //     if (debug) std::cerr << "Recognized an args rule\n";

  //     std::vector<Item*> args_vec;

  //     /*
  //     Populate the new vector of args, avoid making a copy just in case.
  //     */
  //     while (!parsed_items.empty()) {
  //       auto arg = parsed_items.front();
  //       parsed_items.erase(parsed_items.begin());
  //       args_vec.push_back(arg);
  //     }

  //     auto args = new varArguments(args_vec);
  //   }
  // };

  // template<> struct action < full_array_access_rule > {
  //   template< typename Input >
  //   static void apply( const Input & in, Program & p) {
  //     if (debug) std::cerr << "Recognized an full_array rule\n";

  //     std::vector<Item*> accesses_vec;

  //     /*
  //     Populate the new vector of access args, avoid making a copy just in case.
  //     */
  //     while (!parsed_items.empty()) {
  //       auto access = parsed_items.front();
  //       parsed_items.erase(parsed_items.begin());
  //       accesses_vec.push_back(access);
  //     }

  //     /*
  //     TODO need to wrap up here once i figure out what arrAccess class is for.
  //     */

  //     auto args = new varArguments()
  //   }
  // };

  /*
  Label actions.
  */
  template<> struct action < Label_label_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // label
      if (debug) std::cerr << "Recognized an Label_label rule\n";

      auto f = p.functions.back();
      auto label_temp = parsed_items.back();
      parsed_items.pop_back();

      auto label = dynamic_cast<Label*>(label_temp);
      if (!label) {
        if (debug) std::cerr << "Program is incorrect, " << label->print() << " should be of Label type.\n";
      }

      auto i = new labelInstruction(label);
      f->instructions.push_back(i);
    }
  };


  /*
  Instruction actions.
  */
  template<> struct action < Instruction_type_declaration_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // type var
      if (debug) std::cerr << "Recognized an Instruction_type_declaration rule\n";

      auto f = p.functions.back();
      auto var_temp = parsed_items.back();
      parsed_items.pop_back();
      auto type_temp = parsed_items.back();
      parsed_items.pop_back();

      auto var = dynamic_cast<Variable*>(var_temp);
      if (!var) {
        if (debug) std::cerr << "Program is incorrect, " << var->print() << " should be of variable type.\n";
      }

      auto type = dynamic_cast<Type*>(type_temp);
      if (!type) {
        if (debug) std::cerr << "Program is incorrect, " << type->print() << " should be of Type type.\n";
      }

      /*
      Associate the variable within the containing function's maps.
      */
      std::string var_name = var->print();
      f->variableNameToPointer[var_name] = var;
      // f->variableToTypeMapping[var] = type;
      f->variableNameToType[var_name] = type;

      auto i = new declarationInstruction(type, var);
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_assignment_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // var <- s
      if (debug) std::cerr << "Recognized an Instruction_assignment rule\n";

      auto f = p.functions.back();
      auto s = parsed_items.back();
      parsed_items.pop_back();
      auto var_temp = parsed_items.back();
      parsed_items.pop_back();

      auto var = dynamic_cast<Variable*>(var_temp);
      if (!var) {
        if (debug) std::cerr << "Program is incorrect, " << var->print() << " should be of variable type.\n";
      }

      auto i = new Assignment(var, s);
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_operation_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // var <- t1 op t2
      if (debug) std::cerr << "Recognized an Instruction_operation rule\n";

      auto f = p.functions.back();
      auto t2 = parsed_items.back();
      parsed_items.pop_back();
      auto op_temp = parsed_items.back();
      parsed_items.pop_back();
      auto t1 = parsed_items.back();
      parsed_items.pop_back();
      auto var_temp = parsed_items.back();
      parsed_items.pop_back();

      auto var = dynamic_cast<Variable*>(var_temp);
      if (!var) {
        if (debug) std::cerr << "Program is incorrect, " << var->print() << " should be of variable type.\n";
      }

      auto op = dynamic_cast<Operator*>(op_temp);
      if (!op) {
        if (debug) std::cerr << "Program is incorrect, " << op->print() << " should be of Operator type.\n";
      }

      auto i = new operationInstruction(var, t1, op, t2);
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_load_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // var1 <- var2[t]...
      if (debug) std::cerr << "Recognized an Instruction_load rule\n";

      auto f = p.functions.back();
      auto var1_temp = parsed_items.front();
      parsed_items.erase(parsed_items.begin());
      auto var2_temp = parsed_items.front();
      parsed_items.erase(parsed_items.begin());

      auto var1 = dynamic_cast<Variable*>(var1_temp);
      if (!var1) {
        if (debug) std::cerr << "Program is incorrect, " << var1->print() << " should be of variable type.\n";
      }

      auto var2 = dynamic_cast<Variable*>(var2_temp);
      if (!var2) {
        if (debug) std::cerr << "Program is incorrect, " << var2->print() << " should be of variable type.\n";
      }

      auto i = new loadInstruction(var1, var2);

      while (!parsed_items.empty()) {
        auto arg = parsed_items.front();
        parsed_items.erase(parsed_items.begin());
        i->index_args_vec.push_back(arg);
      }

      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_store_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // var1[t]... <- s
      if (debug) std::cerr << "Recognized an Instruction_store rule\n";

      auto f = p.functions.back();
      auto s = parsed_items.back();
      parsed_items.pop_back();
      auto var1_temp = parsed_items.front();
      parsed_items.erase(parsed_items.begin());

      auto var1 = dynamic_cast<Variable*>(var1_temp);
      if (!var1) {
        if (debug) std::cerr << "Program is incorrect, " << var1->print() << " should be of variable type.\n";
      }

      auto i = new storeInstruction(var1, s);

      while (!parsed_items.empty()) {
        auto arg = parsed_items.front();
        parsed_items.erase(parsed_items.begin());
        i->index_args_vec.push_back(arg);
      }

      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_dim_length_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // var1 <- length var2 t
      if (debug) std::cerr << "Recognized an Instruction_dim_length rule\n";

      auto f = p.functions.back();
      auto t = parsed_items.back();
      parsed_items.pop_back();
      auto var2_temp = parsed_items.back();
      parsed_items.pop_back();
      auto var1_temp = parsed_items.back();
      parsed_items.pop_back();

      auto var1 = dynamic_cast<Variable*>(var1_temp);
      if (!var1) {
        if (debug) std::cerr << "Program is incorrect, " << var1->print() << " should be of variable type.\n";
      }

      auto var2 = dynamic_cast<Variable*>(var2_temp);
      if (!var2) {
        if (debug) std::cerr << "Program is incorrect, " << var2->print() << " should be of variable type.\n";
      }

      auto i = new arrLength(var1, var2, t);
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_length_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // var1 <- length var2
      if (debug) std::cerr << "Recognized an Instruction_length rule\n";

      auto f = p.functions.back();
      auto var2_temp = parsed_items.back();
      parsed_items.pop_back();
      auto var1_temp = parsed_items.back();
      parsed_items.pop_back();

      auto var1 = dynamic_cast<Variable*>(var1_temp);
      if (!var1) {
        if (debug) std::cerr << "Program is incorrect, " << var1->print() << " should be of variable type.\n";
      }

      auto var2 = dynamic_cast<Variable*>(var2_temp);
      if (!var2) {
        if (debug) std::cerr << "Program is incorrect, " << var2->print() << " should be of variable type.\n";
      }

      auto i = new tupleLength(var1, var2);
      f->instructions.push_back(i);
    }
  };

 template<> struct action < Instruction_call_error_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // call callee ( args? )
      if (debug) std::cerr << "Recognized an Instruction_error_call rule\n";

      auto f = p.functions.back();
      auto callee = parsed_items.front();
      parsed_items.erase(parsed_items.begin());

      Error *i;
      auto it = stringToErrorEnum.find(callee->print());
      if (it != stringToErrorEnum.end()) {
        i = new Error(it->second);
      } else {
        if (debug) std::cerr << "Something went wrong, parser recognized an error call rule but callee " << callee->print() << " is not an error type.\n";
      }

      while (!parsed_items.empty()) {
        auto arg = parsed_items.front();
        parsed_items.erase(parsed_items.begin());
        i->args.push_back(arg);
      }

      f->instructions.push_back(i);
    }
  };

 template<> struct action < Instruction_call_error_assignment_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // call callee ( args? )
      if (debug) std::cerr << "Recognized an Instruction_error_call_assignment rule\n";

      auto f = p.functions.back();
      auto var_temp = parsed_items.front();
      parsed_items.erase(parsed_items.begin());
      auto callee = parsed_items.front();
      parsed_items.erase(parsed_items.begin());

      Error *i;
      auto it = stringToErrorEnum.find(callee->print());
      if (it != stringToErrorEnum.end()) {
        i = new Error(it->second);
      } else {
        if (debug) std::cerr << "Something went wrong, parser recognized an error call rule but callee " << callee->print() << " is not an error type.\n";
      }

      auto var = dynamic_cast<Variable*>(var_temp);
      if (!var) {
        if (debug) std::cerr << "Program is incorrect, " << var->print() << " should be of variable type.\n";
      }

      i->has_dest = true;
      i->dest = var;

      while (!parsed_items.empty()) {
        auto arg = parsed_items.front();
        parsed_items.erase(parsed_items.begin());
        i->args.push_back(arg);
      }

      f->instructions.push_back(i);
    }
  };


  template<> struct action < Instruction_call_function_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // call callee ( args? )
      if (debug) std::cerr << "Recognized an Instruction_call_function rule\n";

      auto f = p.functions.back();
      auto callee = parsed_items.front();
      parsed_items.erase(parsed_items.begin());

      auto i = new VoidCallInstruction(callee);

      while (!parsed_items.empty()) {
        auto arg = parsed_items.front();
        parsed_items.erase(parsed_items.begin());
        i->args.push_back(arg);
      }

      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_call_function_assignment_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // var <- call callee ( args? )
      if (debug) std::cerr << "Recognized an Instruction_call_function_assignment rule\n";

      auto f = p.functions.back();
      auto var_temp = parsed_items.front();
      parsed_items.erase(parsed_items.begin());
      auto callee = parsed_items.front();
      parsed_items.erase(parsed_items.begin());

      auto var = dynamic_cast<Variable*>(var_temp);
      if (!var) {
        if (debug) std::cerr << "Program is incorrect, " << var->print() << " should be of variable type.\n";
      }

      auto i = new NonVoidCallInstruction(var, callee);

      while (!parsed_items.empty()) {
        auto arg = parsed_items.front();
        parsed_items.erase(parsed_items.begin());
        i->args.push_back(arg);
      }

      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_array_initialization_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // var <- new Array( args )
      if (debug) std::cerr << "Recognized an Instruction_array_initialization rule\n";

      auto f = p.functions.back();
      auto var_temp = parsed_items.front();
      parsed_items.erase(parsed_items.begin());

      auto var = dynamic_cast<Variable*>(var_temp);
      if (!var) {
        if (debug) std::cerr << "Program is incorrect, " << var->print() << " should be of variable type.\n";
      }


      auto i = new newArray(var, counter);
      counter++;

      while (!parsed_items.empty()) {
        auto arg = parsed_items.front();
        parsed_items.erase(parsed_items.begin());
        i->args.push_back(arg);
      }

      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_tuple_initialization_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // var <- new Tuple( t )
      if (debug) std::cerr << "Recognized an Instruction_tuple_initialization rule\n";

      auto f = p.functions.back();
      auto t = parsed_items.back();
      parsed_items.pop_back();
      auto var_temp = parsed_items.back();
      parsed_items.pop_back();

      auto var = dynamic_cast<Variable*>(var_temp);
      if (!var) {
        if (debug) std::cerr << "Program is incorrect, " << var->print() << " should be of variable type.\n";
      }


      auto i = new newTuple(var, t,counter);
      counter++;
      f->instructions.push_back(i);
    }
  };


  /*
  Terminator actions.
  */
  template<> struct action < Terminator_single_branch_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // br label
      if (debug) std::cerr << "Recognized an Terminator_single_branch rule\n";

      auto f = p.functions.back();
      auto label_temp = parsed_items.back();
      parsed_items.pop_back();

      auto label = dynamic_cast<Label*>(label_temp);
      if (!label) {
        if (debug) std::cerr << "Program is incorrect, " << label->print() << " should be of Label type.\n";
      }

      auto i = new oneSuccBranch(label);
      f->instructions.push_back(i);
    }
  };
  
  template<> struct action < Terminator_double_branch_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // br t label1 label2
      if (debug) std::cerr << "Recognized an Terminator_double_branch rule\n";

      auto f = p.functions.back();
      auto label2_temp = parsed_items.back();
      parsed_items.pop_back();
      auto label1_temp = parsed_items.back();
      parsed_items.pop_back();
      auto t = parsed_items.back();
      parsed_items.pop_back();

      auto label1 = dynamic_cast<Label*>(label1_temp);
      if (!label1) {
        if (debug) std::cerr << "Program is incorrect, " << label1->print() << " should be of Label type.\n";
      }

      auto label2 = dynamic_cast<Label*>(label2_temp);
      if (!label2) {
        if (debug) std::cerr << "Program is incorrect, " << label2->print() << " should be of Label type.\n";
      }

      auto i = new twoSuccBranch(t, label1, label2);
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Terminator_return_val_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // return t
      if (debug) std::cerr << "Recognized an Terminator_return_val rule\n";

      auto f = p.functions.back();
      auto t = parsed_items.back();
      parsed_items.pop_back();

      auto i = new trueReturn(t);
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Terminator_return_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      // return
      if (debug) std::cerr << "Recognized an Terminator_return rule\n";

      auto f = p.functions.back();

      auto i = new falseReturn();
      f->instructions.push_back(i);
    }
  };


  /*
  Function actions.
  */
  template<> struct action < Function_header_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized a Function_header rule,\n";


      auto f = p.functions.back();

      if (debug) std::cerr << "Here if the function's number of parameters: " << f->parameters.size() << "\n";

      // if (debug) std::cerr << "made it past grabbing the current function\n";

      // // auto type = parsed_items.front()
      // // parsed_items.erase(parsed_items.begin());
      // // auto fname = parsed_items.front()
      // // parsed_items.erase(parsed_items.begin());
      // // ^ i think the above should have already have been taken care of.

      // /*
      // There should be an even number of items in the parsed_items stack now, a function definition
      // is made up of (type var) starred.
      // */

      // if (debug) std::cerr << "parsed_items has size " << parsed_items.size() << " before the while loop\n";

      // while (!parsed_items.empty()) {
      //   auto type = parsed_items.front();         // it seems like we aren't currently doing anything with this
      //   if (debug) std::cerr << "made it past grabbing the type " << type->print() << "\n";
      //   parsed_items.erase(parsed_items.begin());
      //   if (debug) std::cerr << "made it past erasing the type\n";
      //   auto var_temp = parsed_items.front();
      //   if (debug) std::cerr << "made it past grabbing the var " << var_temp->print() << "\n";
      //   parsed_items.erase(parsed_items.begin());
      //   if (debug) std::cerr << "made it past erasing the var\n";

      //   auto var = dynamic_cast<Variable*>(var_temp);
      //   if (!var) {
      //     if (debug) std::cerr << "Program is incorrect, " << var->print() << " should be of variable type.\n";
      //   }

      //   f->parameters.push_back(var);
      // }

    }
  };


  template<> struct action < Function_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized a Function rule\n";
    }
  };


  /*
  Program actions.
  */
  template<> struct action < entry_point_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      if (debug) std::cerr << "Recognized an entry_point rule, finished parsing the IR program!\n";
    }
  };




  
  /*
  Program Parser.
  */
  Program parse_file (char *fileName) {
    /* 
    Check the grammar for some possible issues.
    */
    if (pegtl::analyze< full_grammar >() != 0){
      std::cerr << "There are problems with the grammar\n";
      exit(1);
    }

    std::cerr << "No problems with the grammar.\n";

    /*
     * Parse.
     */
    file_input< > fileInput(fileName);
    Program p;
    parse< full_grammar, action >(fileInput, p);

    return p;
  }

  















}