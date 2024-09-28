.data
len: .asciz "type len (from 1 to 10) :: "
elem: .asciz "type elem for array: "
result1: .asciz "after counting " 
result2: .asciz " elements, sum of array = "
err: .asciz "error: wrong n"
endl: .asciz "\n"
.align 2
arr: .space 40
arrend: 

.text
# len borders
li t5, 1
li t6, 10
j input_n

error: # wrong len
li a7 4
la a0,err
ecall
la a0,endl
ecall

input_n:  # inputtting len
la a0, len
li a7, 4
ecall
li a7, 5
ecall

# len check
blt a0, t5, error
bgt a0,t6, error

# preparation for filling
mv t2, a0
li t1, 0
la t0, arr

fill: # array filling
addi t1, t1, 1
la a0, elem
li a7, 4
ecall
li a7, 5
ecall
sw a0 (t0)
addi t0, t0, 4
bltu t1, t2, fill

# preparation for summing
li t3, 0
li t1, 0
la t0, arr

sum: # summing
lw a0 (t0)
# checking on overflow
bltz a0 less_zero
add a1, a0, t3
blt a1, t3, final
j continue

less_zero:
add a1, a0, t3
bgt a1, t3, final

# if checking was good
continue:
addi t1,t1,1
mv t3, a1
addi t0, t0, 4
bltu t1, t2, sum

final: # output result
la a0, result1
li a7, 4
ecall
mv a0, t1
li a7, 1
ecall 
la a0, result2
li a7, 4
ecall
#finishing program
mv a0, t3
li a7,1
ecall

li a0,0 
li a7,10
ecall