#include <stdint.h>
#include <stddef.h>
#include "fix24.hpp"

using namespace Fp;


const uint8_t LOG2_TBL_DEGREE = 6;
const uint32_t FIX24_LOG2_TBL[] = {
    0, 
      375270,   744810,  1108793,  1467383,  
     1820738,  2169009,  2512340,  2850868,  
     3184728,  3514044,  3838941,  4159533,  
     4475935,  4788255,  5096595,  5401057,  
     5701737,  5998727,  6292118,  6581994, 
     6868440,  7151536,  7431359,  7707984,  
     7981483,  8251926,  8519380,  8783912,  
     9045584,  9304457,  9560591,  9814042, 
    10064867, 10313120, 10558852, 10802114, 
    11042956, 11281425, 11517568, 11751428, 
    11983051, 12212479, 12439752, 12664911, 
    12887994, 13109041, 13328087, 13545168, 
    13760320, 13973576, 14184969, 14394532, 
    14602297, 14808293, 15012551, 15215099, 
    15415967, 15615181, 15812769, 16008758, 
    16203172, 16396036, 16587377, 16777216
};

const uint8_t EXP2_TBL_DEGREE = 6;
const uint32_t FIX24_EXP2_TBL[] = {
    33554432,
    33919816, 34289178, 34662563, 35040014, 
    35421574, 35807290, 36197206, 36591368, 
    36989821, 37392614, 37799793, 38211406, 
    38627501, 39048127, 39473333, 39903169, 
    40337686, 40776935, 41220967, 41669834, 
    42123588, 42582284, 43045975, 43514715, 
    43988559, 44467563, 44951783, 45441276, 
    45936099, 46436310, 46941968, 47453133, 
    47969864, 48492221, 49020267, 49554062, 
    50093671, 50639155, 51190579, 51748008, 
    52311507, 52881142, 53456980, 54039088, 
    54627535, 55222390, 55823723, 56431603, 
    57046103, 57667294, 58295250, 58930044, 
    59571750, 60220444, 60876201, 61539100, 
    62209216, 62886630, 63571421, 64263668, 
    64963454, 65670859, 66385968, 67108864
};


fix24_t interpolate_table(fix24_t x, const uint32_t *tbl, uint8_t tbl_degree)
{
    /*
    Look up f(x) in tbl, using linear interpolation if x
    falls between entries of tbl.  tbl entries are
    assumed to supply f(x) for evenly-spaced x in [1, 2].

    Because we've shifted x to lie in [1, 2) (in +7.24
    representation), we know that bit 24 is 1, and
    25-31 are 0.  The highest degree bits of x 
    (not including bit 24) give us the lowest index
    into the lookup table, i0.  The remainder of x
    gives us the interpolation fraction between tbl[i0]
    and tbl[i0+1].
    */
    fix24_t xd, rem, y, y0, y1;
    uint32_t i0;

    xd = x - FIX24_1;
    i0 = xd.rawVal >> (24 - tbl_degree);
    rem.rawVal = (xd.rawVal - (i0 << (24 - tbl_degree))) << tbl_degree;

    y0.rawVal = tbl[i0];
    y1.rawVal = tbl[i0 + 1];

    y = y0 + (y1 - y0)*rem;

    return y;
}


fix24_t log2(fix24_t x)
{
    fix24_t n, y;

    // Using the relation log2(x) = n + log2(x/2^n),
    // scale x to lie in [1, 2)
    n = FIX24_0;
    for (; x >= FIX24_2; x.rawVal >>= 1)
        n += FIX24_1;
    for (; x < FIX24_1; x.rawVal <<= 1)
        n -= FIX24_1;

    y = interpolate_table(x, FIX24_LOG2_TBL, LOG2_TBL_DEGREE);

    return n + y;
}


fix24_t exp2(fix24_t x)
{
    // Use the relationship 2^x = (2^(x/2))*(2^(x/2))
    // to calculate 2^x using a table.

    if (x == FIX24_0)
        return FIX24_1;

    int8_t n;
    fix24_t y;

    // Determine the number of squaring steps that will 
    // be required at the end
    n = 0;
    for (; x >= FIX24_2; x.rawVal >>= 1)
        n++;
    for (; x < FIX24_1; x.rawVal <<= 1)
        n--;

    // Estimate y = 2^x using the table
    y = interpolate_table(x, FIX24_EXP2_TBL, EXP2_TBL_DEGREE);

    // Perform the final squaring / square root steps
    for (; n > 0; n--)
        y = y * y;
    for (; n < 0; n++)
        y = sqrt(y);

    return y;
}


fix24_t sqrt(fix24_t x)
{
    if (x == FIX24_0)
        return FIX24_0;

    // Find y = sqrt(x) by Newton's method.
    int8_t n;
    fix24_t y;
    fix24_t error;

    // Scale x to lie in [1, 4), by successive
    // multiplications or divisions by 4.
    n = 0;
    for (; x < FIX24_1; x.rawVal <<= 2)
        n--;
    for (; x >= FIX24_4; x.rawVal >>= 2)
        n++;

    // Solve y^2 - x = 0 for y, knowing that
    // x is in [1, 4), and therefore that 
    // y is in [1, 2).  Start with a guess of
    // y = 1, unrolling the first iteration
    // as many of the values are constant.
    // Implicit: y = FIX24_1;
    error = FIX24_1 - x;
    y = FIX24_1 - error / FIX24_2;
    error = y*y - x;

    // Perform the Raphson updates
    while (fabs(error) > FIX24_SQRT_EPS)
    {
        y -= error / (FIX24_2*y);
        error = y*y - x;
    }

    // Undo the initial shifts by multiplying
    // by 2^n.
    if (n > 0)
        y.rawVal <<= n;
    else if (n < 0)
        y.rawVal >>= (-n);

    return y;
}
