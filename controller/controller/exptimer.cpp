#include "exptimer.h"
#include "fix24.h"


const fix24_t min_log2t = fix24_t(2.0);     // 4.0
const fix24_t max_log2t = fix24_t(5.0);     // 32.0
const fix24_t min_log2Phi = fix24_t(-4.0);  // 16/255
const fix24_t max_log2Phi = fix24_t(0.0);   // 255/255
const fix24_t min_2log2N = fix24_t(1.0);    // f/1.4
const fix24_t max_2log2N = fix24_t(5.0);    // f/32
const fix24_t min_2log2s = fix24_t(8.0);    // 16 cm
const fix24_t max_2log2s = fix24_t(14.0);   // 128 cm

// Fundamental equation:
// log2H + K = log2t - 2log2N + log2Phi - 2log2s


fix24_t Exposure::change_log2H(fix24_t delta)
{
  // Try to change log2Phi to effect the desired changed in log2H,
  // leaving t constant.
  // Only if log2Phi will go out of bounds, do we change t.
  // If both log2Phi and t would go out of bounds, change nothing.

  //    log2H + delta = log2t - 2log2N + log2Phi - 2log2s
  // => log2Phi + log2t = log2H + delta + 2log2N + 2log2s
  // So we need to distribute delta across log2Phi and log2t,
  // with priority to changing log2Phi.

  if (log2Phi + delta < min_log2Phi)
  {
    // Underflow.  Put what we can into log2Phi and carry the 
    // rest for t.
    delta -= (log2Phi - min_log2Phi);
    log2Phi = min_log2Phi;
  }
  else if (log2Phi + delta > max_log2Phi)
  {
    // Overflow.  Put what we can into log2Phi and carry the 
    // rest for t.
    delta -= (max_log2Phi - log2Phi);
    log2Phi = max_log2Phi;
  }
  else
  {
    // In bounds.
    log2Phi += delta;
    delta = 0.0;
  }

  if (log2t + delta < min_log2t)
  {
    // Underflow.  Put what we can into log2t, and return the rest.
    delta -= (log2t - min_log2t);
    log2t = min_log2t;
  }
  else if (log2t + delta > max_log2t)
  {
    // Overflow.  Put what we can into log2t, and return the rest.
    delta -= (max_log2t - log2t);
    log2t = max_log2t;
  }
  else
  {
    // In bounds.
    log2t += delta;
    delta = 0.0;
  }

  // Update log2H to match.
  log2H = log2t - log2N2 + log2Phi - log2s2;

  return delta;
}


fix24_t Exposure::change_t(fix24_t delta)
{
  // Try to change log2Phi to effect the desired change in t, 
  // leaving H constant.
  // If log2Phi will go out of bounds, don't change anything.

  if (log2Phi + delta < min_log2Phi)
  {
    // Underflow.  Put what we can into log2Phi, and return the rest.
    delta -= (log2Phi - min_log2Phi);
    log2Phi = min_log2Phi;
  }
  else if (log2Phi + delta > max_log2Phi)
  {
    // Overflow.  Put what we can into log2Phi, and return the rest.
    delta -= (max_log2Phi - log2Phi);
    log2Phi = max_log2Phi;
  }
  else
  {
    // In bounds.
    log2Phi += delta;
    delta = 0.0;
  }

  // Update log2t to match.
  log2t = log2H + log2N2 - log2Phi + log2s2;

  return delta;
}


fix24_t Exposure::set_N(fix24_t new_N)
{

}


fix24_t Exposure::set_s(fix24_t new_s)
{
  
}

