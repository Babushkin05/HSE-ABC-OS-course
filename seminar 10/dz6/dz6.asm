.global main
.eqv SIZE 100
.include "strcpy.asm"
.include "iolib.asm"

.data
	test: .asciz "hello"

.text
	# main fuction	
	main:
	li a0 SIZE
	li a7 9
	ecall
	mv s0 a0
	
	print_str("type 0 if want to print string by yourself, or something else if test auto :: ")
	read_int(a0)
	
	bnez a0 exec
	
	print_str("type string :: ")
	la a0 test
	li a1 100
	li a7 8
	ecall
	
	exec:
	strcpy(s0, test)
	
	print_str("string by buffer addres :: ")
	mv a0 s0
	li a7 4
	ecall
	li a7 10
	ecall
	
		