scalarToIndividual = function(v, params)
{
    newind = list(gt = list(channels = list(r = list(), g = list(), b = list())))
    j = 1
    for (i in 1:params$counts[["r"]])
    {
        newind$gt$channels$r[[i]] = list(x = v[j], y = v[j+1], i = 1)
        j = j + 2
    }
    for (i in 1:params$counts[["g"]])
    {
        newind$gt$channels$g[[i]] = list(x = v[j], y = v[j+1], i = 1)
        j = j + 2
    }
    for (i in 1:params$counts[["b"]])
    {
        newind$gt$channels$b[[i]] = list(x = v[j], y = v[j+1], i = 1)
        j = j + 2
    }
    newind$gt$height = v[j]
    newind$gt$reflective_walls = params$reflections
    newind$pt = estimateBrightness(newind$gt, params)
    newind
}


objectiveFunctionScalar = function(v, params)
{
    newind = scalarToIndividual(v, params)
    objectiveFunction(newind, params)
}


params.de = function(
    grid = list(
        x = seq(-35, 35, 5),
        y = seq(-35, 35, 5)),
    walls = list(
        xmin = -40, xmax = 40,
        ymin = -40, ymax = 40),
    counts = c(r = 8, g = 10, b = 10),
    height.min = 10,
    height.max = 100,
    spacing.min = 10,
    inhom.weights = c(r = 2, g = 9, b = 9),
    pop.size = 1000,
    pop.gens = 200,
    reflections = TRUE,
    reflection_brightness = 0.95,
    penalty.factor.inhomogeneity = 10,
    penalty.factor.inefficiency = 0,
    penalty.factor.crowding = 50
    )
{
    params = list(
        grid = grid,
        walls = walls,
        counts = counts,
        height.min = height.min,
        height.max = height.max,
        spacing.min = spacing.min,
        inhom.weights = inhom.weights,
        pop.size = pop.size,
        pop.gens = pop.gens,
        reflections = reflections,
        reflection_brightness = reflection_brightness,
        penalty.factor.inhomogeneity = penalty.factor.inhomogeneity,
        penalty.factor.inefficiency = penalty.factor.inefficiency,
        penalty.factor.crowding = penalty.factor.crowding
        )
    params
}


optim.de = function(params, ...)
{
    control = DEoptim.control(NP = params$pop.size, itermax = params$pop.gens, ...)
    DEoptim(
        fn = objectiveFunctionScalar, 
        lower = c(rep(c(params$walls$xmin, params$walls$ymin), sum(params$counts)), params$height.min),
        upper = c(rep(c(params$walls$xmax, params$walls$ymax), sum(params$counts)), params$height.max),
        params = params, control = control)
}

