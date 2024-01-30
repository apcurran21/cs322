#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <liveness_analysis.h>
#include "L2.h"


using namespace std;

// use 1 for debug statements, 0 for no printing
int debug = 1;

namespace L2{

    /*
    Utility vectors for calling convention checks
    */
    std::vector<std::string> arguments_vec{
            "rdi",
            "rsi",
            "rdx",
            "rcx",
            "r8",
            "r9"
    };
    std::vector<std::string> caller_save_vec{
            "r10",
            "r11",
            "r8",
            "r9",
            "rax",
            "rcx",
            "rdi",
            "rdx",
            "rsi"
    };
    std::vector<std::string> callee_save_vec{
            "r12",
            "r13",
            "r14",
            "r15",
            "rbp",
            "rbx"
    };

    /*
    In and Out Set Storage
    */
    In_Out_Store::In_Out_Store(Program *p) {
        for (auto functionptr : p->functions) {
            std::unordered_map<Instruction*, std::set<Variable*>> in_set_map;
            std::unordered_map<Instruction*, std::set<Variable*>> out_set_map;
            for (auto instructionptr : functionptr->instructions) {
                in_set_map[instructionptr] = std::set<Variable*>();
                out_set_map[instructionptr] = std::set<Variable*>();
            }
            In_Set.push_back(in_set_map);
            Out_Set.push_back(out_set_map);
        }
    }

    /*
    Gen and Kill Set Storage
    */
    Gen_Kill_Store::Gen_Kill_Store(Program *p) {
        for (auto functionptr : p->functions) {
            std::unordered_map<Instruction*, std::set<Variable*>> gen_set_map;
            std::unordered_map<Instruction*, std::set<Variable*>> kill_set_map;
            for (auto instructionptr : functionptr->instructions) {
                gen_set_map[instructionptr] = std::set<Variable*>();
                kill_set_map[instructionptr] = std::set<Variable*>();
            }
            Gen_Set.push_back(gen_set_map);
            Kill_Set.push_back(kill_set_map);
        }
    }
    

    /*
    Full Liveness Analysis
    */
    void liveness_analysis(Program *p){
        

        if (debug) std::cerr << "Running Liveness Analysis..." << std::endl;

        /*
        Initialize empty Gen, Kill, In, and Out sets
        */
        Gen_Kill_Store gen_kill_sets = Gen_Kill_Store(p);
        In_Out_Store in_out_sets = In_Out_Store(p);

        /*
        Run the liveness analysis algorithm
        */
        for (int function_index = 0; function_index < p->functions.size(); function_index++) {
            Function* fptr = p->functions[function_index];
            if (debug) std::cerr << "Running liveness analysis on a new function..." << std::endl;
            
            /*
            Calculate Uses and Defs sets for each instruction in the current function (calls UseDefs Visitor)
            */
            fptr->calculateUseDefs();

            /*
            Calcuate Gen and Kill sets for each instruction in the current function using the Uses/Defs sets and calling convention rules
            */
            for (auto instruction_ptr : fptr->instructions) {
                /*
                Define pointer references to the current instruction's Gen/Kill sets for convenience
                */
                std::set<Variable*>* gen_set_ptr = &gen_kill_sets.Gen_Set[i][instruction_ptr];
                std::set<Variable*>* kill_set_ptr = &gen_kill_sets.Kill_Set[i][instruction_ptr];
                /*
                Place Uses into Gen
                */
                for (auto variable_ptr : instruction_ptr->used) {
                    gen_set_ptr->insert(variable_ptr);
                }
                /*
                Place Defs into Kill
                */
                for (auto variable_ptr : instruction_ptr->defined) {
                    gen_set_ptr->insert(variable_ptr);
                }
                /*
                Dynamic cast to check for special calling convention cases
                */
                Call_uN_Instruction* call_uN_instruction_ptr = dynamic_cast<Call_uN_Instruction*>(instruction_ptr);
                if (call_uN_instruction_ptr) {
                    /*
                    --- call u N ---
                    Gen <- {u, args used}
                    Kill <- {caller-saved}
                    */
                    // Gen, finding 'u' 
                    Variable* variable_ptr = dynamic_cast<Variable*>(call_uN_instruction_ptr->u);
                    if (variable_ptr) {
                        Register* register_ptr = dynamic_cast<Register*>(variable_ptr);
                        if (register_ptr) {
                            gen_set_ptr->insert(fptr->variable_allocator.allocate_variable(register_ptr->name, VariableType::reg));
                        } else {
                            gen_set_ptr->insert(fptr->variable_allocator.allocate_variable(variable_ptr->name, VariableType::var));
                        }
                    }
                    // Gen, finding 'args used'
                    for (int i = 0; i < fptr->arguments->value) {
                        gen_set_ptr->insert(fptr->variable_allocator.allocate_variable(arguments_vec[i], VariableType::reg));
                    }
                    // Kill, finding 'caller-saved'
                    for (auto register_string : caller_save_vec) {
                        kill_set_ptr->insert(fptr->variable_allocator.allocate_variable(register_string, VariableType::reg));
                    }
                }
                /*
                --- call RUNTIME N ---
                Gen <- {args used}
                Kill <- {caller-saved}
                */
                Call_print_Instruction* call_print_instruction_ptr = dynamic_cast<Call_print_Instruction*>(instruction_ptr);
                if (call_print_instruction_ptr) {
                    /*
                    call print 1
                    */
                    // Gen, finding 'args used'
                    gen_set_ptr->insert(fptr-variable_allocator.allocate_variable(arguments_vec[0], VariableType::reg));
                    // Kill, finding 'caller-saved'
                    for (auto register_string : caller_save_vec) {
                        kill_set_ptr->insert(fptr->variable_allocator.allocate_variable(register_string, VariableType::reg));
                    }
                }
                Call_input_Instruction* call_input_instruction_ptr = dynamic_cast<Call_input_Instruction*>(instruction_ptr);
                if (call_input_instruction_ptr) {
                    /*
                    call input 1
                    */
                    // Kill, finding 'caller-saved'
                    for (auto register_string : caller_save_vec) {
                        kill_set_ptr->insert(fptr->variable_allocator.allocate_variable(register_string, VariableType::reg));
                    }
                }
                Call_allocate_Instruction* call_allocate_instruction_ptr = dynamic_cast<Call_allocate_Instruction*>(instruction_ptr);
                if (call_allocate_instruction_ptr) {
                    /*
                    call allocate 2
                    */
                    // Gen, finding 'args used'
                    for (int i = 0; i < 2; i++) {
                        gen_set_ptr->insert(fptr->variable_allocator.allocate_variable(arguments_vec[i], VariableType::reg));
                    }
                    // Kill, finding 'caller-saved'
                    for (auto register_string : caller_save_vec) {
                        kill_set_ptr->insert(fptr->variable_allocator.allocate_variable(register_string, VariableType::reg));
                    }
                }
                Call_tuple_Instruction* call_tuple_instruction_ptr = dynamic_cast<Call_tuple_Instruction*>(instruction_ptr);
                if (call_tuple_instruction_ptr) {
                    /*
                    call tuple-error 3
                    */
                    // Gen, finding 'args used'
                    for (int i = 0; i < 3; i++) {
                        gen_set_ptr->insert(fptr->variable_allocator.allocate_variable(arguments_vec[i], VariableType::reg));
                    }
                    // Kill, finding 'caller-saved'
                    for (auto register_string : caller_save_vec) {
                        kill_set_ptr->insert(fptr->variable_allocator.allocate_variable(register_string, VariableType::reg));
                    }
                }
                Call_tenserr_Instruction* call_tenserr_instruction_ptr = dynamic_cast<Call_tenserr_Instruction*>(instruction_ptr);
                if (call_tenserr_instruction_ptr) {
                    /*
                    call tensor-error F
                    */
                    // Gen, finding 'args used'
                    for (int i = 0; i < call_tenserr_instruction_ptr->F->value; i++) {
                        gen_set_ptr->insert(fptr->variable_allocator.allocate_variable(arguments_vec[i], VariableType::reg));
                    }
                    // Kill, finding 'caller-saved'
                    for (auto register_string : caller_save_vec) {
                        kill_set_ptr->insert(fptr->variable_allocator.allocate_variable(register_string, VariableType::reg));
                    }
                }
                Instruction_ret* return_instruction_ptr = dynamic_cast<Instruction_ret*>(instruction_ptr);
                if (return_instruction_ptr) {
                    /*
                    --- return ---
                    Gen <- {rax, callee-saved}
                    Kill <- {}
                    */
                    // Gen, finding 'rax'
                    gen_set_ptr->insert(fptr->variable_allocator.allocate_variable('rax', VariableType::reg));
                    // Gen, finding 'callee-saved'
                    for (auto register_string : callee_save_vec) {
                        gen_set_ptr->insert(fptr->variable_allocator.allocate_variable(register_string, VariableType::reg));
                    }
                }
            }   // finished with Gen and Kill
            
            /*
            Calcuate Predeccesors and Successors sets for each instruction in the current function with our algorithm
            */
            fptr->calculateCFG();


            if (debug) std::cerr << "CFG brrrr" << std::endl;
            int instruction_number;  
            bool changed;
            do {
                changed = false;
                instruction_number = 0;
                // for (int j = 0; j < fptr->instructions.size(); j++) {
                for (auto iptr : fptr->instructions) {
                    // note that iptr should be of type Instruction*
                    // auto iptr = fptr->instructions[j];
                    /*
                        Define the Gen and Kill sets for the current instruction
                    */
                    std::set<Variable*> Gen_Set = iptr->used;
                    if (debug) std::cerr << "computing In and Out sets for the current instruction..." << std::endl;

                    std::set<Variable*> Kill_Set = iptr->defined;

                    /*
                        Define state of the sets before any potential changes are made
                    */

                    std::set<Variable*> in_set_prev = in_out_sets.In_Set[i][iptr];
                    std::set<Variable*> out_set_prev = in_out_sets.Out_Set[i][iptr];

                    /*
                        Get pointers to the sets so that we don't have to keep array accessing
                    */
                    // std::set<Variable>* in_ptr = &in_out_sets.In_Set[i][j];
                    // std::set<Variable>* out_ptr = &in_out_sets.Out_Set[i][j];
                    std::set<Variable*>* in_ptr = &in_out_sets.In_Set[i][iptr];
                    std::set<Variable*>* out_ptr = &in_out_sets.Out_Set[i][iptr];

                    /*
                        Do the set operations
                    */
                    std::set<Variable*> In_Result;
                    std::set<Variable*> Out_Kill_Diff;
                    std::set_difference(
                        out_ptr->begin(), out_ptr->end(),
                        Kill_Set.begin(), Kill_Set.end(),
                        std::inserter(Out_Kill_Diff, Out_Kill_Diff.begin())
                    );
                    std::set_union(
                        Gen_Set.begin(), Gen_Set.end(),
                        Out_Kill_Diff.begin(), Out_Kill_Diff.end(),
                        std::inserter(In_Result, In_Result.begin())
                    );

                    std::set<Variable*> Out_Result;
                    for (auto successor : iptr->successors) {
                        // note that successor should be of type Instruction*
                        std::set<Variable*> successor_in_set = in_out_sets.In_Set[i][successor];
                        std::set<Variable*> temp_union_set;
                        std::set_union(
                            Out_Result.begin(), Out_Result.end(),
                            successor_in_set.begin(), successor_in_set.end(),
                            std::inserter(temp_union_set, temp_union_set.begin())
                        );
                        Out_Result = std::move(temp_union_set);
                    }

                    *in_ptr = In_Result;
                    *out_ptr = Out_Result;

                    /*
                        Check against the initial state after performing the operations.
                    */
                    instruction_number++;
                    changed = changed || ((in_set_prev != *in_ptr) || (out_set_prev != *out_ptr));
                }
            } while (changed);
            
        }




        /*
        Print the contents of our freshly computed In and Out sets to the file
        */

        for (int f = 0; f < p->functions.size(); f++) {
            std::cout << "(\n";
            Function* fptr = p->functions[f];
            std::unordered_map<Instruction*, std::set<Variable*>> in_map = in_out_sets.In_Set[f];
            std::cout << "(in\n";
            for (auto iptr : fptr->instructions) {
                std::cout << "(";
                for (auto variable = in_map[iptr].begin(); variable != in_map[iptr].end(); variable++) {
                    auto print = *variable;
                    std::cout << print->print() << " ";
                    std::cout<<"in ";

                }
                //std::cout << ")";
            }
            std::cout << ")\n\n";

            std::unordered_map<Instruction*, std::set<Variable*>> out_map = in_out_sets.Out_Set[f];
            std::cout << "(out\n";
            for (auto iptr : fptr->instructions) {
                std::cout << "(";
                for (auto variable = out_map[iptr].begin(); variable != out_map[iptr].end(); variable++) {
                    auto print = *variable;
                    std::cout << print->print() << " ";
                    std::cout<<"out ";
                }
                //std::cout << ")";
            }
            std::cout << ")\n";
            
            std::cout << ")\n";
        }
    }

}