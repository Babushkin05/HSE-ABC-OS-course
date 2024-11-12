.include "iolib.asm"
.include "geomlib.asm"


.data
	# points data
	p1x: .double 0.0
	p1y: .double 0.0
	p2x: .double 0.0
	p2y: .double 0.0
	p3x: .double 0.0
	p3y: .double 0.0
	p4x: .double 0.0
	p4y: .double 0.0
	
.text
	# void func input data from console
	input_data:
		# prolog
		addi sp sp -64
		fsd fs0 56(sp)
		fsd fs1 48(sp)
		fsd fs2 40(sp)
		fsd fs3 32(sp)
		fsd fs4 24(sp)
		fsd fs5 16(sp)
		fsd fs6 8(sp)
		fsd fs7 (sp)
	
		# input
		print_str("\ninput 4 points:\n")
		input_double(fs0, "x0: ")
		input_double(fs1, "y0: ")
		input_double(fs2, "x1: ")
		input_double(fs3, "y1: ")
		input_double(fs4, "x2: ")
		input_double(fs5, "y2: ")
		input_double(fs6, "x3: ")
		input_double(fs7, "y3: ")
		
		# save data to labels
		fsd fs0 p1x t0
		fsd fs1 p1y t0
		fsd fs2 p2x t0
		fsd fs3 p2y t0
		fsd fs4 p3x t0
		fsd fs5 p3y t0
		fsd fs6 p4x t0
		fsd fs7 p4y t0
		
		# epilog 
		fld fs0 56(sp)
		fld fs1 48(sp)
		fld fs2 40(sp)
		fld fs3 32(sp)
		fld fs4 24(sp)
		fld fs5 16(sp)
		fld fs6 8(sp)
		fld fs7 (sp)
		addi sp sp 64
		
		ret
	
	# swap data and return 1 if two some points and return 2 if 3 points in one line	
	data_handler:
		fld ft0 p1x t0
		fld ft1 p1y t0
		fld ft2 p2x t0
		fld ft3 p2y t0
		fld ft4 p3x t0
		fld ft5 p3y t0
		fld ft6 p4x t0
		fld ft7 p4y t0
		
		feq.d t0 ft0 ft2
		beqz t0 p1x_ne_p2x
			swap_double(ft2, ft6, ft8)
			swap_double(ft3, ft7, ft8)
		p1x_ne_p2x:
		feq.d t0 ft0 ft4
		beqz t0 p1x_ne_p3x
			swap_double(ft4, ft6, ft8)
			swap_double(ft5, ft7, ft8)
		p1x_ne_p3x:
		feq.d t0 ft0 ft2
		bnez t0 p1x_e_p2x
		feq.d t0 ft0 ft4
		bnez t0 p1x_e_p3x
		
		j y_handler
		
		p1x_e_p2x:
			feq.d t0 ft1 ft3
			bnez t0 two_some_point
			j final_if
			
		p1x_e_p3x:
			feq.d t0 ft1 ft5 
			bnez t0 two_some_point
			j final_if
			
		final_if:
			feq.d t0 ft1 ft7
			bnez t0 two_some_point
			li a0 2 # three points on same line
			j end_of_data_handler
			
		two_some_point:
			li a0 1 # of course yes
			j end_of_data_handler
			
		y_handler:
			feq.d t0 ft1 ft3
			beqz t0 p1y_ne_p2y
				swap_double(ft2, ft6, ft8)
				swap_double(ft3, ft7, ft8)
			p1y_ne_p2y:
			feq.d t0 ft1 ft5
			beqz t0 p1y_ne_p3y
				swap_double(ft4, ft6, ft8)
				swap_double(ft5, ft7, ft8)
			p1y_ne_p3y:
			feq.d t0 ft1 ft3
			bnez t0 triple_y
			feq.d t0 ft1 ft5
			bnez t0 triple_y
			
			li a0 0 # nothing interesting
			j end_of_data_handler
			
			triple_y:
			li a0 2 # three point on same line
			
			
		end_of_data_handler: 
			#save data back
			fsd ft0 p1x t0
			fsd ft1 p1y t0
			fsd ft2 p2x t0
			fsd ft3 p2y t0
			fsd ft4 p3x t0
			fsd ft5 p3y t0
			fsd ft6 p4x t0
			fsd ft7 p4y t0
			ret
	
	# returns 1 if a triangle else 0.
	is_triangle:
		fld ft0 p1x t0
		fld ft1 p1y t0
		fld ft2 p2x t0
		fld ft3 p2y t0
		fld ft4 p3x t0
		fld ft5 p3y t0
		fld ft6 p4x t0
		fld ft7 p4y t0
		
		feq.d a0 ft0 ft2
		feq.d t0 ft1 ft5
		and a0 a0 t0
		feq.d t0 ft4 ft6
		and a0 a0 t0
		feq.d t0 ft3 ft7
		and a0 a0 t0
		
		ret
	
	# main part
	solve:
	
		# prolog
		addi sp sp -72
		sw ra 64(sp)
		fsd fs0 56(sp)
		fsd fs1 48(sp)
		fsd fs2 40(sp)
		fsd fs3 32(sp)
		fsd fs4 24(sp)
		fsd fs5 16(sp)
		fsd fs6 8(sp)
		fsd fs7 (sp)
		
		# load data from memory
		fld fs0 p1x t0
		fld fs1 p1y t0
		fld fs2 p2x t0
		fld fs3 p2y t0
		fld fs4 p3x t0
		fld fs5 p3y t0
		fld fs6 p4x t0
		fld fs7 p4y t0
		
		# output
		print_str("\nfor this data:")
		print_double(fs0, "\nx0 = ")
		print_double(fs1, ", y0 = ")
		print_double(fs2, ", x1 = ")
		print_double(fs3, ", y1 = ")
		print_double(fs4, ", x2 = ")
		print_double(fs5, ", y2 = ")
		print_double(fs6, ", x3 = ")
		print_double(fs7, ", y3 = ")
		print_str("\nthe answer is ")
		
		# triangle
		call is_triangle
		bnez a0 yes_triangle
		
		# handle data
		call data_handler
		addi a0 a0 -1
		beqz a0 precision_yes
		addi a0 a0 -1
		beqz a0 no
	
		# find 2 sides of triangle
		fld fs0 p1x t0
		fld fs1 p1y t0
		fld fs2 p2x t0
		fld fs3 p2y t0
		two_points_to_a_registers(fs0, fs1, fs2, fs3)
		call function_of_line
		fmv.d fs4 fa0 # k1
		fmv.d fs5 fa1 # c1
		
		fld fs0 p1x t0
		fld fs1 p1y t0
		fld fs2 p3x t0
		fld fs3 p3y t0
		two_points_to_a_registers(fs0, fs1, fs2, fs3)
		call function_of_line
		fmv.d fs6 fa0 # k2
		fmv.d fs7 fa1 # c2
		
		# find perpendicular bisector (seredinnyi perpendicular)
		fld fs0 p1x t0
		fld fs1 p2x t0
		two_doubles_to_fa_registers(fs0, fs1) 
		call mean_double #find x of intersection of side and perpendicular bisector
		fmv.d fs0 fa0
		preparing_to_finding_perpendicular_line(fs4, fs5, fs0)
		call function_of_perpendicular_line
		fmv.d fs4 fa0 # k'1
		fmv.d fs5 fa1 # c'1
		
		fld fs0 p1x t0
		fld fs1 p3x t0
		two_doubles_to_fa_registers(fs0, fs1) 
		call mean_double #find x of intersection of side and perpendicular bisector
		fmv.d fs0 fa0
		preparing_to_finding_perpendicular_line(fs6, fs7, fs0)
		call function_of_perpendicular_line
		fmv.d fs6 fa0 # k'2
		fmv.d fs7 fa1 # c'2
		
		# find intersection of perpendicular bisector
		preparing_for_finding_intersection(fs4, fs5, fs6, fs7)
		call linear_functions_intersection
		fmv.d fs0 fa0 # x of centre of circle
		fmv.d fs1 fa1 # y of centre of circle
		
		# find radius
		fld fs2 p1x t0
		fld fs3 p1y t0
		two_points_to_a_registers(fs0, fs1, fs2, fs3)
		call distance
		fmv.d fs2 fa0
		
		# is last point in this circle
		fld fs3 p4x t0
		fld fs4 p4y t0
		two_points_to_a_registers(fs0, fs1, fs3, fs4)
		call distance
		feq.d t0 fa0 fs2
		beqz t0 no
		j yes
		
		precision_yes:
			print_str("yes! because input has same points\n")
			j epilog
			
		yes:
			print_str("yes!")
			print_double(fs0, "\nx: ")
			print_double(fs1, "\ny: ")
			print_double(fs2, "\nr: ")
			print_str("\n")
			j epilog
		
		no:
			print_str("no(\n")
			j epilog
			
		yes_triangle:
			print_str("yes! because this is triangle\n")
		epilog:
		lw ra 64(sp)
		fld fs0 56(sp)
		fld fs1 48(sp)
		fld fs2 40(sp)
		fld fs3 32(sp)
		fld fs4 24(sp)
		fld fs5 16(sp)
		fld fs6 8(sp)
		fld fs7 (sp)
		addi sp sp 72
		
		ret
		
		
		
		
		
		
	