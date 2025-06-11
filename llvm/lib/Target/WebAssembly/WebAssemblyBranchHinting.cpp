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
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
//?
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"


using namespace llvm;

#define DEBUG_TYPE "wasm-branch-hint"

namespace {
class WebAssemblyBranchHinting final : public FunctionPass,
                               public InstVisitor<WebAssemblyBranchHinting> {
  StringRef getPassName() const override {
    return "WebAssembly Branch Hint";
  }

  bool runOnFunction(Function &F) override;

public:
  static char ID;
  WebAssemblyBranchHinting() : FunctionPass(ID) {}

  void visitBranchInst(BranchInst &I);
};
} // End anonymous namespace

char WebAssemblyBranchHinting::ID = 0;
INITIALIZE_PASS(WebAssemblyBranchHinting, DEBUG_TYPE,
                "Emit WebAssembly branch hints",
                false, false)

FunctionPass *llvm::createWebAssemblyBranchHinting() {
  return new WebAssemblyBranchHinting();
}

void WebAssemblyBranchHinting::visitBranchInst(BranchInst &I) {
  // Check for profiling metadata of the right size and contents (beginning with
  // "branch_weights").
  MDNode *ProfMD = I.getMetadata(LLVMContext::MD_prof);
  if (!ProfMD)
    return;
  if (ProfMD->getNumOperands() == 0)
    return;
  MDString *MDName = dyn_cast<MDString>(ProfMD->getOperand(0));
  if (!MDName || MDName->getString() != "branch_weights")
    return;

  // We expect two integers, for the true and false weights.
  if (ProfMD->getNumOperands() < 3)
    return;

  // Operand indices for weights, assuming no "expected" operand appears before
  // them (which we ignore).
  unsigned TrueWeightOp = 1;
  unsigned FalseWeightOp = 2;

  // Skip "expected", if present.
  if (isa<MDString>(ProfMD->getOperand(1))) {
    if (ProfMD->getNumOperands() < 4)
      return;
    ++TrueWeightOp;
    ++FalseWeightOp;
  }

  ConstantInt *TrueWeight =
    mdconst::extract<ConstantInt>(ProfMD->getOperand(TrueWeightOp));
  ConstantInt *FalseWeight =
    mdconst::extract<ConstantInt>(ProfMD->getOperand(FalseWeightOp));

  if (!TrueWeight || !FalseWeight)
    return;

  std::cout << "waka " << TrueWeight->getZExtValue() << " : " << FalseWeight->getZExtValue() << '\n';

  // Generate metadata for wasm.
  // TODO
}

bool WebAssemblyBranchHinting::runOnFunction(Function &F) {
  LLVM_DEBUG(dbgs() << "********** Emit wasm branch hints **********\n"
                       "********** Function: "
                    << F.getName() << '\n');

  visit(F);
  return true;
}
