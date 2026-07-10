/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct ExtendedTabbedButtonBar : juce::TabbedButtonBar, juce::DragAndDropTarget, juce::DragAndDropContainer
{
    ExtendedTabbedButtonBar() ;
    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;

    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    void itemDragMove(const SourceDetails& dragSourceDetails) override;
    void itemDragExit(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override;

    void mouseDown(const juce::MouseEvent& e) override;

    juce::TabBarButton* createTabButton(const juce::String& tabName, int tabIndex) override;

    struct Listener 
    {
        virtual ~Listener() = default;
        virtual void tabOrderChange (Audio_pluginAudioProcessor::DSP_Order newOrder) = 0;
    };

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    juce::TabBarButton* findDragItem(const SourceDetails& dragSourceDetails);
    int findDraggedItemIndex(const SourceDetails& dragSourceDetails);
    juce::Array<juce::TabBarButton*> getTabs();
    juce::ScaledImage dragImage;
    juce::ListenerList<Listener> listeners;
};


struct HorizontalConstrainer : juce::ComponentBoundsConstrainer
{
    HorizontalConstrainer(std::function<juce::Rectangle<int>()> confinerBoundsGetter,
        std::function<juce::Rectangle<int>()> confineeBoundsGetter);

    void checkBounds(juce::Rectangle<int>& bounds,
        const juce::Rectangle<int>& previousBounds,
        const juce::Rectangle<int>& limits,
        bool isStretchingTop,
        bool isStretchingLeft,
        bool isStretchingBottom,
        bool isStretchingRight) override;

private:
    std::function<juce::Rectangle<int>()> boundsToConfineToGetter;
    std::function<juce::Rectangle<int>()> boundsOfConfineeGetter;
};

struct ExtendedTabBarButton : juce::TabBarButton
{
    ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& owner,
        Audio_pluginAudioProcessor::DSP_OPTION dspOption);
    juce::ComponentDragger dragger;
    std::unique_ptr<HorizontalConstrainer> constrainer;

    void mouseDown(const juce::MouseEvent& e) override;
   
    void mouseDrag(const juce::MouseEvent& e) override;

    Audio_pluginAudioProcessor::DSP_OPTION getOption() const {return option;}

    int getBestTabLength(int depth) override;
private:
    Audio_pluginAudioProcessor::DSP_OPTION option;
};

struct RotarySliderWithLabels;

struct DSP_GUI : juce::Component 
{
    DSP_GUI(Audio_pluginAudioProcessor& p);
    void resized()  override;

    void paint(juce::Graphics& g) override;
    void rebuildInterface(std::vector < juce::RangedAudioParameter* > params);
    
    Audio_pluginAudioProcessor& processor;

    std::vector < std::unique_ptr < RotarySliderWithLabels> > sliders;
    std::vector < std::unique_ptr < juce::ComboBox> > comboBoxes;
    std::vector < std::unique_ptr < juce::Button> > buttons;
    std::vector < std::unique_ptr < juce::AudioProcessorValueTreeState::SliderAttachment> > sliderAttachements;
    std::vector < std::unique_ptr < juce::AudioProcessorValueTreeState::ComboBoxAttachment> > comboBoxAttachements;
    std::vector < std::unique_ptr < juce::AudioProcessorValueTreeState::ButtonAttachment> > buttonAttachements;
};

//==============================================================================
/**
*/
class Audio_pluginAudioProcessorEditor  : public juce::AudioProcessorEditor, 
    ExtendedTabbedButtonBar::Listener,
    juce::Timer
{
public:
    Audio_pluginAudioProcessorEditor (Audio_pluginAudioProcessor&);
    ~Audio_pluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void tabOrderChange(Audio_pluginAudioProcessor::DSP_Order newOrder) override;
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Audio_pluginAudioProcessor& audioProcessor;
    DSP_GUI dspGUI{audioProcessor};

    ExtendedTabbedButtonBar tabbedComponent;
    
    void addTabsFromDSPOrder(Audio_pluginAudioProcessor::DSP_Order);
    void rebuildInterface();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Audio_pluginAudioProcessorEditor)
};
