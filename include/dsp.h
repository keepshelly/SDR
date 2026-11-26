#pragma once
#include <complex>
#include <stdint.h>

typedef std::complex<float> cf;

void to_complex(const int16_t*buf,int n,cf*x);
float threshold(const cf*x,int n);
float snr_db(const cf*x,int n,float thr);
float fix_offset(cf*x,int n,float thr,int m);
