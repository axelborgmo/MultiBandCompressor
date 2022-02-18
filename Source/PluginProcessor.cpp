/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MultiBandCompressorAudioProcessor::MultiBandCompressorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
     // pointers are stored as ranged audio parameters, base class that these parameters come from.
    // we need to cast these ranged audio parameters to the correct type before we can assign them to the cached instanses that we declared.
    
    attack = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Attack"));
    jassert(attack != nullptr); // in case we typed the parameter name incorrectly. The get function will return a nullptr if the name we provided is not found in the
    
    release = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Release"));
    jassert(release != nullptr);
    
    threshold = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Threshold"));
    jassert(threshold != nullptr);
    
    ratio = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("Ratio"));
    jassert(ratio != nullptr);
    
    bypassed = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter("Bypassed"));
    jassert(ratio != nullptr);
    
    
}

MultiBandCompressorAudioProcessor::~MultiBandCompressorAudioProcessor()
{
}

//==============================================================================
const juce::String MultiBandCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MultiBandCompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MultiBandCompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MultiBandCompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MultiBandCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MultiBandCompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MultiBandCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MultiBandCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MultiBandCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void MultiBandCompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MultiBandCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    juce::dsp::ProcessSpec spec; // we need to prepare the compress by passing a ProcessSpec to it
    spec.maximumBlockSize = samplesPerBlock; // needs to know the maximum number of samples
    spec.numChannels = getTotalNumOutputChannels(); // needs to know the number of channels
    spec.sampleRate = sampleRate; // sample rate to be passed to compressor
    
    // we can now pass these values to the compressor
    compressor.prepare(spec);
}

void MultiBandCompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MultiBandCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MultiBandCompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    
    // using get function to get the value from the parameter layout 
    // float have a get function that we can use, and we need to call the apppropriate compressor value with this parameter value
    compressor.setAttack(attack->get());
    compressor.setRelease(release->get());
    compressor.setThreshold(threshold->get());
    
    // the ratio is stored in a string array, and we need float value of the choice from the string, and we use a helper function in the string class to get it. (getFloatValue)
    compressor.setRatio(ratio->getCurrentChoiceName().getFloatValue() );
    
    // we also need to initialize the member variables pointers so that they aren't null.
    
    
    // the compressor needs a context, so we need to create an audioBlock (using the buffer in this function).
    auto block = juce::dsp::AudioBlock<float>(buffer); // creating an audioBlock
    auto context = juce::dsp::ProcessContextReplacing<float>(block); // create context
    
    // For the Bypass button: we can toggle whether the audio is processed or not by setting the "is bypassed flag" on the context
    context.isBypassed = bypassed->get();
    
    
    compressor.process(context);
    

    
    
}

//==============================================================================
bool MultiBandCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MultiBandCompressorAudioProcessor::createEditor()
{
   // return new MultiBandCompressorAudioProcessorEditor (*this);
    
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void MultiBandCompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    juce::MemoryOutputStream mos(destData, true); // creating memory stream to save and load parameters data
    apvts.state.writeToStream(mos); // writing to memory output stream
}

juce::AudioProcessorValueTreeState::ParameterLayout MultiBandCompressorAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;
    
    using namespace juce;
    
    layout.add(std::make_unique<AudioParameterFloat>("Threshold",
                                                    "Threshold",
                                                     NormalisableRange<float>(-60, 12, 1, 1), 0)); // Rangestart, range end, interval, skew factor, default value.
    
    auto attackReleaseRange = NormalisableRange<float>(5, 500, 1, 1);
    
    layout.add(std::make_unique<AudioParameterFloat>("Attack",
                                                     "Attack",
                                                     attackReleaseRange,
                                                     50)); // milliseconds
    
    layout.add(std::make_unique<AudioParameterFloat>("Release",
                                                     "Release",
                                                     attackReleaseRange,
                                                     250)); // milliseconds
    
    
    
    // AudioParameter Choice requires a juce::StringArray as a constructor argument
    
    auto choices = std::vector<double>{1, 1.5, 2, 3, 4, 5, 6, 7, 8, 10, 15, 20, 50, 100}; // different choices of ratio values, hardcoded
    
    juce::StringArray stringArray;
    
    for (auto choice : choices) // convert choices into string juce objects
    {
        stringArray.add (juce::String(choice, 1)); // 1 is number of decimal places
    }
    
    layout.add(std::make_unique<AudioParameterChoice>("Ratio", "Ratio", stringArray, 3)); // 3 is defalut value
    
    
    
    
    // bypass button
    layout.add(std::make_unique<AudioParameterBool>("Bypassed", "Bypassed", false)); // false meaning it is default as false (not bypassed)
    
    
    
    return layout;
}

void MultiBandCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes); // to restore memory from parameters
    if (tree.isValid()) // we need to check if the tree is valid before copy it into valueTreeState
    {
        apvts.replaceState(tree);
    }
    
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiBandCompressorAudioProcessor();
}
