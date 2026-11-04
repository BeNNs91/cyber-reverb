#pragma once
#include <JuceHeader.h>
#include "ReverbDSP.h"

struct ParamSpec { const char* id; const char* name; float lo,hi,def,skew; const char* unit; };
inline constexpr ParamSpec specs[] = {
    {"predelay","PRE-DELAY",0,250,12,.6f," ms"},{"size","SIZE",0,100,45,1," %"},
    {"decay","DECAY",.1f,30,.85f,.4f," s"},{"diffusion","DIFFUSION",0,100,75,1," %"},
    {"damping","DAMPING",0,100,50,1," %"},{"width","WIDTH",0,200,100,1," %"},
    {"mix","MIX",0,100,35,1," %"},{"input","INPUT",-24,24,0,1," dB"},
    {"output","OUTPUT",-24,24,0,1," dB"},{"highpass","HIGH-PASS",20,2000,120,.3f," Hz"},
    {"lowpass","LOW-PASS",1000,20000,12000,.5f," Hz"},
    {"threshold","THRESHOLD",-60,0,-24,1," dB"},{"ratio","RATIO",1,20,4,.6f," : 1"},
    {"attack","ATTACK",.1f,100,5,.4f," ms"},{"release","RELEASE",20,2000,250,.4f," ms"},
    {"duck","DUCK AMOUNT",0,100,70,1," %"},{"moddepth","MODULATION",0,100,10,1," %"},
    {"modrate","MOD RATE",.01f,5,.3f,.4f," Hz"}
};
constexpr int numContinuous=int(std::size(specs));
struct AnalysisFrame { float dry,wet,l,r,sidechain; };
class ReverbProcessor final : public juce::AudioProcessor {
public:
    ReverbProcessor();
    void prepareToPlay(double,int) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override {return true;}
    const juce::String getName() const override {return "cyber-reverb";}
    bool acceptsMidi() const override {return false;} bool producesMidi() const override {return false;} bool isMidiEffect() const override {return false;}
    double getTailLengthSeconds() const override {return frozen->load()>.5f?3600.0:double(raw[2]->load())+1.0;}
    int getNumPrograms() override;int getCurrentProgram() override {return currentPreset.load();}
    void setCurrentProgram(int) override;const juce::String getProgramName(int) override;void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;void setStateInformation(const void*,int) override;
    juce::AudioProcessorParameter* getBypassParameter() const override {return bypassParam;}
    juce::UndoManager undo;
    juce::AudioProcessorValueTreeState parameters;
    std::array<std::atomic<float>,2> inputMeter{},outputMeter{};
    std::atomic<float> reduction{0},cpu{0};
    std::atomic<bool> analysing{false};
    std::atomic<bool> externalConnected{false};
    juce::AbstractFifo analysisFifo{32768};std::array<AnalysisFrame,32768> analysis{};
    void applyFactory(int);void setParam(const juce::String&,float);
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    void process(juce::AudioBuffer<float>&,bool);
    cyber::Reverb reverb;
    std::array<std::atomic<float>*,numContinuous> raw{};
    std::array<float,numContinuous> smoothed{};
    std::atomic<float> *engine=nullptr,*character=nullptr,*source=nullptr,*frozen=nullptr,*wetOnly=nullptr,*tail=nullptr,*bypass=nullptr;
    juce::AudioProcessorParameter* bypassParam=nullptr;
    double sr=48000;float smoothing=.001f,detector=0,duckGain=1,freezeSmooth=0,vintageSmooth=0,wetSmooth=0,bypassSmooth=0,cutSmooth=1;
    std::array<float,2> hpIn{},hpOut{},lpOut{};
    int controlCounter=0;float hpA=.98f,lpA=.5f,attackA=.99f,releaseA=.999f;
    bool forceBypass=false;
    std::atomic<int> currentPreset{0};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbProcessor)
};
