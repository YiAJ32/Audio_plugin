/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

auto getPhaserRateName() { return juce::String("Phaser RateHz"); }
auto getPhaserCenterFreqName() { return juce::String("Phaser Center FreqHz"); }
auto getPhaserDepthName() { return juce::String("Phaser Depth %"); }
auto getPhaserFeedbackName() { return juce::String("Phaser Feedback %"); }
auto getPhaserMixName() { return juce::String("Phaser Mix %"); }



/*
Chorus:
Rate: hz
Deph: 0 to 1
Center delay: ms(1 to 100)
Feedback: -1 to 1
Mix: 0 to 1
*/
auto getChorusRateName() { return juce::String("Chorus RateHz"); }
auto getChorusDepthName() { return juce::String("Chorus Depth %"); }
auto getChorusCenterDelayName() { return juce::String("Chorus Center Delay ms"); }
auto getChorusFeedbackName() { return juce::String("Chorus Feedback %"); }
auto getChorusMixName() { return juce::String("Chorus Mix %"); }

//==============================================================================
Audio_pluginAudioProcessor::Audio_pluginAudioProcessor()
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
    auto phaserParams = std::array{
        &phaserRateHz,
        &phaserCenterFreqHz,
        &phaserDepthPercent,
        &phaserFeedbackPercent,
        &phaserMixPercent
    };

    auto phaserFuncs = std::array{
        &getPhaserRateName,
        &getPhaserCenterFreqName,
        &getPhaserDepthName,
        &getPhaserFeedbackName,
        &getPhaserMixName
    };

    for (size_t i = 0; i < phaserParams.size(); ++i) {
        auto ptrToParamPtr = phaserParams[i];
        *ptrToParamPtr = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(phaserFuncs[i]()));
        jassert(*ptrToParamPtr != nullptr);
    }

}

Audio_pluginAudioProcessor::~Audio_pluginAudioProcessor()
{
}

//==============================================================================
const juce::String Audio_pluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Audio_pluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Audio_pluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Audio_pluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Audio_pluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Audio_pluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Audio_pluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Audio_pluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Audio_pluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void Audio_pluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Audio_pluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void Audio_pluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Audio_pluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

juce::AudioProcessorValueTreeState::ParameterLayout
Audio_pluginAudioProcessor::createParameterLayout() {

    juce::AudioProcessorValueTreeState::ParameterLayout layout;


    const int versionHint = 1;
    auto name = getPhaserRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(0.01f, 2.f, 0.01f, 1.f),
        0.2f,
        "Hz"));

    //phaser depth 0 - 1
    name = getPhaserDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(0.01f, 2.f, 0.01f, 1.f),
        0.2f,
        "Hz"));

    //phaser center freq: audio hz
    name = getPhaserCenterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name, 
        juce::NormalisableRange<float>(0.01f, 1.f,0.01f, 1.f ),
        0.05f,
        "Hz"));


    //phaser feedback: -1 to 1
    name = getPhaserFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f),
        0.0f,
        "%"));


    //phaser mix: 0 to 1
    name = getPhaserMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(0.01f, 1.f, 0.01f, 1.f),
        0.05f,
        "%"));
    return layout;
}

void Audio_pluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    //TO DO[DONE]: Add apvts
    //TO DO: create audio param for all dps choices
    //TO DO: update dsp for audio params
    //TO DO: save/load settings
    //TO DO: save/load dps order
    //TO DO: Drag to reorder guid
    //TO DO: metering
    //TO DO: preparing all dsp 
    //TO DO: web/dry knob
    //TO DO: mono & stereo options
    //TO DO: modulators
    //TO DO: thread save filtering update
    //TO DO: pre/post filtering
    //TO DO: delay module


    auto newDSPOrder = DSP_Order();

    while (dspOrderFifo.pull(newDSPOrder)) {

    }

    if (newDSPOrder != DSP_Order()) 
        dspOrder = newDSPOrder;
    
    DSP_Pointers dspPointers;

    for (size_t i = 0; i < dspPointers.size(); ++i) {
        switch (dspOrder[i]) {
        case DSP_OPTION::Phase:
            dspPointers[i] = &phaser;
            break;
        case DSP_OPTION::Chorus:
            dspPointers[i] = &chorus;
            break;
        case DSP_OPTION::Overdrive:
            dspPointers[i] = &overdrive;
            break;
        case DSP_OPTION::LadderFilter:
            dspPointers[i] = &ladderfilter;
            break;
        case DSP_OPTION::END_OF_LIST:
            jassertfalse;
            break;

        }
    }

    //process and create an audio block
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block); //fail 

    for (size_t i = 0; i < dspPointers.size(); ++i) {
        if (dspPointers[i] != nullptr) {
            dspPointers[i]->process(context);
        }
    }

}

//==============================================================================
bool Audio_pluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Audio_pluginAudioProcessor::createEditor()
{
    return new Audio_pluginAudioProcessorEditor (*this);
}

//==============================================================================
void Audio_pluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Audio_pluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Audio_pluginAudioProcessor();
}
