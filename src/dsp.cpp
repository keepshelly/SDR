#include "dsp.h"
#include "sdr.h"
#include <math.h>

void to_complex(const int16_t*buf,int n,cf*x){
  for(int i=0;i<n;i++){
    x[i]=cf(buf[2*i],buf[2*i+1]);
  }
}

float threshold(const cf*x,int n){
  float mx=0;
  for(int i=0;i<n;i++){
    if(std::abs(x[i])>mx){
      mx=std::abs(x[i]);
    }
  }
  return mx/2;
}

float snr_db(const cf*x,int n,float thr){
  float ps=0,pn=0;
  int ns=0,nn=0;
  for(int i=0;i<n;i++){
    float p=std::norm(x[i]);
    if(std::abs(x[i])>thr){
      ps+=p;
      ns++;
    }else{
      pn+=p;
      nn++;
    }
  }
  if(ns==0||nn==0||pn==0){
    return 0;
  }
  return 10*log10f((ps/ns)/(pn/nn));
}

static cf power_m(cf v,int m){
  cf p=v*v;
  if(m==4){
    p=p*p;
  }
  return p/std::abs(p);
}

float fix_offset(cf*x,int n,float thr,int m){
  cf sum=0;
  for(int i=0;i+1<n;i++){
    if(std::abs(x[i])>thr&&std::abs(x[i+1])>thr){
      sum+=power_m(x[i+1],m)*std::conj(power_m(x[i],m));
    }
  }
  float w=std::arg(sum)/m;
  for(int i=0;i<n;i++){
    x[i]*=std::polar(1.0f,-w*i);
  }
  cf sp=0;
  for(int i=0;i<n;i++){
    if(std::abs(x[i])>thr){
      sp+=power_m(x[i],m);
    }
  }
  float ph=std::arg(sp)/m;
  if(m==4){
    ph=(std::arg(sp)-(float)M_PI)/4;
  }
  for(int i=0;i<n;i++){
    x[i]*=std::polar(1.0f,-ph);
  }
  return w*SAMPLE_RATE/(2*M_PI);
}
