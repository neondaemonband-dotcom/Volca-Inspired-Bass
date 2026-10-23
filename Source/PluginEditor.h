#pragma once
#include "PluginProcessor.h"
class VolcaInspiredAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
 explicit VolcaInspiredAudioProcessorEditor(VolcaInspiredAudioProcessor&);
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 VolcaInspiredAudioProcessor& processor;
 std::vector<std::unique_ptr<juce::Slider>> sliders;
 std::vector<std::unique_ptr<juce::Label>> labels;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VolcaInspiredAudioProcessorEditor)
};
