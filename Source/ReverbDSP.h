#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>

namespace cyber {
constexpr float pi = 3.14159265358979323846f;
inline float clamp(float x,float lo,float hi) { return std::max(lo,std::min(x,hi)); }
inline float dbGain(float x) { return std::pow(10.0f,x/20.0f); }
struct Stereo { float l=0,r=0; };
struct Delay {
    std::vector<float> data; int pos=0;
    void prepare(int n) { data.assign(size_t(n+8),0); pos=0; }
    void clear() { std::fill(data.begin(),data.end(),0.f); pos=0; }
    float read(float samples) const noexcept {
        // Keep wrapping in integer space. Adding a tiny negative fractional
        // position to a large float buffer length can round up to size(),
        // producing a one-past-the-end read in the original implementation.
        const int size=int(data.size());
        if(size<4)return 0;
        samples=std::isfinite(samples)?clamp(samples,1,float(size-3)):1.f;
        const int whole=int(samples);const float fraction=samples-float(whole);
        int current=pos-whole;if(current<0)current+=size;
        const int previous=current==0?size-1:current-1;
        return data[size_t(current)] + fraction*(data[size_t(previous)]-data[size_t(current)]);
    }
    void write(float x) noexcept { data[size_t(pos)]=x; if(++pos==int(data.size())) pos=0; }
};
struct Allpass {
    Delay d; float time=1;
    void prepare(double sr,float ms) { time=float(sr)*ms*.001f; d.prepare(int(time)+8); }
    float tick(float x,float g) noexcept { float z=d.read(time); float y=z-g*x; d.write(x+g*y); return y; }
    void clear() { d.clear(); }
};
struct Settings {
    float size=.65f,decay=2.8f,diffusion=.75f,damping=.5f,modDepth=.1f,modRate=.3f,vintage=0,freeze=0;
};

// Orthogonal Hadamard feedback matrix; energy is preserved by 1/sqrt(N).
// Different line counts, delay sets and injection/output vectors tune the two halls.
template<int N> class Hall {
    std::array<Delay,N> lines;
    std::array<float,N> lengths{},state{},gains{},base{},phase{};
    std::array<Allpass,4> ap;
    float sr=48000,lp=0.5f,feed=1,depth=0,rate=.3f,freeze=0;
public:
    void prepare(double sampleRate) {
        sr=float(sampleRate);
        constexpr float times[16]={29.7f,37.1f,41.9f,47.3f,53.9f,61.7f,67.1f,73.9f,79.3f,83.9f,89.3f,97.1f,103.1f,109.7f,113.9f,127.7f};
        for(int i=0;i<N;++i) { base[i]=times[i]*(N==16?1.23f:.34f); lines[i].prepare(int(sr*.4f)); phase[i]=float(i)/float(N); }
        for(int i=0;i<4;++i) ap[i].prepare(sr,N==16?4.3f+float(i)*3.73f:1.1f+float(i)*1.37f);
        state.fill(0); update({});
    }
    void clear() { for(auto& d:lines)d.clear(); for(auto& a:ap)a.clear(); state.fill(0); }
    float diffusion=.75f;
    void update(const Settings& s) noexcept {
        diffusion=s.diffusion; freeze=s.freeze; feed=1-freeze;
        float cutoff=18000*std::pow(.035f,s.damping)*(1-.65f*s.vintage);
        lp=(1-std::exp(-2*pi*cutoff/sr))*(1-freeze)+freeze;
        depth=(s.modDepth*.7f+s.vintage*.45f)*sr*.001f*(1-freeze);
        rate=s.modRate;
        for(int i=0;i<N;++i) {
            lengths[i]=sr*.001f*base[i]*(.38f+s.size*1.25f);
            // RT60 amplitude loss per traversal; freeze has unity feedback.
            gains[i]=std::pow(.001f,lengths[i]/(sr*s.decay))*(1-freeze)+freeze*.999999f;
        }
    }
    Stereo tick(Stereo in) noexcept {
        float x=(in.l+in.r)*.5f*feed;
        for(auto& a:ap) x=a.tick(x,.15f+diffusion*.57f);
        std::array<float,N> y{};
        Stereo out;
        for(int i=0;i<N;++i) {
            // Smooth triangle modulation avoids per-sample trig in the feedback loops.
            phase[i]+=rate*(1+float(i)*.017f)/sr; if(phase[i]>=1)phase[i]-=std::floor(phase[i]);
            float tri=4*std::abs(phase[i]-.5f)-1;
            float v=lines[i].read(lengths[i]+depth*tri);
            state[i]+=lp*(v-state[i]); y[i]=state[i]*gains[i];
            out.l+=v*((i&1)?-.25f:.25f); out.r+=v*((i&2)?-.25f:.25f);
        }
        for(int step=1;step<N;step*=2) for(int i=0;i<N;i+=2*step) for(int j=0;j<step;++j) {
            float a=y[i+j],b=y[i+j+step]; y[i+j]=a+b;y[i+j+step]=a-b;
        }
        const float norm=1/std::sqrt(float(N));
        for(int i=0;i<N;++i) {
            float injection=x*norm + ((i&1)?in.l-in.r:in.r-in.l)*.07f*feed;
            float v=y[i]*norm+injection;
            // Emergency numerical guard, outside normal operating level.
            lines[i].write(clamp(v,-4,4));
        }
        return {out.l*(N==16?1.7f:1.f),out.r*(N==16?1.7f:1.f)};
    }
};

// Compact room: discrete stereo early reflections followed by a short-path FDN.
// This is our own design; Valhalla Room is a listening direction, not measured data.
class Room {
    Hall<8> late;std::array<Delay,2> early;float sr=48000,scale=1,feed=1;
public:
    void prepare(double rate){sr=float(rate);late.prepare(rate);for(auto& d:early)d.prepare(int(sr*.12f));}
    void clear(){late.clear();for(auto& d:early)d.clear();}
    void update(const Settings& s){late.update(s);scale=.35f+s.size*.9f;feed=1-s.freeze;}
    Stereo tick(Stereo in){
        constexpr float times[]={3.7f,7.1f,11.3f,17.9f,23.3f,31.7f};
        constexpr float gains[]={.36f,.27f,.21f,.16f,.12f,.09f};Stereo reflections;
        for(int i=0;i<6;++i){float t=sr*.001f*times[i]*scale;
            reflections.l+=gains[i]*(early[0].read(t)*.8f+early[1].read(t*1.13f)*.2f);
            reflections.r+=gains[i]*(early[1].read(t*1.07f)*.8f+early[0].read(t*.93f)*.2f);}
        early[0].write(in.l*feed);early[1].write(in.r*feed);auto tail=late.tick(in);
        return {reflections.l*.65f+tail.l*.85f,reflections.r*.65f+tail.r*.85f};
    }
};

// Dattorro-inspired cross-coupled tanks: input diffusion, tank allpass,
// modulated delay, damping, second allpass and delay with cross feedback.
class Plate {
    std::array<Allpass,4> input;
    std::array<Allpass,2> a,b;
    std::array<Delay,2> d1,d2;
    std::array<float,2> lpState{},last{};
    float sr=48000,t1=1,t2=1,g=.8f,lp=.5f,feed=1,phase=0,rate=.3f,depth=0,diff=.7f;
public:
    void prepare(double sampleRate) {
        sr=float(sampleRate);
        for(int i=0;i<4;++i) input[i].prepare(sr,1.77f+float(i)*1.83f);
        for(int i=0;i<2;++i) { a[i].prepare(sr,22.58f+float(i)*7.99f);b[i].prepare(sr,60.48f+float(i)*14.83f);d1[i].prepare(int(sr*.3f));d2[i].prepare(int(sr*.3f)); }
        lpState.fill(0);last.fill(0);update({});
    }
    void clear() { for(auto& x:input)x.clear();for(int i=0;i<2;++i){a[i].clear();b[i].clear();d1[i].clear();d2[i].clear();}lpState.fill(0);last.fill(0); }
    void update(const Settings& s) noexcept {
        float scale=.35f+s.size*1.1f;t1=sr*.038f*scale;t2=sr*.029f*scale;
        g=std::pow(.001f,(t1+t2+sr*.09f)/(sr*s.decay))*(1-s.freeze)+s.freeze*.999999f;
        float cutoff=21000*std::pow(.12f,s.damping)*(1-s.vintage*.65f);
        lp=(1-std::exp(-2*pi*cutoff/sr))*(1-s.freeze)+s.freeze;
        feed=1-s.freeze; rate=s.modRate;depth=(s.modDepth+s.vintage*.55f)*sr*.001f*(1-s.freeze);diff=.12f+s.diffusion*.43f;
    }
    Stereo tick(Stereo in) noexcept {
        float x=(in.l+in.r)*.35f*feed;for(auto& p:input)x=p.tick(x,diff);
        phase+=rate/sr;if(phase>=1)phase-=1;
        float mod=(4*std::abs(phase-.5f)-1)*depth;
        std::array<float,2> next{},tap{};
        for(int i=0;i<2;++i) {
            float z=a[i].tick(x+last[1-i]*g,-.72f);
            float y=d1[i].read(t1*(i==0?1.f:1.137f)+(i==0?mod:-mod));d1[i].write(clamp(z,-4,4));
            lpState[i]+=lp*(y-lpState[i]);
            z=b[i].tick(lpState[i],.5f);
            next[i]=d2[i].read(t2*(i==0?1.f:.917f));d2[i].write(clamp(z,-4,4));
            tap[i]=y*.6f+next[i]*.4f;
        }
        last=next;return {tap[0],tap[1]};
    }
};

class Reverb {
    Room room; Hall<16> hall; Plate plate;
    std::array<Delay,2> predelay;
    std::array<float,3> weights{1,0,0};
    float sr=48000,pre=24,weightStep=.001f;
    bool firstSample=true;
public:
    void prepare(double sampleRate) { sr=float(sampleRate);room.prepare(sr);hall.prepare(sr);plate.prepare(sr);for(auto&d:predelay)d.prepare(int(sr*.26f));weightStep=1-std::exp(-1/(sr*.025f));firstSample=true; }
    void clear() { room.clear();hall.clear();plate.clear();for(auto&d:predelay)d.clear(); }
    void update(Settings s,float preMs) { room.update(s);hall.update(s);plate.update(s);pre=preMs*sr*.001f; }
    Stereo tick(Stereo in,int engine) noexcept {
        if(firstSample){for(int i=0;i<3;++i)weights[i]=i==engine?1.f:0.f;firstSample=false;}
        Stereo x;
        // At zero pre-delay use current input (no unintended one-sample latency).
        x.l=pre<1?in.l:predelay[0].read(pre);x.r=pre<1?in.r:predelay[1].read(pre);
        predelay[0].write(in.l);predelay[1].write(in.r);
        std::array<Stereo,3> y{room.tick(x),hall.tick(x),plate.tick(x)};
        Stereo out;
        for(int i=0;i<3;++i) {weights[i]+=weightStep*((engine==i?1.f:0.f)-weights[i]);out.l+=weights[i]*y[i].l;out.r+=weights[i]*y[i].r;}
        return out;
    }
};
}
