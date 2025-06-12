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
//?
#include "MCTargetDesc/WebAssemblyMCTargetDesc.h"
#include "WebAssembly.h"
#include "WebAssemblyMachineFunctionInfo.h"
#include "WebAssemblySubtarget.h"
#include "llvm/CodeGen/MachineBranchProbabilityInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "wasm-branch-hinting"

namespace {
class WebAssemblyBranchHinting final : public MachineFunctionPass {
  StringRef getPassName() const override {
    return "WebAssembly emit branch hints";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineBranchProbabilityInfoWrapperPass>();
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

public:
  static char ID; // Pass identification, replacement for typeid
  WebAssemblyBranchHinting() : MachineFunctionPass(ID) {}
};
} // end anonymous namespace

char WebAssemblyBranchHinting::ID = 0;
INITIALIZE_PASS(WebAssemblyBranchHinting, DEBUG_TYPE,
                "Emits branch hints", false, false)

FunctionPass *llvm::createWebAssemblyBranchHinting() {
  return new WebAssemblyBranchHinting();
}

bool WebAssemblyBranchHinting::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "********** Emitting branch hints **********\n"
                       "********** Function: "
                    << MF.getName() << '\n');

  errs() << "Pass\n";

  const MachineBranchProbabilityInfo *MBPI =
      &getAnalysis<MachineBranchProbabilityInfoWrapperPass>().getMBPI();

  auto &MFI = *MF.getInfo<WebAssemblyFunctionInfo>();
  const auto &TII = *MF.getSubtarget<WebAssemblySubtarget>().getInstrInfo();
  auto &MRI = MF.getRegInfo();

  for (auto &MBB : MF) {
    for (MachineInstr &MI : llvm::make_early_inc_range(MBB)) {
      if (MI.getOpcode() != WebAssembly::BR_UNLESS &&
          MI.getOpcode() != WebAssembly::BR_IF)
        continue;

      // This is a BR. It has two successors, and perhaps branch probability
      // info between them.
      errs() << MI << '\n';
      assert(MBB.succ_size() == 2);
      auto iter = MBB.succ_begin();
      MachineBasicBlock* first = *iter;
      iter++;
      MachineBasicBlock* second = *iter;

      BranchProbability probFirst = MBPI->getEdgeProbability(&MBB, first);
      BranchProbability probSecond = MBPI->getEdgeProbability(&MBB, second);
      errs() << probFirst << " : " << probSecond << '\n';
    }
  }

  errs() << "and Pass\n";

  return true;
}
