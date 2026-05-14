/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct ExtendedTabbedButtonBar : juce::TabbedButtonBar, juce::DragAndDropTarget
{
    ExtendedTabbedButtonBar() : juce::TabbedButtonBar(juce::TabbedButtonBar::Orientation::TabsAtTop){}
    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override { return false; }
    void itemDropped (const SourceDetails& dragSourceDetails) override { }


    
};


//==============================================================================
/**
*/
class Audio_pluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Audio_pluginAudioProcessorEditor (Audio_pluginAudioProcessor&);
    ~Audio_pluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Audio_pluginAudioProcessor& audioProcessor;
    juce::TextButton dspOrderButton{ "dsp order" };

    ExtendedTabbedButtonBar tabbedComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Audio_pluginAudioProcessorEditor)
};
