// WebAssemblyAsmPrinter.h - WebAssembly implementation of AsmPrinter-*- C++ -*-
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYASMPRINTER_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYASMPRINTER_H

#include "WebAssemblyMachineFunctionInfo.h"
#include "WebAssemblySubtarget.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class WebAssemblyTargetStreamer;

class LLVM_LIBRARY_VISIBILITY WebAssemblyAsmPrinter final : public AsmPrinter {
public:
  static char ID;

private:
  const WebAssemblySubtarget *Subtarget;
  const MachineRegisterInfo *MRI;
  WebAssemblyFunctionInfo *MFI;
  bool signaturesEmitted = false;

  // A branch hint for an instruction. We gather them all (& by function) so we
  // can emit the section at the end, knowing how many hints are present (which
  // must be emitted before the hints, so we can't do it in a streaming manner).
  // And we must gather during emitInstruction(), as we must emit a label for
  // each branch we want to annotate (so we can refer to it).
  struct BranchHint {
    // The location the hint refers to.
    MCSymbol *Label;
    // Whether the branch there is likely.
    bool IsLikely;
  };

  struct FuncBranchHints {
    Function *F;
    std::vector<BranchHint> Hints;
  };

  std::vector<FuncBranchHints> AllFuncBranchHints;

public:
  explicit WebAssemblyAsmPrinter(TargetMachine &TM,
                                 std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer), ID), Subtarget(nullptr),
        MRI(nullptr), MFI(nullptr) {}

  StringRef getPassName() const override {
    return "WebAssembly Assembly Printer";
  }

  const WebAssemblySubtarget &getSubtarget() const { return *Subtarget; }

  //===------------------------------------------------------------------===//
  // MachineFunctionPass Implementation.
  //===------------------------------------------------------------------===//

  bool runOnMachineFunction(MachineFunction &MF) override {
    Subtarget = &MF.getSubtarget<WebAssemblySubtarget>();
    MRI = &MF.getRegInfo();
    MFI = MF.getInfo<WebAssemblyFunctionInfo>();
    return AsmPrinter::runOnMachineFunction(MF);
  }

  //===------------------------------------------------------------------===//
  // AsmPrinter Implementation.
  //===------------------------------------------------------------------===//

  void emitEndOfAsmFile(Module &M) override;
  void EmitProducerInfo(Module &M);
  void EmitTargetFeatures(Module &M);
  void EmitFunctionAttributes(Module &M);
  void EmitBranchHints(Module &M);
  void emitSymbolType(const MCSymbolWasm *Sym);
  void emitGlobalVariable(const GlobalVariable *GV) override;
  void emitJumpTableInfo() override;
  void emitConstantPool() override;
  void emitFunctionBodyStart() override;
  void emitInstruction(const MachineInstr *MI) override;
  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &OS) override;
  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                             const char *ExtraCode, raw_ostream &OS) override;

  MVT getRegType(unsigned RegNo) const;
  std::string regToString(const MachineOperand &MO);
  WebAssemblyTargetStreamer *getTargetStreamer();
  MCSymbolWasm *getMCSymbolForFunction(const Function *F, bool EnableEmEH,
                                       wasm::WasmSignature *Sig,
                                       bool &InvokeDetected);
  MCSymbol *getOrCreateWasmSymbol(StringRef Name);
  void emitDecls(const Module &M);

  // See if there is a branch hint for an instruction, and if so, if it is true
  // or false.
  std::optional<bool> getBranchHint(const MachineInstr& MI);
};

} // end namespace llvm

#endif
