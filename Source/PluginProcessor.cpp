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
auto getPhaserBypassName() { return juce::String("Phaser Bypass"); }

auto getChorusRateName() { return juce::String("Chorus RateHz"); }
auto getChorusDepthName() { return juce::String("Chorus Depth %"); }
auto getChorusCenterDelayName() { return juce::String("Chorus Center Delay ms"); }
auto getChorusFeedbackName() { return juce::String("Chorus Feedback %"); }
auto getChorusMixName() { return juce::String("Chorus Mix %"); }
auto getChorusBypassName() { return juce::String("Chorus Bypass"); }

auto getOverdriveSaturationName() { return juce::String("Overdrive Saturation"); }
auto getOverdriveBypassName() { return juce::String("Overdrive Bypass"); }

auto getLadderFilterModeName() { return juce::String("Ladder Filter Mode"); }
auto getLadderFilterCutoffName() { return juce::String("Ladder Filter Cutoff Hz"); }
auto getLadderFilterResonanceName() { return juce::String("Ladder Filter Resonance"); }
auto getLadderFilterDriveName() { return juce::String("Ladder Filter Drive"); }
auto getLadderFilterBypassName() { return juce::String("Ladder Filter Bypass"); }

auto getLadderFilterChoices() {

    return  juce::StringArray
      {
        "LPF12",  // low-pass  12 dB/octave
        "HPF12",  // high-pass 12 dB/octave
        "BPF12",  // band-pass 12 dB/octave
        "LPF24",  // low-pass  24 dB/octave
        "HPF24",  // high-pass 24 dB/octave
        "BPF24"   // band-pass 24 dB/octave
      };

}


auto getGeneralFilterModeName() { return juce::String("General Filter Mode"); }
auto getGeneralFilterFreqName() { return juce::String("General Filter Freq hz"); }
auto getGeneralFilterQualityName() { return juce::String("General Filter Quality"); }
auto getGeneralFilterGainName() { return juce::String("General Filter Gain"); }
auto getGeneralFilterBypassName() { return juce::String("General Filter Bypass"); }

auto getGeneralFilterChoices() {
    return juce::StringArray{
        "peak", 
        "bandpass", 
        "notch",
        "allpass"
    };

};


auto getSelectedTabName() { return juce::String("Selected Tab"); }

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

    for (size_t i = 0; i < static_cast<size_t>(DSP_OPTION::END_OF_LIST); ++i) {
        dspOrder[i] = static_cast<DSP_OPTION>(i);
    }

    restoreDspOrderFifo.push(dspOrder);

    auto floatParams = std::array{
        &phaserRateHz,
        &phaserCenterFreqHz,
        &phaserDepthPercent,
        &phaserFeedbackPercent,
        &phaserMixPercent,

        &chorusRateHz,
        &chorusDepthPercent,
        &chorusCenterDelayMs,
        &chorusFeedbackPercent,
        &chorusMixPercent,

        &overdriveSaturation,

        &ladderFilterCutoff,
        &ladderFilterResonance,
        &ladderFilterDrive,

        &generalFilterFreqHz,
        &generalFilterQuality,
        &generalFilterGain
    };

    auto floatNameFuncs = std::array{
        &getPhaserRateName,
        &getPhaserCenterFreqName,
        &getPhaserDepthName,
        &getPhaserFeedbackName,
        &getPhaserMixName,

        &getChorusRateName,
        &getChorusDepthName,
        &getChorusCenterDelayName,
        &getChorusFeedbackName,
        &getChorusMixName,

        &getOverdriveSaturationName,

        &getLadderFilterCutoffName,
        &getLadderFilterResonanceName,
        &getLadderFilterDriveName,

        &getGeneralFilterFreqName,
        &getGeneralFilterQualityName,
        &getGeneralFilterGainName

    };

 
    initCashedParams<juce::AudioParameterFloat*>(floatParams, floatNameFuncs);
    auto choiceParams = std::array{
        &ladderFilterMode,
        &generalFilterMode
    };

    auto choiceNameFuncs = std::array{
        &getLadderFilterModeName,
        &getGeneralFilterModeName
    };

    initCashedParams<juce::AudioParameterChoice*>(choiceParams, choiceNameFuncs);

    auto bypassParams = std::array{
        &phaserBypass,
        &chorusBypass,
        &overdriveBypass,
        &ladderFilterBypass,
        &generalFilterBypass
    };

    auto bypassNameFuncs = std::array{
     &getPhaserBypassName,
     &getChorusBypassName,
     &getOverdriveBypassName,
     &getLadderFilterBypassName,
     &getGeneralFilterBypassName
    };



    initCashedParams<juce::AudioParameterBool*>(bypassParams, bypassNameFuncs);

    auto intParams = std::array
    {
        &selectedTab
    };

    auto intFuncs = std::array
    {
        &getSelectedTabName
    };

    initCashedParams<juce::AudioParameterInt*>(intParams, intFuncs);
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
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;

    leftChannel.prepare(spec);
    rightChannel.prepare(spec);
    

    for(auto smoother : getSmoothers() ){
        smoother->reset(sampleRate, 0.005);
    }

    updateSmoothersFromParams(1, SmootherUpdateMode::initialize);
}

void Audio_pluginAudioProcessor::updateSmoothersFromParams(int numSampleToSkip, SmootherUpdateMode init) {
    auto parramsNeedingSmoothing = std::vector
    {
        phaserRateHz,
        phaserCenterFreqHz,
        phaserDepthPercent,
        phaserFeedbackPercent,
        phaserMixPercent,
        chorusRateHz,
        chorusDepthPercent,
        chorusCenterDelayMs,
        chorusFeedbackPercent,
        chorusMixPercent,
        overdriveSaturation,
        ladderFilterCutoff,
        ladderFilterResonance,
        ladderFilterDrive,
        generalFilterFreqHz,
        generalFilterQuality,
        generalFilterGain
    };

    auto smoothers = getSmoothers();
    jassert(smoothers.size() == parramsNeedingSmoothing.size());

    for (size_t i = 0; i < smoothers.size(); ++i) {
        auto smoother = smoothers[i];
        auto param = parramsNeedingSmoothing[i];

        if (init == SmootherUpdateMode::initialize) {
            smoother->setCurrentAndTargetValue(param->get());
        }
        else {
            smoother->setValue(param->get());
        }

        smoother->skip(numSampleToSkip);
    }
}

std::vector<juce::SmoothedValue<float>*>Audio_pluginAudioProcessor::getSmoothers()

{
    auto smoothers = std::vector
    {
        &phaserRateHzSmoother,
        &phaserCenterFreqHzSmoother,
        &phaserDepthPercentSmoother,
        &phaserFeedbackPercentSmoother,
        &phaserMixPercentSmoother,
        &chorusRateHzSmoother,
        &chorusDepthPercentSmoother,
        &chorusCenterDelayMsSmoother,
        &chorusFeedbackPercentSmoother,
        &chorusMixPercentSmoother,
        &overdriveSaturationSmoother,
        &ladderFilterCutoffSmoother,
        &ladderFilterResonanceSmoother,
        &ladderFilterDriveSmoother,
        &generalFilterFreqHzSmoother,
        &generalFilterQualitySmoother,
        &generalFilterGainSmoother
    };

    return smoothers;
}

void Audio_pluginAudioProcessor::MonoChannelDSP::prepare(const juce::dsp::ProcessSpec& spec) {
    jassert(spec.numChannels == 1);
    std::vector<juce::dsp::ProcessorBase*> dsp
    {
        &phaser,
        &chorus,
        &overdrive,
        &ladderfilter,
        &generalFilter
    };


    for (auto p : dsp)
    {
        p->prepare(spec);
        p->reset();
    }

    overdrive.dsp.setCutoffFrequencyHz(20000.f);
};

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


    //phaser depth 0 - 100
    name = getPhaserDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(0.0f, 100.f, 0.1f, 1.f),
        5.f,
        "%"));

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
        juce::NormalisableRange<float>(-100.f, 100.f, 0.1f, 1.f),
        0.0f,
        "%"));


    //phaser mix: 0 to 1
    name = getPhaserMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(0.0f, 100.f, 0.1f, 1.f),
        5.f,
        "%"));


     //phaser bypass
    name = getPhaserBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(name, versionHint),
        name,false));

       /*
        Chorus:
        Rate: hz
        Deph: 0 to 1
        Center delay: ms(1 to 100)
        Feedback: -1 to 1
        Mix: 0 to 1
       */

    //rate: hz
    name = getChorusRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(0.01f, 100.f, 0.01f, 1.f),
        0.02f,
        "Hz"));

    //depth: 0 to 1
    name = getChorusDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(0.0f, 100.f, 0.1f, 1.f),
        5.f,
        "%"));

    //center delay: ms (1 to 100)
    name = getChorusCenterDelayName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1.f),
        7.f,
        "ms"));

    //feedback: -1 to 1
    name = getChorusFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(-100.f, 100.f, 0.1f, 1.f),
        0.0f,
        "%"));

    //mix: 0 to 1
    name = getChorusMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(0.0f, 100.f, 0.1f, 1.f),
        5.f,
        "%"));

    //chorus bypass
    name = getChorusBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(name, versionHint),
        name, false));


    /*
    * overdrive: uses the drive portion of the ladder filter class for now
    */

    //overdrive: 1 to 100
    name = getOverdriveSaturationName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1.f),
        1.f,
        ""));

    //overdrive bypass
    name = getOverdriveBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(name, versionHint),
        name, false));


    /*
    Ladder filter:
    mode: ladderfilter mode (enum)
    cutoff: Hz
    Center delay: ms(1 to 100)
    resonance: 0 to 1
    drive: 1 to 100
   */


    name = getLadderFilterModeName();
    auto choices = getLadderFilterChoices();
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(name, versionHint),
        name,
        choices,
        0));

    name = getLadderFilterCutoffName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(20.f, 20000.f, 0.1f, 1.f),
        20000.f,
        "Hz"));

    name = getLadderFilterResonanceName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(0.f, 100.f, 0.1f, 1.f),
        0.f,
        "%"));


    name = getLadderFilterDriveName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1.f),
        1.f,
        ""));

    //Ladder filter bypass
    name = getLadderFilterBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(name, versionHint),
        name, false));


    /*General filter:
    * Mode: peak, bandpass, notch, allpass
    * freq:20.hz - 20,000hz in 1 hr steps
    * Q:0.1-10 in 0.05 steps
    * gain: -24db to +24db in 0.5db increments
    */

    name = getGeneralFilterModeName();
    choices = getGeneralFilterChoices();
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(name, versionHint),
        name,
        choices,
        0));

    name = getGeneralFilterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
        750.f,
        "Hz"));

    name = getGeneralFilterQualityName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(0.01f, 100.f, 0.01f, 1.f),
        0.72f,
        ""));

    name = getGeneralFilterGainName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(name, versionHint),
        name,
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.f,
        "dB"));


    //general filter bypass
    name = getGeneralFilterBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(name, versionHint),
        name, false));

    name = getSelectedTabName();
    layout.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{ name, versionHint },
        name,
        0,
        static_cast<int>(DSP_OPTION::END_OF_LIST) -1,
        static_cast<int>(DSP_OPTION::Chorus)));

    return layout;
}


void Audio_pluginAudioProcessor::MonoChannelDSP::updateDSPFromParams() 
{
    phaser.dsp.setRate(p.phaserRateHzSmoother.getCurrentValue());
    phaser.dsp.setCentreFrequency(p.phaserCenterFreqHzSmoother.getCurrentValue());
    phaser.dsp.setDepth(p.phaserDepthPercentSmoother.getCurrentValue() * 0.01f);
    phaser.dsp.setFeedback(p.phaserFeedbackPercentSmoother.getCurrentValue() * 0.01f);
    phaser.dsp.setMix(p.phaserMixPercentSmoother.getCurrentValue() * 0.01f);

    chorus.dsp.setRate(p.chorusRateHzSmoother.getCurrentValue());
    chorus.dsp.setDepth(p.chorusDepthPercentSmoother.getCurrentValue() * 0.01f);
    chorus.dsp.setCentreDelay(p.chorusCenterDelayMsSmoother.getCurrentValue());
    chorus.dsp.setFeedback(p.chorusFeedbackPercentSmoother.getCurrentValue() * 0.01f);
    chorus.dsp.setMix(p.chorusMixPercentSmoother.getCurrentValue() * 0.01f);

    overdrive.dsp.setDrive(p.overdriveSaturationSmoother.getCurrentValue());

    ladderfilter.dsp.setMode(static_cast<juce::dsp::LadderFilterMode>(p.ladderFilterMode->getIndex()));
    ladderfilter.dsp.setCutoffFrequencyHz(p.ladderFilterCutoffSmoother.getCurrentValue());
    ladderfilter.dsp.setResonance(p.ladderFilterResonanceSmoother.getCurrentValue() * 0.01f);
    ladderfilter.dsp.setDrive(p.ladderFilterDriveSmoother.getCurrentValue());

    // TO DO: update general filter coefficients here

    auto sampleRate = p.getSampleRate();
    auto genMode = p.generalFilterMode->getIndex();
    auto genHz = p.generalFilterFreqHz->get();
    auto genQ = p.generalFilterQuality->get();
    auto genGain = p.generalFilterGain->get();

    bool filterChanged = false;
    filterChanged |= (filterFreq != genHz);
    filterChanged |= (filterQ != genQ);
    filterChanged |= (filterGain != genGain);

    auto updatedMode = static_cast<GeneralFilterMode>(genMode);
    filterChanged |= (filterMode != updatedMode);

    if (filterChanged)
    {
        filterMode = updatedMode;
        filterFreq = genHz;
        filterQ = genQ;
        filterGain = genGain;

        juce::dsp::IIR::Coefficients<float>::Ptr coefficients;
        switch (filterMode)
        {
        case GeneralFilterMode::Peak:
        {
            coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                filterFreq,
                filterQ,
                juce::Decibels::decibelsToGain(filterGain));
            break;
        }
        case GeneralFilterMode::Bandpass:
        {
            coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate,
                filterFreq,
                filterQ);
            break;
        }
        case GeneralFilterMode::Notch:
        {
            coefficients = juce::dsp::IIR::Coefficients<float>::makeNotch(sampleRate,
                filterFreq,
                filterQ);
            break;
        }
        case GeneralFilterMode::Allpass:
        {
            coefficients = juce::dsp::IIR::Coefficients<float>::makeAllPass(sampleRate,
                filterFreq,
                filterQ);
            break;
        }
        case GeneralFilterMode::END_OF_LIST:
        {
            jassertfalse;
            break;
        }
        }

        if (coefficients != nullptr)
        {
           /*if( generalFilter.dsp.coefficients->coefficients.size() != coefficients->coefficients.size() )
               {
                    jassertfalse;
              }*/

            *generalFilter.dsp.coefficients = *coefficients;
            generalFilter.reset();
        }
    }

};

std::vector<juce::RangedAudioParameter*> Audio_pluginAudioProcessor::getParamsForOption(DSP_OPTION option)
{
    switch (option) 
    {
        case DSP_OPTION::Phase:
        {
            return
            {
                phaserRateHz,
                phaserCenterFreqHz,
                phaserDepthPercent,
                phaserFeedbackPercent,
                phaserMixPercent,
                phaserBypass
            };
        }
        case DSP_OPTION::Chorus:
        {
            return
            {
                chorusRateHz,
                chorusDepthPercent,
                chorusCenterDelayMs,
                chorusFeedbackPercent,
                chorusMixPercent,
                chorusBypass
            };
        }
        case DSP_OPTION::Overdrive:
        {
            return
            {
                overdriveSaturation,
                overdriveBypass
            };
        }
        case DSP_OPTION::LadderFilter:
        {
            return
            {
                ladderFilterMode,
                ladderFilterCutoff,
                ladderFilterResonance,
                ladderFilterDrive, 
                ladderFilterBypass
            };
        }
        case DSP_OPTION::GeneralFilter:
        {
            return
            {
                generalFilterMode,
                generalFilterFreqHz,
                generalFilterQuality,
                generalFilterGain,
                generalFilterBypass
            };
        }
        case DSP_OPTION::END_OF_LIST:
            break;
    }

        jassertfalse;
        return { };
    
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

    //[DONE]: Add apvts
    //[DONE]: create audio param for all dps choices
    //[DONE]: update dsp for audio params
    //[DONE]: bypass params for each dps elements
    //[DONE]: Update general filter coefficients
    //[DONE]: filters are mono, no stereo
    //[DONE]: add smoothers for all param updates 
    //[DONE]: save/load settings
    //[DONE]: save/load dps order
    //[DONE]: bypass dsp
    //[DONE]: Drag to reorder guid
    //TO DO: metering
    //TO DO: preparing all dsp 
    //TO DO: web/dry knob
    //TO DO: mono & stereo options
    //TO DO: modulators
    //TO DO: thread save filtering update
    //TO DO: pre/post filtering
    //TO DO: delay module
    //[DONE]: restore tabs in the gui when loading
    //TO DO: save/load preset
    //TO DO: GUI desing for all dsp instances
    //[DONE]: snapped drag the tab to the right position
    //[DONE]: hide drag tab img or prevent it to move

    
    leftChannel.updateDSPFromParams();
    rightChannel.updateDSPFromParams();
    
    auto newDSPOrder = DSP_Order();

    while (dspOrderFifo.pull(newDSPOrder)) {
#if VERIFY_BYPASS_FUNCTIONALITY
        jassertfalse;
#endif
    }

    if (newDSPOrder != DSP_Order()) 
        dspOrder = newDSPOrder;
    
    auto samplesRemaining = buffer.getNumSamples();
    auto maxSamplesToProcess = juce::jmin(samplesRemaining, 64);


    auto block = juce::dsp::AudioBlock<float>(buffer);
    size_t startSample = 0;

    while (samplesRemaining > 0) {

        auto samplesToProcess = juce::jmin(samplesRemaining, maxSamplesToProcess);
        updateSmoothersFromParams(samplesToProcess, SmootherUpdateMode::liveInRealTime);


        leftChannel.updateDSPFromParams();
        rightChannel.updateDSPFromParams();

        auto sublock = block.getSubBlock(startSample, samplesToProcess);

        leftChannel.process(sublock.getSingleChannelBlock(0), dspOrder);
        rightChannel.process(sublock.getSingleChannelBlock(1), dspOrder);

        startSample += samplesToProcess;
        samplesRemaining -= samplesToProcess;
    }


}

void Audio_pluginAudioProcessor::MonoChannelDSP::process(juce::dsp::AudioBlock<float> block, const DSP_Order& dspOrder)
{
    DSP_Pointers dspPointers;
    dspPointers.fill({});

    for (size_t i = 0; i < dspPointers.size(); ++i) {
        switch (dspOrder[i]) {
        case DSP_OPTION::Phase:
            dspPointers[i].processor = &phaser;
            dspPointers[i].bypass = p.phaserBypass->get();
            break;
        case DSP_OPTION::Chorus:
            dspPointers[i].processor = &chorus;
            dspPointers[i].bypass = p.chorusBypass->get();
            break;
        case DSP_OPTION::Overdrive:
            dspPointers[i].processor = &overdrive;
            dspPointers[i].bypass = p.overdriveBypass->get();
            break;
        case DSP_OPTION::LadderFilter:
            dspPointers[i].processor = &ladderfilter;
            dspPointers[i].bypass = p.ladderFilterBypass->get();
            break;
        case DSP_OPTION::GeneralFilter:
            dspPointers[i].processor = &generalFilter;
            dspPointers[i].bypass = p.generalFilterBypass->get();
            break;
        case DSP_OPTION::END_OF_LIST:
            jassertfalse;
            break;

        }
    }

    //process and create an audio block
    auto context = juce::dsp::ProcessContextReplacing<float>(block);

    for (size_t i = 0; i < dspPointers.size(); ++i) {
        if (dspPointers[i].processor != nullptr) {
            juce::ScopedValueSetter<bool>svs(context.isBypassed, dspPointers[i].bypass);
#if VERIFY_BYPASS_FUNCTIONALITY
            if (context.isBypassed) {
                jassertfalse;
            }
            if (dspPointers[i].processor == &generalFilter) {
                continue;
            }
#endif





            dspPointers[i].processor->process(context);
        }
    }

};
//==============================================================================
bool Audio_pluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Audio_pluginAudioProcessor::createEditor()
{
   return new Audio_pluginAudioProcessorEditor (*this);
   //return new juce::GenericAudioProcessorEditor(*this);
}


template<>
struct juce::VariantConverter<Audio_pluginAudioProcessor::DSP_Order> {

    static Audio_pluginAudioProcessor::DSP_Order fromVar(const juce::var& v) {
        using T = Audio_pluginAudioProcessor::DSP_Order;
        T dspOrder;

        jassert(v.isBinaryData());

        if (v.isBinaryData() == false) {
            dspOrder.fill(Audio_pluginAudioProcessor::DSP_OPTION::END_OF_LIST);
        }
        else {
            auto mb = *v.getBinaryData();
            juce::MemoryInputStream mis(mb, false);
            std::vector<int> arr;
            while (!mis.isExhausted()) {
                arr.push_back(mis.readInt());
            }

            jassert(arr.size() == dspOrder.size());
            for (size_t i = 0; i < dspOrder.size(); ++i) {
                dspOrder[i] = static_cast<Audio_pluginAudioProcessor::DSP_OPTION>(arr[i]);
            }
        }

        return dspOrder;

    }
    
    static juce::var toVar(const Audio_pluginAudioProcessor::DSP_Order& t) {
        juce::MemoryBlock mb; 
        {
            juce::MemoryOutputStream mos(mb, false);

            for (const auto& v : t) {
                mos.writeInt(static_cast<int> (v));
            }
        }

        return mb;
    }


};

//==============================================================================
void Audio_pluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    apvts.state.setProperty("dspOrder",
                            juce::VariantConverter<Audio_pluginAudioProcessor::DSP_Order>::toVar(dspOrder),
                            nullptr);

    juce::MemoryOutputStream mos(destData, false);
        apvts.state.writeToStream(mos);
    
}

void Audio_pluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);

        if (apvts.state.hasProperty("dspOrder")) {
            auto order = 
                juce::VariantConverter<Audio_pluginAudioProcessor::DSP_Order>::fromVar(apvts.state.getProperty("dspOrder"));
            dspOrderFifo.push(order);
            restoreDspOrderFifo.push(order);
        }

        DBG(apvts.state.toXmlString());

#if VERIFY_BYPASS_FUNCTIONALITY
        juce::Timer::callAfterDelay(1000, [this]()
            {
                DSP_Order order;
                order.fill(DSP_OPTION::LadderFilter);
                order[0] = DSP_OPTION::Chorus;

                chorusBypass->setValueNotifyingHost(1.f);
                dspOrderFifo.push(order);
            });

#endif

    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Audio_pluginAudioProcessor();
}
