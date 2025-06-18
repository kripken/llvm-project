# RUN: llvm-mc -filetype=obj -triple=wasm32-unknown-unknown -o %t.o %s
# RUN: obj2yaml %t.o | FileCheck %s

	.file	"c.ll"
	.functype	bw_bh_test (i32, i32) -> (i32)
	.section	.text.bw_bh_test,"",@
	.globl	bw_bh_test                      # -- Begin function bw_bh_test
	.type	bw_bh_test,@function
bw_bh_test:                             # @bw_bh_test
	.functype	bw_bh_test (i32, i32) -> (i32)
# %bb.0:
	block   	
	local.get	0
	local.get	1
	i32.ge_u
.Ltmp0:
	br_if   	0                               # 0: down to label0
# %bb.1:                                # %fail
	i32.const	-1
	return
.LBB0_2:                                # %success
	end_block                               # label0:
	i32.const	0
                                        # fallthrough-return
	end_function
                                        # -- End function
	.section	.text.bw_bh_test,"",@
	.section	.custom_section.metadata.code.branch_hint,"",@
	.int8	1
	.uleb128 bw_bh_test
	.int8	1
	.uleb128 .Ltmp0-bw_bh_test
	.int8	1
	.int8	0
	.section	.text.bw_bh_test,"",@

## Test handling of ULEB128 fields in the branch hints section.
## TODO checks
