.include "IOmacroslib.asm"
.eqv max_array_size_in_bytes 40

.data 
    .align 2
    array_A: .space max_array_size_in_bytes
    array_A_size: .word 0
    array_B: .space max_array_size_in_bytes
    array_B_size: .word 0

# put array data to a registers 
.macro array_data_to_a_registers(%array_label, %array_size_label, %array_name)
	.data 
		name: .asciz %array_name
	.text
		la a0 %array_label
		lw a1 %array_size_label
		la a2 name
.end_macro

# put arrays data to a registers
.macro arrays_data_to_a_registers(%arrayA_label, %arrayA_size_label, %arrayB_label, %arrayB_size_label, %smallest)
	la a0 %arrayA_label
	lw a1 %arrayA_size_label
	la a2 %arrayB_label
	lw a3 %arrayB_size_label
	mv a4 %smallest
.end_macro


# set register1 to min(register1, register2)
.macro set_first_to_min(%register1, %register2)
	ble %register1 %register2 end_of_macros
	mv %register1 %register2
	end_of_macros:	
.end_macro



.text
	# gets length in a0 return 0 in a0 if correct lenght else 1
	check_len: 
		.eqv correct_length 0
   		.eqv incorrect_length 1
   		.eqv max_length 10
   		.eqv min_length 1
   		
   		mv t2 a0
   		li a0 correct_length
   		li t0 min_length
   		li t1 max_length
   		bgt t2 t1 bad_length
   		blt t2 t0 bad_length
   		j end_of_func
   		bad_length:
   			li a0 incorrect_length
   		end_of_func:
   			ret
   	
   	# read array length and put it in a0
   	read_array_lenght:
   		addi sp sp -8 # prolog
   		sw ra 4(sp)
    	sw s0 (sp)
    
    	j begin
    	error:
        	print_str("ERROR! wrong length. ")
    	begin:
    	print_str("Input len of array A (from 1 to 10) :: ")
    	read_int(s0)
    	register_to_a0(s0)
    	call check_len # check len from a0. return 0 in a0 if correct. else 1.
    	bnez a0 error
    	mv a0 s0
    
    	lw ra 4(sp) # epilog
    	lw s0 (sp) 
    	addi sp sp 8
    	
    	ret
    
    # fills array A. Gets aray addres in a0, and array size in a1
    fill_array_from_console:
    	mv t0 a0 # adress of next value
    	li t1 0 # index of next value
    	mv t2 a1 # array size
    	fill_loop:
    		print_str("Inpute element at index ")
    		print_int_from_register(t1)
    		print_str(": ")
        	read_int(t3)
        	sw t3 (t0)
        	addi t0 t0 4
        	addi t1 t1 1
        	blt t1 t2 fill_loop
        ret
    
    # print array in terminal. Gets aray addres in a0, and array size in a1, and name to a2
	show_array:
		mv t0 a0
    	li t1 0
    	mv t2 a1
    	print_str("This is ")
    	print_from_adress(a2)
    	print_str(" array : \n[ ")
    	beqz t2 end_of_showing
    	show_loop:
    		lw t3 (t0)
    		print_int_from_register(t3)
    		print_char(' ')
    		addi t0 t0 4
    		addi t1 t1 1
    		blt t1 t2 show_loop
    	end_of_showing:
    		print_str("]\n")
    		ret 
    
    # returns minimum element frim array. Gets aray addres in a0, and array size in a1 
    get_array_min:
    	mv t0 a0 # adress of next value
    	li t1 0 # index of next value
    	mv t2 a1 # array size
    	.eqv max_int 2147483647
    li t3 max_int
    find_min_loop:
    	lw t4(t0)
    	set_first_to_min(t3, t4)
    	addi t0 t0 4
    	addi t1 t1 1
    	blt t1 t2 find_min_loop
    mv a0 t3 
    ret
	
	# returns len of array without this elem. Gets aray addres in a0  array size in a1  and elem in a3
	get_len_of_array_without_elem:
		mv t0 a0 # adress of next value
    	li t1 0 # index of next value
    	mv t2 a1 # array size
    	mv t3 a3 # elem
    	li t4 0 # len of B
    	get_len_loop:
    		lw t5 (t0)
    		beq t5 t3 end_of_get_len_loop
    		addi t4 t4 1
    		end_of_get_len_loop:
    		addi t1 t1 1
    		addi t0 t0 4
    		blt t1 t2 get_len_loop
    	mv a0 t4
    	ret
    
    # fill B array of non minimal A arrays values. Gets A array adress in a0, A len in a1, B adress in a2, B len in a3, minimal A value in a4
    fill_B_array:
    	addi sp sp -4 # prolog
    	sw s0 (sp)
    
    	mv t0 a0 # adress of the next A array value
    	mv t1 a1 # len of A array
    	li t2 0 # A array index
    	mv t3 a2 # adress of the next B array value
    	mv t4 a3 # len of B array
	    li t5 0 # B array index
	    mv s0 a4 # smallest value
	    fill_B_loop:
	    	lw t6 (t0)
	    	beq s0 t6 end_of_fill_B_loop
	    	sw t6 (t3)
	    	addi t3 t3 4
	    	addi t5 t5 1
	    	end_of_fill_B_loop:
	    		addi t2 t2 1
	    		addi t0 t0 4
	    		blt t5 t4 fill_B_loop
	    
	    lw s0 (sp) #epilog
	    addi sp sp 4
	    ret
	    	
	    	
	    	
	    
