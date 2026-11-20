#include "PluginEditor.h"
#include <BinaryData.h>
#include <cmath>

namespace {
const juce::Colour blue(0xff08101b),panel(0xff1e293b),aqua(0xff77dce8),
 gold(0xffdbb45d),muted(0xffa2afc6),violet(0xffaa88ff);
constexpr float canvasWidth=1492.f,canvasHeight=991.f;
constexpr std::array<int,11> tabStops{{0,176,388,507,667,793,959,1112,1228,1355,1492}};
const char* labelsEN[]={"OSCILLATORS","WAVETABLE EDITOR","FILTERS","MODULATION","EFFECTS","ARPEGGIATOR","SEQUENCER","MATRIX","PRESETS","SETTINGS"};
const char* labelsFR[]={"OSCILLATEURS","ÉDITEUR WAVETABLE","FILTRES","MODULATION","EFFETS","ARPÉGIATEUR","SÉQUENCEUR","MATRICE","PRESETS","RÉGLAGES"};
const char* names[]={"BRIGHTNESS","MOVEMENT","WARMTH","SPACE","WIDTH","TEXTURE","ENERGY","SERENITY"};
const char* namesFR[]={"BRILLANCE","MOUVEMENT","CHALEUR","ESPACE","LARGEUR","TEXTURE","ÉNERGIE","SÉRÉNITÉ"};
const char* categories[]={"All","Favorites","Bass","Pads","Cinematic","Leads","Plucks","Keys","Arps","EDM","Ambient","Experimental","User"};
const char* categoriesFR[]={"Tous","Favoris","Basses","Nappes","Cinématique","Leads","Plucks","Claviers","Arpèges","EDM","Ambient","Expérimental","Utilisateur"};
}

juce::AudioProcessorEditor* SerenityAudioProcessor::createEditor(){return new SerenityEditor(*this);}
SerenityEditor::SerenityEditor(SerenityAudioProcessor& p):juce::AudioProcessorEditor(&p),synth(p){
 const std::array<std::pair<const void*,int>,10> skinsData={{
  {BinaryData::skin_oscillators_png,BinaryData::skin_oscillators_pngSize},
  {BinaryData::skin_editor_png,BinaryData::skin_editor_pngSize},
  {BinaryData::skin_filters_png,BinaryData::skin_filters_pngSize},
  {BinaryData::skin_modulation_png,BinaryData::skin_modulation_pngSize},
  {BinaryData::skin_effects_png,BinaryData::skin_effects_pngSize},
  {BinaryData::skin_arpeggiator_png,BinaryData::skin_arpeggiator_pngSize},
  {BinaryData::skin_sequencer_png,BinaryData::skin_sequencer_pngSize},
  {BinaryData::skin_matrix_png,BinaryData::skin_matrix_pngSize},
  {BinaryData::skin_presets_png,BinaryData::skin_presets_pngSize},
  {BinaryData::skin_settings_png,BinaryData::skin_settings_pngSize}
 }};
 for(size_t i=0;i<skins.size();i++)
  skins[i]=juce::ImageCache::getFromMemory(skinsData[i].first,skinsData[i].second);
 search.setTextToShowWhenEmpty("Rechercher par nom, étiquette ou auteur...",juce::Colours::grey);
 search.setColour(juce::TextEditor::backgroundColourId,juce::Colour(0xff0a111c));
 search.setColour(juce::TextEditor::textColourId,juce::Colour(0xffd6e0ec));
 search.setColour(juce::TextEditor::outlineColourId,juce::Colour(0xff303d51));
 search.setFont(juce::Font(14.f));
 search.onTextChange=[this]{listOffset=0;repaint();};
 addAndMakeVisible(search);
 // No scroll bars, no manual resize, and no clipped controls on a 13-inch screen.
 // Keep the original 1492:991 aspect and use the desktop usable area.
 setResizable(false,false);
 const auto *display=juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
 const auto usable=display!=nullptr ? display->userArea : juce::Rectangle<int>(0,0,1440,900);
 const int availableW=std::max(900,usable.getWidth()-42);
 const int availableH=std::max(610,usable.getHeight()-58);
 const int fittedW=juce::jmin(1380,availableW,(int)std::floor(availableH*canvasWidth/canvasHeight));
 const int fittedH=(int)std::round(fittedW*canvasHeight/canvasWidth);
 setSize(fittedW,fittedH);
 startTimerHz(20);
}
bool SerenityEditor::hasTenOriginalScreens() const {
 for(const auto& sourceImage:skins)
  if(sourceImage.isNull() || sourceImage.getWidth()!=2048 || sourceImage.getHeight()!=1361)
   return false;
 return true;
}
void SerenityEditor::selectTabForSmokeTest(int index){
 tab=juce::jlimit(0,9,index);
 resized();
 repaint();
}
void SerenityEditor::resized(){
 search.setVisible(tab==8);
 search.setBounds(scaleRect(283,157,673,37).toNearestInt());
}
juce::Point<float> SerenityEditor::logical(juce::Point<float> p) const {
 return {p.x*canvasWidth/getWidth(),p.y*canvasHeight/getHeight()};
}
juce::Rectangle<float> SerenityEditor::scaleRect(float x,float y,float w,float h) const {
 return {x*getWidth()/canvasWidth,y*getHeight()/canvasHeight,w*getWidth()/canvasWidth,h*getHeight()/canvasHeight};
}
juce::String SerenityEditor::tr(const char* fr,const char* en)const {return synth.french()?juce::String(fr):juce::String(en);}
void SerenityEditor::drawText(juce::Graphics& g,const juce::String& s,float x,float y,float w,float h,juce::Colour colour,float pts,bool bold){
 g.setColour(colour);
 g.setFont(juce::Font(pts*getWidth()/canvasWidth,bold?juce::Font::bold:juce::Font::plain));
 g.drawFittedText(s,scaleRect(x,y,w,h).toNearestInt(),juce::Justification::centredLeft,1);
}
void SerenityEditor::paintTabTitle(juce::Graphics& g,int which,float x,float y,const char* fr,const char* en,float width){
 if(tab!=which)return;
 g.setColour(juce::Colour(0xff1e2939));
 g.fillRect(scaleRect(x,y,width,22));
 drawText(g,tr(fr,en),x+3,y,width-6,22,juce::Colour(0xffbdcddd),13,true);
}
void SerenityEditor::paintTop(juce::Graphics& g){
 // The user-supplied screenshot is the baseline, these overlays carry live state.
 g.setColour(juce::Colour(0xff101722));g.fillRect(scaleRect(0,63,1492,42));
 for(int i=0;i<10;i++){
  float x=(float)tabStops[(size_t)i],w=(float)(tabStops[(size_t)i+1]-tabStops[(size_t)i]);
  if(i==tab){g.setColour(juce::Colour(0xff272b2c));g.fillRect(scaleRect(x,63,w,40));
   g.setColour(gold);g.fillRect(scaleRect(x,102,w,2));}
  auto t=tr(labelsFR[i],labelsEN[i]);
  g.setColour(i==tab?gold:muted.withAlpha(.83f));
  g.setFont(juce::Font((i==1?12:12.4f)*getWidth()/canvasWidth,juce::Font::bold));
  g.drawFittedText(t,scaleRect(x+6,68,w-12,27).toNearestInt(),juce::Justification::centred,1);
 }
 // Preset field: real current selection (not a frozen screenshot name).
 g.setColour(blue);g.fillRoundedRectangle(scaleRect(367,5,262,45),8.f);
 const auto& all=serenity::factoryPresets();
 int i=juce::jlimit(0,(int)all.size()-1,synth.currentPreset());
 drawText(g,juce::String(all[(size_t)i].name),379,10,235,31,juce::Colour(0xffdbe7ed),15,true);
 // Language toggle in unused top-left header gap, without moving reference controls.
 g.setColour(juce::Colour(0xff151e2d));g.fillRoundedRectangle(scaleRect(259,14,46,25),4.f);
 drawText(g,synth.french()?"FR":"EN",267,14,32,25,gold,12,true);
 // Dynamic voice and master meter.
 drawText(g,juce::String(synth.getVoiceCount()),1315,14,40,26,muted,13,true);
 g.setColour(aqua.withAlpha(.72f));
 g.fillRect(scaleRect(1381,13,100.f*juce::jlimit(0.f,1.f,synth.getOutputLevel()),5));
}
void SerenityEditor::paintPresets(juce::Graphics& g){
 if(tab!=8)return;
 const auto all=filteredPresetIndices();
 // Render native interactive names on top of the static reference list.
 g.setColour(juce::Colour(0xff0a111b));g.fillRect(scaleRect(283,205,671,584));
 for(int row=0;row<17;row++)for(int col=0;col<2;col++){
  int ix=listOffset+row+col*17;
  if(ix>=(int)all.size())continue;
  int pi=all[(size_t)ix];
  const auto &preset=serenity::factoryPresets()[(size_t)pi];
  float x=297+col*329,y=212+row*33.6f;
  if(pi==synth.currentPreset()){
   g.setColour(juce::Colour(0xff273e4c));g.fillRoundedRectangle(scaleRect(x-7,y,317,29),3.f);
  }
  drawText(g,synth.isFavorite(pi)?"★":"☆",x,y,17,26,gold,15);
  drawText(g,juce::String(preset.name),x+24,y,207,26,
   pi==synth.currentPreset()?aqua:juce::Colour(0xffd3d9e3),12.9f,true);
  drawText(g,juce::String(preset.category).toUpperCase(),x+227,y,83,26,muted,10.5f,true);
 }
 // Categories are also native and translated, accurate counts are computed.
 g.setColour(panel);g.fillRect(scaleRect(18,153,229,550));
 for(int row=0;row<13;row++){
  const float y=157+row*42.3f;
  if(category==row){g.setColour(juce::Colour(0xff3b434d));g.fillRoundedRectangle(scaleRect(23,y,223,35),5.f);}
  auto label=tr(categoriesFR[row],categories[row]);
  drawText(g,label,38,y+1,145,32,category==row?gold:juce::Colour(0xffe0e2e8),14,category==row);
  int count=0;
  if(row==0)count=100;
  if(row==1){for(int i=0;i<100;i++)if(synth.isFavorite(i))count++;}
  else if(row>1&&row<12){for(const auto& preset:serenity::factoryPresets())if(preset.category==categories[row])count++;}
  drawText(g,juce::String(count),207,y+1,32,32,muted,12);
 }
 // Actual selected preset inspector replaces the default empty preview.
 g.setColour(panel);g.fillRect(scaleRect(988,146,487,160));
 const auto& current=serenity::factoryPresets()[(size_t)synth.currentPreset()];
 drawText(g,tr("PRESET SÉLECTIONNÉ","SELECTED PRESET"),1004,149,380,26,muted,14,true);
 drawText(g,juce::String(current.name),1005,182,453,31,aqua,18,true);
 drawText(g,juce::String(current.category)+" · Liberty Music Software",1005,220,445,25,muted,12);
}
void SerenityEditor::paintSettings(juce::Graphics& g){
 if(tab!=9)return;
 g.setColour(panel);g.fillRoundedRectangle(scaleRect(760,148,722,333),8.f);
 drawText(g,tr("APPRENTISSAGE MIDI","MIDI LEARN"),778,155,420,30,violet,15,true);
 const int learning=synth.currentlyLearning();
 juce::String message=learning>=0
  ?tr("En attente : bouger un contrôleur MIDI CC...","Waiting: move a MIDI CC controller...")
  :tr("Clic droit sur un paramètre → Apprentissage MIDI.","Right-click a parameter → MIDI Learn.");
 drawText(g,message,780,193,665,34,learning>=0?gold:muted,14,true);
 drawText(g,tr("Les associations sont sauvegardées dans le projet.","Mappings are restored with the project."),780,226,630,24,muted,12);
 int found=0;
 for(int i=0;i<serenity::parameterCount;i++){
  int cc=synth.mappedCC(i);if(cc<0)continue;
  int row=found%7,col=found/7;
  if(col>1)break;
  juce::String s=juce::String("CC ")+juce::String(cc)+"  →  "+juce::String(serenity::specs[(size_t)i].id);
  drawText(g,s,784+col*350,277+row*25,342,23,aqua,12);
  ++found;
 }
 if(found==0)drawText(g,tr("Aucun contrôleur associé pour le moment.","No mapped controllers yet."),782,296,480,32,muted,13);
 drawText(g,tr("LANGUE : FRANÇAIS / ANGLAIS","LANGUAGE: FRENCH / ENGLISH"),792,447,550,27,gold,13,true);
}
void SerenityEditor::paintOscillators(juce::Graphics& g){
 if(tab!=0)return;
 const int levels[]={serenity::oscALevel,serenity::oscBLevel,serenity::oscCLevel,serenity::oscDLevel};
 for(int i=0;i<4;i++){
  float y=126+i*129.f;
  const bool enabled=synth.getValue(levels[i])>.001f;
  g.setColour(enabled?juce::Colour(0xff234850):juce::Colour(0xff0b1220));
  g.fillRoundedRectangle(scaleRect(188,y,60,26),12.f);
  drawText(g,tr(enabled?"ACTIF":"ARRÊT",enabled?"ON":"OFF"),199,y+2,47,21,
       enabled?aqua:muted,11,true);
 }
}
void SerenityEditor::paintEditor(juce::Graphics& g){
 if(tab!=1)return;
 for(int i=0;i<4;i++){
  float x=24+i*104.f;
  g.setColour(selectedOsc==i?gold:juce::Colour(0xff111825));
  g.drawRoundedRectangle(scaleRect(x,157,90,33),5.f,selectedOsc==i?2.f:1.f);
 }
 const int waves[]={serenity::oscAWave,serenity::oscBWave,serenity::oscCWave,serenity::oscDWave};
 int wave=(int)synth.getValue(waves[selectedOsc]);
 int col=wave%4,row=wave/4;
 g.setColour(gold);
 g.drawRoundedRectangle(scaleRect(24+col*103.5f,205+row*94.9f,96,80),4.f,2.f);
}

void SerenityEditor::paintMatrix(juce::Graphics& g){
 if(tab!=7)return;
 const char* sourceNames[]={"ENV 1","ENV 2","ENV 3","ENV 4","ENV 5","ENV 6",
  "LFO 1","LFO 2","LFO 3","LFO 4","LFO 5","LFO 6","LFO 7","LFO 8", "VEL","KEY","MOD WHEEL"};
 const char* destEN[]={"Pitch","Wavetable","Cutoff","Amplitude","Pan"};
 const char* destFR[]={"Hauteur","Position table","Fréquence","Amplitude","Panoramique"};
 for(int p=0;p<3;p++){
  float x=1099.f+p*123;
  g.setColour(p==matrixPage?juce::Colour(0xff4c4535):blue);
  g.fillRoundedRectangle(scaleRect(x,120,118,33),4.f);
  drawText(g,juce::String(p*24+1)+"–"+juce::String(std::min(64,(p+1)*24)),x+13,122,100,29,
           p==matrixPage?gold:muted,12,true);
 }
 for(int col=0;col<2;col++)for(int row=0;row<12;row++){
  int slot=matrixPage*24+col*12+row;
  if(slot>=64)continue;
  const auto v=synth.getRoute(slot);
  float x=24.f+col*744.f,y=245.f+row*44.9f;
  g.setColour(juce::Colour(0xff0b1320));
  g.fillRoundedRectangle(scaleRect(x+48,y,181,31),4.f);
  g.fillRoundedRectangle(scaleRect(x+257,y,181,31),4.f);
  drawText(g,juce::String(slot+1).paddedLeft('0',2),x+8,y+2,39,26,muted,12,true);
  drawText(g,v.source>=0?juce::String(sourceNames[v.source]):"Off",x+60,y+3,152,25,
           v.source>=0?aqua:muted,13);
  drawText(g,v.destination>=0?juce::String(synth.french()?destFR[v.destination]:destEN[v.destination]):
           tr("Aucune","None"),x+270,y+3,150,25,v.destination>=0?aqua:muted,12);
  drawText(g,juce::String(v.amount,2),x+606,y+2,83,26,muted,12);
 }
}
void SerenityEditor::showRouteMenu(int slot,bool source){
 if(slot<0||slot>=64)return;
 auto route=synth.getRoute(slot);
 juce::PopupMenu menu;
 if(source){
  menu.addSectionHeader(tr("SOURCE DE MODULATION","MODULATION SOURCE"));
  menu.addItem(1,tr("Désactivée","Off"));
  for(int i=0;i<6;i++)menu.addItem(i+2,"ENV "+juce::String(i+1));
  for(int i=0;i<8;i++)menu.addItem(i+8,"LFO "+juce::String(i+1));
  menu.addItem(16,"VELOCITY");menu.addItem(17,"KEY TRACKING");menu.addItem(18,"MOD WHEEL");
 }else{
  menu.addSectionHeader(tr("DESTINATION","DESTINATION"));
  menu.addItem(1,tr("Aucune","None"));
  const char* names[]={"Pitch","Wavetable","Filter cutoff","Amplitude","Pan"};
  for(int i=0;i<5;i++)menu.addItem(i+2,names[i]);
 }
 juce::Component::SafePointer<SerenityEditor> safe(this);
 menu.showMenuAsync(juce::PopupMenu::Options(),[safe,slot,source](int choice){
  if(!safe||choice==0)return;
  auto current=safe->synth.getRoute(slot);
  if(source)current.source=choice-2;
  else current.destination=choice-2;
  if(current.source>=0&&current.destination>=0&&current.amount==0)current.amount=.5f;
  safe->synth.setRoute(slot,current);
  safe->repaint();
 });
}

void SerenityEditor::paintInteraction(juce::Graphics& g){
 if(selectedParameter>=0&&selectedParameter<serenity::parameterCount){
  auto hs=hotspots();
  for(const auto &spot:hs)if(spot.parameter==selectedParameter){
   g.setColour(violet.withAlpha(.90f));
   auto center=scaleRect(spot.x-spot.r,spot.y-spot.r,spot.r*2,spot.r*2);
   g.drawEllipse(center,2.f);
  }
 }
 if(statusCountdown>0){
  g.setColour(juce::Colour(0xee111b2b));g.fillRoundedRectangle(scaleRect(745,845,726,37),5.f);
  drawText(g,status,762,849,692,27,aqua,13,true);
 }
}
void SerenityEditor::paint(juce::Graphics& g){
 g.fillAll(blue);
 if(!skins[(size_t)tab].isNull())
  g.drawImageWithin(skins[(size_t)tab],0,0,getWidth(),getHeight(),juce::RectanglePlacement::stretchToFit);
 paintTop(g);
 // Translate prominent panel titles without reinterpreting or redesigning their geometry.
 paintTabTitle(g,0,279,120,"WAVETABLE · OSC A","WAVETABLE · OSC A",210);
 paintTabTitle(g,1,475,120,"WAVETABLE 3D · OSC A","3D WAVETABLE · OSC A",327);
 paintTabTitle(g,2,25,120,"RÉPONSE DES FILTRES","FILTER RESPONSE",286);
 paintTabTitle(g,3,25,120,"ENVELOPPES","ENVELOPES",166);
 paintTabTitle(g,4,21,164,"SATURATION","DRIVE",131);
 paintTabTitle(g,5,26,120,"ARPÉGIATEUR À PAS","STEP ARPEGGIATOR",310);
 paintTabTitle(g,6,25,120,"SÉQUENCEUR À PAS","STEP SEQUENCER",298);
 paintTabTitle(g,7,26,120,"MATRICE DE MODULATION","MODULATION MATRIX",270);
 if(tab==0)paintOscillators(g);
 if(tab==1)paintEditor(g);
 if(tab==7)paintMatrix(g);
 if(tab==8)paintPresets(g);
 if(tab==9)paintSettings(g);
 paintInteraction(g);
}
std::vector<SerenityEditor::Hotspot> SerenityEditor::hotspots()const {
 using namespace serenity;
 std::vector<Hotspot> hs={{1119,36,24,master}};
 const float centers[]={61,150,240,330,420,509,598,687};
 for(int i=0;i<8;i++)hs.push_back({centers[i],916,23,macroBrightness+i});
 switch(tab){
  case 0: hs.insert(hs.end(),{{1041,316,30,position},{1165,316,30,position},
   {1415,315,30,oscALevel},{522,711,27,detune},{612,711,27,spread},
   {441,711,27,oscAUnison},{65,710,27,cutoff}});break;
  case 1: hs.insert(hs.end(),{{500,715,33,position},{798,713,33,position},
   {1093,714,33,position}});break;
  case 2: hs.insert(hs.end(),{{64,676,34,cutoff},{149,676,31,resonance},
   {803,676,34,filter2Cutoff},{888,676,31,filter2Resonance},
   {1280,676,31,filter2Mix}});break;
  case 3: hs.insert(hs.end(),{{150,580,30,attack},{330,581,30,decay},{421,581,30,sustain},
   {512,581,30,release},{803,639,30,lfoRate},{805,508,30,lfoDepth}});break;
  case 4: hs.insert(hs.end(),{{76,315,24,drive},{953,546,25,delayMix},
   {51,780,25,reverbMix},{1315,542,24,master}});break;
  case 9:hs.insert(hs.end(),{{88,215,30,master},{378,215,31,cutoff},{519,215,30,master}});break;
  default:break;
 }
 return hs;
}
int SerenityEditor::paramAt(juce::Point<float> pt)const {
 int found=-1;float distance=1e10f;
 for(auto h:hotspots()){
  float dx=pt.x-h.x,dy=pt.y-h.y,d=dx*dx+dy*dy;
  if(d<h.r*h.r&&d<distance){found=h.parameter;distance=d;}
 }
 return found;
}
std::vector<int> SerenityEditor::filteredPresetIndices()const {
 std::vector<int> indices;
 const auto& presets=serenity::factoryPresets();
 juce::String term=search.getText().trim().toLowerCase();
 for(int i=0;i<(int)presets.size();i++){
  const auto& patch=presets[(size_t)i];
  if(category>1&&category<12&&patch.category!=categories[category])continue;
  if(category==1&&!synth.isFavorite(i))continue;
  if(category==12)continue; // User preset file import is supported; separate user library pending.
  if(!term.isEmpty()&&!juce::String(patch.name).containsIgnoreCase(term)&&
     !juce::String(patch.category).containsIgnoreCase(term)&&
     !juce::String(patch.author).containsIgnoreCase(term))continue;
  indices.push_back(i);
 }
 return indices;
}
void SerenityEditor::showStatus(const juce::String& message){status=message;statusCountdown=65;repaint();}
void SerenityEditor::selectPreset(int i){
 synth.applyPreset(i);
 showStatus(tr("Preset chargé : ","Loaded preset: ")+synth.getProgramName(i));
 repaint();
}
void SerenityEditor::savePatch(bool import){
 const juce::String title=import
  ?tr("Importer un son Serenity","Import Serenity sound")
  :tr("Exporter le son Serenity","Export Serenity sound");
 fileChooser=std::make_unique<juce::FileChooser>(title,juce::File{},"*.serenitypreset");
 const int flags=import?(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles):
                       (juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles);
 juce::Component::SafePointer<SerenityEditor> safe(this);
 fileChooser->launchAsync(flags,[safe,import](const juce::FileChooser& f){
  if(!safe)return;
  auto selected=f.getResult();if(selected==juce::File{})return;
  if(import){
   juce::MemoryBlock data;
   if(selected.loadFileAsData(data)){
    safe->synth.setStateInformation(data.getData(),(int)data.getSize());
    safe->showStatus(safe->tr("Preset importé.","Preset imported."));
   }
  }else{
   juce::MemoryBlock data;
   safe->synth.getStateInformation(data);
   if(selected.replaceWithData(data.getData(),data.getSize()))
    safe->showStatus(safe->tr("Preset exporté.","Preset exported."));
  }
  safe->repaint();
 });
}
void SerenityEditor::changeLanguage(){
 synth.setFrench(!synth.french());
 search.setTextToShowWhenEmpty(tr("Rechercher un son...","Search presets..."),juce::Colours::grey);
 showStatus(tr("Langue : Français","Language: English"));
}
void SerenityEditor::showParameterMenu(int index){
 if(index<0||index>=serenity::parameterCount)return;
 const auto spec=serenity::specs[(size_t)index];
 juce::PopupMenu menu;
 menu.addSectionHeader(juce::String(spec.id));
 menu.addItem(1,tr("Apprentissage MIDI CC","MIDI CC Learn"));
 menu.addItem(2,tr("Effacer l'association MIDI","Clear MIDI mapping"),synth.mappedCC(index)>=0);
 menu.addItem(3,tr("Réinitialiser le paramètre","Reset parameter"));
 juce::Component::SafePointer<SerenityEditor> safe(this);
 menu.showMenuAsync(juce::PopupMenu::Options(),[safe,index,spec](int selected){
  if(!safe)return;
  if(selected==1){safe->synth.armMidiLearn(index);safe->showStatus(safe->tr("Bougez maintenant un contrôleur MIDI CC.","Now move a MIDI CC control."));}
  if(selected==2){safe->synth.clearMapping(index);safe->showStatus(safe->tr("Association supprimée.","Mapping cleared."));}
  if(selected==3){safe->synth.setValue(index,spec.initial);safe->repaint();}
 });
}
void SerenityEditor::mouseDown(const juce::MouseEvent& e){
 auto p=logical(e.position);
 downPos=p;
 if(p.y>=63&&p.y<105){
  for(int i=0;i<10;i++)if(p.x>=tabStops[(size_t)i]&&p.x<tabStops[(size_t)i+1]){
   tab=i;resized();repaint();return;
  }
 }
 if(p.x>255&&p.x<308&&p.y>=8&&p.y<52){changeLanguage();return;}
 if(p.y<58){
  if(p.x>725&&p.x<771){savePatch(false);return;}
  if(p.x>956&&p.x<1020){
   if(selectedParameter>=0){
    synth.armMidiLearn(selectedParameter);
    showStatus(tr("Apprentissage MIDI : bougez un contrôleur CC.","MIDI Learn: move a CC controller."));
   }else showStatus(tr("Survolez un bouton, puis cliquez sur LEARN.","Hover a knob, then click LEARN."));
   return;
  }
  if(p.x>316&&p.x<358){selectPreset((synth.currentPreset()+99)%100);return;}
  if(p.x>637&&p.x<680){selectPreset((synth.currentPreset()+1)%100);return;}
 }
 if(tab==0&&p.x>186&&p.x<253&&p.y>121&&p.y<624){
  int which=juce::jlimit(0,3,(int)((p.y-121)/129.f));
  const int levels[]={serenity::oscALevel,serenity::oscBLevel,serenity::oscCLevel,serenity::oscDLevel};
  const bool enabled=synth.getValue(levels[which])>.001f;
  synth.setValue(levels[which],enabled?0.f:.65f);
  showStatus(juce::String("OSC ")+juce::String::charToString((juce_wchar)('A'+which))+
             (enabled?" OFF":" ON"));return;
 }
 if(tab==1){
  if(p.x>20&&p.x<426&&p.y>153&&p.y<193){
   selectedOsc=juce::jlimit(0,3,(int)((p.x-20)/104.f));repaint();return;
  }
  if(p.x>24&&p.x<425&&p.y>=205&&p.y<572){
   int col=juce::jlimit(0,3,(int)((p.x-24)/103.5f));
   int row=juce::jlimit(0,3,(int)((p.y-205)/94.9f));
   const int waves[]={serenity::oscAWave,serenity::oscBWave,serenity::oscCWave,serenity::oscDWave};
   synth.setValue(waves[selectedOsc],(float)(row*4+col));repaint();return;
  }
  if(p.x>=458&&p.x<1133&&p.y>=599&&p.y<=665){
   int frame=juce::jlimit(0,7,(int)((p.x-458)/84.f));
   synth.setValue(serenity::position,frame/7.f);repaint();return;
  }
 }
 if(tab==2){
  if(p.y>154&&p.y<191&&p.x>=1265&&p.x<=1481){
   synth.setValue(serenity::filterRouting,p.x>1371?1.f:0.f);
   showStatus(tr(p.x>1371?"Filtres en parallèle":"Filtres en série",
                 p.x>1371?"Parallel filters":"Serial filters"));return;
  }
  if(p.x>1373&&p.x<1486&&p.y>571&&p.y<605){
   const bool enabled=synth.getValue(serenity::filter2Enabled)>.5f;
   synth.setValue(serenity::filter2Enabled,enabled?0.f:1.f);
   showStatus(tr(enabled?"Filtre 2 désactivé":"Filtre 2 activé",
                 enabled?"Filter 2 off":"Filter 2 on"));return;
  }
 }
 if(tab==7){
  if(p.y>=118&&p.y<157&&p.x>=1099){
   matrixPage=juce::jlimit(0,2,(int)((p.x-1099)/123));repaint();return;
  }
  if(p.y>=245&&p.y<790){
   int col=p.x>=752?1:0, row=(int)((p.y-245)/44.9f);
   int slot=matrixPage*24+col*12+row;
   if(slot<64&&row>=0&&row<12){
    float localX=p.x-col*744;
    if(localX>=72&&localX<=256){showRouteMenu(slot,true);return;}
    if(localX>=280&&localX<=465){showRouteMenu(slot,false);return;}
    if(localX>=473&&localX<=620){
     auto r=synth.getRoute(slot);
     r.amount=juce::jlimit(-1.f,1.f,(localX-474.f)/146.f*2.f-1.f);
     synth.setRoute(slot,r);repaint();return;
    }
   }
  }
 }
 if(tab==8){
  if(p.y>=733&&p.y<=782&&p.x>=1000&&p.x<=1316){
   if(p.x<1116)savePatch(false);
   else if(p.x<1222)savePatch(true);
   else {search.clear();category=0;listOffset=0;repaint();}
   return;
  }
  if(p.x>20&&p.x<250&&p.y>=155&&p.y<710){
   category=juce::jlimit(0,12,(int)((p.y-157)/42.3f));listOffset=0;repaint();return;
  }
  if(p.x>=283&&p.x<954&&p.y>=207&&p.y<789){
   const int col=(p.x>626)?1:0,row=(int)((p.y-212)/33.6f);
   const auto filtered=filteredPresetIndices();
   const int index=listOffset+juce::jlimit(0,16,row)+col*17;
   if(index>=0&&index<(int)filtered.size()){
    int preset=filtered[(size_t)index];
    const float localX=p.x-col*329;
    if(localX>=288&&localX<=326){synth.toggleFavorite(preset);repaint();}
    else selectPreset(preset);
   }
   return;
  }
 }
 if(tab==9&&p.x>760&&p.x<1485&&p.y>150&&p.y<185){
  if(selectedParameter>=0){synth.armMidiLearn(selectedParameter);
   showStatus(tr("Apprentissage MIDI activé.","MIDI Learn armed."));}
  else showStatus(tr("Clic droit sur un bouton pour sélectionner une cible MIDI.",
   "Right-click a knob to select a MIDI target."));
  return;
 }
 const int param=paramAt(p);
 if(param>=0){
  selectedParameter=param;
  if(e.mods.isRightButtonDown()){showParameterMenu(param);return;}
  draggedParameter=param;startValue=synth.getValue(param);repaint();return;
 }
}
void SerenityEditor::mouseDrag(const juce::MouseEvent& e){
 if(draggedParameter<0)return;
 const auto &spec=serenity::specs[(size_t)draggedParameter];
 auto p=logical(e.position);
 float change=(downPos.y-p.y)/245.f*(spec.hi-spec.lo)*(e.mods.isShiftDown() ? .12f:1.0f);
 synth.setValue(draggedParameter,juce::jlimit(spec.lo,spec.hi,startValue+change));
 showStatus(juce::String(spec.id)+" : "+juce::String(synth.getValue(draggedParameter),3));
}
void SerenityEditor::mouseUp(const juce::MouseEvent&){draggedParameter=-1;}
void SerenityEditor::mouseDoubleClick(const juce::MouseEvent& e){
 int index=paramAt(logical(e.position));
 if(index>=0){synth.setValue(index,serenity::specs[(size_t)index].initial);
  showStatus(tr("Paramètre réinitialisé.","Parameter reset."));}
}
void SerenityEditor::mouseMove(const juce::MouseEvent& e){
 int i=paramAt(logical(e.position));
 if(i>=0 && i!=selectedParameter){selectedParameter=i;repaint();}
}
void SerenityEditor::mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel){
 auto p=logical(e.position);
 if(tab==8&&p.x>280&&p.y>200&&p.y<790){
  const int count=(int)filteredPresetIndices().size();
  listOffset=juce::jlimit(0,std::max(0,count-34),listOffset-(int)std::round(wheel.deltaY*6));
  repaint();return;
 }
 int index=paramAt(p);
 if(index>=0){const auto& s=serenity::specs[(size_t)index];
  synth.setValue(index,juce::jlimit(s.lo,s.hi,synth.getValue(index)+wheel.deltaY*(s.hi-s.lo)*.04f));repaint();}
}
void SerenityEditor::timerCallback(){
 if(statusCountdown>0)statusCountdown--;
 repaint();
}
