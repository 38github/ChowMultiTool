#include "ChowMultiTool.h"
#include "gui/PluginEditor.h"
#include "state/PresetManager.h"
#if HAS_CLAP_JUCE_EXTENSIONS
#include "state/PresetDiscovery.h"
#endif

namespace
{
const juce::String logFileSubDir = "ChowdhuryDSP/ChowMultiTool/Logs";
const juce::String logFileNameRoot = "ChowMultiTool_Log_";
} // namespace

ChowMultiTool::ChowMultiTool() : chowdsp::PluginBase<State> (&undoManager, createBusLayout()),
                                 logger (logFileSubDir, logFileNameRoot)
{
    juce::Logger::writeToLog (chowdsp::PluginDiagnosticInfo::getDiagnosticsString (*this));
    pluginSettings->initialise (chowdsp::toString (settingsFilePath));

    presetManager = std::make_unique<state::presets::PresetManager> (*this);
    programAdaptor = std::make_unique<chowdsp::presets::frontend::PresetsProgramAdapter> (presetManager);
}

juce::AudioProcessor::BusesProperties ChowMultiTool::createBusLayout()
{
    return BusesProperties()
        .withInput ("Main", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Main", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Band-Split (Low)", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Band-Split (Mid)", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Band-Split (High)", juce::AudioChannelSet::stereo(), true);
}

bool ChowMultiTool::isBusesLayoutSupported (const BusesLayout& layout) const
{
    const auto mainInputLayout = layout.getMainInputChannelSet();
    if (mainInputLayout.isDisabled() || mainInputLayout.isDiscreteLayout())
        return false;

    for (const auto& bus : layout.outputBuses)
    {
        if (bus.isDiscreteLayout() || bus.isDisabled())
            return false;
    }

    return true;
}

void ChowMultiTool::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    processor.prepare ({ sampleRate, (uint32_t) samplesPerBlock, (uint32_t) getMainBusNumInputChannels() });
}

void ChowMultiTool::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{
    juce::ScopedNoDenormals noDenormals;

    state.getParameterListeners().callAudioThreadBroadcasters();

    processor.processBlock (buffer, midiBuffer);

    chowdsp::BufferMath::sanitizeBuffer (buffer);
}

juce::AudioProcessorEditor* ChowMultiTool::createEditor()
{
    return new gui::PluginEditor { *this };
}

#if HAS_CLAP_JUCE_EXTENSIONS
bool ChowMultiTool::remoteControlsPageFill (uint32_t pageIndex,
                                            juce::String& sectionName,
                                            uint32_t& pageID,
                                            juce::String& pageName,
                                            std::array<juce::AudioProcessorParameter*, CLAP_REMOTE_CONTROLS_COUNT>& params) noexcept
{
    remoteControls.pageFill (pageIndex, sectionName, pageID, pageName, params);
    return true;
}

bool ChowMultiTool::presetLoadFromLocation (uint32_t location_kind,
                                            const char* location,
                                            const char* load_key) noexcept
{
    return state::presets::discovery::presetLoadFromLocation (*presetManager, location_kind, location, load_key);
}
#endif

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChowMultiTool();
}

#if HAS_CLAP_JUCE_EXTENSIONS
static const struct clap_preset_discovery_factory presetDiscoveryFactory
{
    .count = &state::presets::discovery::count,
    .get_descriptor = &state::presets::discovery::get_descriptor,
    .create = &state::presets::discovery::create,
};

const void* clapJuceExtensionCustomFactory (const char* factory_id)
{
    if (strcmp (factory_id, CLAP_PRESET_DISCOVERY_FACTORY_ID) == 0)
    {
        return &presetDiscoveryFactory;
    }

    return nullptr;
}
#endif
