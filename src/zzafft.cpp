#include "zzafft.h"

#include <cmath>
#include <iostream>

// Constructor
zzafft::zzafft(unsigned int size) {
    size_ = size;
    log2_size_ = 0;
    unsigned int temp = size;
    while (temp > 1) {
        log2_size_++;
        temp >>= 1;
    }

    bitswap_lut_ = new unsigned int[size];
    // Initialise lookup table
    unsigned int j = 0;
    for (unsigned int i = 0; i < size; i++) {
        // cout << i << " " << j << endl;
        bitswap_lut_[i] = j;
        // "increment" j by adding bits into the top.
        bool carry = true;
        unsigned int add_bit = size >> 1;
        while (carry) {
            if ((j & add_bit) == 0) {
                j |= add_bit;
                carry = false;
            } else {
                j &= ~add_bit;
            }
            add_bit >>= 1;
        }
    }

    // Omega factors 
    omega_factor_.resize(log2_size_ + 1);
    for (unsigned int l = 0, p = 1; l <= log2_size_; l++, p<<=1) {
        float angle = 2 * PI / p;
        omega_factor_[l] = {cos(angle), sin(angle)};
        // cout << l << " " << p << " " << omega_factor_[l] << endl;
    } 

    data_.resize(size_, { 0.0F, 0.0F });

 }

// Calculate
void zzafft::calculate(std::vector<float>* data) {
    // Copy data from buffer
    for (size_t ix = 0; ix < data->size(); ix++) {
        data_[ix] = { (*data)[ix], 0.0F };
    }
    // Swap the data samples
    for (unsigned int i = 0; i < size_; i++) {
        unsigned int j = bitswap_lut_[i];
        if ( i < j ) {
            swap(data_[i], data_[j]);
        }
    }

    // Perform the FFT - start at the smallest unit - pairs, then quartets, octets etc.
    for (unsigned int len = 2, ix = 1; len <= size_; len <<= 1, ix++) {
        // Step along each pair/quartet/...
        for (unsigned int i = 0; i < size_; i += len) {
            std::complex<float> omega(1);
            for (unsigned int j = 0; j < len / 2; j++) {
                std::complex<float> u = data_[i+j]; 
                std::complex<float> v = data_[i+j+len/2] * omega;
                data_[i+j] = u + v;
                data_[i+j+len/2] = u - v;
                omega *= omega_factor_[ix];
            }
        }
    }

    // Copy data back to buffer
    for (size_t ix = 0; ix < data->size(); ix++) {
        (*data)[ix] = std::abs(data_[ix]);
    }
}