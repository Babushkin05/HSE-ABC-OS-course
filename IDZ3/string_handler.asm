.text
	# takes letter in a0, returns letter type: 1 - vowel, 2 - consonant, 0 - other
	letter_type:
		mv t0 a0
		.data
			vowels: .asciz "eyuioaEYUIOA"
			consonants: .asciz "qwrtpsdfghjklzxcvbnmQWRTPSDFGHJKLZXCVBNM"
		.text
		la t0 vowels
		li t1 0 # vowels string index
		li t2 10 # vowels string length
		
		vowel_loop:
			lw t3 (t0)
			beq t3 a0 is_vowel
			addi t0 t0 4
			addi t1 t1 1
			blt t1 t2 vowel_loop
		
		la t0 consonants
		li t1 0
		li t2 40
		consonant_loop:
			lw t3 (t0)
			beq t3 a0 is_consonant
			addi t0 t0 4
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
	
	# epilog
	addi sp sp 24
	sw s0 20(sp)
	sw s1 16(sp)
	sw s2 12(sp)
	sw s3 8(sp)
	sw s4 4(sp)
	sw ra(sp)
	
	mv s0 a0
	mv s1 a1
	li s2 0 # index in string
	li s3 0 # vowels count
	li s4 0 # consonants count
	
	string_loop:
		lw a0 (s0)
		call letter_type
		beqz a0 iteration
		addi a0 a0 -1
		beqz a0 vowel
		
		addi s4 s4 1
		j iteration
		
		vowel:
		addi s3 s3 1
		
		iteration:
		addi a0 a0 4
		addi s2 s2 1
		blt s2 s1 string_loop
	
	# data to return
	mv a0 s3
	mv a1 s4
	
	# prolog
	lw s0 20(sp)
	lw s1 16(sp)
	lw s2 12(sp)
	lw s3 8(sp)
	lw s4 4(sp)
	lw ra(sp)
	
	ret
	
	
		
	