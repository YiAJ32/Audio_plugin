/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


static juce::String getDspOptionName(Audio_pluginAudioProcessor::DSP_OPTION option)
{
    switch (option)
    {
    case Audio_pluginAudioProcessor::DSP_OPTION::Phase:
        return "PHASE";
    case Audio_pluginAudioProcessor::DSP_OPTION::Chorus:
        return "CHORUS";
    case Audio_pluginAudioProcessor::DSP_OPTION::Overdrive:
        return "OVERDRIVE";
    case Audio_pluginAudioProcessor::DSP_OPTION::LadderFilter:
        return "LADDERFILTER";
    case Audio_pluginAudioProcessor::DSP_OPTION::GeneralFilter:
        return "GEN FILTER";
    case Audio_pluginAudioProcessor::DSP_OPTION::END_OF_LIST:
        jassertfalse;
        
    }

    return "NO SELECTION";
}

HorizontalConstrainer::HorizontalConstrainer(std::function<juce::Rectangle<int>()> confinerBoundsGetter,
    std::function<juce::Rectangle<int>()> confineeBoundsGetter) :
    boundsToConfineToGetter(std::move(confinerBoundsGetter)),
    boundsOfConfineeGetter(std::move(confineeBoundsGetter))
{

}

void HorizontalConstrainer::checkBounds(juce::Rectangle<int>& bounds,
    const juce::Rectangle<int>& previousBounds,
    const juce::Rectangle<int>& limits,
    bool isStretchingTop,
    bool isStretchingLeft,
    bool isStretchingBottom,
    bool isStretchingRight)
{
    bounds.setY(previousBounds.getY());

    if (boundsToConfineToGetter != nullptr &&
        boundsOfConfineeGetter != nullptr)
    {
        auto boundsToConfineTo = boundsToConfineToGetter();
        auto boundsOfConfinee = boundsOfConfineeGetter();

        bounds.setX(juce::jlimit(boundsToConfineTo.getX(),
            boundsToConfineTo.getRight() - boundsOfConfinee.getWidth(),
            bounds.getX()));
    }
    else
    {
        bounds.setX(juce::jlimit(limits.getX(),
            limits.getY(),
            bounds.getX()));
    }
}

ExtendedTabBarButton::ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& owner) : 
    juce::TabBarButton(name, owner)
{
    constrainer = std::make_unique<HorizontalConstrainer>([&owner]() { return owner.getLocalBounds(); },
        [this]() { return getBounds(); });

     constrainer->setMinimumOnscreenAmounts(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
}

juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton(const juce::String& tabName, int tabIndex)
{
    return new ExtendedTabBarButton(tabName, *this);
}
//==============================================================================
Audio_pluginAudioProcessorEditor::Audio_pluginAudioProcessorEditor (Audio_pluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    dspOrderButton.onClick = [this]()
        {
            juce::Random r;
            Audio_pluginAudioProcessor::DSP_Order dspOrder;
            
            auto range = juce::Range<int>(static_cast<int>(Audio_pluginAudioProcessor::DSP_OPTION::Phase),
                                        static_cast<int>(Audio_pluginAudioProcessor::DSP_OPTION::END_OF_LIST));


            tabbedComponent.clearTabs();
            for(auto& v : dspOrder) {
                auto entry = r.nextInt(range);
                v = static_cast<Audio_pluginAudioProcessor::DSP_OPTION>(entry);
                tabbedComponent.addTab(getDspOptionName(v), juce::Colours::white, -1);
            }

            DBG(juce::Base64::toBase64(dspOrder.data(),dspOrder.size() ));
            
            
            audioProcessor.dspOrderFifo.push(dspOrder);
        };



    addAndMakeVisible(dspOrderButton);
    addAndMakeVisible(tabbedComponent);
    setSize (400, 300);
}

Audio_pluginAudioProcessorEditor::~Audio_pluginAudioProcessorEditor()
{
}

//==============================================================================
void Audio_pluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void Audio_pluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    dspOrderButton.setBounds(bounds.removeFromTop(30).withSizeKeepingCentre(150,30));
    bounds.removeFromTop(10);
    tabbedComponent.setBounds(bounds.withHeight(30));

}
