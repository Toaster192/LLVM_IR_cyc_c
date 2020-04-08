#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <vector>

using namespace llvm;


class FunctionCFGInfo {
    public:
        FunctionCFGInfo(const Function &F){
            // The BB graph has a single entry vertex from which the other BBs should
            // be discoverable - the function entry block.
            setEdges(walkBBs(&F.getEntryBlock()));
            setNodes(ExploredBBs.size());
        }
        int getEdges(){ return this->edges; };
        int getNodes(){ return this->nodes; };
        int getCycC(){ return this->edges - this->nodes + 2; };

    private:
        int edges;
        int nodes;
        void setEdges(int x){ this->edges = x; };
        void setNodes(int x){ this->nodes = x; };

        typedef SmallVector<const BasicBlock *, 32> BBVector;
        BBVector ExploredBBs;

        unsigned walkBBs(const BasicBlock *BB) {
            // Adding to explored BBs early so we don't get into an infinite loop
            ExploredBBs.push_back(BB);
            const Instruction *Inst = BB->getTerminator();
            unsigned edges = Inst->getNumSuccessors();
            for (unsigned I = 0, NSucc = Inst->getNumSuccessors(); I < NSucc; ++I) {
                BasicBlock *Succ = Inst->getSuccessor(I);
                if (!std::count(ExploredBBs.begin(),
                                ExploredBBs.end(),
                                Succ)) {
                    // TODO: remove recursion
                    edges += walkBBs(Succ);
                }
            }
            return edges;
        }
};

class CalculateCC : public FunctionPass {
    public:
        CalculateCC()
            : FunctionPass(ID){}

        virtual bool runOnFunction(Function &F) {
            FunctionCFGInfo FI(F);
            outs() << "Function: " << F.getName() << "\n"
                  << "Nodes: " << FI.getNodes() << "\n"
                  << "Edges: " << FI.getEdges() << "\n"
                  << "CyclomaticComplexity: " << FI.getCycC() << "\n";
            return true;
        }

        // The address of this member is used to uniquely identify the class. This is
        // used by LLVM's own RTTI mechanism.
        static char ID;
};

char CalculateCC::ID = 0;

int main(int argc, char **argv) {
    if (argc < 2) {
        // Using very basic command-line argument parsing here...
        errs() << "Usage: " << argv[0] << " <IR file>\n";
        return 1;
    }

    // Parse the input LLVM IR file into a module.
    SMDiagnostic Err;
    LLVMContext Context;
    std::unique_ptr<Module> Mod(parseIRFile(argv[1], Err, Context));
    if (!Mod) {
        Err.print(argv[0], errs());
        return 1;
    }

    // Create a pass manager and fill it with the passes we want to run.
    legacy::PassManager PM;
    PM.add(new CalculateCC());
    PM.run(*Mod);

    return 0;
}
