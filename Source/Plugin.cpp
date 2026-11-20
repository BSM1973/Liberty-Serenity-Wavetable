#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <cmath>
#include <memory>
#include <vector>

namespace serenity {
enum Param {
    master, cutoff, resonance, position, attack, decay, sustain, release,
    delayMix, reverbMix, oscALevel, oscBLevel, oscCLevel, oscDLevel,
    waveA, waveB, waveC, waveD,
    brightness, movement, warmth, space, width, texture, energy, serenityMacro,
    parameterCount
};
struct Specification { const char* id; float lo, hi, initial; };
static constexpr std::array<Specification, parameterCount> definitions {{
    {"master",0.f,1.5f,0.8f}, {"cutoff",0.f,1.f,0.85f},
    {"resonance",0.f,1.f,0.12f}, {"position",0.f,1.f,0.5f},
    {"attack",0.002f,8.f,0.008f}, {"decay",0.005f,8.f,0.4f},
    {"sustain",0.f,1.f,0.7f}, {"release",0.005f,12.f,0.4f},
    {"delayMix",0.f,1.f,0.f}, {"reverbMix",0.f,1.f,0.1f},
    {"oscALevel",0.f,1.f,0.7f}, {"oscBLevel",0.f,1.f,0.f},
    {"oscCLevel",0.f,1.f,0.f}, {"oscDLevel",0.f,1.f,0.f},
    {"waveA",0.f,3.f,1.f}, {"waveB",0.f,3.f,0.f},
    {"waveC",0.f,3.f,2.f}, {"waveD",0.f,3.f,3.f},
    {"brightness",0.f,1.f,0.5f}, {"movement",0.f,1.f,0.5f},
    {"warmth",0.f,1.f,0.5f}, {"space",0.f,1.f,0.5f},
    {"width",0.f,1.f,0.5f}, {"texture",0.f,1.f,0.5f},
    {"energy",0.f,1.f,0.5f}, {"serenity",0.f,1.f,0.5f}
}};
static const char* const tabEN[] = {
    "OSCILLATORS","WAVETABLE EDITOR","FILTERS","MODULATION","EFFECTS",
    "ARPEGGIATOR","SEQUENCER","MATRIX","PRESETS","SETTINGS"
};
static const char* const tabFR[] = {
    "OSCILLATEURS","ÉDITEUR WAVETABLE","FILTRES","MODULATION","EFFETS",
    "ARPÉGIATEUR","SÉQUENCEUR","MATRICE","PRESETS","RÉGLAGES"
};
static const char* const macroEN[] = {
    "BRIGHTNESS","MOVEMENT","WARMTH","SPACE",
    "WIDTH","TEXTURE","ENERGY","SERENITY"
};
static const char* const macroFR[] = {
    "BRILLANCE","MOUVEMENT","CHALEUR","ESPACE",
    "LARGEUR","TEXTURE","ÉNERGIE","SÉRÉNITÉ"
};
static juce::String factoryName(int index) {
    static const char* const names[] = {
        "Serenity Awakening","Liquid Dreams","Aurora Motion","Crystal Horizon",
        "Velvet Universe","Midnight Reflections","Celestial Waves","Quantum Drift",
        "Metallic Pulse","Digital Emotions","Golden Frequency","Ocean of Light",
        "Infinite Atmosphere","Ethereal Memories","Future Harmony","Obsidian Growl"
    };
    return juce::String(index + 1).paddedLeft('0',3) + " · " + names[index % 16];
}
static constexpr float pi = 3.14159265358979323846f;
static float waveform(int type, double phase) noexcept {
    const float p = (float)(phase - std::floor(phase));
    const float s = std::sin(2.f*pi*p);
    switch (type) {
        case 1: return 2.f * p - 1.f;
        case 2: return p < .5f ? 1.f : -1.f;
        case 3: return 1.f - 4.f * std::abs(p - .5f);
        default: return s;
    }
}
}

class SerenityProcessor final : public juce::AudioProcessor {
public:
    using TreeState = juce::AudioProcessorValueTreeState;
    static TreeState::ParameterLayout layout() {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
        for (int i=0; i<serenity::parameterCount; ++i) {
            const auto& spec = serenity::definitions[(size_t)i];
            const float step = i>=serenity::waveA && i<=serenity::waveD ? 1.f : 0.0001f;
            p.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID{spec.id,1}, spec.id,
                juce::NormalisableRange<float>{spec.lo,spec.hi,step},spec.initial));
        }
        return {p.begin(),p.end()};
    }

    SerenityProcessor() :
        juce::AudioProcessor(BusesProperties().withOutput("Stereo",juce::AudioChannelSet::stereo(),true)),
        params(*this,nullptr,"SERENITY",layout()) {
        for (auto& m: mappings) m.store(-1);
        for (auto& v: overrides) v.store(-1.f);
    }
    const juce::String getName() const override { return "Serenity WAVETABLE"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Serenity"; }
    void changeProgramName(int, const juce::String&) override {}
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    bool isBusesLayoutSupported(const BusesLayout& l) const override {
        return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }

    void prepareToPlay(double sr, int) override {
        rate = std::max(sr, 8000.0);
        for (auto& v: voices) v = {};
        const size_t n = (size_t)std::max(1.0,rate * 0.75);
        delayLeft.assign(n,0.f); delayRight.assign(n,0.f);
        delayHead = 0; reverbL = reverbR = filterL = filterR = 0.f;
    }
    void releaseResources() override {}
    void setParamNormalized(int index, float normalized) {
        if (index<0 || index>=serenity::parameterCount) return;
        if (auto* p=params.getParameter(serenity::definitions[(size_t)index].id)) {
            overrides[(size_t)index].store(-1.f);
            p->beginChangeGesture();
            p->setValueNotifyingHost(juce::jlimit(0.f,1.f,normalized));
            p->endChangeGesture();
        }
    }
    float normalized(int index) const {
        if (index<0 || index>=serenity::parameterCount) return 0.f;
        if (auto* p=params.getParameter(serenity::definitions[(size_t)index].id))
            return p->getValue();
        return 0.f;
    }
    void armLearn(int index) { learn.store(index); }
    int getLearnTarget() const { return learn.load(); }
    int mappedCC(int index) const {return mappings[(size_t)index].load();}
    void clearMappings() {
        for (auto& v:mappings) v.store(-1);
        for (auto& v:overrides) v.store(-1.f);
        learn.store(-1);
    }
    bool isFrench() const {return french.load();}
    void setFrench(bool value) {french.store(value);}
    void loadFactoryPreset(int index) {
        if (index<0||index>=100) return;
        selectedPreset.store(index);
        setParamNormalized(serenity::waveA,(float)(index%4)/3.f);
        setParamNormalized(serenity::waveB,(float)((index/3)%4)/3.f);
        setParamNormalized(serenity::oscBLevel,(float)((index%5)+1)/10.f);
        setParamNormalized(serenity::position,(float)(index%17)/16.f);
        setParamNormalized(serenity::cutoff,.18f+(float)(index%12)/15.f);
        setParamNormalized(serenity::attack,(float)(index%9)/14.f);
        setParamNormalized(serenity::release,(float)(index%8)/12.f);
    }
    int preset() const {return selectedPreset.load();}

    void processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi) override {
        juce::ScopedNoDenormals denormals;
        buffer.clear();
        if (buffer.getNumChannels()<2) return;
        std::array<float,serenity::parameterCount> p{};
        for (int i=0;i<serenity::parameterCount;++i) {
            const auto& d = serenity::definitions[(size_t)i];
            const auto* raw=params.getRawParameterValue(d.id);
            const float override=overrides[(size_t)i].load(std::memory_order_relaxed);
            p[(size_t)i]=override>=0.f ? d.lo+(d.hi-d.lo)*override :
                raw!=nullptr ? raw->load(std::memory_order_relaxed) : d.initial;
        }
        auto* l=buffer.getWritePointer(0); auto* r=buffer.getWritePointer(1);
        int cursor=0;
        for (const auto event:midi) {
            const int until=juce::jlimit(cursor,buffer.getNumSamples(),event.samplePosition);
            render(l,r,cursor,until,p);
            cursor=until;
            const auto& msg=event.getMessage();
            if (msg.isNoteOn()) noteOn(msg.getNoteNumber(),msg.getFloatVelocity());
            else if (msg.isNoteOff()) noteOff(msg.getNoteNumber());
            else if (msg.isAllNotesOff()||msg.isAllSoundOff()) allOff();
            else if (msg.isController()) {
                const int cc=msg.getControllerNumber();
                const float value=msg.getControllerValue()/127.f;
                const int target=learn.exchange(-1);
                if (target>=0 && target<serenity::parameterCount)
                    mappings[(size_t)target].store(cc);
                for (int i=0;i<serenity::parameterCount;++i) {
                    if (mappings[(size_t)i].load()==cc) {
                        overrides[(size_t)i].store(value);
                        const auto& d=serenity::definitions[(size_t)i];
                        p[(size_t)i]=d.lo+(d.hi-d.lo)*value;
                    }
                }
            }
        }
        render(l,r,cursor,buffer.getNumSamples(),p);
    }

    void getStateInformation(juce::MemoryBlock& dest) override {
        auto state=params.copyState();
        state.setProperty("language",french.load() ? "FR":"EN",nullptr);
        state.setProperty("selectedPreset",selectedPreset.load(),nullptr);
        for(int i=0;i<serenity::parameterCount;++i)
            state.setProperty("cc"+juce::String(i),mappings[(size_t)i].load(),nullptr);
        if (auto xml=state.createXml()) copyXmlToBinary(*xml,dest);
    }
    void setStateInformation(const void* data,int length) override {
        auto xml=getXmlFromBinary(data,length);
        if (!xml) return;
        const auto state=juce::ValueTree::fromXml(*xml);
        if (!state.isValid()) return;
        french.store(state.getProperty("language","FR").toString()!="EN");
        selectedPreset.store((int)state.getProperty("selectedPreset",-1));
        for(int i=0;i<serenity::parameterCount;++i)
            mappings[(size_t)i].store((int)state.getProperty("cc"+juce::String(i),-1));
        params.replaceState(state);
    }
    TreeState params;

private:
    struct Voice {
        bool on=false; int note=0,stage=0; unsigned age=0;
        float velocity=0, env=0;
        std::array<double,4> phase{};
    };
    std::array<Voice,32> voices{};
    std::array<std::atomic<int>,serenity::parameterCount> mappings{};
    std::array<std::atomic<float>,serenity::parameterCount> overrides{};
    std::atomic<int> learn{-1}, selectedPreset{-1};
    std::atomic<bool> french{true};
    unsigned ageCounter=0;
    double rate=44100.0;
    std::vector<float> delayLeft,delayRight;
    size_t delayHead=0;
    float reverbL=0,reverbR=0,filterL=0,filterR=0;

    void noteOn(int note,float velocity) {
        Voice* target=nullptr;
        for(auto& v:voices) if(!v.on){target=&v;break;}
        if(!target) {
            target=&voices[0];
            for(auto& v:voices) if(v.age<target->age) target=&v;
        }
        *target={};
        target->on=true; target->note=note;
        target->velocity=velocity; target->stage=0; target->age=++ageCounter;
    }
    void noteOff(int note) {
        for(auto& v:voices) if(v.on&&v.note==note) v.stage=3;
    }
    void allOff() {for(auto& v:voices) v.on=false;}
    void render(float* left,float* right,int from,int to,const std::array<float,serenity::parameterCount>& p) {
        const float master=p[serenity::master];
        const float sustain=p[serenity::sustain];
        const float attack=1.f/(float)(rate*std::max(.002f,p[serenity::attack]));
        const float decay=1.f/(float)(rate*std::max(.005f,p[serenity::decay]));
        const float release=1.f/(float)(rate*std::max(.005f,p[serenity::release]));
        const float cutoff=juce::jlimit(.003f,.99f,
             .003f+std::pow(juce::jlimit(0.f,1.f,p[serenity::cutoff]+(p[serenity::brightness]-.5f)*.2f),3.f)*.65f);
        const float delayMix=p[serenity::delayMix],reverbMix=p[serenity::reverbMix];
        for(int i=from;i<to;i++) {
            float sample=0;
            for(auto& v:voices) {
                if(!v.on)continue;
                switch(v.stage) {
                    case 0: v.env+=attack; if(v.env>=1.f){v.env=1.f;v.stage=1;} break;
                    case 1: v.env-=decay*(1.f-sustain); if(v.env<=sustain){v.env=sustain;v.stage=2;} break;
                    case 2: v.env=sustain; break;
                    default: v.env-=release; if(v.env<=0){v.on=false;v.env=0;continue;} break;
                }
                const double base=440.0*std::pow(2.0,((double)v.note-69.0)/12.0);
                float mix=0,weight=0;
                for(int o=0;o<4;o++) {
                    const float level=p[(size_t)serenity::oscALevel+(size_t)o];
                    if(level<=.0001f)continue;
                    const int kind=juce::jlimit(0,3,(int)std::round(p[(size_t)serenity::waveA+(size_t)o]));
                    const double octave=(o==3? .5:1.);
                    const double increment=base*octave/rate;
                    v.phase[(size_t)o]+=increment;
                    if(v.phase[(size_t)o]>=1.)v.phase[(size_t)o]-=std::floor(v.phase[(size_t)o]);
                    mix+=level*serenity::waveform(kind,v.phase[(size_t)o]);weight+=level;
                }
                if(weight>0)sample+=(mix/weight)*v.env*v.velocity*.14f;
            }
            // Two stable low-pass poles; parameters are smoothed by the feedback network.
            filterL+=cutoff*(sample-filterL);
            filterR+=cutoff*(filterL-filterR);
            float wetL=filterR,wetR=filterR;
            if (!delayLeft.empty()) {
                const size_t n=delayLeft.size();
                const size_t d=(size_t)juce::jlimit(1.0,rate*.35,(double)n-1.);
                const size_t read=(delayHead+n-d)%n;
                const float dl=delayLeft[read], dr=delayRight[read];
                delayLeft[delayHead]=wetL+dr*.28f;
                delayRight[delayHead]=wetR+dl*.28f;
                delayHead=(delayHead+1)%n;
                wetL+=dl*delayMix*.4f; wetR+=dr*delayMix*.4f;
            }
            reverbL=.82f*reverbL+.18f*wetR;
            reverbR=.85f*reverbR+.15f*wetL;
            left[i]=juce::jlimit(-1.f,1.f,(wetL+reverbL*reverbMix*.16f)*master);
            right[i]=juce::jlimit(-1.f,1.f,(wetR+reverbR*reverbMix*.16f)*master);
        }
    }
};

class LearnableSlider final: public juce::Slider {
public:
    std::function<void()> rightClick;
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.mods.isPopupMenu()) {
            if(rightClick) rightClick();
            return;
        }
        juce::Slider::mouseDown(e);
    }
};

class SerenityEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit SerenityEditor(SerenityProcessor& processor)
       : juce::AudioProcessorEditor(processor), proc(processor) {
        // Never call setSize before child controls exist: JUCE immediately invokes resized().
        setResizable(false,false);
        for(int i=0;i<10;i++) {
            auto b=std::make_unique<juce::TextButton>();
            b->onClick=[this,i]{ activeTab=i;refresh(); };
            addAndMakeVisible(*b);
            tabs[(size_t)i]=std::move(b);
        }
        for(int i=0;i<serenity::parameterCount;i++) {
            auto k=std::make_unique<LearnableSlider>();
            k->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            k->setTextBoxStyle(juce::Slider::TextBoxBelow,false,90,21);
            k->setRange(0,1,0.001);
            k->setValue(proc.normalized(i),juce::dontSendNotification);
            k->setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xff72d9e7));
            k->setColour(juce::Slider::thumbColourId,juce::Colour(0xffd9b964));
            k->onValueChange=[this,i]{proc.setParamNormalized(i,(float)knobs[(size_t)i]->getValue());};
            k->rightClick=[this,i] {
                proc.armLearn(i);
                hint.setText(proc.isFrench() ?
                    "MIDI Learn : déplacez un contrôleur CC pour " + juce::String(serenity::definitions[(size_t)i].id) :
                    "MIDI Learn: move a CC controller for " + juce::String(serenity::definitions[(size_t)i].id),
                    juce::dontSendNotification);
            };
            addAndMakeVisible(*k);
            knobs[(size_t)i]=std::move(k);
            auto label=std::make_unique<juce::Label>();
            label->setJustificationType(juce::Justification::centred);
            label->setColour(juce::Label::textColourId,juce::Colour(0xffb5c1d2));
            label->setFont(juce::Font(juce::FontOptions(12.0f)));
            label->setText(juce::String(serenity::definitions[(size_t)i].id),juce::dontSendNotification);
            addAndMakeVisible(*label);
            names[(size_t)i]=std::move(label);
        }
        presets.onChange=[this] {
            const int index=presets.getSelectedId()-1;
            if(index>=0) {proc.loadFactoryPreset(index);refreshKnobs();}
        };
        for(int i=0;i<100;i++)presets.addItem(serenity::factoryName(i),i+1);
        addAndMakeVisible(presets);
        french.setButtonText("FR");
        english.setButtonText("EN");
        french.onClick=[this]{proc.setFrench(true);refresh();};
        english.onClick=[this]{proc.setFrench(false);refresh();};
        addAndMakeVisible(french);addAndMakeVisible(english);
        midi.setButtonText("MIDI LEARN");
        midi.onClick=[this]{activeTab=9;refresh();};
        addAndMakeVisible(midi);
        addAndMakeVisible(hint);
        hint.setJustificationType(juce::Justification::centredLeft);
        hint.setFont(juce::Font(juce::FontOptions(13.f)));
        hint.setColour(juce::Label::textColourId,juce::Colour(0xffb9c5d7));
        // All tabs, knobs, labels, and header controls are fully constructed.
        uiReady = true;
        setSize(1380,900);
        refresh();
        startTimerHz(5);
    }

    void paint(juce::Graphics& g) override {
        using namespace juce;
        g.fillAll(Colour(0xff0a0f1a));
        g.setGradientFill(ColourGradient(Colour(0xff253348),0,0,Colour(0xff0d1524),0,(float)getHeight(),false));
        g.fillRect(getLocalBounds());
        auto panel=[&](float x,float y,float w,float h,Colour edge=Colour(0xff3a4a62)) {
            g.setColour(Colour(0xff1b2639)); g.fillRoundedRectangle(x,y,w,h,10.f);
            g.setColour(edge);g.drawRoundedRectangle(x,y,w,h,10.f,1.5f);
        };
        g.setColour(Colour(0xffd9e3f0));
        g.setFont(27.f);g.drawText("SERENITY",26,8,230,37,Justification::centredLeft);
        g.setColour(Colour(0xff74cbd7));
        g.setFont(14.f);g.drawText("W A V E T A B L E",28,43,245,23,Justification::centredLeft);
        g.setColour(Colour(0xffd8b66b));
        g.setFont(12.f);g.drawText("by LIBERTY MUSIC SOFTWARE",284,22,268,24,Justification::centredLeft);
        panel(10,110,1360,548);
        g.setColour(Colour(0xffd8b66b));g.setFont(15.f);
        g.drawText(proc.isFrench()?serenity::tabFR[activeTab]:serenity::tabEN[activeTab],
                   26,118,850,28,Justification::centredLeft);
        g.setColour(Colour(0xff6f8da7));g.drawHorizontalLine(150,22.f,1356.f);
        if(activeTab==1 || activeTab==0) {
            panel(24,162,activeTab==0?1000.f:1328.f,396.f);
            g.setColour(Colour(0xff223e4c));
            g.fillRoundedRectangle(40.f,205.f,activeTab==0?967.f:1292.f,280.f,5.f);
            juce::Path wave;
            for(int i=0;i<1000;i++) {
                const float x=56.f+i*(activeTab==0?0.93f:1.22f);
                const float y=341.f-75.f*std::sin((float)i*.023f)*(0.7f+.3f*std::sin((float)i*.004f));
                if(i==0)wave.startNewSubPath(x,y);else wave.lineTo(x,y);
            }
            g.setColour(Colour(0xff83e9f6));g.strokePath(wave,juce::PathStrokeType(2.1f));
            g.setColour(Colour(0xffa4b8cf));g.setFont(15.f);
            g.drawText(proc.isFrench() ? "Visualisation du moteur sonore" : "Sound engine waveform",
                       58,512,480,25,Justification::centredLeft);
        }
        else if(activeTab==7) {
            panel(24,167,1328,448);
            g.setFont(13.f);
            g.setColour(Colour(0xffbbcee5));
            g.drawText(proc.isFrench() ? "Matrice 64 routages · implantation complète en cours" :
                       "64-slot modulation matrix · full implementation in progress",
                       48,191,990,30,Justification::centredLeft);
            for(int i=0;i<24;i++) {
                const int col=i/12,row=i%12;
                const int x=49+col*650,y=239+row*29;
                g.setColour(Colour(0xff273950));
                g.fillRoundedRectangle((float)x,(float)y,609.f,24.f,3.f);
                g.setColour(Colour(0xff8e9fb6));
                g.drawText(String(i+1).paddedLeft('0',2)+"      OFF                    NONE                             0.00",
                           x+12,y,590,24,Justification::centredLeft);
            }
        } else {
            panel(24,165,1328,447);
            if(activeTab==5||activeTab==6) {
                g.setColour(Colour(0xff3f566c));
                for(int i=0;i<16;i++) {
                    auto x=53.f+i*79.f;
                    g.fillRoundedRectangle(x,302.f,50.f,(float)(i%4+1)*29.f,3.f);
                }
            }
            g.setColour(Colour(0xffa9bad1));g.setFont(17.f);
            juce::String message;
            if(activeTab==2)message=proc.isFrench()?"Filtre principal · fréquence et résonance":"Main filter · cutoff and resonance";
            if(activeTab==3)message=proc.isFrench()?"Enveloppe d'amplitude · contrôle ADSR":"Amplitude envelope · ADSR controls";
            if(activeTab==4)message=proc.isFrench()?"Traitements actifs : délai et réverbération":"Active processing: delay and reverb";
            if(activeTab==5)message=proc.isFrench()?"Arpégiateur avancé : développement en cours":"Advanced arpeggiator: in development";
            if(activeTab==6)message=proc.isFrench()?"Séquenceur 64 pas : développement en cours":"64-step sequencer: in development";
            if(activeTab==8)message=proc.isFrench()?"100 définitions de presets · non validées individuellement":"100 preset definitions · not individually validated";
            if(activeTab==9)message=proc.isFrench()?"Clic droit sur un potentiomètre pour apprendre un contrôleur MIDI CC.":
                                              "Right-click a knob to learn a MIDI CC controller.";
            g.drawText(message,46,185,1250,40,Justification::centredLeft);
        }
        panel(10,672,1100,210,Colour(0xff4a517e));
        g.setColour(Colour(0xffb2a6e8));g.setFont(13.f);
        g.drawText(proc.isFrench()?"MACROS · CLIC DROIT POUR MIDI LEARN":"MACROS · RIGHT CLICK FOR MIDI LEARN",
                   25,681,660,27,Justification::centredLeft);
        panel(1121,672,249,210);
        g.setColour(Colour(0xff99bed5));g.drawText("PERFORMANCE",1143,688,200,23,Justification::centredLeft);
        g.setColour(Colour(0xff74d9ea));
        g.fillRoundedRectangle(1150.f,770.f,180.f,5.f,2.f);
    }

    void resized() override {
        // Guard against framework-driven resize callbacks during construction.
        if (!uiReady) return;
        const int width=getWidth();
        for(int i=0;i<10;i++)tabs[(size_t)i]->setBounds(10+i*136,74,133,30);
        presets.setBounds(560,14,330,35);
        french.setBounds(width-430,14,47,32);
        english.setBounds(width-378,14,47,32);
        midi.setBounds(width-315,14,117,32);
        hint.setBounds(29,618,width-55,32);
        for(int i=0;i<serenity::parameterCount;i++) {
            knobs[(size_t)i]->setVisible(false);
            names[(size_t)i]->setVisible(false);
        }
        setKnob(serenity::master,width-172,9,76,65,"MASTER");
        for(int i=0;i<8;i++) {
            const int index=serenity::brightness+i;
            const int x=38+i*133;
            setKnob(index,x,725,93,109,proc.isFrench()?serenity::macroFR[i]:serenity::macroEN[i]);
        }
        if(activeTab==0) {
            for(int i=0;i<4;i++){
                setKnob(serenity::oscALevel+i,72+(i%2)*500,204+(i/2)*174,103,115,
                        juce::String("OSC ") + juce::String::charToString((juce_wchar)('A'+i))+" LEVEL");
                setKnob(serenity::waveA+i,235+(i%2)*500,204+(i/2)*174,106,115,"WAVE");
            }
        } else if(activeTab==1) setKnob(serenity::position,1107,371,100,115,"POSITION");
        else if(activeTab==2) {
            setKnob(serenity::cutoff,110,290,145,142,"CUTOFF");
            setKnob(serenity::resonance,330,290,145,142,"RESONANCE");
        } else if(activeTab==3) {
            const int ids[]={serenity::attack,serenity::decay,serenity::sustain,serenity::release};
            const char* captions[]={"ATTACK","DECAY","SUSTAIN","RELEASE"};
            for(int i=0;i<4;i++)setKnob(ids[i],90+i*256,307,144,130,captions[i]);
        } else if(activeTab==4) {
            setKnob(serenity::delayMix,111,307,150,130,"DELAY MIX");
            setKnob(serenity::reverbMix,362,307,150,130,"REVERB MIX");
        }
    }
private:
    SerenityProcessor& proc;
    std::array<std::unique_ptr<juce::TextButton>,10> tabs;
    std::array<std::unique_ptr<LearnableSlider>,serenity::parameterCount> knobs;
    std::array<std::unique_ptr<juce::Label>,serenity::parameterCount> names;
    juce::ComboBox presets;
    juce::TextButton french,english,midi;
    juce::Label hint;
    int activeTab=0;
    bool uiReady=false;
    void setKnob(int index,int x,int y,int w,int h,const juce::String& title) {
        auto& slider=*knobs[(size_t)index];
        slider.setBounds(x,y,w,h-26);
        slider.setVisible(true);
        auto& label=*names[(size_t)index];
        label.setText(title,juce::dontSendNotification);
        label.setBounds(x,y+h-29,w,26);
        label.setVisible(true);
    }
    void refreshKnobs() {
        for(int i=0;i<serenity::parameterCount;i++)
            knobs[(size_t)i]->setValue(proc.normalized(i),juce::dontSendNotification);
    }
    void refresh() {
        const bool fr=proc.isFrench();
        for(int i=0;i<10;i++){
            auto& b=*tabs[(size_t)i];
            b.setButtonText(fr?serenity::tabFR[i]:serenity::tabEN[i]);
            b.setColour(juce::TextButton::buttonColourId,
                i==activeTab?juce::Colour(0xff866c39):juce::Colour(0xff1c283c));
            b.setColour(juce::TextButton::textColourOffId,
                i==activeTab?juce::Colours::white:juce::Colour(0xffa5bad2));
        }
        french.setColour(juce::TextButton::buttonColourId,fr?juce::Colour(0xff66572d):juce::Colour(0xff1e2b3d));
        english.setColour(juce::TextButton::buttonColourId,!fr?juce::Colour(0xff66572d):juce::Colour(0xff1e2b3d));
        if(proc.getLearnTarget()<0)hint.setText(fr?"FR par défaut · clic droit sur un contrôle = MIDI Learn":
            "EN selected · right-click a control = MIDI Learn",juce::dontSendNotification);
        resized();repaint();
    }
    void timerCallback() override {
        if(proc.getLearnTarget()<0 && hint.getText().containsIgnoreCase("MIDI Learn"))
            hint.setText(proc.isFrench()?"FR par défaut · clic droit sur un contrôle = MIDI Learn":
                "EN selected · right-click a control = MIDI Learn",juce::dontSendNotification);
    }
};
juce::AudioProcessorEditor* SerenityProcessor::createEditor() {return new SerenityEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {return new SerenityProcessor();}
