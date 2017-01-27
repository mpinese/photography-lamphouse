#pragma once

#include "Fp32f.hpp"

using namespace Fp;

typedef Fp32f<24> fix24_t;


const fix24_t FIX24_0 = fix24_t(int32_t(0));
const fix24_t FIX24_1 = fix24_t(int32_t(1));
const fix24_t FIX24_2 = fix24_t(int32_t(2));
const fix24_t FIX24_4 = fix24_t(int32_t(4));
const fix24_t FIX24_SQRT_EPS = fix24_t(0.0001);

fix24_t log2(fix24_t x);
fix24_t exp2(fix24_t x);
fix24_t sqrt(fix24_t x);
