.include "program.asm"


.text	
	# make testes.
	tester:
		addi sp sp -4 # prolog
        sw ra (sp)
        		
		newline # test1
		call store_test_data1 # void func without params
		call main_part # void func without params
		
		newline # test2
		call store_test_data2 # void func without params
		call main_part # void func without params
		
		newline # test3
		call store_test_data3 # void func without params
		call main_part # void func without params
		
		newline #test4
		call store_test_data4 # void func without params
		call main_part # void func without params
		
		newline # test5
		call store_test_data5 # void func without params
		call main_part # void func without params
		
		lw ra (sp) # epilog
        addi sp sp 4
        
        ret
	
	# test1
	store_test_data1:
		la t0 array_A
		li t1 1
		sw t1 (t0)
		li t1 2
		sw t1 4(t0)
		li t1 3
		sw t1 8(t0)
		li t1 4
		sw t1 12(t0)
		li t1 5
		sw t1 16(t0)
		la t0 array_A_size
		li t1 5
		sw t1 (t0)
		ret
	
	# test2
	store_test_data2:
		la t0 array_A
		li t1 1
		sw t1 (t0)
		li t1 1
		sw t1 4(t0)
		li t1 1
		sw t1 8(t0)
		li t1 1
		sw t1 12(t0)
		li t1 1
		sw t1 16(t0)
		la t0 array_A_size
		li t1 5
		sw t1 (t0)
		ret
	
	# test3
	store_test_data3:
		la t0 array_A
		li t1 1
		sw t1 (t0)
		li t1 1
		sw t1 4(t0)
		li t1 1
		sw t1 8(t0)
		li t1 1
		sw t1 12(t0)
		li t1 2
		sw t1 16(t0)
		la t0 array_A_size
		li t1 5
		sw t1 (t0)
		ret
	
	# test4
	store_test_data4:
		la t0 array_A
		li t1 0
		sw t1 (t0)
		li t1 -1
		sw t1 4(t0)
		li t1 -2
		sw t1 8(t0)
		li t1 -3
		sw t1 12(t0)
		li t1 -4
		sw t1 16(t0)
		la t0 array_A_size
		li t1 5
		sw t1 (t0)
		ret
	
	# test5
	store_test_data5:
		la t0 array_A
		li t1 0
		sw t1 (t0)
		la t0 array_A_size
		li t1 1
		sw t1 (t0)
		ret
	