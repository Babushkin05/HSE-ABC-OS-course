# read int from console
.macro read_int (%register)
	li a7 5
	ecall
	mv %register a0
.end_macro

.macro print_str(%string)
	.data
		string: .asciz %string
	.text
		la a0 string
		li a7 4
		ecall
.end_macro

.macro input_double(%ft, %string)
	print_str(%string)
	li a7 7
	ecall
	fmv.d %ft fa0
.end_macro

.macro print_double(%ft, %string)
	print_str(%string)
	fmv.d fa0 %ft
	li a7 3
	ecall
.end_macro

.macro swap_double(%ft1, %ft2, %temp)
	fmv.d %temp %ft1
	fmv.d %ft1 %ft2
	fmv.d %ft2 %temp
.end_macro