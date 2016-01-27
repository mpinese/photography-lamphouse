type Point
	x::Float64
	y::Float64
end
+(p1::Point, p2::Point) = Point(p1.x + p2.x, p1.y + p2.y)
-(p1::Point, p2::Point) = Point(p1.x - p2.x, p1.y - p2.y)
*(p::Point, k::Float64) = Point(p.x*k, p.y*k)
norm(p::Point) = sqrt(p.x^2 + p.y^2)

type Intersection
	p::Point
	theta::Float64
end

type Ray
	source::Point
	delta::Point
end

type Line
	source::Point
	delta::Point
end

type Circle
	centre::Point
	radius::Float64
end

type Photon
	ray::Ray
	index::Float64
end

type CircularLens
	boundary::Circle
	index::Float64
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


intersect(Ray(Point(-5,-5), Point(1,1)), Circle(Point(1,1), 3))
intersect(Ray(Point(5,5), Point(1,1)), Circle(Point(1,1), 3))

function refract(p::Photon, )