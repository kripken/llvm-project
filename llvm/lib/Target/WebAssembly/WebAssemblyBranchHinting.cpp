//===-- WebAssemblyBranchHinting.cpp - Emit branch hints from LLVM IR    --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Convert LLVM IR branch_weights
/// (https://llvm.org/docs/LangRef.html#branch-weights) into metadata that will
/// then be emitted as a wasm custom section for branch hints
/// (https://github.com/WebAssembly/branch-hinting).
///
//===----------------------------------------------------------------------===//

#include "WebAssembly.h"
#include "llvm/IR/Dominators.h" // ?
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "wasm-branch-hint"

namespace {
class WebAssemblyBranchHinting final : public FunctionPass,
                               public InstVisitor<WebAssemblyBranchHinting> {
  StringRef getPassName() const override {
    return "WebAssembly Branch Hint";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    FunctionPass::getAnalysisUsage(AU);
  }

  bool runOnFunction(Function &F) override;

  DominatorTree *DT = nullptr;

public:
  static char ID;
  WebAssemblyBranchHinting() : FunctionPass(ID) {}

  void visitCallBase(CallBase &CB);
};
} // End anonymous namespace

char WebAssemblyBranchHinting::ID = 0;
INITIALIZE_PASS(WebAssemblyBranchHinting, DEBUG_TYPE,
                "Emit WebAssembly branch hints",
                false, false)

FunctionPass *llvm::createWebAssemblyBranchHinting() {
  return new WebAssemblyBranchHinting();
}

void WebAssemblyBranchHinting::visitCallBase(CallBase &CB) {
  for (unsigned I = 0, E = CB.arg_size(); I < E; ++I)
    if (CB.paramHasAttr(I, Attribute::Returned)) {
      Value *Arg = CB.getArgOperand(I);
      // Ignore constants, globals, undef, etc.
      if (isa<Constant>(Arg))
        continue;
      // Like replaceDominatedUsesWith but using Instruction/Use dominance.
      Arg->replaceUsesWithIf(&CB,
                             [&](Use &U) { return DT->dominates(&CB, U); });
    }
}

bool WebAssemblyBranchHinting::runOnFunction(Function &F) {
  LLVM_DEBUG(dbgs() << "********** Emit wasm branch hints **********\n"
                       "********** Function: "
                    << F.getName() << '\n');

  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  visit(F);
  return true;
}
