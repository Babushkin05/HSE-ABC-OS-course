.include "tester.asm"
.global main

.text
	main:
	
		print_str("type 0 if you want run tests, or something else to fill array from terminal :: ")
		read_int(t0) # choosing a program mode
		
		beqz t0 to_test # if user wnat autotesting
		# input data
		call input_data	
		# solve
		call solve
		j finish
		
		# test cases
		to_test:
			call tester
		
		# finish program with exit code 0
		finish:
		li a7 10
		ecall

		
		