
; RUN: llc -mtriple=wasm32-unknown-unknown -filetype=obj -o %t1.o < %s
; RUN: llc -mtriple=wasm32-unknown-unknown -filetype=obj -o %t2.o < %S/Inputs/branch-hints-multifile.ll
; RUN: wasm-ld -o %t.wasm %t1.o %t2.o --no-entry --no-gc-sections
; RUN: obj2yaml %t.wasm | FileCheck %s

define i32 @bw_bh_test_1(i32 %a, i32 %b) {
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
