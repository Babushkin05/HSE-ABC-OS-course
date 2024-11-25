# print string
.macro print_str(%string)
	.data
		string: .asciz %string
	.text
		la a0 string
		li a7 4
		ecall
.end_macro

# read int from console
.macro read_int (%register)
	li a7 5
	ecall
	mv %register a0
.end_macro