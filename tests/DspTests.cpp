#include "ReverbDSP.h"
#include <iostream>
#include <chrono>
#include <stdexcept>
#include <random>
#if defined(_M_X64) || defined(__x86_64__)
#include <xmmintrin.h>
#endif
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){try{
#if defined(_M_X64) || defined(__x86_64__)
    _mm_setcsr(_mm_getcsr() | 0x8040); // Match the plugin's denormal protection.
#endif
    // Fractional taps just before the circular-buffer origin must interpolate
    // the final and first samples without rounding to an out-of-range index.
    {cyber::Delay delay;delay.prepare(19200);std::fill(delay.data.begin(),delay.data.end(),1.f);delay.data.back()=2;delay.pos=1;
        require(std::abs(delay.read(1.0001f)-1.0001f)<1.e-5f,"Fractional delay wrap reads outside the buffer");}
    for(int size:{441,4096,19200,76800}){cyber::Delay delay;delay.prepare(size);
        for(int i=0;i<int(delay.data.size());++i)delay.data[i]=std::sin(float(i)*.3f);
        for(int pos:{0,1,37,size-1})for(float tap:{1.f,1.0001f,1.9999f,37.0001f,float(size)-.0001f}){
            delay.pos=pos;double wrapped=double(pos)-double(tap);while(wrapped<0)wrapped+=double(delay.data.size());int a=int(wrapped),b=(a+1)%int(delay.data.size());double fraction=wrapped-a;
            double expected=delay.data[a]+fraction*(double(delay.data[b])-delay.data[a]);require(std::abs(delay.read(tap)-expected)<2.e-6,"Fractional delay interpolation differs from double-precision reference");}}
    for(double sr:{44100.,48000.,88200.,96000.,192000.})for(int engine=0;engine<3;++engine){
        cyber::Reverb reverb;reverb.prepare(sr);cyber::Settings s;s.decay=1.5f;s.modDepth=0;reverb.update(s,0);
        double early=0,late=0,total=0,peak=0;auto start=std::chrono::steady_clock::now();
        for(int i=0;i<int(sr*5);++i){auto out=reverb.tick({i==0?1.f:0.f,i==0?1.f:0.f},engine);
            require(std::isfinite(out.l)&&std::isfinite(out.r),"Nonfinite impulse response");double e=double(out.l)*out.l+double(out.r)*out.r;
            if(i>int(sr*.1)&&i<int(sr*.6))early+=e;if(i>int(sr*4))late+=e;total+=e;peak=std::max(peak,double(std::max(std::abs(out.l),std::abs(out.r))));
        }
        require(total>1.e-5,"Silent engine");require(late<early*.01,"Tail does not decay");require(peak<2,"Unstable impulse response");
        double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
        std::cout<<"SR "<<sr<<" engine "<<engine<<" peak "<<peak<<" energy "<<total<<" late/early "<<late/early<<" CPU "<<seconds/5*100<<"%\n";
    }
    for(int engine=0;engine<3;++engine){cyber::Reverb held;held.prepare(48000);cyber::Settings freeze;freeze.modDepth=0;double early=0,late=0;
        for(int i=0;i<48000*32;++i){if(i%32==0){freeze.freeze=i>=48000?1.f:0.f;held.update(freeze,24);}float input=i<48000?std::sin(float(i)*.07f)*.05f:0;auto y=held.tick({input,input},engine);double e=double(y.l)*y.l+double(y.r)*y.r;if(i>=48000*2&&i<48000*3)early+=e;if(i>=48000*31)late+=e;}
        std::cout<<"Freeze engine "<<engine<<" energy retention after 30s "<<late/early<<"\n";require(late<early*1.2,"Freeze builds energy");require(late>early*.05,"Freeze loses its tail too quickly");}
    cyber::Reverb reverb;reverb.prepare(48000);cyber::Settings s;
    float maximum=0;
    // Extremes, changing engine, delay, damping and freeze during a live tail.
    for(int block=0;block<15000;++block){float alpha=1-std::exp(-32.f/(48000*.025f));s.size+=alpha*((block%2?0.f:1.f)-s.size);s.decay+=alpha*((block%3?30.f:.1f)-s.decay);s.damping+=alpha*((block%2?1.f:0.f)-s.damping);s.freeze+=alpha*((block>3000&&block<12000?1.f:0.f)-s.freeze);s.modDepth=1;s.modRate=5;
        reverb.update(s,float(block%251));for(int i=0;i<32;++i){float input=block<3000?std::sin(float(block*32+i)*.05f)*.1f:0;auto y=reverb.tick({input,-input*.8f},block%3);require(std::isfinite(y.l)&&std::isfinite(y.r),"Extreme/freeze NaN");maximum=std::max(maximum,std::max(std::abs(y.l),std::abs(y.r)));}}
    std::cout<<"Extreme/freeze peak "<<maximum<<"\n";require(maximum<8,"Extreme settings cause feedback buildup");std::cout<<"All DSP checks passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
