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

//==============================================================================
void ExtendedTabBarButton::mouseDown(const juce::MouseEvent& e) 
{
        toFront(true);
        dragger.startDraggingComponent(this, e);
        juce::TabBarButton::mouseDown(e);
}
void ExtendedTabBarButton::mouseDrag(const juce::MouseEvent& e)
{
        dragger.dragComponent(this, e, constrainer.get());
}

//==============================================================================
 ExtendedTabbedButtonBar::ExtendedTabbedButtonBar() :
    juce::TabbedButtonBar(juce::TabbedButtonBar::Orientation::TabsAtTop) {}

bool ExtendedTabbedButtonBar::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
    if (dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get()))
        return true;
    return false; 
}



void ExtendedTabbedButtonBar::itemDragEnter(const SourceDetails& dragSourceDetails)
{
    DBG("ExtendedTabbedButtonBar::itemDragExit");
    juce::DragAndDropTarget::itemDragEnter(dragSourceDetails);

}

void ExtendedTabbedButtonBar::itemDragExit(const SourceDetails& dragSourceDetails)
{
    DBG("ExtendedTabbedButtonBar::itemDragExit");
    juce::DragAndDropTarget::itemDragExit(dragSourceDetails);

}

juce::Array<juce::TabBarButton*>  ExtendedTabbedButtonBar::getTabs()
{
    auto numTabs = getNumTabs();
    auto tabs = juce::Array<juce::TabBarButton*>();
    tabs.resize(numTabs);
    for (int i = 0; i < numTabs; ++i) {
        tabs.getReference(i) = getTabButton(i);
    }

    return tabs;
}

int ExtendedTabbedButtonBar::findDraggedItemIndex(const SourceDetails& dragSourceDetails)
{
    if (auto tabButtonBeingDrag = dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get()))
    {
        auto tabs = getTabs();
        auto idx = tabs.indexOf(tabButtonBeingDrag);
        return idx;
    }

    return -1;
}

juce::TabBarButton* ExtendedTabbedButtonBar::findDragItem(const SourceDetails &dragSourceDetails)
{
    return getTabButton(findDraggedItemIndex(dragSourceDetails));
}

void ExtendedTabbedButtonBar::itemDragMove(const SourceDetails& dragSourceDetails)
{
    if (auto tabButtonBeingDrag = dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get() )) {
          
        auto idx = findDraggedItemIndex(dragSourceDetails);
        if (idx == -1) {
            DBG("Failed to find drag tab in the list of tabs");
            jassertfalse;
            return;
        }

        auto previousTabIndex = idx - 1;
        auto nextTabIndex = idx + 1;
        auto previousTab = getTabButton(previousTabIndex);
        auto nextTab = getTabButton(nextTabIndex);

        if (previousTab == nullptr && nextTab != nullptr) {

            if (tabButtonBeingDrag->getX() > nextTab->getBounds().getCentreX()) {
                moveTab(idx, nextTabIndex);
            }
        }
        else if (previousTab != nullptr && nextTab == nullptr) {

            if (tabButtonBeingDrag->getX() < previousTab->getBounds().getCentreX()) {
                moveTab(idx, previousTabIndex);
            }
        }
        else {
            if (tabButtonBeingDrag->getX() > nextTab->getBounds().getCentreX()) {
                moveTab(idx, nextTabIndex);
            }else if (tabButtonBeingDrag->getX() < previousTab->getBounds().getCentreX()) {
                moveTab(idx, previousTabIndex);
            }
        }

    }

}

void ExtendedTabbedButtonBar::itemDropped(const SourceDetails& dragSourceDetails)  
{
    resized();
}

void ExtendedTabbedButtonBar::mouseDown(const juce::MouseEvent& e)
{
    DBG("ExtendedTabbedButtonBar::mouseDown");
    if (auto tabButtonBeingDrag = dynamic_cast<ExtendedTabBarButton*>(e.originalComponent)) {
        startDragging(tabButtonBeingDrag->TabBarButton::getTitle(), tabButtonBeingDrag);
    }
}

juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton(const juce::String& tabName, int tabIndex)
{
    auto etbb = std::make_unique<ExtendedTabBarButton>(tabName, *this);

    etbb->addMouseListener(this, false);
    return etbb.release();

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
