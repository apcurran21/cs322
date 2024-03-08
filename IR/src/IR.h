#pragma once 

#include <set>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <variant>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <tuple>

namespace IR{

    /*
    Forward declarations.
    */
    class Function;
    class Variable;
    class teInstruction;
    class Operator;

    class Token {
        public:
        virtual ~Token() = default;
    };

    /*
    Can we make this derive from Item so that it can be placed onto the parsed_items stack?
    */
    // class Type : public Token{};

    // class TupleType : public Type {
    //     friend class Singleton;
    //     TupleType() = default;
    // };

    // class CodeType : public Type {
    //     friend class Singleton;
    //     CodeType() = default;
    // };

    // class IntType : public Type {
    //     friend class Singleton;
    //     IntType() = default;
    // };

    // class VoidType : public Type{
    //     friend class Singleton;
    //     VoidType() = default;
    // };

    /*
    Terminal Items.
    */
    class Item : public Token {
        public:
            virtual std::string print() const = 0;
    };

    enum TypeEnum {
        int64,  // encodes both single and array int64s using its dims field
        tuple,
        code,
        voidt,
        bracks  // to catch the "[]" dimensions in int64 arrays
    };

    extern std::map<std::string, TypeEnum> stringToTypeEnum;

    class Type : public Item {
        public:
            explicit Type(TypeEnum t);
            std::string print() const override;
            TypeEnum type;
            int64_t dims;
    };

    class Label : public Item {
        public:
            explicit Label(std::string name);
            std::string print() const override;
            std::string name;
    };

    class Number : public Item {
        public:
            explicit Number(int64_t value);
            std::string print() const override;
            const int64_t value; 
    };

    class Variable : public Item{
        public:
            explicit Variable(std::string name);
            std::string print() const override;
            std::string name;
    };

    // class arrAccess: public Item{
    //     public:
    //         explicit arrAccess(Variable *obj);
    //         Variable *const object;
    // };

    class userFuncName : public Item {
        public:
        /*
        Is this for the I_rule? like is this for both function definition names and callee names?
        */
            // Constructor
            explicit userFuncName(const std::string& name);
            std::string print() const override;
            std::string name;
        };

    enum OperatorEnum {
        gt,
        geq,
        eq,
        leq,
        times,
        minus,
        plus, 
        lt,
        left,
        right,
        amp
    };

    // std::map<std::string, OperatorEnum> stringToOperatorEnum = {
    //     {">", OperatorEnum::gt},
    //     {">=", OperatorEnum::geq},
    //     {"=", OperatorEnum::eq},
    //     {"<=", OperatorEnum::leq},
    //     {"*", OperatorEnum::times},
    //     {"-", OperatorEnum::minus},
    //     {"+", OperatorEnum::plus},
    //     {"<", OperatorEnum::lt},
    //     {"<<", OperatorEnum::left},
    //     {">>", OperatorEnum::right},
    //     {"&", OperatorEnum::amp}
    // };

    extern std::map<std::string, OperatorEnum> stringToOperatorEnum;

    class Operator : public Item {
        public:
            /*
            Didn't define ItemVisitor yet.
            */
            // void accept(ItemVisitor *v) override;
            friend class OperatorSingletons; 
            explicit Operator(OperatorEnum id);
            std::string print() const override;
            OperatorEnum getID() const;
            OperatorEnum id; 
    };

    class OperatorSingletons {
    public:
        inline static Operator *const EQ = new Operator(OperatorEnum::eq);
        inline static Operator *const GEQ = new Operator(OperatorEnum::geq);
        inline static Operator *const GT = new Operator(OperatorEnum::gt);
        inline static Operator *const PLUS = new Operator(OperatorEnum::plus);
        inline static Operator *const MINUS = new Operator(OperatorEnum::minus);
        inline static Operator *const RIGHT = new Operator(OperatorEnum::right);
        inline static Operator *const LT = new Operator(OperatorEnum::lt);
        inline static Operator *const LEQ = new Operator(OperatorEnum::leq);
        inline static Operator *const TIMES = new Operator(OperatorEnum::times);
        inline static Operator *const AMP = new Operator(OperatorEnum::amp);
        inline static Operator *const LEFT = new Operator(OperatorEnum::left);
    };

    // class Singleton {
    //     public:
    //         inline static IntType *const INT_TYPE = new IntType();
    //         inline static VoidType *const VOID_TYPE = new VoidType();
    //         inline static TupleType *const TUPLE_TYPE = new TupleType();
    //         inline static CodeType *const CODE_TYPE = new CodeType();
    // };

    class Visitor {
        // public:
        //    virtual void visit(declarationInstruction *instruction) = 0;
        //    virtual void visit(Assignment *instruction) = 0;
        //    virtual void visit(labelInstruction *instruction) = 0;
        //    virtual void visit(operationInstruction *instruction) = 0;
        //    virtual void visit(loadInstruction *instruction) = 0;
        //    virtual void visit(storeInstruction *instruction) = 0;
        //    virtual void visit(arrLength *instruction) = 0;
        //    virtual void visit(tupleLength *instruction) = 0;
        //    virtual void visit(VoidCallInstruction *instruction) = 0;
        //    virtual void visit(NonVoidCallInstruction *instruction) = 0;
        //    virtual void visit(newArray *instruction) = 0;
        //    virtual void visit(newTuple *instruction) = 0;
        //    virtual void visit(oneSuccBranch *instruction) = 0;
        //    virtual void visit(twoSuccBranch *instruction) = 0;
        //    virtual void visit(falseReturn *instruction) = 0;
        //    virtual void visit(trueReturn *instruction) = 0;
        //    virtual void visit(labelInstruction *instruction) = 0;
    };



    // Specific IR declarations

    /*
    Instructions
    */
    class Instruction : public Token {
        public:
            virtual void gen(Function *f, std::ofstream &outputFile) = 0;

    };
    class voidInstruction : public Instruction {
        public:
            virtual void gen(Function *f, std::ofstream &outputFile) = 0;
    }; 

    class nonVoidInstruction : public Instruction {
        public:
            virtual void gen(Function *f, std::ofstream &outputFile) = 0;
            explicit nonVoidInstruction(Variable *dst);
            Variable *const dst;
    };

    class declarationInstruction : public voidInstruction {
        public:
            void gen(Function *f, std::ofstream &outputFile) override;
            explicit declarationInstruction(Type *type, Variable *var);
            Type *type;
            Variable *var;
    };

    class Assignment : public nonVoidInstruction{
        public:
            void gen(Function *f, std::ofstream &outputFile) override;
            explicit Assignment(Variable *dst, Item *src);
            Item *const src;
    };
    class labelInstruction : public voidInstruction {
        public:
            void gen(Function *f, std::ofstream &outputFile) override;
            explicit labelInstruction(Label *label);
            Label *label;
    };
    class operationInstruction: public nonVoidInstruction{
        public:
            void gen(Function *f, std::ofstream &outputFile) override;

            explicit operationInstruction(Variable *dst, Item *t1, Operator *op, Item *t2);
            
            Item *const t1;
            Item *const t2;
            Operator *const op;
    };
    class loadInstruction : public nonVoidInstruction{
        public:
            void gen(Function *f, std::ofstream &outputFile) override;
            explicit loadInstruction(Variable *dst, Item *var);
            // arrAccess *const access;
            Item *const var;
            std::vector<Item *> index_args_vec;
    };
    class storeInstruction: public nonVoidInstruction {
        public:
            void gen(Function *f, std::ofstream &outputFile) override;
            /*
            In this constructor, var has to be the source since dst was already defined in the base nonVoidInstruction class.
            */
            explicit storeInstruction(Variable *dst, Item *var);

            // arrAccess *const access;
            // Variable *dst;
            Item *const var;
            std::vector<Item *> index_args_vec;
    };
    class arrLength : public nonVoidInstruction{
        public:
            void gen(Function *f, std::ofstream &outputFile) override;
            explicit arrLength(Variable *dst, Variable *arr, Item *dim);
            Variable *const arr;
            Item *const dim;
    };
    class tupleLength : public nonVoidInstruction{
        public:
            void gen(Function *f, std::ofstream &outputFile) override;
            explicit tupleLength(Variable *dst, Variable *tuple);
            Variable *const tuple;
    };

    class VoidCallInstruction : public voidInstruction {
        public:
            void gen(Function *f, std::ofstream &outputFile) override;
            explicit VoidCallInstruction(Item *callee);
            Item *const callee;
            std::vector<Item *> args; 
    };
    class NonVoidCallInstruction : public nonVoidInstruction {
    public:
        void gen(Function *f, std::ofstream &outputFile) override;
        explicit NonVoidCallInstruction(Variable *dst, Item *callee);
        Item *const callee;
        std::vector<Item *> args; // Now included directly in this class
    };

    class newArray : public nonVoidInstruction {
        public:
            void gen(Function *f, std::ofstream &outputFile) override;
            explicit newArray(Variable *dst, int64_t counter);
            std::vector<Item *> args; 
            void calculate_array(Function *f, std::ofstream &outputFile);
            int offset;
            int64_t count;
            std::vector<std::string> variableDimensions;
            std::vector<int> dimensions;
    };
    class newTuple : public nonVoidInstruction {
        public:
            void gen(Function *f, std::ofstream &outputFile) override;
            explicit newTuple(Variable *dst, Item *size,int64_t counter);
            Item *const size;
            int64_t count;

            
    };


    class Block : public Token {
        public:
            explicit Block(std::string label);
            void appendInstruction(Instruction *i);
            std::string getLabel();
            bool marked = false;
            std::vector<Block *> successors;
            std::vector<Instruction*> instructionBody;
            std::string label;
            teInstruction *terminator = nullptr;
    };
    class teInstruction : public voidInstruction {
        public:
        
    };
    // class tensorError : public teInstruction {
    //     public:
    //         void gen(Function *f, std::ofstream &outputFile) override;
    //         explicit tensorError();
    //         // flag variable for accessing the optional destination field
    //         bool has_dest = false;
    //         Variable *dest = nullptr;
    //         std::vector<Item *> args; 
    // };
    // class tupleError : public teInstruction {
    //     public:
    //         void gen(Function *f, std::ofstream &outputFile) override;
    //         explicit tupleError();
    //         // flag variable for accessing the optional destination field
    //         bool has_dest = false;
    //         Variable *dest = nullptr;
    //         std::vector<Item *> args; 
    // };



    class Error : public teInstruction {
        public:
            void gen(Function *f, std::ofstream &outputFile) override;
            explicit Error(std::string error_type);
            std::string error_type;
            std::vector<Item *> args; 
            Variable *dest = nullptr;
    };

    class oneSuccBranch : public teInstruction {
        public: 
            void gen(Function *f, std::ofstream &outputFile) override;
            // explicit oneSuccBranch(Block *block);
            explicit oneSuccBranch(Label *label);
            // std::vector<Block *> getSuccessors() override;
            // Block *const block;

            Label *label;
    };

    class twoSuccBranch : public teInstruction {
        public:
            void gen(Function *f, std::ofstream &outputFile) override;
            // twoSuccBranch(Item *t, Block *trueBBlock , Block *falseB);
            explicit twoSuccBranch(Item *t, Label *labelTrue, Label *labelFalse);
            // std::vector<Block*> getSuccessors() override;
            // Block *const trueB;
            // Block *const falseB;

            Item *const t;  // do we need it const?
            Label *labelTrue;
            Label *labelFalse;
    };

    class falseReturn : public teInstruction{
        public:
            void gen(Function *f, std::ofstream &outputFile) override;
            // std::vector<Block *> getSuccessors() override;
            explicit falseReturn();
    };

    class trueReturn : public teInstruction {
        public:
            void gen(Function *f, std::ofstream &outputFile) override;
            explicit trueReturn(Item *returnVal);
            // std::vector<Block *> getSuccessors() override;

            Item *const returnVal;
    };
    class blockVisitor{
        public:
            virtual void forBlock(Function *function)= 0;
    };
    class Function {
        public:
            explicit Function(userFuncName* functionName, Type* returnType);
            // void accept(blockVisitor* visitor); 
            // void assignTypeToVariable(Variable* variable, Type* type);
            // Block* getBlockByName(const std::string& blockName);
            // Variable* getVariableByName(const std::string& variableName);
            // userFuncName* getFunctionName() const;
            std::vector<Instruction *> instructions;
            std::vector<Variable*> parameters;
            // std::vector<Item*> parameters;
            std::vector<Block*> codeBlocks;
            std::vector<Block*> executionTraceOrder;
            std::unordered_map<std::string, Variable*> variableNameToPointer;
            std::unordered_map<std::string, newTuple*> variableNameToTuple;
            // std::unordered_map<Variable*, Type*> variableToTypeMapping;
            std::unordered_map<std::string, Type*> variableNameToType;
            std::unordered_map<std::string,newArray*>variableNameToArray;
            std::unordered_map<std::string, Block*> blockNameToPointer;
            std::unordered_map<std::string, std::string> variableNameToParameter;
            Type* returnType;
            userFuncName* functionName;
    };

    class ProgramVisitor;

    class Program {
        public:
            // void accept(ProgramVisitor *v);
            std::vector<Function *> functions;
            userFuncName *getFunctionName(const std::string& name);
            std::unordered_map<std::string, userFuncName *> functionNames;
    };
    

}