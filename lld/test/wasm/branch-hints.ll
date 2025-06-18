
; RUN: llc -mtriple=wasm32-unknown-unknown -filetype=obj -o %t.o < %s
; RUN: wasm-ld -o %t.wasm %t.o --no-entry --no-gc-sections
; RUN: obj2yaml %t.wasm | FileCheck %s

define i32 @bw_bh_test(i32 %a, i32 %b) {
; The weights 42 (true branch) and 1337 (false branch) mean the false
; branch is significantly more likely. The test should generate a hint of "0"
; (unlikely).
entry:
  %1 = icmp ult i32 %a, %b
  br i1 %1, label %fail, label %success, !prof !0

fail:
  ret i32 -1

success:
  ret i32 0
}

!0 = !{!"branch_weights", i32 42, i32 1337}

; CHECK:       - Type:            CUSTOM
; CHECK-NEXT:    Name:            metadata.code.branch_hint
; CHECK-NEXT:    Payload:         '01818080800001080100'
;                                  ^^ one function
;                                    ^^^^^^^^^^ LEB of function index 1
;                                              ^^ one hint in function
;                                                ^^ offset 8
;                                                  ^^ hint size 1
;                                                    ^^ hint value: 0
