#pragma once

#include "fix24.h"


enum ETStatus {
  OK = 0
};



// log2H + K = log2t - 2log2N + log2Phi - 2log2s


class Exposure
{
private:
  fix24_t log2H, log2t, log2N2, log2Phi, log2s2;

public:
  fix24_t change_log2H(fix24_t delta);
  fix24_t change_t(fix24_t delta);
  fix24_t change_log2Phi(fix24_t delta);

  fix24_t set_N(fix24_t new_N);
  fix24_t set_s(fix24_t new_s);

  fix24_t get_H();
  fix24_t get_t();
  fix24_t get_Phi();
  fix24_t get_N();
  fix24_t get_s();
};


class ExposureTimer
{
//  Exposure green, blue;
};


