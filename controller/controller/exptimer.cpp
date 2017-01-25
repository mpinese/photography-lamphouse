#include "exptimer.h"
#include "fix24.h"


const fix24_t min_t = fix24_t(4.0);
const fix24_t max_t = fix24_t(32.0);
const fix24_t min_log2Phi = fix24_t(-4.0);  // 16/255
const fix24_t max_log2Phi = fix24_t(0.0);   // 255/255
const fix24_t min_2log2N = fix24_t(1.0);    // f/1.4
const fix24_t max_2log2N = fix24_t(5.0);    // f/32
const fix24_t min_2log2s = fix24_t(8.0);    // 16 cm
const fix24_t max_2log2s = fix24_t(14.0);   // 128 cm

// log2H + K = log2t - 2log2N + log2Phi - 2log2s


fix24_t Exposure::change_log2H(fix24_t delta)
{
  
}


fix24_t Exposure::change_t(fix24_t delta)
{
  
}


fix24_t Exposure::change_log2Phi(fix24_t delta)
{
  if (log2Phi + delta < min_log2Phi)
    log2Phi = min_log2Phi;
  else if (log2Phi + delta > max_log2Phi)
    log2Phi = max_log2Phi;
    
}


fix24_t Exposure::set_N(fix24_t new_N)
{

}


fix24_t Exposure::set_s(fix24_t new_s)
{
  
}


fix24_t Exposure::get_H()
{
  
}


fix24_t Exposure::get_t()
{
  
}


fix24_t Exposure::get_Phi()
{
  
}


fix24_t Exposure::get_N()
{
  
}


fix24_t Exposure::get_s()
{
  
}

