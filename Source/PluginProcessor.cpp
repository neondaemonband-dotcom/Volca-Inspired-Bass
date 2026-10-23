#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <algorithm>
VolcaInspiredAudioProcessor::VolcaInspiredAudioProcessor()
 : AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),apvts(*this,nullptr,"PARAMETERS",makeParams()) {}
juce::AudioProcessorValueTreeState::ParameterLayout VolcaInspiredAudioProcessor::makeParams() {
 std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
 auto f=[&](const char* id,const char* name,float lo,float hi,float def){p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(lo,hi),def));};
 f("pitch1","VCO1 Pitch",-24,24,0);f("pitch2","VCO2 Pitch",-24,24,0);f("pitch3","VCO3 Pitch",-24,24,-12);
 f("wave1","VCO1 Wave",0,1,0);f("wave2","VCO2 Wave",0,1,0);f("wave3","VCO3 Wave",0,1,0);
 f("mix1","VCO1 Level",0,1,0.8f);f("mix2","VCO2 Level",0,1,0.7f);f("mix3","VCO3 Level",0,1,0.5f);
 f("cutoff","VCF Cutoff",30,12000,850);f("peak","VCF Peak",0,0.95f,0.4f);f("cuteg","Cutoff EG Int",-5,5,2.0f);
 f("attack","EG Attack",0.001f,2,0.005f);f("decay","EG Decay",0.01f,4,0.35f);f("release","EG Release",0.01f,4,0.22f);
 f("ampEG","Amp EG On",0,1,1);f("lfoRate","LFO Rate",0.05f,30,3);f("lfoInt","LFO Int",0,1,0);
 f("lfoPitch","LFO to Pitch",0,1,0);f("lfoCutoff","LFO to Cutoff",0,1,0);f("lfoAmp","LFO to Amp",0,1,0);
 f("lfoShape","LFO Shape",0,1,0);f("drive","Drive",0.5f,5,1.4f);f("volume","Master",0,0.8f,0.24f);
 f("assign","VCO Note Mode",0,1,0);
 return {p.begin(),p.end()};
}
void VolcaInspiredAudioProcessor::prepareToPlay(double sr,int) {
 sampleRateHz=std::max(8000.0,sr);phase.fill(0);currentNotes.fill(-1);velocities.fill(0);held.fill(false);amp.fill(0);
 ampStage.fill(Stage::idle);filterStage=Stage::idle;filterEnv=lp1=lp2=lfoPhase=pitchBend=0;
}
bool VolcaInspiredAudioProcessor::isBusesLayoutSupported(const BusesLayout& b) const {
 return b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo() || b.getMainOutputChannelSet()==juce::AudioChannelSet::mono();
}
void VolcaInspiredAudioProcessor::processBlock(juce::AudioBuffer<float>& audio,juce::MidiBuffer& midi) {
 juce::ScopedNoDenormals noDenormals;audio.clear();
 auto val=[&](const char* id){return static_cast<double>(apvts.getRawParameterValue(id)->load());};
 const double pitch[3]={val("pitch1"),val("pitch2"),val("pitch3")};
 const bool square[3]={val("wave1")>=0.5,val("wave2")>=0.5,val("wave3")>=0.5};
 const double mix[3]={val("mix1"),val("mix2"),val("mix3")};
 const double cutoff=val("cutoff"),peak=val("peak"),cuteg=val("cuteg");
 const double attack=val("attack"),decay=val("decay"),release=val("release");
 const bool ampEG=val("ampEG")>=0.5,assign=val("assign")>=0.5;
 const double lfoRate=val("lfoRate"),lfoInt=val("lfoInt"),drive=val("drive"),volume=val("volume");
 const bool lfoPitch=val("lfoPitch")>=0.5,lfoCutoff=val("lfoCutoff")>=0.5,lfoAmp=val("lfoAmp")>=0.5;
 const bool lfoSquare=val("lfoShape")>=0.5;
 const double normalizer=std::max(1.0,mix[0]+mix[1]+mix[2]);
 juce::MidiBuffer::Iterator it(midi);juce::MidiMessage msg;int eventSample=0;bool hasEvent=it.getNextEvent(msg,eventSample);
 auto advance=[&](double& value,Stage& stage) {
  const double time=stage==Stage::attack?attack:(stage==Stage::decay?decay:release);
  const double coeff=std::exp(-1.0/(std::max(0.001,time)*sampleRateHz));
  if(stage==Stage::attack) {value=1.0+(value-1.0)*coeff;if(value>=0.995){value=1;stage=Stage::decay;}}
  else if(stage==Stage::decay) {value*=coeff;if(value<0.00001){value=0;stage=Stage::idle;}}
  else if(stage==Stage::release) {value*=coeff;if(value<0.00001){value=0;stage=Stage::idle;}}
 };
 for(int s=0;s<audio.getNumSamples();++s) {
  while(hasEvent && eventSample<=s) {
   if(msg.isNoteOn()) {
    const int n=msg.getNoteNumber();
    if(assign) {int slot=0;for(int i=0;i<3;++i)if(!held[i]){slot=i;break;}currentNotes[slot]=n;velocities[slot]=msg.getFloatVelocity();held[slot]=true;amp[slot]=0;ampStage[slot]=Stage::attack;}
    else {for(int i=0;i<3;++i){currentNotes[i]=n;velocities[i]=msg.getFloatVelocity();held[i]=true;amp[i]=0;ampStage[i]=Stage::attack;}}
    filterEnv=0;filterStage=Stage::attack;
   } else if(msg.isNoteOff()) {
    for(int i=0;i<3;++i)if(currentNotes[i]==msg.getNoteNumber()){held[i]=false;ampStage[i]=Stage::release;}
    if(!held[0]&&!held[1]&&!held[2])filterStage=Stage::release;
   } else if(msg.isAllNotesOff()||msg.isAllSoundOff()) {
    for(int i=0;i<3;++i){held[i]=false;ampStage[i]=Stage::release;}filterStage=Stage::release;
   } else if(msg.isPitchWheel())pitchBend=(msg.getPitchWheelValue()-8192)/8192.0*2.0;
   hasEvent=it.getNextEvent(msg,eventSample);
  }
  advance(filterEnv,filterStage);
  lfoPhase+=lfoRate/sampleRateHz;lfoPhase-=std::floor(lfoPhase);
  const double lfo=lfoSquare?(lfoPhase<0.5?1.0:-1.0):std::sin(juce::MathConstants<double>::twoPi*lfoPhase);
  double signal=0,active=0;
  for(int i=0;i<3;++i) {
   advance(amp[i],ampStage[i]);
   if(currentNotes[i]<0 || amp[i]<0.00001)continue;
   const double semitones=pitch[i]+pitchBend+(lfoPitch?lfo*lfoInt*2.0:0.0);
   const double freq=juce::MidiMessage::getMidiNoteInHertz(currentNotes[i])*std::pow(2.0,semitones/12.0);
   phase[i]+=std::min(freq,sampleRateHz*0.45)/sampleRateHz;phase[i]-=std::floor(phase[i]);
   const double gain=(ampEG?amp[i]:(held[i]?1.0:amp[i]))*velocities[i];
   signal+=mix[i]*gain*osc(phase[i],square[i]);active=std::max(active,gain);
  }
  signal=std::tanh(signal*drive/normalizer);
  const double modOctaves=cuteg*filterEnv+(lfoCutoff?lfo*lfoInt*3.0:0.0);
  const double fc=juce::jlimit(25.0,sampleRateHz*0.20,cutoff*std::pow(2.0,modOctaves));
  const double a=1.0-std::exp(-juce::MathConstants<double>::twoPi*fc/sampleRateHz);
  const double input=std::tanh(signal-3.4*peak*lp2);
  lp1+=a*(input-lp1);lp2+=a*(lp1-lp2);
  const double tremolo=lfoAmp?juce::jlimit(0.0,1.0,1.0-lfoInt*0.5+lfo*lfoInt*0.5):1.0;
  const float output=static_cast<float>(std::tanh(lp2*1.8)*tremolo*volume);
  for(int ch=0;ch<audio.getNumChannels();++ch)audio.setSample(ch,s,output);
 }
}
void VolcaInspiredAudioProcessor::getStateInformation(juce::MemoryBlock& dest){auto xml=apvts.copyState().createXml();copyXmlToBinary(*xml,dest);}
void VolcaInspiredAudioProcessor::setStateInformation(const void* data,int bytes){auto xml=getXmlFromBinary(data,bytes);if(xml&&xml->hasTagName(apvts.state.getType()))apvts.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessorEditor* VolcaInspiredAudioProcessor::createEditor(){return new VolcaInspiredAudioProcessorEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new VolcaInspiredAudioProcessor();}
