.include "macro-syscalls.m"

.eqv    NAME_SIZE 256	# buffer size for filename
.eqv    TEXT_SIZE 512	# buffer size for text part
.eqv 	MAX_TEXT_SIZE 10240 # maximum length of text

.data
	er_name_mes: .asciz "Incorrect file name\n"
	er_read_mes: .asciz "Incorrect read operation\n"

	file_name: .space NAME_SIZE	# filename
	strbuf:	.space TEXT_SIZE # buffer for text part

.text
	# void function witout parameters, fill filename from console
    read_name_of_file:
    	# prolog
    	addi sp sp -4
    	sw ra (sp)
    	
    	li a3 0
    	input_name:
    	print_str ("Input path to file for reading: ") 
    	str_get(file_name, NAME_SIZE) # getting filename from terminal
    	
    	beqz a3 read_name_epilog 
    	call read_string_from_file # if user input wrong name in last time
    	
    	# epilog
    	read_name_epilog:
    	lw ra (sp)
    	addi sp sp 4
    
    # function without parameters, return addres of readed text in a0, and its size in a1 
    read_string_from_file:
    	# prolog
    	addi sp sp -32
    	sw s0 28(sp)
    	sw s1 24(sp)
    	sw s2 20(sp)
    	sw s3 16(sp)
    	sw s4 12(sp)
    	sw s5 8(sp)
    	sw s6 4(sp)
    	sw ra (sp)
    	
    	open(file_name, READ_ONLY)
    	li		s1 -1			# is correct openning
    	beq		a0 s1 er_name	# error name
    	mv   	s0 a0       	# file descriptor
   
    	allocate(TEXT_SIZE)		# allocate buffer size
    	mv 		s3, a0			# addres of begining
    	mv 		s5, a0			# addres of last elem
    	li		s4, TEXT_SIZE	
    	mv		s6, zero		# readed text size
    	
		read_loop:
    		read_addr_reg(s0, s5, TEXT_SIZE) # read addres of block
    		beq		a0 s1 er_read	# error of reading
    		mv   	s2 a0       	# save string length
    		add 	s6, s6, s2		# new text size
    		li t0 MAX_TEXT_SIZE
    		bge s6 t0 end_loop # text size begger than 10kb 
    		bne	s2 s4 end_loop # we read all text
    		allocate(TEXT_SIZE)	# make buffer bigger
    		add		s5 s5 s2		
    		b read_loop				
		end_loop:

    	close(s0)

    	mv	t0 s3		# addres of buffer in heap
    	add t0 t0 s6	# addres of last char
    	addi t0 t0 1	
    	sb	zero (t0)	# make string null terminates

    	mv a0 s3 # adress of begining string
    	mv a1 s6 # string length
    
    	# epilog
    	lw s0 28(sp)
    	lw s1 24(sp)
    	lw s2 20(sp)
    	lw s3 16(sp)
    	lw s4 12(sp)
    	lw s5 8(sp)
    	lw s6 4(sp)
    	lw ra (sp)
    	addi sp sp 32
    	ret
    
    # takes string adress in a0, string length in a1, prints string in file
    save_string_to_file:
    
    	# prolog
    	addi sp sp -32
    	sw s0 28(sp)
    	sw s1 24(sp)
    	sw s2 20(sp)
    	sw s3 16(sp)
    	sw s4 12(sp)
    	sw s5 8(sp)
    	sw s6 4(sp)
    	sw ra (sp)
    	
    	mv s3 a0 # string addres
    	mv s6 a1 # string length

    	print_str ("Input path to file for writing: ")
    	str_get(file_name, NAME_SIZE) # getting filename
    	open(file_name, WRITE_ONLY)
    	li		s1 -1			
    	beq		a0 s1 er_name	# is correct opening
    	mv   	s0 a0       	# file descriptor

    	li   a7, 64       		
    	mv   a0, s0 			# file descriptor
    	mv   a1, s3  			# string addres
    	mv   a2, s6    			# string size
    	ecall             		

   		# epilog
    	lw s0 28(sp)
    	lw s1 24(sp)
    	lw s2 20(sp)
    	lw s3 16(sp)
    	lw s4 12(sp)
    	lw s5 8(sp)
    	lw s6 4(sp)
    	lw ra (sp)
    	addi sp sp 32
   		ret

# prints error of name
er_name:
    # Сообщение об ошибочном имени файла
    li a3 1
    la		a0 er_name_mes
    li		a7 4
    j input_name

# prints error of read
er_read:
    # Сообщение об ошибочном чтении
    li a3 1
    la		a0 er_read_mes
    li		a7 4
    j input_name
   
   	# takes string adress in a0, string length in a1, prints string in file
 	output:
 	addi sp sp -12
 	sw s0 8(sp)
 	sw s1 4(sp)
 	sw ra (sp)
 	
 	mv s0 a0
 	mv s1 a1
 	.data
 		out_to_console: .asciz "\nDo you want to print output into console? (Y/N):: "
 	.text
 	
 	la a0 out_to_console
 	li a7 4
 	ecall 
 	
 	ynloop:
 	li a7 12
 	ecall
 	li t0 89 # 'Y'
 	beq t0 a0 console_out
 	li t0 78 # 'N'
 	beq t0 a0 file_out
 	j ynloop

 	console_out:
 	mv a0 s0
 	li a7 4
 	ecall
 	j epilog
 	
 	file_out:
 	mv a0 s0
 	mv a1 s1
 	call save_string_to_file
 	
 	epilog:
	lw s0 8(sp)
 	lw s1 4(sp)
 	lw ra (sp)
 	addi sp sp 12
 	ret
