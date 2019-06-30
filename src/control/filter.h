#pragma once

#include "math.h"

#include "util/vector.h"

#define	HZ_10	0.004078
#define	HZ_20	0.015952
#define	HZ_30	0.035546
#define	HZ_40	0.062984
#define	HZ_50	0.097857
#define	HZ_60	0.139957
#define	HZ_70	0.190992
#define	HZ_80	0.249072
#define	HZ_90	0.308894
#define	HZ_100	0.397188
#define	HZ_120	0.542488
#define	HZ_140	0.719026
#define	HZ_160	0.928746
#define	HZ_180	1.144837
#define	HZ_200	1.354386
#define	HZ_220	1.611742
#define	HZ_240	1.87532
#define	HZ_260	2.123421
#define	HZ_280	2.377006
#define	HZ_300	2.595641
#define	HZ_320	2.864404
#define	HZ_340	3.052077
#define	HZ_360	3.272997
#define	HZ_380	3.44942
#define	HZ_400	3.679173
#define	HZ_420	3.721861
#define	HZ_440	3.880844
#define	HZ_460	3.908564
#define	HZ_480	3.984022
#define	HZ_500	4.100000

namespace control::filter {

  class kalman {
  public:
    kalman()
      : Q(0.02)
      , R(0.1)
    {
      R = Q / HZ_90;
    }

    vector step(vector in) {
      return vector{
        step(0, in[0]),
        step(1, in[1]),
        step(2, in[2]),
      };
    }

    float step(uint8_t axis, float in) {    
      //do a prediction 
      const float temp_est = last_estimation[axis]; 
      const float temp_p = last_prediction[axis] + Q; 

      const float K = temp_p * (1.0f / (temp_p + R));
      const float estimation = temp_est + K * (in - temp_est);
      const float P = (1 - K) * temp_p;
    
      //update our last's 
      last_prediction[axis] = P; 
      last_estimation[axis] = estimation; 

      return estimation;
    }

  private:
    vector last_estimation;
    vector last_prediction; 
    float Q, R;
  };


  class biquad_lowpass {
  public:
    static const constexpr float butterworth = 0.7071;

    biquad_lowpass(float cutoff, float sample_rate, float q)
      : biquad_lowpass(cutoff / sample_rate, q)
    {}

    biquad_lowpass(float fc, float q) 
      : FC(fc)
      , Q(q)
      , z1(0)
      , z2(0)
    {
      float K = tan(util::pi * FC);
      float norm = 1 / (1 + K / Q + K * K);

      a0 = K * K * norm;
      a1 = 2 * a0;
      a2 = a0;
      b1 = 2 * (K * K - 1) * norm;
      b2 = (1 - K / Q + K * K) * norm;
    }

    inline float step(float in) {
      float out = in * a0 + z1;
      z1 = in * a1 + z2 - b1 * out;
      z2 = in * a2 - b2 * out;
      return out;
    }

  private:
    float FC, Q;
    float a0, a1, a2, b1, b2;
    float z1, z2;
  };

  static float constrain_min_max(float val, float min, float max) {
    return (val < min) ? min : ((val > max) ? max : val);
  }

  static float expo(float val, float exp) {
    return constrain_min_max(val * val * val * exp + val * (1 - exp), -1, 1);
  }
  
}