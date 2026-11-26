#pragma once
#include <SoapySDR/Device.h>  // Инициализация устройства
#include <stdint.h>

const double FREQ1=810e6;
const double FREQ2=820e6;
const double SAMPLE_RATE=2e6;
const double GAIN=45.0;
const long TIMEOUT_US=400000;
const long long TX_DELAY_NS=4000000;

struct Sdr{
  SoapySDRDevice*dev;
  SoapySDRStream*rx;
  SoapySDRStream*tx;
  long long last_time;
  long long count;
};

void sdr_open(Sdr*s,const char*uri,double rx_freq,double tx_freq);
int sdr_step(Sdr*s,int16_t*rx_buffer,int16_t*tx_buffer);
void sdr_close(Sdr*s);
