/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace params;

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

    const auto& params = GetParams();

    auto floatHelper = [&aptvs = this->aptvs, &params](auto& param, const auto& paramName)
    {
        param =
            dynamic_cast<juce::AudioParameterFloat*>(aptvs.getParameter(params.at(paramName)));
            jassert(param != nullptr);
    };

    auto choiceHelper = [&aptvs = this->aptvs, &params](auto& param, const auto& paramName)
    {
        param =
            dynamic_cast<juce::AudioParameterChoice*>(aptvs.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    auto boolHelper = [&aptvs = this->aptvs, &params](auto& param, const auto& paramName)
    {
        param =
            dynamic_cast<juce::AudioParameterBool*>(aptvs.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    floatHelper(compressor.attack, Names::attackLowBand);
    floatHelper(compressor.release, Names::releaseLowBand);
    floatHelper(compressor.threshold, Names::thresholdLowBand);
    choiceHelper(compressor.ratio, Names::ratioLowBand);
    boolHelper(compressor.bypassed, Names::bypassedLowBand);
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

    juce::dsp::ProcessSpec processSpec;
    processSpec.maximumBlockSize = samplesPerBlock;
    processSpec.numChannels = getTotalNumOutputChannels();
    processSpec.sampleRate = sampleRate;

    compressor.prepare(processSpec);
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

    compressor.updateCompressorSettings();
    compressor.process(buffer);

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool MultiBandCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MultiBandCompressorAudioProcessor::createEditor()
{
    //return new MultiBandCompressorAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}
 
//==============================================================================
void MultiBandCompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    juce::MemoryOutputStream outputStream(destData, true);
    aptvs.state.writeToStream(outputStream);
}

void MultiBandCompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        aptvs.replaceState(tree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout MultiBandCompressorAudioProcessor::createParameterLayout()
{
    using namespace juce;
    APTVS::ParameterLayout layout;
    auto attackReleaseRange = NormalisableRange<float>(5, 500, 1, 1);
    auto ratioChoices = std::vector<double>{ 1, 1.5, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 100 };
    juce::StringArray stringArray;
    const auto& params = GetParams();

    for (auto ratio : ratioChoices) 
    {
        stringArray.add(juce::String(ratio, 1));
    }

    layout.add(std::make_unique<AudioParameterFloat>(
        params.at(Names::thresholdLowBand),
        params.at(Names::thresholdLowBand),
        NormalisableRange<float>(-60, 12, 1, 1), 
        0));

    layout.add(std::make_unique<AudioParameterFloat>(
        params.at(Names::attackLowBand),
        params.at(Names::attackLowBand),
        attackReleaseRange,
        50));

    layout.add(std::make_unique<AudioParameterFloat>(
        params.at(Names::releaseLowBand),
        params.at(Names::releaseLowBand),
        attackReleaseRange,
        250));  

    layout.add(std::make_unique<AudioParameterChoice>(
        params.at(Names::ratioLowBand),
        params.at(Names::ratioLowBand),
        stringArray,
        3));

    layout.add(std::make_unique<AudioParameterBool>(
        params.at(Names::bypassedLowBand),
        params.at(Names::bypassedLowBand),
        false));


    return layout;

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiBandCompressorAudioProcessor();
}
