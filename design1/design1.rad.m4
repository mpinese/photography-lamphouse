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

# Aiming for sandblasted glass here
#                 5mm-3/16”    5mm-3/16”    10mm-3/8”    6mm-1/4”
#                 Matte        Satin        Satin        Low Iron Satin
# Transmission    41%          85%          84%          89%
# Reflectance     --            8%           7%           8%
#
# Glass generally has ~4% refl loss at each surface, and ~2% absorbance loss.
#
# From the above, colour should be 0.98 (=> 2% absorbance loss).
# Therefore transmittance should be 0.41/0.98 = 0.42
#
# >= 2 ambient bounces required -- probably a lot more for this application.
# 
# modifier trans identifier
# Ø
# Ø
# 7 R G B <- (Colour)
# spec rough <- (specularity & roughness)
# trans tspec <-(transmission & transmitted specularity)
# 
# http://www.schorsch.com/en/software/rayfront/manual/transdef.html:
# Specularity: The fraction of incident light that is immediately reflected in mirror like fashion.
# Roughness: The rms slope of surface facets.
# Transmissivity: The fraction of penetrating light that travels all the way through the material.
# Transmissive Specularity: The fraction of transmitted light that is not diffusely scattered.
#
# http://www.graphics.cornell.edu/pubs/2005/LT05b.pdf
# The slope of a rough surface is here taken as the ratio of the RMS roughness to the autocorrelation length, namely, σ/τ.
# These quantities are used as inputs for the He-Torrance and many other BRDF models.
# we append subscripts to these symbols: r for the raw height data (σr, τr,(σ/τ )r), and m (for measured) for the small-scale roughness data (σm, τm,(σ/τ )m).
# Material                            (σ/τ)b   Uniform diffuse term ab
# Aluminized Ground Glass, 120grit    0.166    0.44
# Aluminized Ground Glass, 240grit    0.158    0.44
# Aluminized Acid-etched Glass        0.023    0.00
# Al coating blocked light, so no subsurface scattering.
# ab term therefore proxy for diffuse reflection.
#
# http://www.iof.fraunhofer.de/content/dam/iof/de/documents/Publikationen/Fachbeitraege/Surface%20characterization%20techniques%20for%20determining%20the%20root-mean-square%20roughness%20and%20power%20spectral%20densities%20of%20optical%20components.pdf
# 
# Ok, so set up some eqns based on the above.
# Variables: 
#  colour a
#  spec s
#  rough r
#  trans t
#  trans spec u
#  a = 0.98
#  r = 0.15
#  s = 0.04 (front unblasted face)
#  u = 0.01 (essentially diffuse transmission)
#  (1-s)*a*(1-t) = 0.44
#  => t = 0.47 (diffuse reflection from sandblasted back face)
# 
# So:
#   Specular reflectance =   s               = 0.04
#   Absorbtion =             (1-s)*(1-a)     = 0.02
#   Diffuse reflectance =    (1-s)*a*(1-t)   = 0.50
#   Diffuse transmittance =  (1-s)*a*t*(1-u) = 0.44
#   Specular transmittance = (1-s)*a*t*u     = 0.00
#   TOTAL =                  1               = 1.00
# 
# Not *quite* right, as it discounts the double absorbtion loss 
# of a ray passing through the smooth side, getting reflected 
# on the inside rough side, then passing *back* through the smooth
# side... but close enough.

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
