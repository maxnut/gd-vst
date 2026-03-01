#pragma once

#include "juce_gui_basics/juce_gui_basics.h"

class HelpIcon : public juce::Component, public juce::TooltipClient {
public:
    HelpIcon (juce::String tip) : m_tip(tip) {}
    HelpIcon() = default;

    void paint (juce::Graphics& g) override {
        g.setColour (juce::Colours::black);
        g.setFont (20.0f);
        g.drawText ("?", getLocalBounds(), juce::Justification::centred, false);
    }

    void setTooltip(juce::String tooltip) { m_tip = tooltip; }
    juce::String getTooltip() override { return m_tip; }

private:
    juce::String m_tip;
};