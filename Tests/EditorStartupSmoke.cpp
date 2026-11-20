#include <JuceHeader.h>
#include <memory>
#include <iostream>

extern juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

int main() {
    // Run the same Editor constructor and resized() path as AU/VST3/Standalone.
    juce::ScopedJuceInitialiser_GUI juceInit;
    std::unique_ptr<juce::AudioProcessor> processor(createPluginFilter());
    if (!processor) return 10;

    // In v0.1.1 this call immediately segfaulted in SerenityEditor::resized().
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor->createEditor());
    if (!editor) return 11;
    if (editor->getWidth() != 1380 || editor->getHeight() != 900) return 12;

    // Exercise synchronous layout callbacks, without opening another window.
    editor->setSize(1379, 899);
    editor->setSize(1380, 900);

    // Trigger the 10 page buttons to exercise repaint/layout on every tab.
    for (int i = 0; i < 10; ++i) {
        auto* button = dynamic_cast<juce::Button*>(editor->getChildComponent(i));
        if (!button) return 13;
        button->triggerClick();
    }

    if (editor->getWidth() != 1380 || editor->getHeight() != 900) return 14;
    editor.reset();
    processor.reset();
    std::cout << "SERENITY_EDITOR_STARTUP_OK\n";
    return 0;
}
