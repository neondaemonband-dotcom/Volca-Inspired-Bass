#pragma once
#include <JuceHeader.h>
#include <array>
class VolcaInspiredAudioProcessor final : public juce::AudioProcessor {
public:
 VolcaInspiredAudioProcessor();
 void prepareToPlay(double, int) override;
 void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override { return true; }
 const juce::String getName() const override { return "Volca Inspired Bass"; }
 bool acceptsMidi() const override { return true; }
 bool producesMidi() const override { return false; }
 bool isMidiEffect() const override { return false; }
 double getTailLengthSeconds() const override { return 0.0; }
 int getNumPrograms() override { return 1; }
 int getCurrentProgram() override { return 0; }
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override { return {}; }
 void changeProgramName(int, const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*, int) override;
 juce::AudioProcessorValueTreeState apvts;
 static juce::AudioProcessorValueTreeState::ParameterLayout makeParams();
private:
 double sampleRateHz = 44100.0;
 std::array<double,3> phase{};
 double env=0.0, filterEnv=0.0, lp1=0.0, lp2=0.0;
 int note=-1;
 bool gate=false;
 float velocity=0.0f;
 static double osc(double phase, bool square) { return square ? (phase < 0.5 ? 1.0 : -1.0) : (2.0 * phase - 1.0); }
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VolcaInspiredAudioProcessor)
};
