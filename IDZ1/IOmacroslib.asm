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

# returns 0 in a0 if %len is correct else 1
.macro check_length (%len)
   .eqv correct_length 0
   .eqv incorrect_length 1
   .eqv max_length 10
   .eqv min_length 1
   
   li a0 correct_length
   li t0 min_length
   li t1 max_length
   bgt %len t1 bad_length
   blt %len t0 bad_length
   j end_of_macro
   bad_length:
   li a0 incorrect_length
   end_of_macro:
.end_macro

# read array length and put it in a0
.macro read_array_lenght
    addi sp sp -4 # prolog
    sw s0 (sp)
    
    j begin
    error:
        print_str("ERROR! wrong length. ")
    begin:
    print_str("Input len of array A (from 1 to 10) :: ")
    read_int(s0)
    check_length(s0)
    bnez a0 error
    mv a0 s0
    
    lw s0 (sp) # epilog
    addi sp sp 4
.end_macro

# finish program
.macro exit
    li a7, 10
    ecall
.end_macro


