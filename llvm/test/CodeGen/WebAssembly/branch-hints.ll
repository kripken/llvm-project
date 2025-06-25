; RUN: llc -mtriple=wasm32-unknown-unknown -filetype=asm -o - < %s | FileCheck %s

define i32 @bw_bh_test(i32 %a, i32 %b) {
entry:
  %1 = icmp ult i32 %a, %b
  br i1 %1, label %fail, label %success, !prof !0

; The weights below mean we are far more likely to go to %fail and return -1.
; Codegen will emit the -1 first (the same as appearing here):

; CHECK:	i32.const	-1
; CHECK:	i32.const	0

; Given that layout, we must emit a hint of 0, below, for the value of the hint:
; the VM can assume the condition of the br_if is likely *false*, which means we
; likely fall through to return -1.

; CHECK:	    .section	.custom_section.metadata.code.branch_hint,"",@

; Number of functions with hints (1, padded LEB to 5 bytes
; CHECK-NEXT: .asciz "\201\200\200\200"

; Function with the hint.
; CHECK-NEXT:	.uleb128_int32 bw_bh_test

; Number of hints in function.
; CHECK-NEXT:	.int8	1

; Offset of the hint.
; CHECK-NEXT:	.uleb128 .Ltmp0-bw_bh_test

; Size of the hint.
; CHECK-NEXT:	.int8	1

; Value of the hint.
; CHECK-NEXT:	.int8	0

fail:
  ret i32 -1

success:
  ret i32 0
}

!0 = !{!"branch_weights", !"expected", i32 2000, i32 1}

; TODO: a test that starts as asm, and checks either disassembly/objdump or YAML output. Something like llvm/test/MC/WebAssembly/debuginfo-relocs.s or the similar tests in there.
