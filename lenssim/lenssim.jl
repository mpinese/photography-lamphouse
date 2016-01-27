import Cairo

immutable type Point
	x::AbstractFloat
	y::AbstractFloat
end
+(p1::Point, p2::Point) = Point(p1.x + p2.x, p1.y + p2.y)
-(p1::Point, p2::Point) = Point(p1.x - p2.x, p1.y - p2.y)
*(p::Point, k::AbstractFloat) = Point(p.x*k, p.y*k)
norm(p::Point) = sqrt(p.x^2 + p.y^2)

# p is the location of the intersection.  theta is the angle the intersecting ray makes with the intersected surface normal.
type Intersection
	p::Point
	theta::AbstractFloat
end

# p = source + k*delta, k unconstrained
type Line
	source::Point
	delta::Point
end

# p = source + k*delta, k >= 0
type Ray
	source::Point
	delta::Point
end

# p = source + k*delta, 0 <= k <= 1
type Segment
	source::Point
	delta::Point
end
function draw(cr::CairoContext, e::circle)
	Cairo.move_to(cr, e.source.x, e.source.y)
	Cairo.line_to(cr, e.source.x + e.delta.x, e.source.y + e.delta.y)
end

# norm(p - centre) = radius
type Circle
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
	index::AbstractFloat
end

BoundaryGeometry = Union{Circle, Segment}

# Note current setup cannot handle cemented groups.
type OpticalElement
	boundary::BoundaryGeometry
	index::AbstractFloat
end
draw(cr::CairoContext, e::OpticalElement) = draw(cr, e.boundary)


type System
	elements::Array{OpticalElement, 1}
	photons::Array{Array{Photon, 1}, 1}
end


function intersect(l::Line, c::Circle)
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

	int_point_1 = int_point_const + int_point_delta
	int_point_2 = int_point_const - int_point_delta

	int_angle_1 = atan((int_point_1.y - c.centre.y) / (int_point_1.x - c.centre.x)) + atan((int_point_1.y - l.source.y) / (int_point_1.x - l.source.x))
	int_angle_2 = atan((int_point_2.y - c.centre.y) / (int_point_2.x - c.centre.x)) + atan((int_point_2.y - l.source.y) / (int_point_2.x - l.source.x))

	return [Intersection(int_point_1, int_angle_1), Intersection(int_point_2, int_angle_2)]
end


function intersect(l1::Line, l2::Line)
	# l1.source.x + a*l1.delta.x = l2.source.x + b*l2.delta.x
	# l1.source.y + a*l1.delta.y = l2.source.y + b*l2.delta.y

	D = l1.delta.y*l2.delta.x-l1.delta.x*l2.delta.y

	if D == 0
		return	# No intersection
	end

	source_delta = l1.source - l2.source

	a = (l2.delta.y*source_delta.x - l2.delta.x*source_delta.y) / D
	# b = (l1.delta.y*source_delta.x - l1.delta.x*source_delta.y) / D

	int_point = l1.source + a*l1.delta

	#int_angle = atan() - atan() TODO
	int_angle = NaN

	return [Intersection(int_point, int_angle)]
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


intersect(Ray(Point(-5,-5), Point(1,1)), Circle(Point(1,1), 3))
intersect(Ray(Point(5,5), Point(1,1)), Circle(Point(1,1), 3))

function plotSystem(system::System)
	c = Cairo.CairoRGBSurface(256, 256)
	cr = Cairo.CairoContext(c)

	Cairo.save(cr)
	Cairo.set_source_rgb(cr, 0, 1, 0)
	for element in system.elements
		draw(cr, element)
	end
	Cairo.restore(cr)
end

