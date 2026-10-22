#pragma once
#include <JuceHeader.h>
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <fstream>
#include <iostream>
#include <cmath>
using namespace juce;
static var obj(){return var(new DynamicObject);}
static void put(var&v,const String&k,const var&x){v.getDynamicObject()->setProperty(k,x);}
static void ensure(bool b,const char*why){if(!b)throw std::runtime_error(why);}
struct Clock {
 HANDLE timer=CreateWaitableTimerExW(nullptr,nullptr,CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,TIMER_ALL_ACCESS);
 ~Clock(){if(timer)CloseHandle(timer);}
 void wait(std::chrono::steady_clock::time_point deadline){auto ns=std::chrono::duration_cast<std::chrono::nanoseconds>(deadline-std::chrono::steady_clock::now()).count();if(ns<=0)return;if(!timer){std::this_thread::sleep_until(deadline);return;}LARGE_INTEGER due;due.QuadPart=-jmax<int64_t>(1,ns/100);if(SetWaitableTimer(timer,&due,0,nullptr,nullptr,FALSE))WaitForSingleObject(timer,1000);}
};
static var resources(){var r=obj();PROCESS_MEMORY_COUNTERS_EX pm{};pm.cb=sizeof(pm);ensure(GetProcessMemoryInfo(GetCurrentProcess(),(PROCESS_MEMORY_COUNTERS*)&pm,sizeof(pm))!=0,"Resource memory query failed");DWORD handles=0;ensure(GetProcessHandleCount(GetCurrentProcess(),&handles)!=0,"Resource handle query failed");int threads=0;HANDLE h=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);ensure(h!=INVALID_HANDLE_VALUE,"Resource thread snapshot failed");THREADENTRY32 t{};t.dwSize=sizeof(t);if(Thread32First(h,&t))do{if(t.th32OwnerProcessID==GetCurrentProcessId())++threads;}while(Thread32Next(h,&t));CloseHandle(h);put(r,"workingSetBytes",(int64)pm.WorkingSetSize);put(r,"privateBytes",(int64)pm.PrivateUsage);put(r,"handles",(int64)handles);put(r,"threads",threads);return r;}
struct Evidence {
 File dir;var state=obj();Array<var> resourceRows;double start=Time::getMillisecondCounterHiRes(),lastMinute=-60;bool trialSent=false;double seconds;
 explicit Evidence(const String&p,double duration):dir(p),seconds(duration){ensure(!dir.exists(),"Choose a fresh run directory; first failures are never overwritten");ensure(dir.createDirectory().wasOk(),"Cannot create evidence directory");put(state,"passed",false);put(state,"hardwareAudioOpened",false);put(state,"publicNetworkUsed",false);put(state,"requestedSeconds",seconds);put(state,"pid",(int64)GetCurrentProcessId());}
 double elapsed()const{return (Time::getMillisecondCounterHiRes()-start)/1000;}
 void write(const String&name,const var&value){ensure(dir.getChildFile(name).replaceWithText(JSON::toString(value,true)),"Cannot save evidence");}
 void tick(){const double t=elapsed();if(t-lastMinute<60&&lastMinute>=0)return;lastMinute=t;var row=resources();put(row,"elapsedSeconds",t);resourceRows.add(row);put(state,"elapsedSeconds",t);put(state,"soFarPassed",true);put(state,"resources",row);put(state,"phase",trialSent?"continuing-long-session":"ten-minute-trial");write("progress.json",state);FileOutputStream csv(dir.getChildFile("resources.jsonl"));ensure(csv.openedOk(),"Cannot append resource log");csv.setPosition(dir.getChildFile("resources.jsonl").getSize());csv.writeText(JSON::toString(row,true)+"\n",false,false,nullptr);csv.flush();if(!trialSent&&t>=jmin(600.0,seconds)){trialSent=true;put(state,"resourceHistory",resourceRows);write("trial-checkpoint.json",state);state.getDynamicObject()->removeProperty("resourceHistory");std::cout<<"TRIAL_READY "<<dir.getFullPathName()<<std::endl;}
  if(trialSent&&seconds>600&&t>630)ensure(dir.getChildFile("continue.flag").existsAsFile(),"Supervisor did not validate the ten-minute checkpoint");
 }
 void finish(bool passed,const String&error={}){put(state,"passed",passed);put(state,"elapsedSeconds",elapsed());put(state,"phase",passed?"complete":"failed-first-stop");put(state,"error",error);put(state,"resourceHistory",resourceRows);write("report.json",state);write("progress.json",state);}
};
template<class F>static void until(F f,int ms=5000){auto end=Time::getMillisecondCounterHiRes()+ms;while(!f()){ensure(Time::getMillisecondCounterHiRes()<end,"Bounded control wait timed out");Thread::sleep(5);}}
static TextButton* key(Component&c,const String&id){if(auto*b=dynamic_cast<TextButton*>(&c))if(b->getComponentID()==id)return b;for(auto*child:c.getChildren())if(auto*b=key(*child,id))return b;return nullptr;}
