; RUN: llc -mtriple=wasm32-unknown-unknown -filetype=asm -o - < %s | FileCheck %s

define i32 @bw_bh_test(i32 %a, i32 %b) {
; The weights 42 (true branch) and 1337 (false branch) mean the false
; branch is significantly more likely. The test should generate a hint of "0"
; (unlikely).
entry:
  %1 = icmp ult i32 %a, %b
  br i1 %1, label %fail, label %success, !prof !0

; CHECK:	    .section	.custom_section.metadata.code.branch_hint,"",@

; Number of functions with hints.
; CHECK-NEXT:	.int8	1

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

!0 = !{!"branch_weights", !"expected", i32 1, i32 2000}

; TODO: a test that starts as asm, and checks either disassembly/objdump or YAML output. Something like llvm/test/MC/WebAssembly/debuginfo-relocs.s or the similar tests in there.
