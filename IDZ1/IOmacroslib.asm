# read int from console
.macro read_int (%register)
	li a7 5
	ecall
	mv %register a0
.end_macro

# prints string
.macro print_str (%string)
   .data
	str: .asciz %string
   .text
       li a7, 4
       la a0, str
       ecall
.end_macro

# prints string from addres
.macro print_from_adress (%adr)
	li a7 4
	mv a0 %adr
	ecall
.end_macro

# prints int from register
.macro print_int_from_register(%register)
    mv a0 %register
    li a7 1
    ecall
.end_macro

# prints char
.macro print_char(%char)
   li a7, 11
   li a0, %char
   ecall
.end_macro

# prints '\n'
.macro newline
   print_char('\n')
.end_macro

# puts word in register to a0
.macro register_to_a0 (%reg)
	mv a0 %reg
.end_macro

# finish program
.macro exit
    li a7, 10
    ecall
.end_macro


