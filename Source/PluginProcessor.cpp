#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
VolcaInspiredAudioProcessor::VolcaInspiredAudioProcessor()
 : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)), apvts(*this,nullptr,"PARAMETERS",makeParams()) {}
juce::AudioProcessorValueTreeState::ParameterLayout VolcaInspiredAudioProcessor::makeParams() {
 std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
 auto f=[&](const char* id,const char* name,float lo,float hi,float def){p.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(lo,hi),def));};
 f("cutoff","Cutoff",50,12000,1100); f("res","Resonance",0,0.95f,0.42f);
 f("attack","Attack",0.001f,1.5f,0.006f); f("decay","Decay",0.015f,2.5f,0.34f);
 f("sustain","Sustain",0,1,0.65f); f("release","Release",0.01f,3,0.2f);
 f("eg","Filter EG",0,5,2.0f); f("drive","Drive",0.5f,6,1.4f);
 f("mix1","Osc 1",0,1,0.85f); f("mix2","Osc 2",0,1,0.65f); f("mix3","Osc 3",0,1,0.4f);
 f("detune","Detune",0,35,7); f("octave","Octave",-2,2,0);
 f("wave","Waveform",0,1,0); f("volume","Volume",0,0.6f,0.22f);
 return {p.begin(),p.end()};
}
void VolcaInspiredAudioProcessor::prepareToPlay(double sr,int) {sampleRateHz=sr; phase.fill(0); env=filterEnv=lp1=lp2=0; note=-1; gate=false;}
bool VolcaInspiredAudioProcessor::isBusesLayoutSupported(const BusesLayout& b) const {return b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo() || b.getMainOutputChannelSet()==juce::AudioChannelSet::mono();}
void VolcaInspiredAudioProcessor::processBlock(juce::AudioBuffer<float>& audio,juce::MidiBuffer& midi) {
 juce::ScopedNoDenormals noDenormals; audio.clear();
 auto val=[&](const char* id){return apvts.getRawParameterValue(id)->load();};
 const double cutoff=val("cutoff"), resonance=val("res"), attack=val("attack"), decay=val("decay"), sustain=val("sustain"), release=val("release"), eg=val("eg"), drive=val("drive"), detune=val("detune"), octave=std::round(val("octave")), wave=val("wave"), volume=val("volume");
 const double mix[3]={val("mix1"),val("mix2"),val("mix3")};
 const double norm=std::max(1.0,mix[0]+mix[1]+mix[2]);
 juce::MidiBuffer::Iterator it(midi); juce::MidiMessage msg; int eventSample=0; bool hasEvent=it.getNextEvent(msg,eventSample);
 for(int s=0;s<audio.getNumSamples();++s) {
  while(hasEvent && eventSample<=s) {
   if(msg.isNoteOn()) {note=msg.getNoteNumber(); velocity=msg.getFloatVelocity(); gate=true;}
   else if(msg.isAllNotesOff() || msg.isAllSoundOff() || (msg.isNoteOff() && msg.getNoteNumber()==note)) gate=false;
   hasEvent=it.getNextEvent(msg,eventSample);
  }
  const double atk=std::exp(-1.0/(std::max(0.001,attack)*sampleRateHz));
  const double dec=std::exp(-1.0/(std::max(0.001,decay)*sampleRateHz));
  const double rel=std::exp(-1.0/(std::max(0.001,release)*sampleRateHz));
  if(gate) { if(env<0.999) env=1.0+(env-1.0)*atk; else env=sustain+(env-sustain)*dec; }
  else env*=rel;
  if(gate) { if(filterEnv<0.999) filterEnv=1.0+(filterEnv-1.0)*atk; else filterEnv*=dec; }
  else filterEnv*=rel;
  double signal=0;
  if(note>=0 && env>0.00001) {
   const double f=juce::MidiMessage::getMidiNoteInHertz(note)*std::pow(2.0,octave);
   for(int i=0;i<3;++i) {
    const double cents=(i-1)*detune;
    phase[i]+=f*std::pow(2.0,cents/1200.0)/sampleRateHz;
    phase[i]-=std::floor(phase[i]);
    signal+=mix[i]*osc(phase[i],wave>=0.5);
   }
   signal=std::tanh(signal*drive/norm);
  }
  // Resonant 2-pole lowpass with stable, bounded feedback.
  const double fc=juce::jlimit(30.0,sampleRateHz*0.20,cutoff*std::pow(2.0,eg*filterEnv));
  const double a=1.0-std::exp(-2.0*juce::MathConstants<double>::pi*fc/sampleRateHz);
  const double feedback=3.5*resonance;
  const double input=std::tanh(signal-feedback*lp2);
  lp1+=a*(input-lp1); lp2+=a*(lp1-lp2);
  const float out=static_cast<float>(std::tanh(lp2*1.8)*env*velocity*volume);
  for(int ch=0;ch<audio.getNumChannels();++ch) audio.setSample(ch,s,out);
 }
}
void VolcaInspiredAudioProcessor::getStateInformation(juce::MemoryBlock& dest) {auto xml=apvts.copyState().createXml(); copyXmlToBinary(*xml,dest);}
void VolcaInspiredAudioProcessor::setStateInformation(const void* data,int bytes) {auto xml=getXmlFromBinary(data,bytes); if(xml && xml->hasTagName(apvts.state.getType())) apvts.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessorEditor* VolcaInspiredAudioProcessor::createEditor()
{
    return new VolcaInspiredAudioProcessorEditor(*this);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {return new VolcaInspiredAudioProcessor();}
