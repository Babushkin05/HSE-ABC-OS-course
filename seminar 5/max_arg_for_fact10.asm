.data
ans1: .asciz "max factorial in 4 bites word is "
ans2: .asciz "! = "

.text

la a0 ans1
li a7 4
ecall

call fact
li a7, 1
ecall

la a0 ans2
li a7 4
ecall

mv a0 a1
li a7 1
ecall

li a7,10
ecall


fact: # void func. return a0 - n (max argument of factorial), a1 - n!
# save data to stack
addi sp sp -16
sw ra 12(sp)
sw t0 8(sp)
sw t1 4(sp)
sw t2 (sp)

li a0, 1
li a1, 1

call rec_counting
# end of recursion
addi a0, a0, -1

# load data from stack
lw ra 12(sp)
lw t0 8(sp)
lw t1 4(sp)
lw t2 (sp)
addi sp sp 16

ret

check_overflow: # boolean function gets (n-1!) in a1 and n in a0. return a0, 1 if overflow else 0
mul t0 a0 a1
div t1 t0 a0

bne t1 a1 not_eq
li a0 0
ret
not_eq:
li a0 1
ret


rec_counting: # recursive counting for fact
addi t0, a0, 1
mv t1 a1

# save data to stack
addi sp sp -12
sw ra 8(sp)
sw t0 4(sp)
sw t1 (sp)

# checking overlap
call check_overflow

# load from stack
lw ra 8(sp)
lw t0 4(sp)
lw t1 (sp)
addi sp sp 12


bnez a0 rec_end

# save data to stack
addi sp sp -12
sw ra 8(sp)
sw t0 4(sp)
sw t1 (sp)
# recursive call
mv a0 t0
mul a1 t1 t0
call rec_counting
# load from stack
lw ra 8(sp)
lw t0 4(sp)
lw t1 (sp)
addi sp sp 12
ret

rec_end:
mv a0 t0
ret

