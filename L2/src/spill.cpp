#include <map>
#include <set>
#include <string>
#include <stack>
#include <tuple>

#include <L2.h>
#include <graph_coloring.h>
#include <code_generator.h>
#include <interference_graph.h>

namespace L2{
    // bool spillForL2(Program &p, Variable* spilledVar){
    std::tuple<std::set<std::string>,Function*> spillForL2(Function* f, Variable* spilledVar, int spill_count) {    
        // Function* f = p.functions[0]; 
        Function* newFunction = new Function();
        newFunction->variable_allocator = f->variable_allocator;
        newFunction->variable_allocator.remove_variable(spilledVar->name);
        std::string temp = "%S";
        
        // int temp_var = -1;
        // std::string result = temp + std::to_string(temp_var);

        std::string result = temp + std::to_string(0);
        Variable* initial_replacement = f->variable_allocator.allocate_variable(result, VariableType::var);

        Number* stack = new Number(0);
        SpillVisitor * visitor = new SpillVisitor(spilledVar,initial_replacement);
        bool changed = false;
        int assignment_counter = 0;
        visitor->spilled = false;
        Variable* var = newFunction->variable_allocator.allocate_variable("rsp", VariableType::reg);
        std::vector<Instruction *> restore_vector;
        for (size_t i = 0; i < f->instructions.size(); ++i) {
            auto instruction = f->instructions[i];
            auto assignment_instruction = dynamic_cast<Instruction_assignment*>(instruction);
            instruction->accept(visitor);
            if (visitor->spilled) {
                if (assignment_instruction){
                    newFunction->instructions.insert(newFunction->instructions.end(),instruction);
                    Instruction * instruction1 = new Memory_assignment_store(var, visitor->replacementVariable, stack);
                    newFunction->instructions.insert(newFunction->instructions.end(),instruction1);
                    visitor->iterReplacementVariable();
                } else {
                    Instruction* instruction2 = new Memory_assignment_load(visitor->replacementVariable, var, stack);
                    visitor->iterReplacementVariable();
                    newFunction->instructions.insert(newFunction->instructions.end(),instruction2);
                    newFunction->instructions.insert(newFunction->instructions.end(),instruction);
                }
                visitor->spilled = false;
            } else {
                newFunction->instructions.insert(newFunction->instructions.end(),instruction);
            }
        } 
        std::set<std::string> spilled_variables_in_spill;
        std::string s = visitor->replacementVariable->name;
        int number = std::stoi(s.substr(2));  // Extract the number from the string, assuming it's after "S"

    // Loop from 0 to the extracted number
        for (int i = 0; i <= number; ++i) {
        // Generate the string for the current number
            std::string variableName = "%S" + std::to_string(i);
        // Insert it into the set
            spilled_variables_in_spill.insert(variableName);
        }

        // track the spill variables we created so that we don't accidentally spill it later
        return std::make_tuple(spilled_variables_in_spill, newFunction);

    }


    }