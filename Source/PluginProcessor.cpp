#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace {
struct Preset { const char* name;int engine,character;float pre,size,decay,diff,damp,width,mix,hp,lp,mod; };
constexpr Preset presets[]={
 {"Small Room / Tight",0,0,12,45,.85f,75,50,100,35,120,12000,10},
 {"Room / Vocal Air",0,0,18,62,1.2f,80,35,110,25,180,13500,8},
 {"Room / Vintage Drums",0,1,5,35,.55f,65,40,100,28,160,11000,5},
 {"Hall / Smooth Wide",1,0,28,62,3.4f,88,45,120,35,160,12000,15},
 {"Atmosphere / Wide Horizon",1,0,32,88,9.5f,92,45,155,45,100,12500,22},
 {"Synth / Deep Orbit",1,1,65,95,14,88,70,170,42,200,7000,35},
 {"Springy Plate / Snare",2,0,8,45,1.3f,65,25,100,25,150,15000,8},
 {"Springy Plate / Vintage Gold",2,1,42,62,2.6f,65,48,110,30,180,12500,25},
 {"Springy Plate / Long",2,0,18,75,4.2f,75,30,135,35,160,15000,12},
 {"Room / Reference",0,0,0,50,1.f,75,50,100,35,120,12000,0}
};
}
juce::AudioProcessorValueTreeState::ParameterLayout ReverbProcessor::layout() {
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    for(auto& s:specs) {
        auto range=juce::NormalisableRange<float>(s.lo,s.hi,0.f,s.skew);
        p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{s.id,1},s.name,range,s.def,
            juce::AudioParameterFloatAttributes().withLabel(s.unit).withStringFromValueFunction([s](float v,int){
                if((juce::String(s.id)=="highpass"||juce::String(s.id)=="lowpass")&&v>=1000) return juce::String(v/1000,1)+" kHz";
                int decimals=juce::String(s.id)=="decay"||juce::String(s.id)=="modrate"?2:(juce::String(s.unit).contains("dB")||juce::String(s.id)=="ratio"||juce::String(s.id)=="attack"?1:0);
                return juce::String(v,decimals)+s.unit;
            }).withValueFromStringFunction([](const juce::String& t){return t.getFloatValue()*(t.containsIgnoreCase("khz")?1000.f:1.f);} )));
    }
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"engine",1},"Reverb engine",juce::StringArray{"Small Room","Hall","Springy Plate"},0));
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"character",1},"Character",juce::StringArray{"Modern","Vintage"},0));
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"source",1},"Sidechain source",juce::StringArray{"Off","Internal","External (DAW)"},0));
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"tail",1},"Tail behaviour",juce::StringArray{"Natural","Cut","Tail bypass"},0));
    for(auto pair: {std::pair<const char*,const char*>{"freeze","Freeze"},{"wetonly","Wet Only"},{"bypass","Bypass"}})
        p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{pair.first,1},pair.second,false));
    return p;
}
ReverbProcessor::ReverbProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true).withInput("Sidechain",juce::AudioChannelSet::stereo(),false)),parameters(*this,&undo,"CYBER_REVERB",layout()) {
    for(int i=0;i<numContinuous;++i)raw[i]=parameters.getRawParameterValue(specs[i].id);
    engine=parameters.getRawParameterValue("engine");character=parameters.getRawParameterValue("character");source=parameters.getRawParameterValue("source");frozen=parameters.getRawParameterValue("freeze");wetOnly=parameters.getRawParameterValue("wetonly");tail=parameters.getRawParameterValue("tail");bypass=parameters.getRawParameterValue("bypass");bypassParam=parameters.getParameter("bypass");
}
bool ReverbProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
    auto in=l.getMainInputChannelSet(),out=l.getMainOutputChannelSet();
    if(out!=juce::AudioChannelSet::stereo()&&out!=juce::AudioChannelSet::mono())return false;
    if(in!=out&&!(in==juce::AudioChannelSet::mono()&&out==juce::AudioChannelSet::stereo()))return false;
    if(l.inputBuses.size()>1){auto sc=l.inputBuses[1];if(!sc.isDisabled()&&sc!=juce::AudioChannelSet::mono()&&sc!=juce::AudioChannelSet::stereo())return false;}
    return true;
}
void ReverbProcessor::prepareToPlay(double sampleRate,int) {
    sr=sampleRate;reverb.prepare(sr);smoothing=1-std::exp(-1.f/(float(sr)*.025f));
    for(int i=0;i<numContinuous;++i)smoothed[i]=raw[i]->load();
    hpIn.fill(0);hpOut.fill(0);lpOut.fill(0);detector=0;duckGain=1;freezeSmooth=frozen->load();vintageSmooth=character->load();wetSmooth=wetOnly->load();bypassSmooth=0;cutSmooth=1;controlCounter=0;
    analysisFifo.reset();setLatencySamples(0);
}
void ReverbProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&) {process(b,false);}
void ReverbProcessor::processBlockBypassed(juce::AudioBuffer<float>& b,juce::MidiBuffer&) {process(b,true);}
void ReverbProcessor::process(juce::AudioBuffer<float>& buffer,bool hostBypass) {
    juce::ScopedNoDenormals noDenormals;
    auto start=juce::Time::getHighResolutionTicks();
    auto main=getBusBuffer(buffer,false,0);auto input=getBusBuffer(buffer,true,0);
    auto side=getBusBuffer(buffer,true,1);
    const int n=main.getNumSamples(),inChannels=getChannelCountOfBus(true,0),outs=main.getNumChannels();
    const bool hasSide=getBus(true,1)->isEnabled()&&side.getNumChannels()>0;externalConnected.store(hasSide,std::memory_order_relaxed);
    std::array<float,numContinuous> target{};for(int i=0;i<numContinuous;++i){float value=raw[i]->load(std::memory_order_relaxed);target[i]=std::isfinite(value)?cyber::clamp(value,specs[i].lo,specs[i].hi):specs[i].def;}
    auto safeChoice=[](float value,int maximum){return std::isfinite(value)?int(cyber::clamp(value,0.f,float(maximum))):0;};
    const int alg=safeChoice(engine->load(),2),src=safeChoice(source->load(),2),tailMode=safeChoice(tail->load(),2);
    float fr=float(safeChoice(frozen->load(),1)),vint=float(safeChoice(character->load(),1)),wo=float(safeChoice(wetOnly->load(),1));
    float bp=hostBypass||bypass->load()>.5f||tailMode==2?1.f:0.f;
    std::array<float,2> inPeak{},outPeak{};
    const bool capture=analysing.load(std::memory_order_relaxed);
    int write1=0,size1=0,write2=0,size2=0,written=0;
    if(capture)analysisFifo.prepareToWrite(n,write1,size1,write2,size2);
    for(int s=0;s<n;++s) {
        for(int i=0;i<numContinuous;++i)smoothed[i]+=smoothing*(target[i]-smoothed[i]);
        freezeSmooth+=smoothing*(fr-freezeSmooth);vintageSmooth+=smoothing*(vint-vintageSmooth);wetSmooth+=smoothing*(wo-wetSmooth);bypassSmooth+=smoothing*(bp-bypassSmooth);
        cutSmooth+=smoothing*((tailMode==1?0.f:1.f)-cutSmooth);
        if((controlCounter++&31)==0) {
            cyber::Settings st;st.size=smoothed[1]*.01f;st.decay=smoothed[2];st.diffusion=smoothed[3]*.01f;st.damping=smoothed[4]*.01f;st.modDepth=smoothed[16]*.01f;st.modRate=smoothed[17];st.vintage=vintageSmooth;st.freeze=freezeSmooth;
            reverb.update(st,smoothed[0]);
            hpA=std::exp(-2*cyber::pi*std::min(smoothed[9],float(sr)*.45f)/float(sr));
            lpA=1-std::exp(-2*cyber::pi*std::min(smoothed[10],float(sr)*.45f)/float(sr));
            attackA=std::exp(-1.f/(float(sr)*smoothed[13]*.001f));releaseA=std::exp(-1.f/(float(sr)*smoothed[14]*.001f));
        }
        // Read sidechain before writing output: host bus buffers can alias.
        float detected=0;
        if(src==2&&hasSide)for(int c=0;c<side.getNumChannels();++c)detected=std::max(detected,std::abs(side.getSample(c,s)));
        float gain=cyber::dbGain(smoothed[7]);
        cyber::Stereo dry{input.getSample(0,s)*gain,input.getSample(inChannels>1?1:0,s)*gain};
        if(!std::isfinite(dry.l))dry.l=0;if(!std::isfinite(dry.r))dry.r=0;
        if(src==1)detected=std::max(std::abs(dry.l),std::abs(dry.r));
        if(!std::isfinite(detected))detected=0;
        float coeff=detected>detector?attackA:releaseA;detector=coeff*detector+(1-coeff)*detected;
        float over=std::max(0.f,20*std::log10(std::max(detector,1.e-9f))-smoothed[11]);
        float gr=(src==0?0.f:over*(1-1/smoothed[12])*smoothed[15]*.01f);
        float desired=cyber::dbGain(-gr);duckGain+=smoothing*(desired-duckGain);
        cyber::Stereo excite{dry.l*(1-bypassSmooth),dry.r*(1-bypassSmooth)};
        auto wet=reverb.tick(excite,alg);
        // Recover from a non-finite tank value instead of contaminating every
        // following block. clear() only fills already allocated delay storage.
        if(!std::isfinite(wet.l)||!std::isfinite(wet.r)){reverb.clear();wet={};hpIn.fill(0);hpOut.fill(0);lpOut.fill(0);}
        std::array<float,2> w{wet.l,wet.r};
        for(int c=0;c<2;++c){float h=hpA*(hpOut[c]+w[c]-hpIn[c]);hpIn[c]=w[c];hpOut[c]=h;lpOut[c]+=lpA*(h-lpOut[c]);w[c]=lpOut[c]*duckGain*cutSmooth;}
        float mid=(w[0]+w[1])*.5f,sideSignal=(w[0]-w[1])*.5f*smoothed[5]*.01f;
        wet={mid+sideSignal,mid-sideSignal};
        float mix=smoothed[6]*.01f*(1-wetSmooth)+wetSmooth;
        float dg=(1-mix)*(1-bypassSmooth)+bypassSmooth;
        float wg=mix;float outGain=cyber::dbGain(smoothed[8]);
        std::array<float,2> out{(dry.l*dg+wet.l*wg)*outGain,(dry.r*dg+wet.r*wg)*outGain};
        for(auto& value:out)if(!std::isfinite(value))value=0;
        if(outs==1)main.setSample(0,s,(out[0]+out[1])*.5f);else{main.setSample(0,s,out[0]);main.setSample(1,s,out[1]);}
        inPeak[0]=std::max(inPeak[0],std::abs(dry.l));inPeak[1]=std::max(inPeak[1],std::abs(dry.r));
        for(int c=0;c<2;++c)outPeak[c]=std::max(outPeak[c],std::abs(out[c]));
        if(capture&&s<size1+size2) {int index=s<size1?write1+s:write2+s-size1;analysis[size_t(index)]={std::abs(dry.l)>std::abs(dry.r)?dry.l:dry.r,wet.l,wet.l,wet.r,src==1?detector:0.f};++written;}
    }
    if(written)analysisFifo.finishedWrite(written);
    for(int c=0;c<2;++c){
        auto publish=[](std::atomic<float>& meter,float value){float old=meter.load(std::memory_order_relaxed);while(old<value&&!meter.compare_exchange_weak(old,value,std::memory_order_relaxed)){};};
        publish(inputMeter[c],inPeak[c]);publish(outputMeter[c],outPeak[c]);
    }
    reduction.store(-20*std::log10(std::max(duckGain,1.e-9f)));
    if(n>0){double elapsed=juce::Time::highResolutionTicksToSeconds(juce::Time::getHighResolutionTicks()-start);cpu.store(cpu.load()*.9f+float(elapsed*sr/double(n)*100)*.1f);}
}
int ReverbProcessor::getNumPrograms(){return int(std::size(presets));}
const juce::String ReverbProcessor::getProgramName(int i){return presets[juce::jlimit(0,getNumPrograms()-1,i)].name;}
void ReverbProcessor::setCurrentProgram(int i){applyFactory(i);}
void ReverbProcessor::setParam(const juce::String& id,float value){if(auto* p=parameters.getParameter(id)){p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(value));p->endChangeGesture();}}
void ReverbProcessor::applyFactory(int index){
    index=juce::jlimit(0,getNumPrograms()-1,index);const auto& p=presets[index];undo.beginNewTransaction("Load preset");
    for(const auto& s:specs)setParam(s.id,s.def);
    setParam("engine",float(p.engine));setParam("character",float(p.character));
    const float values[]={p.pre,p.size,p.decay,p.diff,p.damp,p.width,p.mix,0,0,p.hp,p.lp};
    for(int i=0;i<11;++i)setParam(specs[i].id,values[i]);setParam("moddepth",p.mod);
    setParam("freeze",0);setParam("wetonly",0);setParam("tail",0);setParam("source",0);setParam("bypass",0);
    currentPreset.store(index);
}
void ReverbProcessor::getStateInformation(juce::MemoryBlock& b){auto state=parameters.copyState();state.setProperty("preset",currentPreset.load(),nullptr);state.setProperty("engineSchema",2,nullptr);if(auto xml=state.createXml())copyXmlToBinary(*xml,b);}
void ReverbProcessor::setStateInformation(const void* d,int n){if(auto xml=getXmlFromBinary(d,n))if(xml->hasTagName(parameters.state.getType())){auto state=juce::ValueTree::fromXml(*xml);
    // Old Hall and Large Hall sessions both load the new Hall. Plate remains slot 2.
    if(int(state.getProperty("engineSchema",1))<2)for(auto child:state)if(child.getProperty("id").toString()=="engine"&&float(child.getProperty("value"))<1.5f)child.setProperty("value",1.f,nullptr);
    parameters.replaceState(state);currentPreset.store(juce::jlimit(0,getNumPrograms()-1,int(state.getProperty("preset",0))));}}
juce::AudioProcessorEditor* ReverbProcessor::createEditor(){return new ReverbEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new ReverbProcessor();}
