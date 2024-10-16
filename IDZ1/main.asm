.include "tester.asm"
.global main

main:
		print_str("type 0 if you want run tests, or something else to fill array from terminal :: ")
		read_int(t0) # choosing a program mode
		
		beqz t0 to_test # if user wnat autotesting
		
		call input # if user test for himself. void func.
		j finish
		
		to_test:
			call tester # void func
		
		finish: 
		exit # finishing program