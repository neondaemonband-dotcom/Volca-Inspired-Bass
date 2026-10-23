#include "PluginEditor.h"
VolcaInspiredAudioProcessorEditor::VolcaInspiredAudioProcessorEditor(VolcaInspiredAudioProcessor& p) : AudioProcessorEditor(&p),processor(p) {
 const char* ids[]={"cutoff","res","attack","decay","sustain","release","eg","drive","mix1","mix2","mix3","detune","octave","wave","volume"};
 const char* names[]={"CUTOFF","PEAK","ATTACK","DECAY","SUSTAIN","RELEASE","EG INT","DRIVE","VCO 1","VCO 2","VCO 3","DETUNE","OCTAVE","WAVE","LEVEL"};
 for(int i=0;i<15;++i) {
  auto slider=std::make_unique<juce::Slider>(); slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); slider->setTextBoxStyle(juce::Slider::TextBoxBelow,false,65,18);
  slider->setColour(juce::Slider::rotarySliderFillColourId,juce::Colours::orange); slider->setColour(juce::Slider::thumbColourId,juce::Colours::whitesmoke);
  addAndMakeVisible(*slider);
  auto label=std::make_unique<juce::Label>(); label->setText(names[i],juce::dontSendNotification); label->setJustificationType(juce::Justification::centred); label->setColour(juce::Label::textColourId,juce::Colours::whitesmoke); addAndMakeVisible(*label);
  attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts,ids[i],*slider));
  sliders.push_back(std::move(slider)); labels.push_back(std::move(label));
 }
 setSize(750,420);
}
void VolcaInspiredAudioProcessorEditor::paint(juce::Graphics& g) {
 g.fillAll(juce::Colour::fromRGB(24,26,29));
 g.setColour(juce::Colours::orange); g.setFont(juce::Font(29.0f,juce::Font::bold)); g.drawText("VOLCA INSPIRED BASS",24,12,550,42,juce::Justification::centredLeft);
 g.setColour(juce::Colours::lightgrey); g.setFont(13.0f); g.drawText("3 VCO  /  RESONANT LPF  /  MONO MIDI SYNTH",26,55,620,20,juce::Justification::centredLeft);
 g.setColour(juce::Colour::fromRGB(70,72,75)); g.drawLine(20,86,730,86,2.0f);
}
void VolcaInspiredAudioProcessorEditor::resized() {
 for(int i=0;i<15;++i) {int col=i%5,row=i/5; int x=25+col*145,y=104+row*99; labels[i]->setBounds(x,y,120,18); sliders[i]->setBounds(x,y+18,120,78);}
}
