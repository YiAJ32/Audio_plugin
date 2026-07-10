/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <RotarySliderWithLabels.h>
#include <Utilities.h>

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

        auto centerX = tabButtonBeingDrag->getBounds().getCentreX();
        if (previousTab == nullptr && nextTab != nullptr) {

            if (centerX > nextTab->getX()) {
                moveTab(idx, nextTabIndex);
            }
        }
        else if (previousTab != nullptr && nextTab == nullptr) {

            if (centerX < previousTab->getX()) {
                moveTab(idx, previousTabIndex);
            }
        }
        else {
            if (centerX > nextTab->getX()) {
                moveTab(idx, nextTabIndex);
            }else if (centerX < previousTab->getRight()) {
                moveTab(idx, previousTabIndex);
            }
        }

        tabButtonBeingDrag->toFront(true);
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


void DSP_GUI::resized() 
{
    auto bounds = getLocalBounds();
    if (buttons.empty() == false)
    {
        auto buttonArea = bounds.removeFromTop(30);
        auto w = buttonArea.getWidth() / buttons.size();

        for (auto& button : buttons)
        {
            button->setBounds(buttonArea.removeFromLeft(static_cast<int> (w)));
        }
    }

    if (comboBoxes.empty() == false)
    {
        auto comboBoxArea = bounds.removeFromLeft(bounds.getWidth()/4);
        auto h = juce::jmin(comboBoxArea.getHeight() / static_cast<int>(comboBoxes.size()),30);

        for (auto& cb : comboBoxes)
        {
            cb->setBounds(comboBoxArea.removeFromTop(static_cast<int> (h)));
        }
    }

    if (sliders.empty() == false)
    {

        auto w = bounds.getWidth() / sliders.size();

        for (auto& slider : sliders)
        {
            slider->setBounds(bounds.removeFromLeft(static_cast<int> (w)));
        }
    }
};

void DSP_GUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black); 
};

void DSP_GUI::rebuildInterface(std::vector < juce::RangedAudioParameter* > params)
{
    sliderAttachements.clear();
    comboBoxAttachements.clear();
    buttonAttachements.clear();

    sliders.clear();
    comboBoxes.clear();
    buttons.clear();

    for (size_t i = 0; i < params.size(); ++i)
    {
        auto p = params[i];

        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(p))
        {
            comboBoxes.push_back(std::make_unique<juce::ComboBox>());
            auto& cb = *comboBoxes.back();
            cb.addItemList(choice->choices,1);

            comboBoxAttachements.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
            (processor.apvts,p->getName(100),cb));
        }
        else if (auto* toggle = dynamic_cast<juce::AudioParameterBool*>(p))
        {
            buttons.push_back(std::make_unique<juce::ToggleButton>("Bypass"));
            auto& btn = *buttons.back();
            buttonAttachements.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>
                (processor.apvts, p->getName(100), btn));
        }
        else
        {
            sliders.push_back(std::make_unique<RotarySliderWithLabels>(p, p->label, p->getName(100) ));
            auto& slider = *sliders.back();
            SimpleMBComp::addLabelPairs(slider.labels, *p, p->label);
            slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
            sliderAttachements.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                (processor.apvts, p->getName(100), slider));
        }
    }

    for (auto& slider : sliders)
        addAndMakeVisible(slider.get());

    for (auto& cb : comboBoxes)
        addAndMakeVisible(cb.get());

    for (auto& btn : buttons)
        addAndMakeVisible(btn.get());

    resized();
}

Audio_pluginAudioProcessorEditor::Audio_pluginAudioProcessorEditor (Audio_pluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    addAndMakeVisible(tabbedComponent);
    addAndMakeVisible(dspGUI);
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

DSP_GUI::DSP_GUI(Audio_pluginAudioProcessor& proc) : processor(proc)
{

}

void Audio_pluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(10);
    tabbedComponent.setBounds(bounds.removeFromTop(30));
    dspGUI.setBounds(bounds);
}

void Audio_pluginAudioProcessorEditor::tabOrderChange(Audio_pluginAudioProcessor::DSP_Order newOrder) 
{
    rebuildInterface();
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

    rebuildInterface();
    audioProcessor.dspOrderFifo.push(newOrder);
}

void Audio_pluginAudioProcessorEditor::rebuildInterface()
{
    auto currentTabIndex = tabbedComponent.getCurrentTabIndex();
    auto currentTab = tabbedComponent.getTabButton(currentTabIndex);
    if (auto etbb = dynamic_cast<ExtendedTabBarButton*>(currentTab))
    {
        auto option = etbb->getOption();
        auto params = audioProcessor.getParamsForOption(option);
        jassert(params.empty() == false);
        dspGUI.rebuildInterface(params);
    }
}