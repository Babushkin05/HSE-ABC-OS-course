.text
	# returns distance between points in fa0, gets x0 in fa0, y0 in fa1, x1 in fa2, y1 in fa3
	distance:
		fsub.d ft0 fa0 fa2 # (x0-x1)
		fsub.d ft1 fa1 fa3 # (y0-y1)
		fmul.d ft0 ft0 ft0 # (x0-x1)^2
		fmul.d ft1 ft1 ft1 # (y0-y1)^2
		fadd.d ft2 ft0 ft1 # (x0-x1)^2 + (y0-y1)^2
		fsqrt.d fa0 ft2 # sqrt((x0-x1)^2 + (y0-y1)^2)
		#fmv.d fa0 ft2
		ret
	
	# returns arguments for a line function y = kx + c: k in fa0 and c in fa1. 
	# Takes x0 in fa0, y0 in fa1, x1 in fa2, y1 in fa3
	function_of_line:
		fsub.d ft0 fa2 fa0 # (x1-x0)
		fsub.d ft1 fa3 fa1 # (y1-y0)
		fdiv.d ft2 ft1 ft0 # k = (y1-y0)/(x1-x0)
		fmul.d ft3 ft2 fa0 # kx1
		fsub.d fa1 fa1 ft3 # y0 - kx1
		fmv.d fa0 ft2
		ret
	
	# returns arguments for a line function y = k'x + c': k' in fa0 and c' in fa1.
	# Takes k in fa0, c in fa1, x of intersection in fa2
	function_of_perpendicular_line:
		li t0 1
		fcvt.d.w ft0 t0
		fdiv.d ft0 ft0 fa0 # 1/k
		fmul.d ft1 fa2 ft0 # x/k
		fmadd.d ft2 fa2 fa0 fa1 # xk + c
		fadd.d ft2 ft2 ft1 # xk + x/k + c
		fmv.d fa0 ft0 
		fneg.d fa0 fa0 # -1/k
		fmv.d fa1 ft2
		ret
		
	
	# return point of intersections for functions y = k0x + c0 and y = k1x + c1: x in fa0 and y in fa1. 
	# Takes k0 in fa0, c0 in fa1, k1 in fa2, c1 in fa3
	linear_functions_intersection:
		fsub.d ft0 fa1 fa3 # (c0-c1)
		fsub.d ft1 fa2 fa0 # (k1-k0)
		fdiv.d ft2 ft0 ft1 # x = (c0-c1)/(k1-k0)
		fmadd.d ft3 ft2 fa0 fa1 # y = kx1 + c1
		fmv.d fa0 ft2
		fmv.d fa1 ft3
		ret
		
	# returns mean or two double numbers
	# rakes x0 in fa0 and x1 in fa1
	mean_double:
		fadd.d fa0 fa0 fa1
		li t0 2
		fcvt.d.w ft0 t0
		fdiv.d fa0 fa0 ft0
		ret

.macro two_points_to_a_registers(%x0,%y0,%x1,%y1)
	fmv.d fa0 %x0
	fmv.d fa1 %y0
	fmv.d fa2 %x1
	fmv.d fa3 %y1
.end_macro
	
.macro preparing_to_finding_perpendicular_line(%k, %c, %x)
	fmv.d fa0 %k
	fmv.d fa1 %c
	fmv.d fa2 %x
.end_macro

.macro preparing_for_finding_intersection(%k0, %c0, %k1 %c1)
	fmv.d fa0 %k0
	fmv.d fa1 %c0
	fmv.d fa2 %k1
	fmv.d fa3 %c1
.end_macro

.macro two_doubles_to_fa_registers(%x0, %x1)
	fmv.d fa0 %x0
	fmv.d fa1 %x1
.end_macro

