/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


static juce::String getNameFromDspOption(Audio_pluginAudioProcessor::DSP_OPTION option)
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

static Audio_pluginAudioProcessor::DSP_OPTION getDSPOptionFromName(juce::String name)
{

    if (name == "PHASE")
        return Audio_pluginAudioProcessor::DSP_OPTION::Phase;
    if (name == "CHORUS")
        return Audio_pluginAudioProcessor::DSP_OPTION::Chorus;
    if (name == "OVERDRIVE")
        return Audio_pluginAudioProcessor::DSP_OPTION::Overdrive;
    if (name == "LADDERFILTER")
        return Audio_pluginAudioProcessor::DSP_OPTION::LadderFilter;
    if (name == "GEN FILTER")
        return Audio_pluginAudioProcessor::DSP_OPTION::GeneralFilter;

    return Audio_pluginAudioProcessor::DSP_OPTION::END_OF_LIST;
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

ExtendedTabBarButton::ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& owner,
    Audio_pluginAudioProcessor::DSP_OPTION dspOption) : 
    juce::TabBarButton(name, owner), option(dspOption)
{
    constrainer = std::make_unique<HorizontalConstrainer>([&owner]() { return owner.getLocalBounds(); },
        [this]() { return getBounds(); });

     constrainer->setMinimumOnscreenAmounts(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);


}

int ExtendedTabBarButton::getBestTabLength(int depth)
{
    auto bestWidth = getLookAndFeel().getTabButtonBestWidth(*this, depth);
    auto& bar = getTabbedButtonBar();

    return juce::jmax(bestWidth, bar.getWidth() / bar.getNumTabs());
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
    juce::TabbedButtonBar(juce::TabbedButtonBar::Orientation::TabsAtTop) 
 {
     auto img = juce::Image(juce::Image::PixelFormat::SingleChannel, 1, 1, true);
     auto gfx = juce::Graphics(img);
     gfx.fillAll(juce::Colours::transparentBlack);

     dragImage = juce::ScaledImage(img, 1.0);
 }

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

    auto tabs = getTabs();
    Audio_pluginAudioProcessor::DSP_Order newOrder;
    jassert(tabs.size() == newOrder.size());

    for (size_t i = 0; i < tabs.size(); ++i) {
        auto tab = tabs[static_cast<int>(i)];

        if (auto* etbb = dynamic_cast<ExtendedTabBarButton*>(tab)) {
            newOrder[i] = etbb->getOption();
        }
    }

    listeners.call([newOrder](Listener& l)
        {
            l.tabOrderChange(newOrder);
        });
   
}

void ExtendedTabbedButtonBar::mouseDown(const juce::MouseEvent& e)
{
    DBG("ExtendedTabbedButtonBar::mouseDown");
    if (auto tabButtonBeingDrag = dynamic_cast<ExtendedTabBarButton*>(e.originalComponent)) {
        startDragging(tabButtonBeingDrag->TabBarButton::getTitle(), tabButtonBeingDrag,
            dragImage);
    }
}

juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton(const juce::String& tabName, int tabIndex)
{
    auto dspOption = getDSPOptionFromName(tabName);
    auto etbb = std::make_unique<ExtendedTabBarButton>(tabName, *this, dspOption);

    etbb->addMouseListener(this, false);
    return etbb.release();

}

void ExtendedTabbedButtonBar::addListener(Listener *l)
{
    listeners.add(l);
}

void ExtendedTabbedButtonBar::removeListener(Listener *l)
{
    listeners.remove(l);
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
                auto name = getNameFromDspOption(v);
                DBG("creating" << name);
                tabbedComponent.addTab(name, juce::Colours::white, -1);
            }

           // DBG(juce::Base64::toBase64(dspOrder.data(),dspOrder.size() ));
            
            
            audioProcessor.dspOrderFifo.push(dspOrder);
        };



    addAndMakeVisible(dspOrderButton);
    addAndMakeVisible(tabbedComponent);

    tabbedComponent.addListener(this);
    startTimerHz(30);
    setSize (600, 400);
}

Audio_pluginAudioProcessorEditor::~Audio_pluginAudioProcessorEditor()
{
    tabbedComponent.removeListener(this);
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

void Audio_pluginAudioProcessorEditor::tabOrderChange(Audio_pluginAudioProcessor::DSP_Order newOrder) 
{
    audioProcessor.dspOrderFifo.push(newOrder);
}


void Audio_pluginAudioProcessorEditor::timerCallback()
{
    if (audioProcessor.restoreDspOrderFifo.getNumAvailableForReading() == 0)
        return;
    using T = Audio_pluginAudioProcessor::DSP_Order;
    T newOrder;
    newOrder.fill(Audio_pluginAudioProcessor::DSP_OPTION::END_OF_LIST);
    auto empty = newOrder;
    while (audioProcessor.restoreDspOrderFifo.pull(newOrder)) 
    {

    }

    if (newOrder != empty)
    {
        addTabsFromDSPOrder(newOrder);
    }
}

void Audio_pluginAudioProcessorEditor::addTabsFromDSPOrder(Audio_pluginAudioProcessor::DSP_Order newOrder)
{
    tabbedComponent.clearTabs();
    for (auto v : newOrder)
    {
        tabbedComponent.addTab(getNameFromDspOption(v), juce::Colours::white, -1);
    }
    audioProcessor.dspOrderFifo.push(newOrder);
}