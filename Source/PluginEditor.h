#pragma once
#include "PluginProcessor.h"

class CyberLook final:public juce::LookAndFeel_V4 {
public:
    CyberLook();
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    void drawButtonText(juce::Graphics&,juce::TextButton&,bool,bool) override;
    juce::Font getTextButtonFont(juce::TextButton&,int height) override {return juce::Font(juce::FontOptions(float(height)*.36f));}
    juce::Label* createSliderTextBox(juce::Slider& slider) override {auto* label=juce::LookAndFeel_V4::createSliderTextBox(slider);label->setFont(juce::FontOptions(float(slider.getTextBoxHeight())*.58f));return label;}
    juce::Font getComboBoxFont(juce::ComboBox& c) override {return juce::Font(juce::FontOptions(float(c.getHeight())*.43f));}
};
class ReverbEditor final:public juce::AudioProcessorEditor,private juce::Timer {
public:
    explicit ReverbEditor(ReverbProcessor&);
    ~ReverbEditor() override;
    void paint(juce::Graphics&) override;void resized() override;
    void savePreview(const juce::File& f);
    void refreshMeasurements() {timerCallback();}
private:
    ReverbProcessor& processor;CyberLook look;
    using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment=juce::AudioProcessorValueTreeState::ButtonAttachment;
    struct Control {juce::Slider slider;juce::Label label;std::unique_ptr<SliderAttachment> attachment;};
    std::array<Control,numContinuous> controls;
    std::array<juce::TextButton,3> engines;
    std::array<juce::TextButton,2> characters;
    juce::TextButton previous{"<"},next{">"},save{"SAVE"},load{"LOAD"},undoButton{"UNDO"},redoButton{"REDO"},freeze{"FREEZE"},wetOnly{"WET ONLY"},advanced{"ADVANCED  v"},bypass{"BYPASS"};
    juce::ComboBox preset,source,tail;
    std::unique_ptr<ComboAttachment> sourceAttachment,tailAttachment;
    std::unique_ptr<ButtonAttachment> freezeAttachment,wetAttachment,bypassAttachment;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::TooltipWindow tooltip{this,650};
    bool expanded=true;int lastPreset=-1;
    std::array<float,480> dryHistory{},wetHistory{},sidechainHistory{};int historyPos=0,bucketCount=0;float bucketDry=0,bucketWet=0,bucketSidechain=0;
    std::array<cyber::Stereo,768> stereoPoints{};int stereoPos=0;
    juce::dsp::FFT fft{10};juce::dsp::WindowingFunction<float> window{1024,juce::dsp::WindowingFunction<float>::hann};
    std::array<float,1024> fftRing{};std::array<float,2048> fftData{};int fftPos=0;
    juce::Image spectrum{juce::Image::RGB,120,128,true};int spectrumPos=0;
    std::array<float,2> inLevels{},outLevels{};
    float scale=1;
    void timerCallback() override;void choosePreset(bool saving);
    void panel(juce::Graphics&,juce::Rectangle<float>);void drawVisualiser(juce::Graphics&);void drawMeters(juce::Graphics&,float,const std::array<float,2>&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbEditor)
};
