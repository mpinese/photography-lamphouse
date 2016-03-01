compileDesign = function(design, path)
{
    design_string = paste(
        "void brightfunc led_dist\n",
        "2 brightness led_dist.cal\n",
        "0\n",
        "0\n",
        "\n",
        "led_dist light led_red\n",
        "0\n",
        "0\n",
        "3  1e6   0   0\n",
        "\n",
        "led_dist light led_green\n",
        "0\n",
        "0\n",
        "3  0   1e6   0\n",
        "\n",
        "led_dist light led_blue\n",
        "0\n",
        "0\n",
        "3  0   0   1e6\n",
        "\n",
        "void plastic white_paint\n",
        "0\n",
        "0\n",
        "5  0.9 0.9 0.9\n",
        "   0.02    0.08\n",
        "\n",
        "void metal al_foil\n",
        "0\n",
        "0\n",
        "5  0.92    0.92    0.92\n",
        "   0.12    0.1\n",
        "\n",
        "white_paint polygon top_wall\n",
        "0\n",
        "0\n",
        "12 0   0   H\n",
        "   0   W   H\n",
        "   W   W   H\n",
        "   W   0   H\n",
        "\n",
        "white_paint polygon bottom_wall\n",
        "0\n",
        "0\n",
        "30 0   0   0\n",
        "   W   0   0\n",
        "   W   W   0\n",
        "   0   W   0\n",
        "   0   0   0\n",
        "   D   D   0\n",
        "   D   E   0\n",
        "   E   E   0\n",
        "   E   D   0\n",
        "   D   D   0\n",
        "\n",
        "white_paint polygon left_wall\n",
        "0\n",
        "0\n",
        "12 0   0   0\n",
        "   0   W   0\n",
        "   0   W   H\n",
        "   0   0   H\n",
        "\n",
        "white_paint polygon right_wall\n",
        "0\n",
        "0\n",
        "12 W   0   0\n",
        "   W   0   H\n",
        "   W   W   H\n",
        "   W   W   0\n",
        "\n",
        "white_paint polygon front_wall\n",
        "0\n",
        "0\n",
        "12 0   0   0\n",
        "   0   0   H\n",
        "   W   0   H\n",
        "   W   0   0\n",
        "\n",
        "white_paint polygon back_wall\n",
        "0\n",
        "0\n",
        "12 0   W   0\n",
        "   W   W   0\n",
        "   W   W   H\n",
        "   0   W   H\n",
        "\n", sep = ""
    )
    
    chip_id = 1
    for (led in design$leds)
    {
        led_string = paste(
            "white_paint cylinder ledN_mount\n",
            "0\n",
            "0\n",
            "7  X   Y   H\n",
            "   X   Y   I\n",
            "   4\n",
            "\n",

            "led_C polygon ledN_chip\n",
            "0\n",
            "0\n",
            "12  X1  Y1  J\n",
            "    X1  Y2  J\n",
            "    X2  Y2  J\n",
            "    X2  Y1  J\n",
            "\n", sep = "")

        led_string = gsub("X1", led$x-1, led_string)
        led_string = gsub("X2", led$x+1, led_string)
        led_string = gsub("Y1", led$x-1, led_string)
        led_string = gsub("Y2", led$x+1, led_string)
        led_string = gsub("X", led$x, led_string)
        led_string = gsub("Y", led$y, led_string)
        led_string = gsub("N", chip_id, led_string)
        led_string = gsub("C", led$colour, led_string)

        design_string = paste(design_string, led_string, collapse = "", sep = "")

        chip_id = chip_id + 1
    }

    design_string = gsub("H", design$height, design_string)
    design_string = gsub("I", design$height - 3, design_string)
    design_string = gsub("J", design$height - 4, design_string)
    design_string = gsub("W", design$width, design_string)
    design_string = gsub("D", (design$width - 70)/2, design_string)
    design_string = gsub("E", (design$width - 70)/2 + 70, design_string)

    design_string
}


evaluateDesign = function(design, density = 8)
{
    file_stem = tempfile()
    rad_file = paste(file_stem, ".rad", sep = "")
    oct_file = paste(file_stem, ".oct", sep = "")
    grid_file = paste(file_stem, ".grid", sep = "")
    value_file = paste(file_stem, ".value", sep = "")
    global_file = paste(file_stem, ".gpm", sep = "")

    design_string = compileDesign(design)

    cat(design_string, file = rad_file)

    system(sprintf("oconv %s > %s", rad_file, oct_file))

    system(sprintf("mkpmap -app %s 1000 10 -bv+ -dp 50 %s", global_file, oct_file))

    grid_points = seq((design$width - 70)/2, (design$width - 70)/2 + 70, length.out = density + 1)
    grid_points = (grid_points[-length(grid_points)] + grid_points[-1]) / 2

    grid = expand.grid(x = grid_points, y = grid_points)
    cat(sprintf("%f %f -1 0 0 1\n", grid$x, grid$y), file = grid_file, append = FALSE)

    system(sprintf("rtrace -app %s -I -h %s < %s > %s", global_file, oct_file, grid_file, value_file))

    values = read.table(value_file)
    colnames(values) = c("R", "G", "B")

    result = cbind(grid, values)
    result
}


design = list(
    height = 500,
    width = 100,
    leds = list(
        list(x = 50, y = 50, colour = "red"),
        list(x = 70, y = 70, colour = "green"),
        list(x = 2, y = 2, colour = "blue")))


test = evaluateDesign(design, density = 20)

image(matrix(test$R, nrow = sqrt(nrow(test)), byrow = TRUE), col = grey(seq(0, 1, 0.01)))
#image(matrix(test$G, nrow = sqrt(nrow(test)), byrow = TRUE), col = grey(seq(0, 1, 0.01)))
#image(matrix(test$B, nrow = sqrt(nrow(test)), byrow = TRUE), col = grey(seq(0, 1, 0.01)))
