.include "arraymacroslib"

 
 .data 
 	is_tester: .word 0
 

.text
	# input arrays data
    input:
    	addi sp sp -4 # prolog
        sw ra (sp)
        
        call read_array_lenght # read array length from terminal
        sw a0 array_A_size t0 # save length 
        
        array_data_to_a_registers(array_A, array_A_size, "A")
        call fill_array_from_console # filling array A from terminal
        
        call main_part
        
        lw ra (sp) # epilog
        addi sp sp 4
        
        ret
        
        # array processing
    main_part:
        addi sp sp -4 # prolog
        sw ra (sp)
        
 		array_data_to_a_registers(array_A, array_A_size, "A")
 		call show_array # print A array to console
 		  
		array_data_to_a_registers(array_A, array_A_size, "A")
        call get_array_min # get minimum from array A
        mv s0 a0 # save minimum element to s0
        
        array_data_to_a_registers(array_A, array_A_size, "A")
        mv a3 s0 # push minimal element to func argument
        call get_len_of_array_without_elem # find len of b array
        sw a0 array_B_size t0 # save lenght
        
        arrays_data_to_a_registers(array_A, array_A_size, array_B, array_B_size, s0)
        call fill_B_array # filling B array
        
        array_data_to_a_registers(array_B, array_B_size, "B")
        call show_array # print B array to console
        	
        lw ra (sp) 
        addi sp sp 4
        	
		ret