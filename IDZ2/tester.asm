.include "program.asm"

# macros for store test data
.macro store_test(%x1, %y1, %x2, %y2, %x3, %y3, %x4, %y4)
	.data 
		x1_test: .double %x1
		y1_test: .double %y1
		x2_test: .double %x2
		y2_test: .double %y2
		x3_test: .double %x3
		y3_test: .double %y3
		x4_test: .double %x4
		y4_test: .double %y4
	.text
		fld ft0 x1_test t0
		fld ft1 y1_test t0
		fld ft2 x2_test t0
		fld ft3 y2_test t0
		fld ft4 x3_test t0
		fld ft5 y3_test t0
		fld ft6 x4_test t0 
		fld ft7 y4_test t0
		
		fsd ft0 p1x t0
		fsd ft1 p1y t0
		fsd ft2 p2x t0
		fsd ft3 p2y t0
		fsd ft4 p3x t0
		fsd ft5 p3y t0
		fsd ft6 p4x t0 
		fsd ft7 p4y t0
.end_macro

.text
	# void function without parameters, that test program
	tester:
		# prolog
		addi sp sp -4
		sw ra (sp)
		
		# test1
		store_test(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0)
		call solve
		
		# test2
		store_test(0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0)
		call solve
		
		# test3
		store_test(0.0, 0.0, 1.0, 0.0, 2.0, 1.0, -1.0, 2.0)
		call solve
		
		# test4 
		store_test(0.0, 0.0, 1.0, 1.0, 0.0, 2.0, -1.0, 1.0)
		call solve
		
		# test5
		store_test(0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 2.0, 2.0)
		call solve
		
		# test6
		store_test(0.0, 0.0, 0.0, 1.0, 0.0, 2.0, 3.0, 3.0)
		call solve
		
		# epilog
		lw ra (sp)
		addi sp sp 4
		
		ret
		
		
