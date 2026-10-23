#include "PluginEditor.h"
VolcaInspiredAudioProcessorEditor::VolcaInspiredAudioProcessorEditor(VolcaInspiredAudioProcessor& p):AudioProcessorEditor(&p),processor(p){
 const char* ids[]={"pitch1","pitch2","pitch3","wave1","wave2","wave3","mix1","mix2","mix3","cutoff","peak","cuteg","attack","decay","release","ampEG","lfoRate","lfoInt","lfoPitch","lfoCutoff","lfoAmp","lfoShape","drive","volume","assign"};
 const char* names[]={"PITCH 1","PITCH 2","PITCH 3","WAVE 1","WAVE 2","WAVE 3","VCO 1","VCO 2","VCO 3","CUTOFF","PEAK","CUTOFF EG","ATTACK","DECAY","RELEASE","AMP EG","LFO RATE","LFO INT","LFO PITCH","LFO VCF","LFO AMP","LFO WAVE","DRIVE","LEVEL","NOTE MODE"};
 for(int i=0;i<25;++i){
  auto slider=std::make_unique<juce::Slider>();slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);slider->setTextBoxStyle(juce::Slider::TextBoxBelow,false,65,18);
  slider->setColour(juce::Slider::rotarySliderFillColourId,juce::Colours::orange);slider->setColour(juce::Slider::thumbColourId,juce::Colours::whitesmoke);addAndMakeVisible(*slider);
  auto label=std::make_unique<juce::Label>();label->setText(names[i],juce::dontSendNotification);label->setJustificationType(juce::Justification::centred);label->setColour(juce::Label::textColourId,juce::Colours::whitesmoke);addAndMakeVisible(*label);
  attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts,ids[i],*slider));sliders.push_back(std::move(slider));labels.push_back(std::move(label));
 }
 setSize(850,690);
}
void VolcaInspiredAudioProcessorEditor::paint(juce::Graphics& g){g.fillAll(juce::Colour::fromRGB(24,26,29));g.setColour(juce::Colours::orange);g.setFont(juce::Font(29.0f,juce::Font::bold));g.drawText("VOLCA ARCHITECTURE BASS",24,12,750,42,juce::Justification::centredLeft);g.setColour(juce::Colours::lightgrey);g.setFont(13.0f);g.drawText("3 VCO / 12 dB LPF / EG / LFO / MIDI",26,55,750,20,juce::Justification::centredLeft);g.setColour(juce::Colour::fromRGB(70,72,75));g.drawLine(20,86,830,86,2.0f);}
void VolcaInspiredAudioProcessorEditor::resized(){for(int i=0;i<25;++i){int col=i%5,row=i/5;int x=20+col*165,y=100+row*112;labels[i]->setBounds(x,y,150,20);sliders[i]->setBounds(x,y+20,150,85);}}
