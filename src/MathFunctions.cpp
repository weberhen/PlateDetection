#include "MathFunctions.hpp"

double Deviation(vector<int> v, double ave)
{
    double E=0;
    // Quick Question - Can vector::size() return 0?
    double inverse = 1.0 / static_cast<double>(v.size());
    for(unsigned int i=0;i<v.size();i++)
    {
        E += pow(static_cast<double>(v[i]) - ave, 2);
    }
    return sqrt(inverse * E);
}

//input: angle in grades
//output: same angle in radians
float radians(float grades)
{
  float PI = 3.14159265;
  return grades * PI / 180.0;
}

//AVERAGE
double Average(vector<int> v)
{      int sum=0;
       for(unsigned int i=0;i<v.size();i++)
               sum+=v[i];
       return sum/v.size();
}
