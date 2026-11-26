#include <SDL.h>
#include <SDL_opengl.h>
#include <atomic>
#include <thread>
#include <stdio.h>
#include <string.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "tx.h"
#include "sdr.h"
#include "dsp.h"

const char*URI_TX="usb:1.2.5";
const char*URI_RX="usb:1.3.5";
const int HIST=8;
const float LIMIT=300;
const char*FONT="/System/Library/Fonts/Supplemental/Arial.ttf";

std::atomic<bool> running(true);
std::atomic<int> use_qpsk(1);
std::atomic<float> rx_gain((float)GAIN);
std::atomic<float> tx_gain((float)GAIN);
int16_t sig[2][2*BUF];
int16_t hist[HIST][2*BUF];

void worker(const char*uri,bool is_rx){
  double rx_freq=FREQ2;
  double tx_freq=FREQ1;
  if(is_rx){
    rx_freq=FREQ1;
    tx_freq=FREQ2;
  }
  Sdr s;
  sdr_open(&s,uri,rx_freq,tx_freq);
  int16_t rx_buffer[2*BUF];
  int16_t tx_buffer[2*BUF];
  int mod=-1;
  int pos=0;
  float rg=GAIN;
  float tg=GAIN;
  while(running){
    if(mod!=use_qpsk){
      mod=use_qpsk;
      memcpy(tx_buffer,sig[mod],sizeof(tx_buffer));
    }
    if(rg!=rx_gain||tg!=tx_gain){
      rg=rx_gain;
      tg=tx_gain;
      SoapySDRDevice_setGain(s.dev,SOAPY_SDR_RX,0,rg);
      SoapySDRDevice_setGain(s.dev,SOAPY_SDR_TX,0,tg);
    }
    sdr_step(&s,rx_buffer,tx_buffer);
    if(is_rx){
      memcpy(hist[pos],rx_buffer,sizeof(rx_buffer));
      pos=(pos+1)%HIST;
    }
  }
  sdr_close(&s);
}

int main(int argc,char*argv[]){
  const char*uri_tx=URI_TX;
  const char*uri_rx=URI_RX;
  if(argc>2){
    uri_tx=argv[1];
    uri_rx=argv[2];
  }

  if(SDL_Init(SDL_INIT_VIDEO)!=0){
    printf("ошибка sdl: %s\n",SDL_GetError());
    return 1;
  }
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,2);
  SDL_Window*win=SDL_CreateWindow("sdr",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1300,750,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
  SDL_GLContext gl=SDL_GL_CreateContext(win);
  SDL_GL_MakeCurrent(win,gl);
  SDL_GL_SetSwapInterval(1);

  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO&io=ImGui::GetIO();
  FILE*f=fopen(FONT,"rb");
  if(f!=NULL){
    fclose(f);
    io.Fonts->AddFontFromFileTTF(FONT,16.0f,NULL,io.Fonts->GetGlyphRangesCyrillic());
  }else{
    printf("шрифт %s не найден\n",FONT);
  }
  ImGui_ImplSDL2_InitForOpenGL(win,gl);
  ImGui_ImplOpenGL3_Init("#version 150");

  make_signal(0,sig[0]);
  make_signal(1,sig[1]);

  std::thread t_tx(worker,uri_tx,false);
  std::thread t_rx(worker,uri_rx,true);

  static int16_t local[HIST][2*BUF];
  static cf x[BUF];
  static float pi[HIST*BUF];
  static float pq[HIST*BUF];
  static float ti[BUF];
  static float tq[BUF];
  int mod=1;
  bool fix=false;
  float rg=GAIN;
  float tg=GAIN;

  while(running){
    SDL_Event e;
    while(SDL_PollEvent(&e)){
      ImGui_ImplSDL2_ProcessEvent(&e);
      if(e.type==SDL_QUIT){
        running=false;
      }
    }

    memcpy(local,hist,sizeof(hist));

    int np=0;
    int best=0;
    float best_thr=0;
    float snr=0;
    float offset=0;
    for(int b=0;b<HIST;b++){
      to_complex(local[b],BUF,x);
      float thr=threshold(x,BUF);
      snr+=snr_db(x,BUF,thr)/HIST;
      float off=0;
      if(fix){
        int m=2;
        if(mod==1){
          m=4;
        }
        off=fix_offset(x,BUF,thr,m);
      }
      for(int i=0;i<BUF;i++){
        if(std::abs(x[i])>thr){
          pi[np]=x[i].real();
          pq[np]=x[i].imag();
          np++;
        }
      }
      if(thr>best_thr){
        best_thr=thr;
        best=b;
        offset=off;
      }
    }
    for(int i=0;i<BUF;i++){
      ti[i]=local[best][2*i];
      tq[i]=local[best][2*i+1];
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("sdr",NULL,ImGuiWindowFlags_NoDecoration);

    ImGui::Text("передатчик %s, приёмник %s",uri_tx,uri_rx);
    ImGui::RadioButton("bpsk",&mod,0);
    ImGui::SameLine();
    ImGui::RadioButton("qpsk",&mod,1);
    use_qpsk=mod;
    ImGui::SameLine();
    ImGui::Checkbox("компенсация сдвига частоты",&fix);
    ImGui::SetNextItemWidth(300);
    ImGui::SliderFloat("усиление приёма, дб",&rg,0,70);
    ImGui::SetNextItemWidth(300);
    ImGui::SliderFloat("усиление передачи, дб",&tg,0,89);
    rx_gain=rg;
    tx_gain=tg;
    ImGui::Text("сигнал/шум: %.1f дб",snr);
    if(fix){
      ImGui::SameLine();
      ImGui::Text("    сдвиг частоты: %.0f гц",offset);
    }

    float h=ImGui::GetContentRegionAvail().y;
    if(ImPlot::BeginPlot("созвездие",ImVec2(h,h),ImPlotFlags_Equal)){
      ImPlot::SetupAxes("i","q");
      ImPlot::SetupAxesLimits(-LIMIT,LIMIT,-LIMIT,LIMIT,ImPlotCond_Once);
      ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle,1.5f);
      ImPlot::PlotScatter("отсчёты",pi,pq,np);
      ImPlot::EndPlot();
    }
    ImGui::SameLine();
    if(ImPlot::BeginPlot("i и q",ImVec2(-1,h))){
      ImPlot::SetupAxes("номер сэмпла","значение");
      ImPlot::SetupAxesLimits(0,BUF,-LIMIT,LIMIT,ImPlotCond_Once);
      ImPlot::PlotLine("i",ti,BUF);
      ImPlot::PlotLine("q",tq,BUF);
      ImPlot::EndPlot();
    }
    ImGui::End();

    ImGui::Render();
    glViewport(0,0,(int)(io.DisplaySize.x*io.DisplayFramebufferScale.x),(int)(io.DisplaySize.y*io.DisplayFramebufferScale.y));
    glClearColor(0.1f,0.1f,0.1f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(win);
  }

  t_tx.join();
  t_rx.join();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  SDL_GL_DeleteContext(gl);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
