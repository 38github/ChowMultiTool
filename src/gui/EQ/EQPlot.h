#pragma once

#include "dsp/EQ/EQProcessor.h"
#include "gui/Shared/DotSlider.h"
#include "state/PluginState.h"

namespace gui::eq
{
constexpr auto numBands = dsp::eq::EQToolParams::numBands;
class EQPlot : public chowdsp::EQ::EqualizerPlotWithParameters<numBands>
{
public:
    EQPlot (chowdsp::PluginState& pluginState, chowdsp::EQ::StandardEQParameters<numBands>& eqParameters);

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    struct QDotSlider : SpectrumDotSlider
    {
        using SpectrumDotSlider::SpectrumDotSlider;
        juce::Rectangle<float> getThumbBounds() const noexcept override;
        double proportionOfLengthToValue (double proportion) override;
        double valueToProportionOfLength (double value) override;
    };

    std::array<std::optional<SpectrumDotSlider>, numBands> freqSliders;
    std::array<std::optional<SpectrumDotSlider>, numBands> gainSliders;
    std::array<std::optional<QDotSlider>, numBands> qSliders;
    std::array<DotSliderGroup, numBands> sliderGroups;

    chowdsp::ScopedCallbackList callbacks;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQPlot)
};
} // namespace gui::eq