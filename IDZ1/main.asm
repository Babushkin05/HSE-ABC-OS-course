.include "tester.asm"
.global main

main:
		print_str("type 0 if you want run tests, or something else to fill array from terminal :: ")
		read_int(t0) # choosing a program mode
		
		beqz t0 to_test # if user wnat autotesting
		
		call input # if user test for himself
		j finish
		
		to_test:
			call tester
		
		finish: 
		exit # finishing program