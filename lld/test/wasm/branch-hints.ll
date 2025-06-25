
; RUN: llc -mtriple=wasm32-unknown-unknown -filetype=obj -o %t.o < %s
; RUN: wasm-ld -o %t.wasm %t.o --no-entry --no-gc-sections
; RUN: obj2yaml %t.wasm | FileCheck %s

define i32 @bw_bh_test(i32 %a, i32 %b) {
; The weights below mean we are far more likely to go to %fail and return -1.
; Codegen will emit the -1 first, so we emit a hint of 0, below, for the value
; of the hint (as in llvm/test/Codegen/WebAssembly/branch-hints.ll).
entry:
  %1 = icmp ult i32 %a, %b
  br i1 %1, label %fail, label %success, !prof !0

fail:
  ret i32 -1

success:
  ret i32 0
}

!0 = !{!"branch_weights", !"expected", i32 2000, i32 1}

; CHECK:       - Type:            CUSTOM
; CHECK-NEXT:    Name:            metadata.code.branch_hint
; CHECK-NEXT:    Payload: '8180808000818080800001080100'
;                          ^^ one function (5-byte padded LEB)
;                                    ^^^^^^^^^^ LEB of function index 1
;                                              ^^ one hint in function
;                                                ^^ offset 8
;                                                  ^^ hint size 1
;                                                    ^^ hint value: 0
