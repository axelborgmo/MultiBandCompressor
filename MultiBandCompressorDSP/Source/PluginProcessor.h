/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class MultiBandCompressorAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    MultiBandCompressorAudioProcessor();
    ~MultiBandCompressorAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    using APVTS = juce::AudioProcessorValueTreeState; // AudioParametersValueTreeState alias
    static APVTS::ParameterLayout createParameterLayout();
    
    APVTS apvts {*this, nullptr, "Parameters", createParameterLayout()};

private:
    
    
    juce::dsp::Compressor<float> compressor; // using juce compressor class
    //apvts has a member function that returns pointer to the parameters in the createParameterLayout
    // we don't need to call this member function for every single parameter, every time processBlock is called.
    // if our bufferSize is small, it can be called 700 times a second. So the cost of looking of these parameters can be expensive.
    
    // we create some member variables that will act as cached versions of our audioParameters to not have to call the parameters every time
    
    // creating pointers for this, with the same type as the parameterLayout:
    juce::AudioParameterFloat *attack { nullptr };
    juce::AudioParameterFloat *release { nullptr };
    juce::AudioParameterFloat *threshold { nullptr };
    juce::AudioParameterChoice *ratio { nullptr };
    juce::AudioParameterBool* bypassed {nullptr }; // bypass pointer
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiBandCompressorAudioProcessor)
};
