/*
	Copyright 2017-2026, Philip Rose, GM3ZZA
	
    This file is part of ZZACOMMON.

    ZZACOMMON is free software: you can redistribute it and/or modify it under the
	terms of the Lesser GNU General Public License as published by the Free Software
	Foundation, either version 3 of the License, or (at your option) any later version.

    ZZACOMMON is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
	PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with ZZACOMMON. 
	If not, see <https://www.gnu.org/licenses/>. 

*/
#pragma once
#include <complex>
#include <vector>

using namespace std;

// Allow simple testing with double or long double

class zc_fft {

    const float PI = 3.14159265358979323846264338327950288419716939937510;
    
    public:
    //! Constructor
    //! \param size Number of FFT bins (must be power of 2)
    zc_fft(unsigned int size);

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