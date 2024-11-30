.include "tester.asm"
.global main

.text
	# master function
	main:
		# is user wants open his file
		print_str("type 'Y' for start autotest, or something else to open your file:: ")
		read_char(s0)
		li t0 89 # 'Y' in ASCII
		beq s0 t0 tests # user type 'Y'
		
		# reads filename from console, void non parametric func
		call read_name_of_file
		# buisness logic, void non parametric func
		call program
		exit
		
		tests:
		# test program by existing files, void non parametric func
		call run_tests 
		exit