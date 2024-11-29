.macro store_last_byte_from_word(%t, %addr)
	# prolog
	addi sp sp -12
	sw s0 8(sp)
	sw a0 4(sp)
	sw a7 (sp)
	
	li a7 9
	li a0 4
	ecall
	
	sw %t (a0)
	addi a0 a0 3
	lb %t (a0)
	sb %t (%addr)
	
	# epilog
	lw s0 8(sp)
	lw a0 4(sp)
	lw a7 (sp)
	addi sp sp 12
.end_macro

.text
	# takes letter in a0, returns letter type: 1 - vowel, 2 - consonant, 0 - other
	letter_type:
		mv t5 a0
		li a7, 11
		ecall
		.data
			vowels: .asciz "eyuioaEYUIOA" 
			.align 2 
			consonants: .asciz "qwrtpsdfghjklzxcvbnmQWRTPSDFGHJKLZXCVBNM"
		.text
		la t0 vowels
		li t1 0 # vowels string index
		li t2 12 # vowels string length
		
		vowel_loop:
			lb t3 (t0)
			beq t3 t5 is_vowel
			addi t0 t0 1
			addi t1 t1 1
			blt t1 t2 vowel_loop
		
		la t0 consonants
		li t1 0 # consonants string index
		li t2 40 # consonants string length
		consonant_loop:
			lb t3 (t0)
			beq t3 t5 is_consonant
			addi t0 t0 1
			addi t1 t1 1
			blt t1 t2 consonant_loop
		
		li a0 0
		ret
		
		is_vowel:
		li a0 1
		ret
		
		is_consonant:
		li a0 2
		ret
		
	# takes adress of string begining in a0, len of string in a1, returns count of vowels in a0 and consonants in a1 in string
	counting_vowels_and_consonants:
	
		# prolog
		addi sp sp 24
		sw s0 20(sp)
		sw s1 16(sp)
		sw s2 12(sp)
		sw s3 8(sp)
		sw s4 4(sp)
		sw ra(sp)
	
		mv s0 a0 # string adres
		mv s1 a1 # len of string
		li s2 0 # index in string
		li s3 0 # vowels count
		li s4 0 # consonants count
	
		string_loop:
			lb a0 (s0)	
			call letter_type
			beqz a0 iteration
			addi a0 a0 -1
			beqz a0 vowel
		
			addi s4 s4 1
			j iteration
		
			vowel:
			addi s3 s3 1
		
			iteration:
			addi s0 s0 1
			addi s2 s2 1
			blt s2 s1 string_loop
	
		# data to return
		mv a0 s3
		mv a1 s4
	
		# epilog
		lw s0 20(sp)
		lw s1 16(sp)
		lw s2 12(sp)
		lw s3 8(sp)
		lw s4 4(sp)
		lw ra(sp)
	
		ret
	
	# creates string by vowels and consonants count, vowels in a0, consonants in a1, returns string addres in a0, string length in a1
	create_string:
	
	# prolog
	addi sp sp -32
	sw s0 28(sp)
	sw s1 24(sp)
	sw s2 20(sp)
	sw s3 16(sp)
	sw s4 12(sp)
	sw s5 8(sp)
	sw s6 4(sp)
	sw s7 (sp)
	
	
	mv s0 a0
	mv s1 a1
	
	.data
		cons: "\nnumber of consonants: "
		vow: "number of vowels: "
	.text
	
	# memory allocation
	li a7 9
	li a0 100
	ecall
	mv s5 a0 # buffer addres for result
	mv s7 a0 # addres for last char
	mv s6 zero # size of result
	
	la s2 vow # cons string addres
	li s3 0 #cons string index
	li s4 18 # cons string size
	vow_loop:
		lb t0 (s2)
		sb t0 (s7)
		addi s7 s7 1
		addi s3 s3 1
		addi s2 s2 1
		blt s3 s4 vow_loop
	
	add s6 s6 s4
	
	li t0 1000
	div t1 s0 t0
	addi t1 t1 30 # 30 is '0' in ascii
	store_last_byte_from_word(t1,s7)
	addi s7 s7 1
	
	li t0 1000
	rem s0 s0 t0
	li t0 100
	div t1 s0 t0
	addi t1 t1 30
	store_last_byte_from_word(t1,s7)
	addi s7 s7 1
	
	li t0 100
	rem s0 s0 t0
	li t0 10 
	div t1 s0 t0
	addi t1 t1 30
	store_last_byte_from_word(t1,s7)
	addi s7 s7 1
	
	li t0 10
	rem s0 s0 t0
	addi t1 s0 30
	store_last_byte_from_word(t1,s7)
	addi s7 s7 1
	
	addi s6 s6 4
	
	la s2 cons # cons string addres
	li s3 0 # cons string index
	li s4 23 # cons string size
	cons_loop:
		lb t0 (s2)
		sb t0 (s7)
		addi s7 s7 1
		addi s3 s3 1
		addi s2 s2 1
		blt s3 s4 cons_loop
	
	add s6 s6 s4
	
	li t0 1000
	div t1 s1 t0
	addi t1 t1 30 # 30 is '0' in ascii
	store_last_byte_from_word(t1,s7)
	addi s7 s7 1
	
	li t0 1000
	rem s1 s1 t0
	li t0 100
	div t1 s1 t0
	addi t1 t1 30
	store_last_byte_from_word(t1,s7)
	addi s7 s7 1
	
	li t0 100
	rem s1 s1 t0
	li t0 10 
	div t1 s1 t0
	addi t1 t1 30
	store_last_byte_from_word(t1,s7)
	addi s7 s7 1
	
	li t0 10
	rem s1 s1 t0
	addi t1 s1 30
	store_last_byte_from_word(t1,s7)
	addi s7 s7 1
	
	addi s6 s6 4
	
	mv a0 s5
	mv a1 s6
	
	# epilog
	lw s0 28(sp)
	lw s1 24(sp)
	lw s2 20(sp)
	lw s3 16(sp)
	lw s4 12(sp)
	lw s5 8(sp)
	lw s6 4(sp)
	lw s7 (sp)
	addi sp sp 32
	
	ret
	
	
	

	
		
	