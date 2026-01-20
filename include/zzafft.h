#pragma once
#include <complex>
#include <vector>

using namespace std;

// Allow simple testing with double or long double

class zzafft {

    const float PI = 3.14159265358979323846264338327950288419716939937510;
    
    public:
    //! Constructor
    //! \param size Number of FFT bins (must be power of 2)
    zzafft(unsigned int size);

    //! Calculate
    //! \param data Contains data to be transformed - returned in place.
    void calculate(std::vector<float>* data);

    //! Complex version for transform
    std::vector<std::complex<float> > data_;
    //! bitswap lookup table
    unsigned int* bitswap_lut_;
    //! Size
    unsigned int size_;
    //! Logarithm base 2 of size_
    unsigned int log2_size_;
    //!  \316\251 (Omega) factors -- 
    std::vector<std::complex<float> > omega_factor_;
 
};