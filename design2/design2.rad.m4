void light led_light
0
0
3	100	100	100

# Building material: Paper
# Reflectance: rho=0.902
# Hopefully pretty close to Dulux Lexicon Quarter (PN2D1)
void plastic white_paint
0
0
5	0.9	0.9	0.9
	0.02 0.08

void trans diffuser
0
0
7	0.98	0.98	0.98
	0.04	0.15
	0.47	0.01

led_light polygon led1
0
0
12	eval(245+600+b-50)	eval(a+d+g+150-5)	eval(h-5)
	eval(245+600+b-50)	eval(a+d+g+150-5)	eval(h+5)
	eval(245+600+b-50)	eval(a+d+g+150+5)	eval(h+5)
	eval(245+600+b-50)	eval(a+d+g+150+5)	eval(h-5)

white_paint polygon right_wall
0
0
12	0	0	-495
	eval(245+600+b)	0	-495
	eval(245+600+b)	eval(a+c+e+f)	-495
	0	eval(a+c+e+f)	-495

white_paint polygon left_wall
0
0
12	0	0	495
	eval(245+600+b)	0	495
	eval(245+600+b)	eval(a+c+e+f)	495
	0	eval(a+c+e+f)	495

white_paint polygon top_wall
0
0
12	0	eval(a+c+e+f)	-495
	0	eval(a+c+e+f)	495
	eval(245+600+b)	eval(a+c+e+f)	495
	eval(245+600+b)	eval(a+c+e+f)	-495

white_paint polygon bottom_wall
0
0
30	0	0	-495
	0	0	495
	eval(245+600+b)	0	495
	eval(245+600+b)	0	-495
	0	0	-495
	245	0	-325
	245	0	325
	eval(245+600)	0	325
	eval(245+600)	0	-325
	245	0	-325

white_paint polygon front_wall
0
0
12	0	0	-495
	0	0	495
	0	eval(a+c+e)	495
	0	eval(a+c+e)	-495

white_paint polygon rear_wall
0
0
12	eval(245+600+b)	0	-495
	eval(245+600+b)	0	495
	eval(245+600+b)	eval(a+c+e+f)	495
	eval(245+600+b)	eval(a+c+e+f)	-495

ifelse(j, `true', `
diffuser polygon bottom_diffuser
0
0
12	0	a	-495
	0	a	495
	eval(245+600+b)	a	495
	eval(245+600+b)	a	-495')

ifelse(k, `true', `
diffuser polygon top_diffuser
0
0
12	0	eval(a+c)	-495
	0	eval(a+c)	495
	eval(245+600+b)	eval(a+d)	495
	eval(245+600+b)	eval(a+d)	-495')

white_paint polygon top_bounce
0
0
12	0	eval(a+c+e)	-495
	0	eval(a+c+e)	495
	eval(245+600+b-i)	eval(a+c+e+f)	495
	eval(245+600+b-i)	eval(a+c+e+f)	-495
