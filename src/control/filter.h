#pragma once

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

  static float constrain_min_max(float val, float min, float max) {
    return (val < min) ? min : ((val > max) ? max : val);
  }
  
}