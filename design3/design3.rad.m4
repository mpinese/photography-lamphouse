# m4 -I /usr/share/doc/m4/examples -Da=2000 -Db=500 -Dc=500 -Dd=10 -De=200 design3.rad.m4 | oconv - | /usr/lib/radiance/rview -av .01 .01 .01 -vp 525 -5000 525 -vd 0 1 0 /dev/stdin

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

# Building material: Aluminium Foil
# Reflectance: rho=0.92
void metal al_foil
0
0
5  0.92 0.92 0.92  0.12 0.1

# Sandblasted glass
void trans diffuser
0
0
7	0.98	0.98	0.98
	0.04	0.15
	0.47	0.01

void dielectric PMMA
0
0
5	0.92	0.92	0.92
	1.490	0

al_foil polygon top_wall
0
0
12	0	a	0
	0	a	1050
	1050	a	1050
	1050	a	0

al_foil polygon bottom_wall
0
0
30	0	0	0
	1050	0	0
	1050	0	1050
	0	0	1050
	0	0	0
	200	0	200
	850	0	200
	850	0	850
	200	0	850
	200	0	200

al_foil polygon left_wall
0
0
12	0	0	0
	0	0	1050
	0	a	1050
	0	a	0

al_foil polygon right_wall
0
0
12	1050	0	0
	1050	a	0
	1050	a	1050
	1050	0	1050

al_foil polygon front_wall
0
0
12	0	0	0
	0	a	0
	1050	a	0
	1050	0	0

al_foil polygon back_wall
0
0
12	0	0	1050
	1050	0	1050
	1050	a	1050
	0	a	1050

led_light polygon led1
0
0
12	eval(e+50)	eval(a-1)	e
	eval(e+50)	eval(a-1)	eval(e+50)
	e	eval(a-1)	eval(e+50)
	e	eval(a-1)	e
	

include(`forloop2.m4')

forloop(`i', `1', d,`
PMMA cylinder `c1_'i
0
0
7	eval((2*i-1)*1050/d/2)	eval(a-b)	0
	eval((2*i-1)*1050/d/2)	eval(a-b)	1050
	eval(1050/d)
')

forloop(`i', `1', d,`
PMMA cylinder `c2_'i
0
0
7	0	eval(a-b-c)	eval((2*i-1)*1050/d/2)
	1050	eval(a-b-c)	eval((2*i-1)*1050/d/2)
	eval(1050/d)
')

