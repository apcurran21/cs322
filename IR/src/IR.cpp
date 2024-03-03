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
    
/*
Token/Item constructors
*/
Label::Label(std::string name) :
    name(name)
{
}

Number::Number(int64_t value) :
    value(value)
{
}

Variable::Variable(std::string name) :
    name(name)
{
}

Operator::Operator(OperatorEnum id) : 
    id(id)
{
}

Block::Block(std::string label) :
    label(label)
{
}


/*
Instruction constructors.
*/

/*
Void
*/
declarationInstruction::declarationInstruction()

labelInstruction::labelInstruction(Label *label) :
    label(label)
{
}

VoidCallInstruction::VoidCallInstruction(Item *callee, std::vector<Item *> args) :
    callee(callee),
    args(args)
{
}

/*
Non-Void
*/
nonVoidInstruction::nonVoidInstruction(Variable *dst) :
    dst(dst)
{
}

Assignment::Assignment(Variable *dst, Item *src) :
    nonVoidInstruction(dst),
    src(src)
{
}

operationInstruction::operationInstruction(Variable *dst, Item *t1, Operator *op, Item *t2) :
    nonVoidInstruction(dst),
    t1(t1),
    op(op),
    t2(t2)
{
}

loadInstruction::loadInstruction(Variable *dst Item *var) :
    nonVoidInstruction(dst),
    var(var)
{
}

storeInstruction::storeInstruction(Variable *dst, Item *var) :
    nonVoidInstruction(dst),
    var(var)
{
}

arrLength::arrLength(Variable *dst, Variable *arr, Item *dim) :
    nonVoidInstruction(dst),
    arr(arr),
    dim(dim)
{
}

tupleLength::tupleLength(Variable *dst, Variable *tuple) :
    nonVoidInstruction(dst),
    tuple(tuple)
{
}

NonVoidCallInstruction::NonVoidCallInstruction(Variable *dest, Item *callee, std::vector<Item *> args) :
    nonVoidInstruction(dest),
    callee(callee),
    args(args)
{
}

newArray::newArray(Variable *dest, std::vector<Item *> args) :
    nonVoidInstruction(dest),
    args(args)
{
}

newTuple::newTuple(Variable *dest, Item *size) :
    nonVoidInstruction(dest),
    size(size)
{
}








}
