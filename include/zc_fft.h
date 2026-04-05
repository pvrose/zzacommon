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
#include <cstdint>
#include <vector>

// Include FFTW3.
#include <fftw3.h>

using namespace std;

class zc_fft {

public:

	//! FFT direction
	enum direction_t : uint8_t { FORWARD, BACKWARD };

    //! Constructor
	//! \param direction Direction of the FFT (FORWARD or BACKWARD)
	//! \param size Size of the FFT (number of points)
	//! \param in_buff Input buffer (array of complex numbers)
	//! \param out_buff Output buffer (array of complex numbers)
	zc_fft( direction_t direction, 
		unsigned int size, 
		std::complex<double>*& in_buff, 
		std::complex<double>*& out_buff
	);

	//! Run the FFT
	void execute();

	//! Destructor
	~zc_fft();

private:
	fftw_plan plan_; //!< FFTW plan
	// Buffers
	fftw_complex* in_buff_; //!< Input buffer (array of complex numbers)
	fftw_complex* out_buff_; //!< Output buffer (array of complex numbers)

};