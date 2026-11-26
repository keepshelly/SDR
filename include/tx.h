#pragma once
#include <stdint.h>

const int BUF=1920;
const int HDR=6;
const int SPS=10;
const int NSYM=(BUF-HDR)/SPS;
const int NUP=NSYM*SPS;
const int AMP=1500<<4;
const int SEED=12345;

struct cplx{
  float re;
  float im;
};

void gen_bits(uint8_t*bits,int n);
void save_bits(const char*name,const uint8_t*bits,int n);
float bit2val(uint8_t bit);
void bpsk(const uint8_t*bits,cplx*sym);
void qpsk(const uint8_t*bits,cplx*sym);
void upsample(const cplx*sym,cplx*up);
void convolve(const cplx*x,const float*h,cplx*y);
void to_buf(const cplx*y,int16_t*buf);
void make_signal(int use_qpsk,int16_t*buf);
