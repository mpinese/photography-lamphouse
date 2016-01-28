import Cairo

immutable type Point
	x::AbstractFloat
	y::AbstractFloat
end
+(p1::Point, p2::Point) = Point(p1.x + p2.x, p1.y + p2.y)
-(p1::Point, p2::Point) = Point(p1.x - p2.x, p1.y - p2.y)
*(p::Point, k::AbstractFloat) = Point(p.x*k, p.y*k)
norm(p::Point) = sqrt(p.x^2 + p.y^2)
⋅(p1::Point, p2::point) = p1.x*p2.x + p1.y*p2.y

# p is the location of the intersection.  normal is the normal of the intersected surface (as a normalized vector).
immutable type Intersection
	p::Point
	normal::Point
end

# p = source + k*delta, k unconstrained
immutable type Line
	source::Point
	delta::Point
end

# p = source + k*delta, k >= 0
immutable type Ray
	source::Point
	delta::Point
end
function draw(cr::CairoContext, r::ray)
	Cairo.move_to(cr, r.source.x, r.source.y)
	# Choose a destination point that we are certain will at least span the entire Cairo surface
	ray_dest = r.source + r.delta / norm(r.delta) * sqrt(width(cr)^2 + height(cr)^2)
	Cairo.line_to(cr, ray_dest)
end

# p = source + k*delta, 0 <= k <= 1
immutable type Segment
	source::Point
	delta::Point
end
function draw(cr::CairoContext, e::circle)
	Cairo.move_to(cr, e.source.x, e.source.y)
	Cairo.line_to(cr, e.source.x + e.delta.x, e.source.y + e.delta.y)
end

# norm(p - centre) = radius
immutable type Circle
	centre::Point
	radius::AbstractFloat
end
function draw(cr::CairoContext, e::circle)
	Cairo.new_sub_path(cr)
	Cairo.arc(cr, e.centre.x, e.centre.y, e.radius, 0, 2*pi)
	Cairo.close_path(cr)
end

# index is the optical index of the current medium of the photon
type Photon
	ray::Ray
	history::Array{Segment, 1}
end
Photon(ray::Ray) = Photon(ray, [])
function draw(cr::CairoContext, p::Photon)
	for past_segment in p.history
		draw(cr, past_segment)
	end
	draw(cr, ray)
end

BoundaryGeometry = Union{Circle, Segment}

# Note current setup cannot handle cemented groups.
type OpticalElement
	boundary::BoundaryGeometry
	index::AbstractFloat
	absorbance::AbstractFloat
end
draw(cr::CairoContext, e::OpticalElement) = draw(cr, e.boundary)


type Source
	intensity::AbstractFloat
	boundary::Segment
end


type LambertianSource <: Source
end
function create_photon(LambertianSource::s)
	emission_position = source.boundary.source + rand()*source.boundary.delta

	# Rejection sampling of an angle theta, where Pr(theta) ~ cos(theta)
	do
		emission_angle = rand(-pi, pi)
	while rand() < cos(emission_angle)

	# Convert the emission angle theta to a delta vector
	theta_emitter = atan(source.boundary.delta.y / source.boundary.delta.x)
	theta_delta = theta_emitter + emission_angle
	delta = Point(cos(theta_delta), sin(theta_delta))

	return Photon(Ray(emission_position, delta))
end


type System
	elements::Array{OpticalElement, 1}
	photons::Array{Photon, 1}
	sources::Array{Source, 1}
end


# (l.source.x + a*l.delta.x - c.centre.x)^2 + (l.source.y + a*l.delta.y - c.centre.y)^2 = c.radius^2
function intersect(l::Line, c::Circle)
	if l.delta.x == 0 && l.delta.y == 0
		return []
	end

	p1 = l.source - c.centre
	p2 = p1 + l.delta

	dr = norm(l.delta)

	D = p1.x*p2.y - p2.x*p1.y
	discrim = (c.radius*dr)^2 - D^2

	if discrim < 0
		return	# No intersection
	end

	int_point_const = Point(D*l.delta.y / dr^2, -D*l.delta.x / dr^2) + c.centre
	int_point_delta = Point(sign(l.delta.y)*l.delta.x*sqrt(discrim) / dr^2, abs(l.delta.y)*sqrt(discrim) / dr^2)

	# Intersection points
	int_point_1 = int_point_const + int_point_delta
	int_point_2 = int_point_const - int_point_delta

	# Surface (circle) normals at intersection points
	int_normal_1 = int_point_1 - c.centre
	int_normal_2 = int_point_2 - c.centre
	int_normal_1 /= norm(int_normal_1)
	int_normal_2 /= norm(int_normal_2)

	return [Intersection(int_point_1, int_normal_1), Intersection(int_point_2, int_normal_2)]
end


# l1 is the test line, l2 the line to be intersected.
# l1.source.x + a*l1.delta.x = l2.source.x + b*l2.delta.x
# l1.source.y + a*l1.delta.y = l2.source.y + b*l2.delta.y
function intersect(l1::Line, l2::Line)
	if l1.delta.x == 0 && l1.delta.y == 0
		return []
	end

	D = l1.delta.y*l2.delta.x-l1.delta.x*l2.delta.y

	if D == 0
		return	# No intersection
	end

	source_delta = l1.source - l2.source

	a = (l2.delta.y*source_delta.x - l2.delta.x*source_delta.y) / D
	# b = (l1.delta.y*source_delta.x - l1.delta.x*source_delta.y) / D

	# Intersection point
	int_point = l1.source + a*l1.delta

	# Surface (line l2) normal
	int_normal = Point(-l2.delta.y, l2.delta.x)
	int_normal /= norm(int_normal)

	return [Intersection(int_point, int_normal)]
end


function intersect(r::Ray, c::Circle)
	line_int = intersect(Line(r.source, r.delta), c)

	# Keep intersection points i which satisfy
	# i = source + k*delta for k >= 0
	filter!(i->(i.p.x - r.source.x) / r.delta.x >= 0, line_int)

	return line_int
end


function intersect(r::Ray, l::Line)
	line_int = intersect(Line(r.source, r.delta), l)

	# Keep intersection points i which satisfy
	# i = source + k*delta for k >= 0
	filter!(i->(i.p.x - r.source.x) / r.delta.x >= 0, line_int)

	return line_int
end


function intersect(r::Ray, s::Segment)
	line_int = intersect(Line(r.source, r.delta), Line(s.source, s.delta))

	p0 = s.source
	p1 = s.source + s.delta
	filter!(i->((i.p.x - p0.x)*(i.p.x - p1.x) <= 0 && (i.p.y - p0.y)*(i.p.y - p1.y) <= 0), line_int)

	return line_int
end


function plotSystem(system::System)
	c = Cairo.CairoRGBSurface(256, 256)
	cr = Cairo.CairoContext(c)

	Cairo.save(cr)
	Cairo.set_source_rgb(cr, 1, 0, 0)
	for photon in system.photons:
		for photon_ray in photon_path:
			draw(cr, photon_ray)
		end
	end

	Cairo.set_source_rgb(cr, 0, 1, 0)
	for element in system.elements
		draw(cr, element)
	end
	Cairo.restore(cr)
end


# Find the first object in system that will be intersected by r.
# If there is no intersection, return (nothing, nothing)
function intersect(r::Ray, system::System)
	bestdist = Inf
	bestint = nothing
	bestelement = nothing

	for element in System.elements
		intersections = intersect(r, element)
		for intersection in intersections
			dist = norm(r.source - intersection.p)
			if dist < bestdist
				bestdist = dist
				bestint = intersection
				bestelement = element
			end
		end
	end

	return (bestint, bestelement)
end


# Simulate the refraction or absorbtion of a ray, 
# by Snell's law and the Fresnel eqns.
# Returns (photon, changed), where:
#   photon is the new photon direction, following interaction
#   changed is a boolean, true if the photon has interacted with an element, else false
function interact(photon::Photon, system::System)
	# If this is a 'stationary' photon (ie it's been absorbed),
	# then don't bother with all the calculations, and return
	# it unchanged.
	if photon.ray.delta.x == 0 && photon.ray.delta.y == 0:
		return (photon, false)

	# Find the first system element the photon will hit
	intposition, intelement = intersect(photon.ray, system)

	if intposition == nothing
		# Nothing was hit; the photon is unchanged
		return (photon, false)
	end

	# Test for absorbance
	if rand() < intelement.absorbance
		# The ray is absorbed, and terminates here.
		# Represent a terminated photon as a position and zero delta vector
		return (Photon(Ray(intposition, Point(0.0, 0.0)), photon.history + Segment(photon.ray.source, intposition.p)), true)
	end

	n1 = photon.n
	n2 = intelement.n

	l = photon.ray.delta / norm(photon.ray.delta)	# Normalised photon direction
	n = intposition.normal 							# Normalised intersecting surface normal

	c = -n⋅l 										# cos(theta1)

	if c > 0
		# The surface normal points toward the photon ray; the 
		# photon is striking the front of the intersected surface.
		# The refractive index transition is then air -> element.n
		n1 = 1.000293
		n2 = intelement.index
	if c < 0
		# The surface normal points away from the photon ray -- ie
		# the photon is striking the *back* of the intersected surface.
		# The refractive index transitions in then element.n -> air.
		n1 = photon.n
		n2 = 1.000293

		# Fix the normals for subsequent equations.
		c = -c
		n = -n
	end

	r = n1 / n2 					# Ratio of refractive indices
	s = sqrt(1 - c^2)				# sin(theta_2) = sqrt(1 - cos(theta_1)^2)

	# Reflection vector
	delta_reflect = l + 2*c*n

	# Check for total internal reflection
	if abs(r*s) > 1
		# TIRF has occurred.  Create a new entirely reflected
		# photon, and return.
		return (Photon(Ray(intposition.p, delta_reflect), photon.history + Segment(photon.ray.source, intposition.p)), true)
	end

	# Refraction vector
	delta_refract = r*l + (r*c - sqrt(1 - (r*s)^2))*n

	# Use Fresnel equations to predict reflection vs refraction
	reflectance_s = ((n1*c - n2*sqrt(1 - (r*s)^2)) / (n1*c + n2*sqrt(1 - (r*s)^2)))^2
	reflectance_p = ((n1*sqrt(1 - (r*s)^2) - n2*c) / (n1*sqrt(1 - (r*s)^2) + n2*c))^2
	reflectance_avg = (reflectance_s + reflectance_p) / 2
	transmittance_avg = 1 - reflectance_avg

	# Sample to determine the path taken
	if rand() < reflectance_avg
		# Reflection
		return (Photon(Ray(intposition.p, delta_reflect), photon.history + Segment(photon.ray.source, intposition.p)), true)
	else
		# Refraction
		return (Photon(Ray(intposition.p, delta_refract), photon.history + Segment(photon.ray.source, intposition.p)), true)
	end
end


function choose_source(system::System)
	if length(system.sources) == 0
		return nothing
	end

	total_intensity = 0.0
	for source in system.sources
		total_intensity += source.intensity
	end

	random_intensity = rand(0.0, total_intensity)
	for source in system.sources
		if random_intensity < source.intensity:
			break
		random_intensity -= source.intensity
	end

	return source
end


# Create a new photon
function create_photon(system::System)
	source = choose_source(system)
	photon = create_photon(source)
	return photon
end


# Create a new photon and trace it through system until
# it terminates.  When done, add the photon to system.
function create_and_trace_photon!(system::System)
	photon = create_photon(system)

	interacted = true
	while interacted == true
		photon, interacted = interact(photon, system)
	end

	system.photons.add(photon)
end


function create_and_trace_photon!(system::System, n::Integer)
	for i in 1:n
		create_and_trace_photon(system)
	end
end


# intersect(Ray(Point(-5,-5), Point(1,1)), Circle(Point(1,1), 3))
# intersect(Ray(Point(5,5), Point(1,1)), Circle(Point(1,1), 3))

