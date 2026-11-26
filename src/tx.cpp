#include "tx.h"
#include <stdio.h>
#include <stdlib.h>

void gen_bits(uint8_t*bits,int n){
  srand(SEED);
  for(int i=0;i<n;i++){
    bits[i]=rand()%2;
  }
}

void save_bits(const char*name,const uint8_t*bits,int n){
  FILE*f=fopen(name,"w");
  for(int i=0;i<n;i++){
    fprintf(f,"%d",bits[i]);
  }
  fclose(f);
}

float bit2val(uint8_t bit){
  if(bit==1){
    return 1;
  }
  return -1;
}

void bpsk(const uint8_t*bits,cplx*sym){
  for(int k=0;k<NSYM;k++){
    sym[k].re=bit2val(bits[k]);
    sym[k].im=0;
  }
}

void qpsk(const uint8_t*bits,cplx*sym){
  for(int k=0;k<NSYM;k++){
    sym[k].re=bit2val(bits[2*k]);
    sym[k].im=bit2val(bits[2*k+1]);
  }
}

void upsample(const cplx*sym,cplx*up){
  for(int i=0;i<NUP;i++){
    up[i].re=0;
    up[i].im=0;
  }
  for(int k=0;k<NSYM;k++){
    up[k*SPS]=sym[k];
  }
}

void convolve(const cplx*x,const float*h,cplx*y){
  for(int i=0;i<NUP;i++){
    y[i].re=0;
    y[i].im=0;
    for(int j=0;j<SPS&&j<=i;j++){
      y[i].re+=x[i-j].re*h[j];
      y[i].im+=x[i-j].im*h[j];
    }
  }
}

void to_buf(const cplx*y,int16_t*buf){
  for(int i=0;i<2*BUF;i++){
    buf[i]=0;
  }
  //заполнение tx_buff значениями сэмплов первые 16 бит - I, вторые 16 бит - Q.
  for(int i=0;i<NUP;i++){
    buf[2*(HDR+i)]=(int16_t)(y[i].re*AMP);    // I
    buf[2*(HDR+i)+1]=(int16_t)(y[i].im*AMP);  // Q
  }
}

void make_signal(int use_qpsk,int16_t*buf){
  uint8_t bits[2*NSYM];
  cplx sym[NSYM];
  cplx up[NUP];
  cplx y[NUP];
  float h[SPS];
  if(use_qpsk==1){
    gen_bits(bits,2*NSYM);
    save_bits("bits_qpsk.txt",bits,2*NSYM);
  }else{
    gen_bits(bits,NSYM);
    save_bits("bits_bpsk.txt",bits,NSYM);
  }
  if(use_qpsk==1){
    qpsk(bits,sym);
  }else{
    bpsk(bits,sym);
  }
  upsample(sym,up);
  for(int i=0;i<SPS;i++){
    h[i]=1;
  }
  convolve(up,h,y);
  to_buf(y,buf);
}
