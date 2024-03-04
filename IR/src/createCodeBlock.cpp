#include <set>
#include <vector>
#include <unordered_map>
#include <string>
#include <variant>
#include <algorithm>
#include <iostream>
#include <list>
#include <map>
#include <tuple>
#include "IR.h"

namespace IR {
    void createCodeBlock(Program *p) {
        // for (auto *fptr : p->functions) {
        //     std::vector<Block *> appendBlock;
        //     Block *currentBlock = nullptr; 

        //     for (auto i : fptr->instructions) {
        //         auto label_cast = dynamic_cast<labelInstruction *>(i);
        //         if (label_cast) {
        //             if (currentBlock) {
        //                 appendBlock.push_back(currentBlock);
        //                 fptr->blockNameToPointer[currentBlock->label] = currentBlock;
        //             }
        //             currentBlock = new Block(label_cast->label->name); 

        //         }
        //         currentBlock->appendInstruction(i); 
        //     }

        //     // After the loop, append the last block to the vector if it exists.
        //     if (currentBlock) {
        //         appendBlock.push_back(currentBlock);
        //         fptr->blockNameToPointer[currentBlock->label] = currentBlock;
        //     }

        //     fptr->codeBlocks = appendBlock; // Assign the vector of blocks to the function's codeBlocks.
        // }
    }
    void createSuccessors(Program *p){
        // for (auto *fptr : p->functions){
        //     std::string curr_label;
        //     for (auto i : fptr->instructions){
        //         auto label_cast = dynamic_cast<labelInstruction *>(i);
        //         auto te_cast = dynamic_cast<teInstruction*>(i);
        //         if (label_cast){
        //             curr_label = label_cast->label->name;
        //         } else if (te_cast) {
        //             auto one_succ_branch = dynamic_cast<oneSuccBranch*>(i);
        //             auto two_succ_branch = dynamic_cast<twoSuccBranch*>(i);
        //             if (one_succ_branch){
        //                 auto block = fptr->blockNameToPointer[curr_label];
        //                 // Assuming that blockNameToPointer maps labels to Block* pointers
        //                 // fptr->blockNameToPointer[one_succ_branch->block->label] = Block* successorBlock;
        //                 fptr->blockNameToPointer[one_succ_branch->label] = Block* successorBlock;
        //                 block->successors.push_back(successorBlock);
        //             } else if (two_succ_branch){
        //                 auto block = fptr->blockNameToPointer[curr_label];
        //                 fptr->blockNameToPointer[two_succ_branch->trueB->label] = Block* trueSuccessorBlock;
        //                 fptr->blockNameToPointer[two_succ_branch->falseB->label] = Block* falseSuccessorBlock;
        //                 block->successors.push_back(trueSuccessorBlock);
        //                 block->successors.push_back(falseSuccessorBlock);
        //             }
        //         }
        //     }
        // }
    }

}
