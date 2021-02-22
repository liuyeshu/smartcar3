/*
Copyright (C): 2020, liuyeshu
*/

#include "pxt.h"
#include <map>
#include <vector>
#include "ReceiverIR.h"
using namespace pxt;
typedef vector<Action> vA;

enum class RemoteButton {
      SET = 0x15,
	  UP = 0x40,
	  DOWN = 0x19,
	  LEFT = 0x07,
	  RIGHT = 0x09,
	  A = 0x45,
	  B = 0x46,
	  C = 0x47,
	  D = 0x44,
      E = 0x43,
      F = 0x0D,
	  NUM0 = 0x16,
	  NUM1 = 0x0C,
	  NUM2 = 0x18,
	  NUM3 = 0x5E,
	  NUM4 = 0x08,
	  NUM5 = 0x1C,
	  NUM6 = 0x5A,
	  NUM7 = 0x42,
	  NUM8 = 0x52,
	  NUM9 = 0x4A
};

enum class Pins{
    P0=  3,
    P1=  2,
    P2=  1,
    P3=  4,
    P4=  5,
    P5=  17,
    P6=  12,
    P7=  11,
    P8=  18,
    P9=  10,
    P10= 6,
    P11= 26,
    P12= 20,
    P13= 23,
    P14= 22,
    P15= 21,
    P16= 16,
    P19= 0,
    P20= 30
};

namespace microbit_IR {
  map<RemoteButton, vA> actions;
  map<RemoteButton, uint32_t> lastact;
  Timer tsb;
  uint8_t buf[32];
  uint32_t now;
  ReceiverIR *rx;
  RemoteIR::Format fmt = RemoteIR::UNKNOWN;
	int ir_code = 0x00;
	int ir_addr = 0x00;
	int data;

  void cA(vA runner){for(int i=0;i<runner.size();i++){runAction0(runner[i]);} }

  void onReceivable(){
    int x = rx->getData(&fmt, buf, 32);
    //uBit.serial.send(buf, 4);
    if(actions.find((RemoteButton)buf[2]) == actions.end()) return;
    now = tsb.read_ms();
    if(now - lastact[(RemoteButton)buf[2]] < 100) return;
    lastact[(RemoteButton)buf[2]] = now;
    cA(actions[(RemoteButton)buf[2]]);
  }

    void onReceivableV2(){
    if(actions.find((RemoteButton)(ir_code&0x00FF)) == actions.end()) return;
    now = tsb.read_ms();
    if(now - lastact[(RemoteButton)(ir_code&0x00FF)] < 100) return;
    lastact[(RemoteButton)(ir_code&0x00FF)] = now;
    cA(actions[(RemoteButton)(ir_code&0x00FF)]);
  }

      int logic_value(){
        uint32_t lasttime = system_timer_current_time_us();
        uint32_t nowtime;
        while(!uBit.io.P16.getDigitalValue());
        nowtime = system_timer_current_time_us();
        if((nowtime - lasttime) > 400 && (nowtime - lasttime) < 700){
            while(uBit.io.P16.getDigitalValue());
            lasttime = system_timer_current_time_us();
            if((lasttime - nowtime)>400 && (lasttime - nowtime) < 700){
                return 0;
            }else if((lasttime - nowtime)>1500 && (lasttime - nowtime) < 1800){
                return 1;
        }
        }
        uBit.serial.printf("error\r\n");
        return -1;
    }

void pulse_deal(){
    int i;
    ir_addr=0x00;
    for(i=0; i<16;i++ )
    {
        if(logic_value() == 1)
        {
            ir_addr |=(1<<i);
        }
    }
   
    ir_code=0x00;
    for(i=0; i<16;i++ )
    {
        if(logic_value() == 1)
        {
            ir_code |=(1<<i);
        }
    }

}

void remote_decode(void){
    data = 0x00;
    uint32_t lasttime = system_timer_current_time_us();
    uint32_t nowtime;
    while(uBit.io.P16.getDigitalValue()){
        nowtime = system_timer_current_time_us();
        if((nowtime - lasttime) > 100000){
            ir_code = 0xff00;
            return;
        }
    }
 
    lasttime = system_timer_current_time_us();
    while(!uBit.io.P16.getDigitalValue());
    nowtime = system_timer_current_time_us();
    if((nowtime - lasttime) < 10000 && (nowtime - lasttime) > 8000){
        while(uBit.io.P16.getDigitalValue());
        lasttime = system_timer_current_time_us();
        if((lasttime - nowtime) > 4000 && (lasttime - nowtime) < 5000){
            pulse_deal();
            data = ir_code;
            return;//ir_code;
        }else if((lasttime - nowtime) > 2000 && (lasttime - nowtime) < 2500){
            while(!uBit.io.P16.getDigitalValue());
            nowtime = system_timer_current_time_us();
            if((nowtime - lasttime) > 500 && (nowtime - lasttime) < 700){
                data = ir_code;
                return;//ir_code;
            }
        }
    }
}

int irCode(){
    remote_decode();
    return data;
}

  void monitorIR(){
    while(1){
      while(0 == irCode())
	  {
        uBit.sleep(50);
      }
	  //uBit.serial.printf("data=0x%X,code = 0x%X\r\n",data,ir_code);
	  onReceivableV2();
    }
  }

  //%
  void init(Pins pin){
    rx = new ReceiverIR((PinName)pin);
    tsb.start(); //interrupt timer for debounce
    create_fiber(monitorIR);
  }

  //%
  void onPressEvent(RemoteButton btn, Action body) {
    actions[btn].push_back(body);
  }

}
