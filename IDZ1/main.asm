.include "arraymacroslib"

.global main

.text
    main:
        read_array_lenght # read array length from terminal
        sw a0 array_A_size t0 # save length 
        fill_array(array_A, array_A_size) # filling array A from terminal
        show_array(array_A, array_A_size, "A") # prints array A in terminal
        get_array_min(array_A, array_A_size) # get minimum from array A
        print_int_from_register(a0)
	exit
