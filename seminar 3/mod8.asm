.data
arg1: .asciz "Input a: "
arg2: .asciz "Input b: "
result: .asciz "a % b = "
err: .asciz "error: b = 0"
endl: .asciz "\n"

.text
#input
la a0,arg1
li a7, 4
ecall
li a7 5
ecall 
mv t0, a0

la a0,arg2
li a7, 4
ecall
li a7 5
ecall 
mv t1, a0

# обработка введенных данных
beqz t1, errorcase

li t2, 1
bltz t0, calz
p1:
bltz t1, cblz
p2:

# вычисление результата
ble t1, t0, divs

# вывод результата
j resultofprogram

divs:
neg t1,t1
add t0,t0,t1
neg t1, t1
ble t1, t0, divs
j resultofprogram


resultofprogram:
la a0, result
li a7 4
ecall
mv a0, t1
mul a0, t0, t2
li a7, 1
ecall
j killprogram

calz: # case 'a' less zero
li t2, -1
neg t0,t0
j p1

cblz: #case 'b' less zero
li t3, -1
neg t1, t1
j p2

errorcase:
la a0, err
li a7, 4
ecall
j killprogram

killprogram:
la a0, endl
li a7, 4
ecall
li a7, 10
li a0, 0
ecall