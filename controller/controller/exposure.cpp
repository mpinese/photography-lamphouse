#include "exposure.hpp"
#include "fix24.hpp"


const fix24_t min_log2t = fix24_t(2.0);     // 4.0
const fix24_t max_log2t = fix24_t(5.0);     // 32.0
const fix24_t min_log2Phi = fix24_t(-5.0);  // 2048/65535
const fix24_t max_log2Phi = fix24_t(0.0);   // 65535/65535
const fix24_t min_2log2N = fix24_t(1.0);    // f/1.4
const fix24_t max_2log2N = fix24_t(5.0);    // f/32
const fix24_t min_2log2s = fix24_t(8.0);    // 16 cm
const fix24_t max_2log2s = fix24_t(14.0);   // 128 cm

// Fundamental equation:
// log2H + K = log2t - 2log2N + log2Phi - 2log2s


fix24_t modify_var_with_carry(fix24_t &var, fix24_t delta, fix24_t min_val, fix24_t max_val)
{
    // Modify the variable var in-place with carry.
    // Try to change var by delta (ie var' = var + delta),
    // but threshold var to the range [min_val, max_val].
    // Return the amount of delta (the carry) that was 
    // not added to var due to bounds being exceeded.
    if (var - min_val < delta)
    {
        // Underflow.  Put what we can into var and carry the rest.
        delta -= (var - min_val);
        var = min_val;
    }
    else if (var - max_val < delta)
    {
        // Overflow.  Put what we can into var and carry the rest.
        delta -= (var - max_val);
        var = max_val;
    }
    else
    {
        // In bounds
        var += delta;
        delta = FIX24_0;
    }

    return delta;
}


fix24_t Exposure::change_log2H(fix24_t delta)
{
    // Try to change log2Phi to effect the desired changed in log2H,
    // leaving t constant.
    // Only if log2Phi will go out of bounds, do we change t.
    // If both log2Phi and t would go out of bounds, change nothing.
    
    // All variables log2, dropped for brevity.  K = 0.
    //    H  = t - 2N + Phi - 2s
    // => H' = H + delta = t - 2N + Phi - 2s + delta
    //                   = (t + Phi + delta) - 2N - 2s
    //                   = t' + Phi' - 2N - 2s
    
    // Therefore, log2t' + log2Phi' = log2t + log2Phi + delta
    // So we need to distribute delta across log2Phi and log2t,
    // with priority to changing log2Phi.

    delta = modify_var_with_carry(log2Phi, delta, min_log2Phi, max_log2Phi);
    delta = modify_var_with_carry(log2t, delta, min_log2t, max_log2t);

    // Calculate the new log2H'
    log2H = log2t - log2N2 + log2Phi - log2s2;

    return delta;
}


fix24_t Exposure::change_log2t(fix24_t delta)
{
    // Try to change log2Phi to effect the desired change in t, 
    // leaving H constant.
    // If log2Phi will go out of bounds, don't change anything.
    
    //    t' = t + delta
    //    H  = t - 2N + Phi - 2s
    //       = t' - 2N + Phi' - 2s
    //       = t + delta - 2N + Phi' - 2s
    // => Phi' = H - t - delta + 2N + 2s
    //    Phi = H - t + 2N + 2s
    // => Phi' = Phi - delta
    
    delta = -modify_var_with_carry(log2Phi, -delta, min_log2Phi, max_log2Phi);
    
    // Calculate the new log2t'
    log2t = log2H + log2N2 - log2Phi + log2s2;
    
    return delta;
}


fix24_t Exposure::change_t(fix24_t delta)
{
    fix24_t target_time, log2_target_time, log2_target_delta, realised_time, realised_delta;
    
    target_time = exp2(log2t) + delta;
    log2_target_time = log2(target_time);
    log2_target_delta = log2_target_time - log2t;
    change_log2t(log2_target_delta);
    realised_time = exp2(log2t);
    realised_delta = target_time - realised_time;
    
    return zapsmall(realised_delta);
}


fix24_t Exposure::set_N(fix24_t new_N)
{
    // Set N to the given value, adjusting other parameters to
    // achieve a constant H.  Adjust log2Phi first,
    // overflowing to log2t if needed.  If a constant H is not
    // possible, make no change to any variables.
    
    // H =  t - 2N + Phi - 2s = t' - 2N' + Phi' - 2s
    //   => Phi'-Phi + t'-t = 2N' - 2N
    //   => delta_Phi + delta_t = delta_N2
    
    fix24_t new_log2N2, delta;
    
    new_log2N2 = log2(new_N);
    new_log2N2.rawVal <<= 1;
    
    delta = new_log2N2 - log2N2;
    
    // delta must be distributed between log2Phi and log2t,
    // with preference to log2Phi.  Do this on test variables,
    // because if we can't distribute delta between Phi and
    // t without exceeding the bounds of those variables, we
    // don't want to change anything at all (as this would
    // result in an automatic change in H, which is disallowed).
    fix24_t test_log2Phi, test_log2t;
    test_log2Phi = log2Phi;
    test_log2t = log2t;
    delta = modify_var_with_carry(test_log2Phi, delta, min_log2Phi, max_log2Phi);
    delta = modify_var_with_carry(test_log2t, delta, min_log2t, max_log2t);
    delta = zapsmall(delta);
    
    // Check if delta was completely consumed.  If so, great -- 
    // make the change to N.  Otherwise, this attempt to change
    // N failed, and so abort it.
    if (delta == FIX24_0)
    {
        log2Phi = test_log2Phi;
        log2t = test_log2t;
        log2N2 = new_log2N2;

        // Recalculate H for consistency (it should not have changed)
        log2H = log2t - log2N2 + log2Phi - log2s2;
    }
    
    return delta;
}


fix24_t Exposure::set_s(fix24_t new_s)
{
    // Set s to the given value, adjusting other parameters to
    // achieve a constant H.  Adjust log2Phi first,
    // overflowing to log2t if needed.  If a constant H is not
    // possible, make no change to any variables.
    
    // H =  t - 2N + Phi - 2s = t' - 2N + Phi' - 2s'
    //   => Phi'-Phi + t'-t = 2s' - 2s
    //   => delta_Phi + delta_t = delta_s
    
    fix24_t new_log2s2, delta;
    
    new_log2s2 = log2(new_s);
    new_log2s2.rawVal <<= 1;
    
    delta = new_log2s2 - log2s2;
    
    // delta must be distributed between log2Phi and log2t,
    // with preference to log2Phi.  Do this on test variables,
    // because if we can't distribute delta between Phi and
    // t without exceeding the bounds of those variables, we
    // don't want to change anything at all (as this would
    // result in an automatic change in H, which is disallowed).
    fix24_t test_log2Phi, test_log2t;
    test_log2Phi = log2Phi;
    test_log2t = log2t;
    delta = modify_var_with_carry(test_log2Phi, delta, min_log2Phi, max_log2Phi);
    delta = modify_var_with_carry(test_log2t, delta, min_log2t, max_log2t);
    delta = zapsmall(delta);
    
    // Check if delta was completely consumed.  If so, great -- 
    // make the change to N.  Otherwise, this attempt to change
    // N failed, and so abort it.
    if (delta == FIX24_0)
    {
        log2Phi = test_log2Phi;
        log2t = test_log2t;
        log2s2 = new_log2s2;

        // Recalculate H for consistency (it should not have changed)
        log2H = log2t - log2N2 + log2Phi - log2s2;
    }
    
    return delta;
}

