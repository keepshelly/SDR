#include <stdint.h>
#include <stdio.h>   //printf
#include <string.h>  //strcmp
#include "tx.h"
#include "sdr.h"

const char*URI_DEFAULT="usb:1.2.5";

int run(const char*uri,bool is_tx,int use_qpsk){
  double rx_freq=FREQ1;
  double tx_freq=FREQ2;
  if(is_tx){
    rx_freq=FREQ2;
    tx_freq=FREQ1;
  }
  const char*filename="rxdata_bpsk.pcm";
  if(is_tx&&use_qpsk==1){
    filename="txdata_qpsk.pcm";
  }
  if(is_tx&&use_qpsk==0){
    filename="txdata_bpsk.pcm";
  }
  if(!is_tx&&use_qpsk==1){
    filename="rxdata_qpsk.pcm";
  }
  Sdr s;
  sdr_open(&s,uri,rx_freq,tx_freq);
  FILE*file=fopen(filename,"w");
  int16_t rx_buffer[2*BUF];
  int16_t tx_buffer[2*BUF];
  make_signal(use_qpsk,tx_buffer);
  while(1){
    int sr=sdr_step(&s,rx_buffer,tx_buffer);
    // Смотрим на количество считаных сэмплов, времени прихода и разницы во
    // времени с чтением прошлого буфера
    printf("буфер: %lld, сэмплов: %d, время: %lld\n",s.count,sr,s.last_time);
    fwrite(rx_buffer,sizeof(rx_buffer),1,file);
  }
}

int main(int argc,char*argv[]){
  const char*uri=URI_DEFAULT;
  if(argc>2){
    uri=argv[2];
  }
  int use_qpsk=0;
  if(argc>3&&strcmp(argv[3],"qpsk")==0){
    use_qpsk=1;
  }
  if(strcmp(argv[1],"rx")==0){
    return run(uri,false,use_qpsk);
  }
  if(strcmp(argv[1],"tx")==0){
    return run(uri,true,use_qpsk);
  }
  printf("error\n");
  return 1;
}
