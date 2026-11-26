#include "sdr.h"
#include "tx.h"
#include <SoapySDR/Formats.h>  // Типы данных, используемых для записи сэмплов
#include <stdio.h>

static void put_header(int16_t*buf,long long tx_time){
  //prepare fixed bytes in transmit buffer
  //that is a flag (FFFF FFFF) followed by the 64 bit timestamp, split into 8 bytes and packed into the lsb of each of the DAC words.
  //DAC samples are left aligned 12-bits, so each byte is left shifted into place
  buf[0]=(int16_t)0xffff;
  buf[1]=(int16_t)0xffff;
  // 8 x timestamp words
  // Переменная для времени отправки сэмплов относительно текущего приема
  // Добавляем время, когда нужно передать блок tx_buff, через tx_time -наносекунд
  for(int i=0;i<8;i++){
    uint8_t b=(tx_time>>(i*8))&0xff;
    buf[2+i]=b<<4;
  }
  buf[10]=(int16_t)0xffff;
  buf[11]=(int16_t)0xffff;
}

void sdr_open(Sdr*s,const char*uri,double rx_freq,double tx_freq){
  char every[16];
  snprintf(every,sizeof(every),"%d",BUF);
  SoapySDRKwargs args={};
  SoapySDRKwargs_set(&args,"driver","plutosdr");
  SoapySDRKwargs_set(&args,"uri",uri);
  SoapySDRKwargs_set(&args,"direct","1");
  SoapySDRKwargs_set(&args,"timestamp_every",every);  // Размер буфера + временные метки
  SoapySDRKwargs_set(&args,"loopback","0");  // Используем антенны или нет
  s->dev=SoapySDRDevice_make(&args);  // Инициализация
  SoapySDRKwargs_clear(&args);
  SoapySDRDevice_setSampleRate(s->dev,SOAPY_SDR_RX,0,SAMPLE_RATE);
  SoapySDRDevice_setSampleRate(s->dev,SOAPY_SDR_TX,0,SAMPLE_RATE);
  SoapySDRDevice_setFrequency(s->dev,SOAPY_SDR_RX,0,rx_freq,NULL);
  SoapySDRDevice_setFrequency(s->dev,SOAPY_SDR_TX,0,tx_freq,NULL);
  SoapySDRDevice_setGain(s->dev,SOAPY_SDR_RX,0,GAIN);
  SoapySDRDevice_setGain(s->dev,SOAPY_SDR_TX,0,GAIN);
  size_t channels[]={0};
  s->rx=SoapySDRDevice_setupStream(s->dev,SOAPY_SDR_RX,SOAPY_SDR_CS16,channels,1,NULL);
  s->tx=SoapySDRDevice_setupStream(s->dev,SOAPY_SDR_TX,SOAPY_SDR_CS16,channels,1,NULL);
  SoapySDRDevice_activateStream(s->dev,s->rx,0,0,0);
  SoapySDRDevice_activateStream(s->dev,s->tx,0,0,0);
  s->last_time=0;
  s->count=0;
}

int sdr_step(Sdr*s,int16_t*rx_buffer,int16_t*tx_buffer){
  void*rx_buffs[]={rx_buffer};
  const void*tx_buffs[]={tx_buffer};
  int flags=0;
  long long time_ns=s->last_time;
  int sr=SoapySDRDevice_readStream(s->dev,s->rx,rx_buffs,BUF,&flags,&time_ns,TIMEOUT_US);
  s->last_time=time_ns;
  if(s->count%2==0){
    long long tx_time=time_ns+TX_DELAY_NS;  // на 4 [мс] в будущее
    put_header(tx_buffer,tx_time);
    // Здесь отправляем наш tx_buff массив
    flags=SOAPY_SDR_HAS_TIME;
    SoapySDRDevice_writeStream(s->dev,s->tx,tx_buffs,BUF,&flags,tx_time,TIMEOUT_US);
  }
  s->count++;
  return sr;
}

void sdr_close(Sdr*s){
  SoapySDRDevice_deactivateStream(s->dev,s->rx,0,0);
  SoapySDRDevice_deactivateStream(s->dev,s->tx,0,0);
  SoapySDRDevice_closeStream(s->dev,s->rx);
  SoapySDRDevice_closeStream(s->dev,s->tx);
  SoapySDRDevice_unmake(s->dev);
}
