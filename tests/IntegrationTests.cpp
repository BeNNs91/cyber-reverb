#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include <stdexcept>
#include <limits>
void require(bool ok,const char* m){if(!ok)throw std::runtime_error(m);}
int main(int argc,char** argv){juce::ScopedJuceInitialiser_GUI init;try{
    auto storage=std::make_unique<ReverbProcessor>();auto& p=*storage;p.setParam("mix",0);p.setRateAndBufferSizeDetails(48000,256);p.prepareToPlay(48000,256);juce::AudioBuffer<float> b(2,256);juce::MidiBuffer midi;
    for(int i=0;i<256;++i){b.setSample(0,i,.2f);b.setSample(1,i,-.1f);}p.processBlock(b,midi);
    for(int i=0;i<256;++i){require(std::abs(b.getSample(0,i)-.2f)<1.e-6f,"Dry signal was filtered");require(std::abs(b.getSample(1,i)+.1f)<1.e-6f,"Stereo dry changed");}
    p.setParam("decay",7.3f);p.setParam("engine",2);p.setParam("character",1);p.setParam("wetonly",1);juce::MemoryBlock state;p.getStateInformation(state);auto restoredStorage=std::make_unique<ReverbProcessor>();auto& restored=*restoredStorage;restored.setStateInformation(state.getData(),int(state.getSize()));
    require(std::abs(restored.parameters.getRawParameterValue("decay")->load()-7.3f)<1.e-4f,"State decay recall failed");require(restored.parameters.getRawParameterValue("engine")->load()==2,"State engine recall failed");require(restored.parameters.getRawParameterValue("wetonly")->load()==1,"Wet Only recall failed");
    p.setParam("source",2);p.prepareToPlay(48000,256);b.clear();p.processBlock(b,midi);require(p.reduction.load()<.001f,"Absent sidechain ducks silence");
    auto layout=p.getBusesLayout();layout.inputBuses.getReference(1)=juce::AudioChannelSet::stereo();require(p.setBusesLayout(layout),"Stereo sidechain rejected");
    p.setParam("source",2);p.prepareToPlay(48000,256);b.setSize(4,256);for(int block=0;block<150;++block){b.clear();for(int i=0;i<256;++i){b.setSample(2,i,.8f);b.setSample(3,i,.8f);}p.processBlock(b,midi);}require(p.reduction.load()>6,"External sidechain does not duck");
    require(p.getLatencySamples()==0,"Unexpected latency");
    // Exercise modulation, automation, mono/stereo layouts and irregular host
    // blocks through the complete processor, with the analyser FIFO saturated.
    for(double rate:{44100.,48000.,88200.,96000.,192000.})for(int channels:{1,2}){
        auto host=std::make_unique<ReverbProcessor>();auto buses=host->getBusesLayout();buses.inputBuses.getReference(0)=channels==1?juce::AudioChannelSet::mono():juce::AudioChannelSet::stereo();buses.outputBuses.getReference(0)=buses.inputBuses[0];require(host->setBusesLayout(buses),"Main layout rejected");
        host->setRateAndBufferSizeDetails(rate,1024);host->prepareToPlay(rate,1024);host->analysing.store(true);juce::AudioBuffer<float> audio(channels,1024);juce::Random noise(19);
        for(int block=0;block<240;++block){int samples=block%4==0?1:block%4==1?63:block%4==2?257:1024;audio.setSize(channels,samples,false,false,true);
            if(block%17==0){host->setParam("predelay",block%251);host->setParam("size",block%2?0:100);host->setParam("decay",block%2?.1f:30);host->setParam("engine",float(block%3));host->setParam("freeze",float(block%2));host->setParam("moddepth",100);host->setParam("modrate",5);}
            for(int c=0;c<channels;++c)for(int i=0;i<samples;++i)audio.setSample(c,i,(noise.nextFloat()*2-1)*.1f);
            if(block==50){audio.setSample(0,0,std::numeric_limits<float>::quiet_NaN());if(samples>1)audio.setSample(0,1,std::numeric_limits<float>::infinity());}
            host->processBlock(audio,midi);for(int c=0;c<channels;++c)for(int i=0;i<samples;++i)require(std::isfinite(audio.getSample(c,i)),"Processor sent invalid audio data");
        }
    }
    // Render a real editor snapshot without launching an audio device.
    if(argc>1){auto preview=std::make_unique<ReverbProcessor>();preview->setRateAndBufferSizeDetails(48000,1600);preview->prepareToPlay(48000,1600);ReverbEditor editor(*preview);require(editor.getWidth()==1080&&editor.getHeight()==720,"Editor is not compact by default");
        juce::AudioBuffer<float> signal(2,1600);juce::Random random(42);
        for(int frame=0;frame<120;++frame){for(int i=0;i<1600;++i){int t=frame*1600+i;float burst=t<4800?(random.nextFloat()*2-1)*std::exp(-float(t)/1000)*.8f:0;signal.setSample(0,i,burst);signal.setSample(1,i,burst);}preview->processBlock(signal,midi);editor.refreshMeasurements();}
        editor.savePreview(juce::File(juce::String::fromUTF8(argv[1])));}
    std::cout<<"Dry isolation, stereo, state recall, sidechain, latency, finite-output automation stress at five rates, and editor checks passed.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
