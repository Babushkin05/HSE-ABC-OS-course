.text
	# char symbol in a0, return 1 if char is '\0', else 1
	is_null:
		mv t0 a0
		li a0 0
		beqz t0 end
		li a0 1
		end:
		ret

# buf addres to save string, str - label to save str
.macro strcpy(%buf, %str)
	addi sp sp -12
		sw s0 8(sp)
		sw s1 4(sp)
		sw ra (sp)
		
		mv s0 %buf
		la s1 %str
		loop:
			lw t0 (s1)
			sw t0 (s0)
			addi s0 s0 4
			addi s1 s1 4
			mv a0 t0
			call is_null
			bnez a0 loop
		
		lw s0 8(sp)
		lw s1 4(sp)
		lw ra (sp)
		addi sp sp 12
.end_macro
