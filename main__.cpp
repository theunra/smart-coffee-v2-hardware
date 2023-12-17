#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "TaskScheduler.h"
#include "max6675.h"
#include "DFRobot_MLX90614.h"
#include "RelayRegister.h"
// #include "RTClib.h"
#include "FuzzyController.h"
#include "ESP32_Servo.h"
#include "NextionInterface.h"

#define DEBUG 0

// const int servo_pin = 2;
// const int flame_pin = 4;
// const int ir_pin = 15;
// const int lighter_pin = 18;
// const int relay_scl = 14;
// const int relay_sda = 25;
// const int relay_lat = 26;
// const int max_do = 19;
// const int max_cs = 23;
// const int max_clk = 5;

TaskHandle_t t0H;
void Core0Task(void*);

void appinit();
// void mlxCallback();
// void maxCallback();
// void fuzzCallback();
// void relayCallback();
// void irCallback();
// void flameCallback();
// void nexListenCallback();

// bool parseNextionMsg(String);

// Task mlx_task(20, TASK_FOREVER, &mlxCallback);
// Task max_task(1000, TASK_FOREVER, &maxCallback);
// Task ir_task(1000, TASK_FOREVER, &irCallback);
// Task flame_task(1000, TASK_FOREVER, &flameCallback);
// Task fuzz_task(20, TASK_FOREVER, &fuzzCallback);
// Task relay_task(100, TASK_FOREVER, &relayCallback);
// Task nexlisten_task(100, TASK_FOREVER, &nexListenCallback);

Scheduler sch;

NextionInterface nex_i(&Serial2);
// RelayRegister relay_reg;
// MAX6675 thermocouple(max_clk, max_cs, max_do);
// DFRobot_MLX90614_I2C mlx_sensor;
// RTC_DS3231 rtc;
// Servo servo;

// FuzzyController fC;
// FuzzyVariable dT("dT");
// FuzzyVariable Valve("Valve");

// float error = 0.0;
// float target_temp = 180.0;
// float valve_deg = 0.0;
// float object_temp = 0.0;
// float thermo_temp = 0.0;
// int relay_i = 0;
// int flame = 0;
// int ir = 0;
// DateTime rtc_now;

// bool serv = false;

void Core0Task(void* vParam)
{
  // for(;;)
  // {
  //   if(valve_deg<0)
  //   {
  //     nex_i.updateObject<int>(5, 100);
  //     nex_i.updateObject<int>(4, 0);
  //   }
  //   else if(valve_deg<=90)
  //   {
  //     nex_i.updateObject<int>(5,100);
  //     nex_i.updateObject<int>(4,valve_deg*100/90);
  //   }
  //   else if(valve_deg<=180)
  //   {
  //     nex_i.updateObject<int>(5, 100 - ((valve_deg-90)*100/90));
  //     nex_i.updateObject<int>(4,100);
  //   }
  //   else if(valve_deg>180)
  //   {
  //     nex_i.updateObject<int>(5, 0);
  //     nex_i.updateObject<int>(4,100);
  //   }

  //   nex_i.updateObject<int>(1, thermo_temp);
  //   nex_i.updateObject<int>(6, thermo_temp);

    // delay(100);
    
    #if DEBUG

    Serial.print("Non-contact temp : ");
    Serial.println(object_temp);
    Serial.print("Thermocouple temp : ");
    Serial.println(thermo_temp);
    Serial.print("Servo deg : ");
    Serial.println(valve_deg);
    Serial.print("Flame : ");
    Serial.println(flame);
    Serial.print("IR : ");
    Serial.println(ir);

    if(!serv) 
    {
      servo.write(0);
      relay_reg.setRelayValue(6, 1);
    }
    else{
      servo.write(180);
      relay_reg.setRelayValue(6, 0);
    }
    
    if(relay_i !=4) relay_i += 1;
    else relay_i = 0;
    
    for(int i = 0; i<4;i++) {
      if(i == relay_i) relay_reg.setRelayValue(i, 1);
      else relay_reg.setRelayValue(i, 0);
    }

    Serial.print("Relay : ");
    Serial.println(relay_i);

    delay(1000);

    #endif
  // }
}

// void mlxCallback()
// {
//     object_temp = mlx_sensor.getObjectTempCelsius();
// }

// void fuzzCallback()
// {
//   //LIBRARY REFACTOR INPROGRESS
//   // dT.Fuzzyfication(error);
  
//   // fC.process();

//   // valve_deg = Valve.Defuzzyfication();

//   // valve_deg = map(valve_deg,0,100,0,180);

//   //cuma buat bikin laporan
//   if(error>30)
//   {
//     valve_deg = 180;
//   }
//   else if(error<30 && error>0){
//     valve_deg = 180*error/30;
//   }
//   else{
//     valve_deg = 0;
//   }

//   servo.write(valve_deg);
// }

// void maxCallback()
// {
//   thermo_temp = thermocouple.readCelsius();
//   error = target_temp - thermo_temp;
// }

// void irCallback()
// {
//   ir = digitalRead(ir_pin);
// }

// void flameCallback()
// {
//   flame = digitalRead(flame_pin);
// }

// void relayCallback()
// {
//   relay_reg.updateRelay();
// }

void nexListenCallback()
{
  String data_nex = nex_i.listen();
  // parseNextionMsg(data_nex);
}

// //pin pwm1 , IO13
// bool parseNextionMsg(String msg)
// {
//   bool ok = 0;
//   if(msg[0]=='*')
//   {
//     ok=true;
//   }

//   if(ok)
//   {
//     bool input_id = false;
//     bool input_val = false;
//     String id = "";
//     String val = "";
//     for(int i = 0;i<msg.length();i++)
//     {
//       if(i==0)
//       {
//         input_id = true;
//         continue;
//       }
//       else if(msg[i]=='#')
//       {
//         input_id = false;
//         input_val = true;
//         continue;
//       }
//       else if(msg[i]=='%')
//       {
//         break;
//       }

//       if(input_id)
//       {
//         id+=msg[i];
//       }
//       else if(input_val)
//       {
//         val+=msg[i];
//       }
//     }

//     if(id == "agi")
//     {
//       if(val == "1")
//       {
//         relay_reg.setRelayValue(1, 0);
//       }
//       else if(val == "0")
//       {
//         relay_reg.setRelayValue(1, 1);
//       }
//     }
//     else if(id == "blw")
//     {
//       if(val == "1")
//       {
//         relay_reg.setRelayValue(2, 0);
//       }
//       else if(val == "0")
//       {
//         relay_reg.setRelayValue(2, 1);
//       }
//     }
//     else if(id == "lig")
//     {
//       if(val == "1")
//       {
//         relay_reg.setRelayValue(3, 0);
//       }
//       else if(val == "0")
//       {
//         relay_reg.setRelayValue(3, 1);
//       }
//     }
//   }

//   return ok;
// }

void appinit()
{
  Serial.begin(9600);
  Wire.begin();
  delay(1000);

  xTaskCreatePinnedToCore(
                  Core0Task,   
                  "Core0Task", 
                  10000,    
                  NULL,     
                  1,        
                  &t0H,     
                  0);

  sch.init();
  // sch.addTask(mlx_task);
  // sch.addTask(fuzz_task);
  // sch.addTask(max_task);
  // sch.addTask(relay_task);
  // sch.addTask(flame_task);
  // sch.addTask(ir_task);
  // sch.addTask(nexlisten_task);
  // mlx_task.enable();
  // fuzz_task.enable();
  // max_task.enable();
  // relay_task.enable();
  // flame_task.enable();
  // ir_task.enable();
  // nexlisten_task.enable();
}

void setup()
{
  appinit();
}

void loop() 
{
  sch.execute();
}