#pragma once

#include "fix24.hpp"

//using namespace Fp;

// log2H + K = log2t - 2log2N + log2Phi - 2log2s


class Exposure
{
private:
    fix24_t log2H, log2t, log2N2, log2Phi, log2s2;

public:
    fix24_t change_log2H(fix24_t delta);
    fix24_t change_log2t(fix24_t delta);
    fix24_t change_t(fix24_t delta);

    fix24_t set_N(fix24_t new_N);
    fix24_t set_s(fix24_t new_s);

    fix24_t get_log2H()     { return log2H; };
    fix24_t get_log2t()     { return log2t; };
    fix24_t get_log2Phi()   { return log2Phi; };
    fix24_t get_log2N()     { return log2N2 / fix24_t(2.0); };
    fix24_t get_log2s()     { return log2s2 / fix24_t(2.0); };
};


struct ExposureSettings
{
private:
    Exposure green, blue;
};


