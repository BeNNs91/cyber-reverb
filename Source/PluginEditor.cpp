#include "PluginEditor.h"
namespace {
const juce::Colour cyan{0xff00e4f3},text{0xffc6e9f7},orange{0xffffaa46},edge{0xff305362};
void label(juce::Graphics& g,juce::String s,float x,float y,float w,float h,float fs=17,juce::Justification j=juce::Justification::centred){g.setColour(text);g.setFont(juce::FontOptions(fs));g.drawText(s,juce::Rectangle<float>{x,y,w,h},j);}
}
CyberLook::CyberLook(){
    setColour(juce::Slider::textBoxTextColourId,text);setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
    setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff08151e));setColour(juce::ComboBox::outlineColourId,edge);setColour(juce::ComboBox::textColourId,text);setColour(juce::ComboBox::arrowColourId,text);
    setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff0b1c27));setColour(juce::PopupMenu::textColourId,text);setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff126070));
    setColour(juce::TextButton::textColourOffId,text);setColour(juce::TextButton::textColourOnId,cyan);
}
void CyberLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float value,float start,float end,juce::Slider&){
    auto box=juce::Rectangle<float>(float(x),float(y),float(w),float(h)).reduced(5);
    float r=std::min(box.getWidth(),box.getHeight())*.46f,cx=box.getCentreX(),cy=box.getCentreY(),angle=start+value*(end-start);
    g.setColour(juce::Colour(0xff02080b));g.fillEllipse(cx-r-4,cy-r-1,2*r+8,2*r+8);
    g.setColour(edge);g.drawEllipse(cx-r-3,cy-r-3,2*r+6,2*r+6,1);
    juce::Path track;track.addCentredArc(cx,cy,r,r,0,start,end,true);g.setColour(juce::Colour(0xff173945));g.strokePath(track,juce::PathStrokeType(4));
    juce::Path arc;arc.addCentredArc(cx,cy,r,r,0,start,angle,true);
    g.setColour(cyan.withAlpha(.10f));g.strokePath(arc,juce::PathStrokeType(13));g.setColour(cyan.withAlpha(.30f));g.strokePath(arc,juce::PathStrokeType(8));g.setColour(cyan);g.strokePath(arc,juce::PathStrokeType(4));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff152832),cx,cy-r,juce::Colour(0xff030b10),cx,cy+r,false));g.fillEllipse(cx-r+6,cy-r+6,2*r-12,2*r-12);
    g.setColour(juce::Colour(0xff29404a));g.drawEllipse(cx-r+6,cy-r+6,2*r-12,2*r-12,1.5f);
    float sn=std::sin(angle),cs=-std::cos(angle);g.setColour(juce::Colour(0xffedfaff));g.drawLine(cx+sn*r*.64f,cy+cs*r*.64f,cx+sn*r*.9f,cy+cs*r*.9f,4);
}
void CyberLook::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool hover,bool down){
    auto r=b.getLocalBounds().toFloat().reduced(1.5f);auto c=b.getName()=="Vintage"?orange:cyan;bool on=b.getToggleState();
    g.setGradientFill(juce::ColourGradient((on?c.withAlpha(.23f):juce::Colour(0xff132630)),0,0,juce::Colour(0xff040c11),0,r.getBottom(),false));g.fillRoundedRectangle(r,7);
    if(on||hover){g.setColour(c.withAlpha(.08f));g.drawRoundedRectangle(r.reduced(2),6,8);}
    g.setColour(on?c:(hover||down?text:edge));g.drawRoundedRectangle(r,7,on?2.f:1.f);
    auto name=b.getButtonText();float sx=float(b.getWidth())/220,sy=float(b.getHeight())/49;
    if(name=="HALL"||name=="SMALL ROOM"){
        g.setColour(on?cyan:text);for(int i=0;i<4;++i){float radius=(19.f-float(i)*4)*sy;float cx=66*sx,cy=29*sy;juce::Path arch;arch.addCentredArc(cx,cy,radius,radius,0,-juce::MathConstants<float>::halfPi,juce::MathConstants<float>::halfPi,true);g.strokePath(arch,juce::PathStrokeType(1.5f*sy));g.drawLine(cx-radius,cy,cx-radius,40*sy,1.5f*sy);g.drawLine(cx+radius,cy,cx+radius,40*sy,1.5f*sy);}g.drawLine(44*sx,40*sy,88*sx,40*sy,1.5f*sy);
    }else if(name=="SPRINGY PLATE"){
        g.setColour(on?cyan:text);for(int i=0;i<7;++i)g.drawLine((45+float(i)*5)*sx,9*sy,(45+float(i)*5)*sx,39*sy,1.5f*sy);
    }else if(name=="FREEZE"){
        float cx=25*float(b.getWidth())/158,cy=float(b.getHeight())*.5f,rad=float(b.getHeight())*.22f;g.setColour(on?cyan:text);
        for(int i=0;i<6;++i){float a=float(i)*juce::MathConstants<float>::pi/3;auto point=[&](float radius,float offset){return juce::Point<float>(cx+std::sin(a+offset)*radius,cy+std::cos(a+offset)*radius);};g.drawLine({point(0,0),point(rad,0)},1.6f);g.drawLine({point(rad*.6f,0),point(rad*.9f,.25f)},1.3f);g.drawLine({point(rad*.6f,0),point(rad*.9f,-.25f)},1.3f);}
    }
}
void CyberLook::drawButtonText(juce::Graphics& g,juce::TextButton& b,bool hover,bool down){
    auto name=b.getButtonText();bool engine=name=="HALL"||name=="SMALL ROOM"||name=="SPRINGY PLATE";
    if(engine||name=="FREEZE"){
        auto area=b.getLocalBounds().toFloat();area.removeFromLeft(area.getWidth()*(engine?.40f:.25f));area.removeFromRight(7);
        g.setFont(getTextButtonFont(b,b.getHeight()));g.setColour(b.getToggleState()?cyan:text);g.drawText(name,area,juce::Justification::centred);return;
    }
    juce::LookAndFeel_V4::drawButtonText(g,b,hover,down);
}
ReverbEditor::ReverbEditor(ReverbProcessor& p):AudioProcessorEditor(p),processor(p){
    setLookAndFeel(&look);setResizable(true,true);setResizeLimits(900,600,1536,1024);getConstrainer()->setFixedAspectRatio(1.5);setSize(900,600);
    for(int i=0;i<numContinuous;++i){auto& c=controls[i];const auto& s=specs[i];
        c.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);c.slider.setRotaryParameters(juce::MathConstants<float>::pi*1.25f,juce::MathConstants<float>::pi*2.75f,true);
        c.slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,110,26);c.slider.setName(s.name);c.slider.setTooltip(juce::String(s.name)+" — drag to adjust; double-click resets. Shift-drag for fine adjustment.");
        c.slider.setDoubleClickReturnValue(true,s.def);c.slider.setVelocityModeParameters(1,1,.05,true);addAndMakeVisible(c.slider);
        c.label.setText(s.name,juce::dontSendNotification);c.label.setJustificationType(juce::Justification::centred);c.label.setColour(juce::Label::textColourId,text);addAndMakeVisible(c.label);
        c.attachment=std::make_unique<SliderAttachment>(p.parameters,s.id,c.slider);
    }
    const char* names[]={"SMALL ROOM","HALL","SPRINGY PLATE"};for(int i=0;i<3;++i){engines[i].setButtonText(names[i]);engines[i].onClick=[this,i]{processor.setParam("engine",float(i));};addAndMakeVisible(engines[i]);}
    for(int i=0;i<2;++i){characters[i].setButtonText(i==0?"MODERN":"VINTAGE");characters[i].setName(i==0?"Modern":"Vintage");characters[i].onClick=[this,i]{processor.setParam("character",float(i));};addAndMakeVisible(characters[i]);}characters[1].setColour(juce::TextButton::textColourOnId,orange);
    for(auto* b:{&previous,&next,&save,&load,&undoButton,&redoButton,&freeze,&wetOnly,&advanced,&bypass})addAndMakeVisible(b);
    for(auto* b:{&freeze,&wetOnly,&bypass})b->setClickingTogglesState(true);
    freeze.setTooltip("Sustain the current tail; incoming excitation is faded out. Disengage to resume natural decay.");wetOnly.setTooltip("100% wet for sends. Your Mix setting is preserved.");bypass.setTooltip("Natural-tail bypass: dry signal passes while the existing reverb fades naturally.");
    freezeAttachment=std::make_unique<ButtonAttachment>(p.parameters,"freeze",freeze);wetAttachment=std::make_unique<ButtonAttachment>(p.parameters,"wetonly",wetOnly);bypassAttachment=std::make_unique<ButtonAttachment>(p.parameters,"bypass",bypass);
    for(int i=0;i<p.getNumPrograms();++i)preset.addItem(p.getProgramName(i),i+1);addAndMakeVisible(preset);
    preset.onChange=[this]{if(preset.getSelectedId()>0){processor.applyFactory(preset.getSelectedId()-1);lastPreset=processor.getCurrentProgram();}};
    previous.onClick=[this]{processor.applyFactory((processor.getCurrentProgram()+processor.getNumPrograms()-1)%processor.getNumPrograms());};next.onClick=[this]{processor.applyFactory((processor.getCurrentProgram()+1)%processor.getNumPrograms());};
    save.onClick=[this]{choosePreset(true);};load.onClick=[this]{choosePreset(false);};
    undoButton.onClick=[this]{processor.undo.undo();};redoButton.onClick=[this]{processor.undo.redo();};
    source.addItemList({"Off","Internal","External (DAW)"},1);tail.addItemList({"Natural","Cut","Tail bypass"},1);addAndMakeVisible(source);addAndMakeVisible(tail);
    sourceAttachment=std::make_unique<ComboAttachment>(p.parameters,"source",source);tailAttachment=std::make_unique<ComboAttachment>(p.parameters,"tail",tail);
    source.setTooltip("External uses the DAW sidechain bus; absent input produces no ducking.");tail.setTooltip("Cut fades the wet output to silence while selected. Tail bypass passes dry audio and preserves the existing tail.");
    advanced.onClick=[this]{expanded=!expanded;advanced.setButtonText(expanded?"ADVANCED  v":"ADVANCED  >");resized();repaint();};
    spectrum.clear(spectrum.getBounds(),juce::Colour(0xff030b14));
    processor.analysing.store(true);resized();timerCallback();startTimerHz(30);
}
ReverbEditor::~ReverbEditor(){stopTimer();processor.analysing.store(false);setLookAndFeel(nullptr);}
void ReverbEditor::choosePreset(bool saving){
    auto folder=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Cyber Audio/Presets/Reverb");folder.createDirectory();
    chooser=std::make_unique<juce::FileChooser>(saving?"Save CYBER-REVERB preset":"Load CYBER-REVERB preset",folder.getChildFile("My Reverb.cyberreverb"),"*.cyberreverb");
    auto flags=juce::FileBrowserComponent::canSelectFiles|(saving?juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::warnAboutOverwriting:juce::FileBrowserComponent::openMode);
    chooser->launchAsync(flags,[safe=juce::Component::SafePointer<ReverbEditor>(this),saving](const juce::FileChooser& fc){if(!safe)return;auto f=fc.getResult();if(f==juce::File{})return;
        if(saving){juce::MemoryBlock data;safe->processor.getStateInformation(data);if(!f.withFileExtension("cyberreverb").replaceWithData(data.getData(),data.getSize()))juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Preset","Could not save the preset.");}
        else {juce::MemoryBlock data;if(f.loadFileAsData(data)){safe->processor.setStateInformation(data.getData(),int(data.getSize()));safe->preset.setText(f.getFileNameWithoutExtension(),juce::dontSendNotification);safe->lastPreset=safe->processor.getCurrentProgram();}}
    });
}
void ReverbEditor::resized(){
    scale=float(getWidth())/1536.f;
    auto bounds=[this](juce::Component& c,float x,float y,float w,float h){c.setBounds(juce::Rectangle<float>(x*scale,y*scale,w*scale,h*scale).toNearestInt());};
    bounds(previous,576,17,44,42);bounds(preset,630,17,302,42);bounds(next,942,17,44,42);bounds(save,997,17,65,42);bounds(load,1072,17,65,42);bounds(undoButton,1147,17,65,42);bounds(redoButton,1222,17,65,42);
    for(int i=0;i<3;++i)bounds(engines[i],251.f+float(i)*245,93,220,49);
    bounds(characters[0],1141,95,163,46);bounds(characters[1],1313,95,163,46);
    const float mainX[]={99,282,463,643,823,1001,1177};
    for(int i=0;i<7;++i){bounds(controls[i].label,mainX[i]-34,541,174,28);bounds(controls[i].slider,mainX[i],574,108,134);}
    bounds(controls[7].label,31,174,104,28);bounds(controls[7].slider,43,205,82,97);
    bounds(controls[8].label,1397,174,104,28);bounds(controls[8].slider,1409,205,82,97);
    bounds(freeze,1353,548,158,65);bounds(wetOnly,1353,640,158,46);
    for(int i=9;i<11;++i){float x=i==9?105.f:260.f;bounds(controls[i].label,x-27,786,135,27);bounds(controls[i].slider,x,817,82,111);}
    bounds(source,860,739,175,31);
    for(int i=11;i<16;++i){float x=477+float(i-11)*116;bounds(controls[i].label,x-26,786,125,27);bounds(controls[i].slider,x,817,78,111);}
    bounds(advanced,1078,735,185,38);
    for(int i=16;i<18;++i){float x=1104+float(i-16)*124;bounds(controls[i].label,x-22,786,120,27);bounds(controls[i].slider,x,817,78,111);controls[i].slider.setVisible(expanded);controls[i].label.setVisible(expanded);}
    bounds(tail,1359,837,146,33);tail.setVisible(expanded);bounds(bypass,1367,967,137,33);
    for(auto& c:controls){c.label.setFont(juce::FontOptions(17*scale));c.slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,int(125*scale),int(29*scale));}
}
void ReverbEditor::panel(juce::Graphics& g,juce::Rectangle<float> r){
    juce::Path p;float x=r.getX(),y=r.getY(),w=r.getWidth(),h=r.getHeight(),c=12;
    p.startNewSubPath(x+c,y);p.lineTo(x+w-c,y);p.lineTo(x+w,y+c);p.lineTo(x+w,y+h-c);p.lineTo(x+w-c,y+h);p.lineTo(x+c,y+h);p.lineTo(x,y+h-c);p.lineTo(x,y+c);p.closeSubPath();
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff0b1a23),x,y,juce::Colour(0xff030b10),x+w*.5f,y+h,false));g.fillPath(p);g.setColour(edge);g.strokePath(p,juce::PathStrokeType(1));
    g.setColour(cyan.withAlpha(.25f));g.drawLine(x+c,y+h,x+w-c,y+h,1);
}
void ReverbEditor::paint(juce::Graphics& g){
    g.fillAll(juce::Colour(0xff030a10));g.addTransform(juce::AffineTransform::scale(scale));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff10222c),760,0,juce::Colour(0xff03090e),760,1024,false));g.fillRect(0,0,1536,1024);
    g.setColour(cyan);juce::Path logo;logo.startNewSubPath(69,14);logo.lineTo(89,25);logo.lineTo(89,48);logo.lineTo(69,60);logo.lineTo(49,49);logo.lineTo(49,26);logo.closeSubPath();g.strokePath(logo,juce::PathStrokeType(5));label(g,"C",52,20,34,35,30);
    g.setFont(juce::FontOptions("Bahnschrift",44.f,juce::Font::bold));g.setColour(juce::Colour(0xfff0faff));g.drawText("CYBER-",123,14,200,47,juce::Justification::centredLeft);g.setColour(cyan);g.drawText("REVERB",318,14,245,47,juce::Justification::centredLeft);
    label(g,"CYBER AUDIO",1300,23,214,25,18);
    g.setColour(edge);g.drawLine(0,74,1536,74);panel(g,{14,84,1508,72});
    label(g,"REVERB ENGINE",60,98,175,40,18);label(g,"CHARACTER",1000,98,125,40,17);
    panel(g,{14,167,140,349});panel(g,{166,167,1204,349});panel(g,{1382,167,140,349});
    drawMeters(g,44,inLevels);drawMeters(g,1410,outLevels);drawVisualiser(g);
    panel(g,{14,531,1316,186});panel(g,{1341,531,181,186});
    for(float x:{250.f,433.f,613.f,793.f,973.f,1148.f}){g.setColour(edge.withAlpha(.65f));g.drawLine(x,562,x,686);}
    panel(g,{14,732,404,206});panel(g,{431,732,621,206});panel(g,{1064,732,458,206});
    label(g,"/  FILTERS (WET)",29,739,320,33,18,juce::Justification::centredLeft);label(g,"SIDECHAIN / DUCKING",491,739,275,33,18,juce::Justification::centredLeft);label(g,"SOURCE",774,743,72,25,13);
    g.setColour(processor.reduction.load()>.5f?cyan:edge);g.fillRoundedRectangle(448,745,28,21,4);label(g,"D",448,745,28,21,14);
    if(expanded)label(g,"TAIL BEHAVIOUR",1353,796,161,26,14);else label(g,"Subtle movement. Open to adjust.",1085,812,417,46,17);
    g.setColour(edge);for(auto r:{juce::Rectangle<float>{30,773,372,1},{447,773,589,1},{1080,773,426,1}})g.fillRect(r);
    for(float x:{222.f,1203.f,1335.f}){g.setColour(edge.withAlpha(.6f));g.drawLine(x,797,x,916);}
    // Lightweight response symbols reflect the wet filter cutoff position.
    for(int i=0;i<2;++i){float x=i==0?41.f:368.f;juce::Path curve;float v=controls[i+9].slider.valueToProportionOfLength(controls[i+9].slider.getValue());curve.startNewSubPath(x,864);if(i==0)curve.cubicTo(x+9,864,x+8+v*10,842,x+34,844);else curve.cubicTo(x+26,842,x+23-v*10,864,x+34,864);g.setColour(cyan);g.strokePath(curve,juce::PathStrokeType(2.5f));}
    g.setColour(edge);g.drawLine(0,954,1536,954);label(g,"C",34,965,45,40,33);label(g,"CYBER AUDIO",89,974,220,25,18,juce::Justification::centredLeft);
    const char* e[]={"SMALL ROOM","HALL","SPRINGY PLATE"};label(g,e[juce::jlimit(0,2,int(processor.parameters.getRawParameterValue("engine")->load()))],1008,974,140,25,14);
    label(g,processor.parameters.getRawParameterValue("character")->load()>.5f?"VINTAGE":"MODERN",1150,974,110,25,16);
    label(g,juce::String(processor.cpu.load(),1)+"% CPU",1261,974,100,25,14);
    if(processor.parameters.getRawParameterValue("source")->load()>1.5f&&!processor.externalConnected.load())label(g,"External sidechain not connected",500,935,530,20,12);
}
void ReverbEditor::drawMeters(juce::Graphics& g,float x,const std::array<float,2>& levels){
    for(int c=0;c<2;++c){float db=juce::Decibels::gainToDecibels(levels[c],-60.f);for(int i=0;i<14;++i){float y=462-float(i)*11;bool on=db>=-48+float(i)*4;g.setColour(on?(i>=12?orange:cyan):juce::Colour(0xff102630));g.fillRect(x+float(c)*28,y,16.f,8.f);if(on){g.setColour(cyan.withAlpha(.08f));g.fillRect(x+float(c)*28-2,y-2,20.f,12.f);}}label(g,c==0?"L":"R",x+float(c)*28-5,480,26,22,14);}
    for(int i=0;i<7;++i)label(g,juce::String(-i*8),x+58,310+float(i)*24,30,20,13);
}
void ReverbEditor::drawVisualiser(juce::Graphics& g){
    label(g,"REVERB RESPONSE (PROCESSED)",252,184,555,30,17,juce::Justification::centredLeft);
    g.setColour(orange);g.fillEllipse(946,194,11,11);label(g,"DRY (INPUT)",968,185,125,30,15,juce::Justification::centredLeft);
    g.setColour(cyan);g.fillEllipse(1113,194,11,11);label(g,"WET (REVERB TAIL)",1135,185,210,30,15,juce::Justification::centredLeft);
    juce::Rectangle<float> plot{237,229,878,240};g.setColour(juce::Colour(0xff020a10));g.fillRect(plot);
    // A real FFT waterfall, newest measurements on the right, four seconds wide.
    g.setOpacity(.85f);
    int first=120-spectrumPos;
    g.drawImage(spectrum,237,229,int(878.f*float(first)/120),240,spectrumPos,0,first,128);
    if(spectrumPos>0)g.drawImage(spectrum,237+int(878.f*float(first)/120),229,int(878.f*float(spectrumPos)/120)+1,240,0,0,spectrumPos,128);
    g.setOpacity(1);
    const char* freq[]={"20k","10k","5k","2k","1k","500","200","100","50"};const float hz[]={20000,10000,5000,2000,1000,500,200,100,50};
    for(int i=0;i<9;++i){float y=229+240*std::log(20000/hz[i])/std::log(1000.f);label(g,freq[i],182,y-10,43,20,14);g.setColour(juce::Colour(0xff356070).withAlpha(.25f));g.drawLine(237,y,1115,y);}
    for(int i=0;i<=8;++i){float x=237+float(i)*109.75f;g.setColour(edge.withAlpha(.23f));g.drawLine(x,229,x,469);label(g,juce::String(-4+float(i)*.5f,1)+" s",x-25,479,60,22,14);}
    for(int type=0;type<2;++type){juce::Path p;auto& hist=type==0?dryHistory:wetHistory;
        for(int i=0;i<480;++i){float x=237+float(i)*878/479;float amp=std::min(100.f,std::sqrt(hist[size_t((historyPos+i)%480)])*85);if(i==0)p.startNewSubPath(x,326-amp);else p.lineTo(x,326-amp);}
        for(int i=479;i>=0;--i){float x=237+float(i)*878/479;float amp=std::min(100.f,std::sqrt(hist[size_t((historyPos+i)%480)])*85);p.lineTo(x,326+amp);}p.closeSubPath();
        auto c=type==0?orange:cyan;g.setColour(c.withAlpha(.23f));g.fillPath(p);g.setColour(c.withAlpha(.8f));g.strokePath(p,juce::PathStrokeType(.8f));
    }
    if(processor.parameters.getRawParameterValue("source")->load()==1.f){
        juce::Path trace;for(int i=0;i<480;++i){float x=237+float(i)*878/479;float envelope=sidechainHistory[size_t((historyPos+i)%480)];float y=459-std::min(60.f,std::sqrt(envelope)*60);if(i==0)trace.startNewSubPath(x,y);else trace.lineTo(x,y);}
        g.setColour(juce::Colours::white.withAlpha(.9f));g.strokePath(trace,juce::PathStrokeType(1.1f));label(g,"SIDECHAIN",972,445,135,19,11);
    }
    panel(g,{1133,227,221,246});label(g,"STEREO FIELD",1150,233,187,32,17);
    float cx=1244,cy=350,r=77;g.setColour(edge);g.drawEllipse(cx-r,cy-r,2*r,2*r,1);g.drawLine(cx-r-4,cy,cx+r+4,cy);g.drawLine(cx,cy-r-4,cx,cy+r+4);
    g.setColour(cyan.withAlpha(.5f));for(auto v:stereoPoints){float side=(v.l-v.r)*.707f,mid=(v.l+v.r)*.707f;float px=cyber::clamp(side*95,-73,73),py=cyber::clamp(mid*95,-73,73);if(std::abs(px)+std::abs(py)>.1f)g.fillEllipse(cx+px,cy-py,1.6f,1.6f);}
    label(g,"L",1142,338,20,22,14);label(g,"R",1325,338,20,22,14);label(g,"WIDTH  "+juce::String(controls[5].slider.getValue(),0)+"%",1164,437,170,26,17);
}
void ReverbEditor::timerCallback(){
    if(lastPreset!=processor.getCurrentProgram()){lastPreset=processor.getCurrentProgram();preset.setSelectedId(lastPreset+1,juce::dontSendNotification);}
    int engine=int(processor.parameters.getRawParameterValue("engine")->load()),character=int(processor.parameters.getRawParameterValue("character")->load());
    for(int i=0;i<3;++i)engines[i].setToggleState(i==engine,juce::dontSendNotification);for(int i=0;i<2;++i)characters[i].setToggleState(i==character,juce::dontSendNotification);
    undoButton.setEnabled(processor.undo.canUndo());redoButton.setEnabled(processor.undo.canRedo());
    int a,n,b,m;processor.analysisFifo.prepareToRead(processor.analysisFifo.getNumReady(),a,n,b,m);
    int count=0;auto consume=[&](int start,int length){for(int i=0;i<length;++i){auto frame=processor.analysis[size_t(start+i)];
        bucketDry=std::max(bucketDry,std::abs(frame.dry));bucketWet=std::max(bucketWet,std::max(std::abs(frame.l),std::abs(frame.r)));
        bucketSidechain=std::max(bucketSidechain,frame.sidechain);
        if(++bucketCount>=std::max(1,int(processor.getSampleRate()/120))){dryHistory[historyPos]=bucketDry;wetHistory[historyPos]=bucketWet;sidechainHistory[historyPos]=bucketSidechain;historyPos=(historyPos+1)%480;bucketCount=0;bucketDry=bucketWet=bucketSidechain=0;}
        fftRing[fftPos]=frame.wet;fftPos=(fftPos+1)%1024;
        if((count++&15)==0){stereoPoints[stereoPos]={frame.l,frame.r};stereoPos=(stereoPos+1)%768;}
    }};consume(a,n);consume(b,m);processor.analysisFifo.finishedRead(n+m);
    if(n+m>0){for(int i=0;i<1024;++i)fftData[i]=fftRing[(fftPos+i)%1024];std::fill(fftData.begin()+1024,fftData.end(),0.f);window.multiplyWithWindowingTable(fftData.data(),1024);fft.performFrequencyOnlyForwardTransform(fftData.data());
        for(int y=0;y<128;++y){float hz=20000*std::pow(20.f/20000,float(y)/127);float bin=hz*1024/float(std::max(44100.0,processor.getSampleRate()));int k=juce::jlimit(1,511,int(bin));float energy=fftData[k]/256;float intensity=juce::jlimit(0.f,1.f,(juce::Decibels::gainToDecibels(energy,-90.f)+85)/70);
            auto c=juce::Colour(0xff030b14).interpolatedWith(juce::Colour(0xff6324d2),intensity*.7f).interpolatedWith(cyan,std::pow(intensity,3));spectrum.setPixelAt(spectrumPos,y,c);}
        spectrumPos=(spectrumPos+1)%120;
    }else{for(int y=0;y<128;++y)spectrum.setPixelAt(spectrumPos,y,juce::Colour(0xff030b14));spectrumPos=(spectrumPos+1)%120;}
    for(int c=0;c<2;++c){inLevels[c]=std::max(processor.inputMeter[c].exchange(0),inLevels[c]*.86f);outLevels[c]=std::max(processor.outputMeter[c].exchange(0),outLevels[c]*.86f);}
    repaint();
}
void ReverbEditor::savePreview(const juce::File& f){timerCallback();auto image=createComponentSnapshot(getLocalBounds(),true,1);juce::MemoryOutputStream stream;juce::PNGImageFormat().writeImageToStream(image,stream);f.replaceWithData(stream.getData(),stream.getDataSize());}
