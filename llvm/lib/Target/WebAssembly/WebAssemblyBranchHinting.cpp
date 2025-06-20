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
  I.eraseMetadataIf([](unsigned MDKind, MDNode *Node) {
    // Look for profiling metadata.
    if (MDKind != LLVMContext::MD_prof)
      return false;

    // Check for profiling metadata of "branch_weights".
    if (Node->getNumOperands() == 0)
      return false;
    MDString *MDName = dyn_cast<MDString>(Node->getOperand(0));
    if (!MDName || MDName->getString() != "branch_weights")
      return false;

    // This is a branch weights metadata. Keep it only if it has the "expected"
    // string. TODO explain
    if (Node->getNumOperands() >= 2) {
      MDString *MDName = dyn_cast<MDString>(Node->getOperand(1));
      if (MDName && MDName->getString() == "expected")
        return false;
    }

    // Discard anything else.
    return true;
  });
}

bool WebAssemblyBranchHinting::runOnFunction(Function &F) {
  LLVM_DEBUG(dbgs() << "********** Emit wasm branch hints **********\n"
                       "********** Function: "
                    << F.getName() << '\n');

  visit(F);
  return true;
}
