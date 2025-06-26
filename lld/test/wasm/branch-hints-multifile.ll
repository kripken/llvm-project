
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

; Test that we combine branch hint sections properly. The number of functions
; should be reported once at the start (even though it appears in each object
; file, and the hints for each object file should then be concatenated).
; CHECK:       - Type:            CUSTOM
; CHECK-NEXT:    Name:            metadata.code.branch_hint
; CHECK-NEXT:    Payload: '8280808000818080800001080100828080800001080101'
;                          ^^ two functions (5-byte padded LEB)
;                                    ^^hint for func 1^
;                                                      ^^hint for func 2^

