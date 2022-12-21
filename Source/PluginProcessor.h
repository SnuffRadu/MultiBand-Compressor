/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


namespace params
{
    enum Names
    {
        lowMidCrossoverFreq,
        midHighCrossoverFreq,

        thresholdLowBand,
        thresholdMidBand,
        thresholdHighBand,

        attackLowBand,
        attackMidBand,
        attackHighBand,

        releaseLowBand,
        releaseMidBand,
        releaseHighBand,

        ratioLowBand,
        ratioMidBand,
        ratioHighBand,

        bypassedLowBand,
        bypassedMidBand,
        bypassedHighBand,

    };

    inline const std::map<Names, juce::String>& GetParams()
    {
        static std::map<Names, juce::String> params = 
        {
            {lowMidCrossoverFreq, "Low-Mid Crossover Frequency"},
            {midHighCrossoverFreq, "Mid-High Crossover Frequency"},
            {thresholdLowBand, "Threshold Low Band"},
            {thresholdMidBand, "Threshold Mid Band"},
            {thresholdHighBand, "Threshold High Band"},
            {attackLowBand, "Attack Low Band"},
            {attackMidBand, "Attack Mid Band"},
            {attackHighBand, "Attack High Band"},
            {releaseLowBand, "Release Low Band"},
            {releaseMidBand, "Release Mid Band"},
            {releaseHighBand, "Release High Band"},
            {ratioLowBand, "Ratio Low Band"},
            {ratioMidBand, "Ratio Mid Band"},
            {ratioHighBand, "Ratio High Band"},
            {bypassedLowBand, "Bypassed Low Band"},
            {bypassedMidBand, "Bypassed Mid Band"},
            {bypassedHighBand, "Bypassed High Band"},
        };
        return params;
    }

    struct CompressorBand
    {
        juce::AudioParameterFloat* attack{ nullptr };
        juce::AudioParameterFloat* release{ nullptr };
        juce::AudioParameterFloat* threshold{ nullptr };
        juce::AudioParameterChoice* ratio{ nullptr };
        juce::AudioParameterBool* bypassed{ nullptr };

        void prepare(const juce::dsp::ProcessSpec& spec)
        {
            compressor.prepare(spec);
        }

        void updateCompressorSettings()
        {
            compressor.setAttack(attack->get());
            compressor.setRelease(release->get());
            compressor.setThreshold(threshold->get());
            compressor.setRatio(ratio->getCurrentChoiceName().getFloatValue());
        }

        void process(juce::AudioBuffer<float>& buffer)
        {
            auto block = juce::dsp::AudioBlock<float>(buffer);
            auto context = juce::dsp::ProcessContextReplacing<float>(block);

            context.isBypassed = bypassed->get();

            compressor.process(context);
        }

    private:
        juce::dsp::Compressor<float> compressor;
    };


    //==============================================================================
    /**
    */
    class MultiBandCompressorAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
        , public juce::AudioProcessorARAExtension
#endif
    {
    public:
        //==============================================================================
        MultiBandCompressorAudioProcessor();
        ~MultiBandCompressorAudioProcessor() override;

        //==============================================================================
        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
        void setCurrentProgram(int index) override;
        const juce::String getProgramName(int index) override;
        void changeProgramName(int index, const juce::String& newName) override;

        //==============================================================================
        void getStateInformation(juce::MemoryBlock& destData) override;
        void setStateInformation(const void* data, int sizeInBytes) override;

        using APTVS = juce::AudioProcessorValueTreeState;
        static APTVS::ParameterLayout createParameterLayout();

        APTVS aptvs{ *this, nullptr, "Parameters", createParameterLayout() };

    private:
        CompressorBand compressor;

        using Filter = juce::dsp::LinkwitzRileyFilter<float>;
        Filter LP, HP;

        juce::AudioParameterFloat* lowCrossover{ nullptr };

        std::array<juce::AudioBuffer<float>, 2> filterBuffers;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiBandCompressorAudioProcessor)
    };
}
