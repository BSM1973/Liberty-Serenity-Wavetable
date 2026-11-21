#include "PluginEditor.h"
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
 search.setTextToShowWhenEmpty(juce::String::fromUTF8("Rechercher par nom, étiquette ou auteur..."),juce::Colours::grey);
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
 const auto fitted=fitEditorSize(usable.getWidth(),usable.getHeight());
 setSize(fitted.x,fitted.y);
 startTimerHz(30);
}
juce::Point<int> SerenityEditor::fitEditorSize(int usableWidth,int usableHeight) noexcept {
 const int availableW=std::max(1,usableWidth-42),availableH=std::max(1,usableHeight-58);
 const int fittedW=std::max(1,juce::jmin(1380,availableW,(int)std::floor(availableH*canvasWidth/canvasHeight)));
 return {fittedW,std::max(1,(int)std::floor(fittedW*canvasHeight/canvasWidth))};
}
void SerenityEditor::selectTabForSmokeTest(int index){
 tab=juce::jlimit(0,9,index);
 resized();
 repaint();
}
void SerenityEditor::resized(){
 search.setVisible(tab==8);
 search.setBounds(scaleRect(283,157,673,37).toNearestInt());
 search.setFont(juce::Font(juce::jmax(10.f,15.f*getWidth()/canvasWidth)));
}
juce::Point<float> SerenityEditor::logical(juce::Point<float> p) const {
 return {p.x*canvasWidth/getWidth(),p.y*canvasHeight/getHeight()};
}
juce::Rectangle<float> SerenityEditor::scaleRect(float x,float y,float w,float h) const {
 return {x*getWidth()/canvasWidth,y*getHeight()/canvasHeight,w*getWidth()/canvasWidth,h*getHeight()/canvasHeight};
}
juce::String SerenityEditor::tr(const char* fr,const char* en)const {
 return juce::String::fromUTF8(synth.french()?fr:en);
}
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
  juce::Path star;const auto center=scaleRect(x,y,17,26).getCentre();const float outer=7.f*getWidth()/canvasWidth;
  for(int j=0;j<10;j++){const float a=-juce::MathConstants<float>::halfPi+j*juce::MathConstants<float>::pi*.2f,r=outer*(j%2==0?1.f:.44f);const float sx=center.x+std::cos(a)*r,sy=center.y+std::sin(a)*r;if(j==0)star.startNewSubPath(sx,sy);else star.lineTo(sx,sy);}star.closeSubPath();g.setColour(gold);if(synth.isFavorite(pi))g.fillPath(star);else g.strokePath(star,juce::PathStrokeType(1.f));
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
 drawText(g,juce::String(current.category)+juce::String::fromUTF8(" · Liberty Music Software"),1005,220,445,25,muted,12);
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
  juce::String s=juce::String("CC ")+juce::String(cc)+juce::String::fromUTF8("  →  ")+juce::String(serenity::specs[(size_t)i].id);
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
  drawText(g,juce::String(p*24+1)+juce::String::fromUTF8("–")+juce::String(std::min(64,(p+1)*24)),x+13,122,100,29,
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
  drawText(g,v.destination>=0?juce::String::fromUTF8(synth.french()?destFR[v.destination]:destEN[v.destination]):
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
 const int highlight=draggedParameter>=0?draggedParameter:hoverParameter;
 if(highlight>=0&&highlight<serenity::parameterCount){
  auto hs=hotspots();
  for(const auto &spot:hs)if(spot.parameter==highlight){
   g.setColour(violet.withAlpha(draggedParameter>=0?.90f:.52f));
   auto center=scaleRect(spot.x-spot.r,spot.y-spot.r,spot.r*2,spot.r*2);
   g.drawEllipse(center,2.f);
  }
 }
 if(statusCountdown>0){
  // Compact feedback stays in the 22-pixel gap between routing rail and footer.
  g.setColour(juce::Colour(0xee111b2b));g.fillRoundedRectangle(scaleRect(919,842,557,18),4.f);
  drawText(g,status,925,843,541,15,aqua,10,true);
 }
}
void SerenityEditor::paint(juce::Graphics& g){
 paintNativePage(g);
 // One native 3D/2D renderer only; no legacy Custom overlay on top.
 if(tab==8)paintPresets(g);
 paintInteraction(g);
}
std::vector<SerenityEditor::Hotspot> SerenityEditor::hotspots()const {
 using namespace serenity;
 std::vector<Hotspot> hs={{1119,36,24,master}};
 const float centers[]={61,150,240,330,420,509,598,687};
 for(int i=0;i<8;i++)hs.push_back({centers[i],916,23,macroBrightness+i});
 switch(tab){
  case 0:{
   const int lev[]={oscALevel,oscBLevel,oscCLevel,oscDLevel};
   const int uni[]={oscAUnison,oscBUnison,oscCUnison,oscDUnison};
   const int selected=juce::jlimit(0,3,selectedOsc);
   hs.insert(hs.end(),{{1043,317,30,position},{1416,317,30,lev[selected]},
     {1168,317,30,oscAWarp+selected},{1288,317,30,oscAShape+selected},
     {65,708,22,oscAOctave+selected},{150,708,22,oscASemi+selected},
     {235,708,22,oscAFine+selected},{321,708,22,oscAPan+selected},
     {527,708,24,detune},{614,708,24,spread},
     {442,708,24,uni[selected]},
     {704,708,24,oscABlend+selected},{790,708,24,oscAPhase+selected},
     {877,708,24,oscARandPhase+selected},
     {1158,709,23,fmBA},{1251,709,23,fmCA},{1340,709,23,fmCB},{1431,709,23,ringAmount}});
   break;
  }
  case 1: hs.insert(hs.end(),{{500,714,30,position},
   {796,714,30,oscAWarp+juce::jlimit(0,3,selectedOsc)},
   {1092,714,30,oscAShape+juce::jlimit(0,3,selectedOsc)}});break;
  case 2: hs.insert(hs.end(),{{63,705,25,cutoff},{149,705,25,resonance},
   {803,705,25,filter2Cutoff},{890,705,25,filter2Resonance},
   {1276,705,23,filter2Mix},{233,705,25,filter1Drive},{328,705,23,filter1Env},{399,705,23,filter1Key},{467,705,23,filter1Vel},{531,705,23,filter1Mix},{977,705,25,filter2Drive},{1073,705,23,filter2Env},{1140,705,23,filter2Key},{1207,705,23,filter2Vel}});break;
  case 3:{
   const float x[]={60,151,240,332,421,513};for(int k=0;k<6;k++)hs.push_back({x[k],582,24,envParameter(selectedEnv,k)});
   hs.push_back({803,639,24,lfoParameter(selectedLfo,0)});hs.push_back({893,639,24,lfoParameter(selectedLfo,2)});
   hs.push_back({603,582,24,env1Curve+2*selectedEnv});hs.push_back({693,582,24,env1Velocity+2*selectedEnv});
   hs.push_back({1074,639,24,lfo1Smooth+3*selectedLfo});
   hs.push_back({985,639,24,lfo1Fade+selectedLfo});break;}
  case 4: hs.insert(hs.end(),{{77,325,24,drive},{351,325,24,cutoff},{419,325,24,resonance},
   {585,547,24,tremRate},{651,547,24,tremDepth},{773,546,24,fxRingFreq},{841,546,24,fxRingMix},
   {955,546,24,delayTime},{1021,546,24,delayFeedback},{1086,546,24,delayTone},{1153,546,24,delayMix},
   {51,779,24,reverbFeedback},{114,779,24,reverbDamping},{177,779,24,reverbSize},{240,779,24,reverbPreDelay},{303,779,24,reverbLowCut},{366,779,24,reverbWidth},{429,779,24,reverbMix},{1315,547,24,master},{1398,547,24,masterWidth}});
   for(int k=0;k<7;k++)hs.push_back({600.f+k*65.5f,325,23,eqLowGain+k});
   for(int k=0;k<6;k++)hs.push_back({1100.f+k*65.f,325,23,compThreshold+k});
   for(int k=0;k<4;k++)hs.push_back({58.f+k*66.f,548,23,modFxRate+k});break;
  case 9:hs.push_back({517,218,30,master});break;
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
  if(selected==3){safe->rememberUndo();safe->synth.setValue(index,spec.initial);safe->repaint();}
 });
}

void SerenityEditor::rememberUndo(){
 juce::MemoryBlock state;
 synth.getStateInformation(state);
 if(undoHistory.empty()||undoHistory.back()!=state){
  undoHistory.push_back(std::move(state));
  if(undoHistory.size()>32)undoHistory.erase(undoHistory.begin());
 }
 redoHistory.clear();
}
void SerenityEditor::undoChange(){
 if(undoHistory.empty()){showStatus(tr("Aucune modification à annuler.","Nothing to undo."));return;}
 juce::MemoryBlock current;
 synth.getStateInformation(current);
 redoHistory.push_back(std::move(current));
 auto state=std::move(undoHistory.back());undoHistory.pop_back();
 synth.setStateInformation(state.getData(),(int)state.getSize());
 showStatus(tr("Modification annulée.","Change undone."));
 repaint();
}
void SerenityEditor::redoChange(){
 if(redoHistory.empty()){showStatus(tr("Aucune modification à rétablir.","Nothing to redo."));return;}
 juce::MemoryBlock current;
 synth.getStateInformation(current);
 undoHistory.push_back(std::move(current));
 auto state=std::move(redoHistory.back());redoHistory.pop_back();
 synth.setStateInformation(state.getData(),(int)state.getSize());
 showStatus(tr("Modification rétablie.","Change redone."));
 repaint();
}
void SerenityEditor::switchAB(int target){
 target=juce::jlimit(0,1,target);
 if(target==activeAB)return;
 juce::MemoryBlock current;
 synth.getStateInformation(current);
 abStates[(size_t)activeAB]=std::move(current);
 if(abStates[(size_t)target].getSize()==0)
  abStates[(size_t)target]=abStates[(size_t)activeAB];
 synth.setStateInformation(abStates[(size_t)target].getData(),
                            (int)abStates[(size_t)target].getSize());
 activeAB=target;
 undoHistory.clear();redoHistory.clear();
 showStatus(juce::String("COMPARER ")+(activeAB==0?"A":"B"));
 repaint();
}
void SerenityEditor::showWaveSelectMenu(int osc){
 osc=juce::jlimit(0,3,osc);
 selectedOsc=osc;
 const char* names[]={"Classic","Analog Saw","Pulse Sweep","Hollow Square",
  "Reso Saw","Vowel Choir","Spectral Peak","FM Bell","FM E-Piano",
  "Metal","Organ","Reed","Phase Choir","Sync Sweep","Chaos FM","Crystal","Custom"};
 juce::PopupMenu m;
 m.addSectionHeader(tr("TABLE D'ONDE","WAVETABLE"));
 const int chosen=(int)std::lround(synth.getValue(serenity::oscAWave+osc));
 for(int i=0;i<=serenity::waveCount;i++)
  m.addItem(i+1,names[i],true,i==chosen);
 juce::Component::SafePointer<SerenityEditor> safe(this);
 m.showMenuAsync(juce::PopupMenu::Options(),[safe,osc](int value){
  if(!safe||value==0)return;
  safe->rememberUndo();
  safe->synth.setValue(serenity::oscAWave+osc,(float)(value-1));
  safe->repaint();
 });
}
void SerenityEditor::showOscTransformMenu(int osc,bool warp){
 osc=juce::jlimit(0,3,osc);
 juce::PopupMenu m;
 m.addSectionHeader(warp?tr("MODE WARP","WARP MODE"):tr("MODE SHAPE","SHAPE MODE"));
 const char* warpNames[]={"Off","Bend","PWM","Sync"};
 const char* shapeNames[]={"Off","Saturate","Wavefold","Quantize"};
 const int idx=(warp?serenity::oscAWarpMode:serenity::oscAShapeMode)+osc;
 const int selected=(int)std::lround(synth.getValue(idx));
 for(int i=0;i<4;i++)m.addItem(i+1,warp?warpNames[i]:shapeNames[i],true,i==selected);
 juce::Component::SafePointer<SerenityEditor> safe(this);
 m.showMenuAsync(juce::PopupMenu::Options(),[safe,idx](int value){
  if(!safe||value==0)return;
  safe->rememberUndo();
  safe->synth.setValue(idx,(float)(value-1));
  safe->repaint();
 });
}
bool SerenityEditor::keyPressed(const juce::KeyPress& key){
 const bool command=key.getModifiers().isCommandDown()||key.getModifiers().isCtrlDown();
 const auto letter=juce::String::charToString(key.getTextCharacter()).toLowerCase();
 if(command&&letter=="z"){
  if(key.getModifiers().isShiftDown())redoChange();else undoChange();
  return true;
 }
 if(command&&letter=="y"){redoChange();return true;}
 return false;
}
void SerenityEditor::showChoiceMenu(int parameter,const juce::StringArray& choices){
 juce::PopupMenu menu;for(int i=0;i<choices.size();i++)menu.addItem(i+1,choices[i],true,(int)synth.getValue(parameter)==i);
 juce::Component::SafePointer<SerenityEditor> safe(this);
 menu.showMenuAsync(juce::PopupMenu::Options(),[safe,parameter](int c){if(safe&&c>0){safe->rememberUndo();safe->synth.setValue(parameter,(float)(c-1));safe->repaint();}});
}
void SerenityEditor::mouseDown(const juce::MouseEvent& e){
 auto p=logical(e.position);
 downPos=p;
 if(p.y>=63&&p.y<105){
  for(int i=0;i<10;i++)if(p.x>=tabStops[(size_t)i]&&p.x<tabStops[(size_t)i+1]){
   tab=i;hoverParameter=-1;resized();repaint();return;
  }
 }
 if(p.x>=1023&&p.x<1053&&p.y>=8&&p.y<52){changeLanguage();return;}
 if(p.y<58){
  if(p.x>=786&&p.x<858){switchAB(p.x<821?0:1);return;}
  if(p.x>=871&&p.x<907){undoChange();return;}
  if(p.x>=913&&p.x<949){redoChange();return;}
  if(p.x>=684&&p.x<719){synth.toggleFavorite(synth.currentPreset());repaint();return;}
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
 if((tab==5||tab==6)&&sequenceClick(p))return;
 if(tab==0&&p.x>=960&&p.x<=1104){
  if(p.y>=696&&p.y<=728){showChoiceMenu(serenity::fmMode,{"Phase Mod","Frequency Mod"});return;}
  if(p.y>=759&&p.y<=791){showChoiceMenu(serenity::ringMode,{"Ring","AM"});return;}
 }
 if(tab==3){
  if(p.x>=23&&p.x<=120&&p.y>=663&&p.y<=688){int id=serenity::env1Loop+selectedEnv;rememberUndo();synth.setValue(id,synth.getValue(id)>.5f?0.f:1.f);repaint();return;}
  if(p.y>=156&&p.y<=188){if(p.x>=22&&p.x<724){selectedEnv=juce::jlimit(0,5,(int)((p.x-22)/117));repaint();return;}
   if(p.x>=766&&p.x<1470){selectedLfo=juce::jlimit(0,7,(int)((p.x-766)/88));repaint();return;}}
  if(p.x>=767&&p.x<=942&&p.y>=568&&p.y<=600){showChoiceMenu(serenity::lfoParameter(selectedLfo,1),{"Sine","Triangle","Saw Up","Square","Saw Down"});return;}
  if(p.x>=1103&&p.x<=1248&&p.y>=568&&p.y<=600){showChoiceMenu(serenity::lfo1Division+3*selectedLfo,{"1/1","1/2","1/4 D","1/4","1/4 T","1/8","1/16 D","1/8 T","1/16","1/16 T","1/32","1/64"});return;}
  if(p.x>=953&&p.x<=1090&&p.y>=568&&p.y<=600){showChoiceMenu(serenity::lfo1Sync+3*selectedLfo,{"Free","Tempo"});return;}
  if(p.x>=1275&&p.x<=1351&&p.y>=570&&p.y<=596){int id=serenity::lfo1Sync+3*selectedLfo;rememberUndo();synth.setValue(id,synth.getValue(id)>.5f?0.f:1.f);repaint();return;}
  if(p.x>=1364&&p.x<=1474&&p.y>=570&&p.y<=596){const int id=serenity::lfoParameter(selectedLfo,3);rememberUndo();synth.setValue(id,synth.getValue(id)>.5f?0.f:1.f);repaint();return;}
 }
 if(tab==4&&p.x>=25&&p.x<=187&&p.y>=710&&p.y<=742){showChoiceMenu(serenity::reverbType,{"Room","Hall","Large Space","Chamber"});return;}
 if(tab==4&&p.x>=23&&p.x<=287&&p.y>=486&&p.y<=518){showChoiceMenu(serenity::modFxType,{"Chorus","Flanger","Phaser"});return;}
 if(tab==4&&p.y>=427&&p.y<=458&&p.x>=323&&p.x<=507){rememberUndo();synth.setValue(serenity::tremMode,p.x>=416?1.f:0.f);repaint();return;}
 if(tab==0&&p.y>=120&&p.y<151&&p.x>=801&&p.x<942){
  selectedOsc=juce::jlimit(0,3,(int)((p.x-801)/35.f));
  repaint();return;
 }
 if(tab==0&&p.x>=981&&p.x<=1223&&p.y>=178&&p.y<=216){
  showWaveSelectMenu(selectedOsc);return;
 }
 if(tab==0&&p.y>=243&&p.y<=280){
  if(p.x>=981&&p.x<=1224){showOscTransformMenu(selectedOsc,true);return;}
  if(p.x>=1233&&p.x<=1475){showOscTransformMenu(selectedOsc,false);return;}
 }
 if(tab==0&&p.x>=50&&p.x<=185&&p.y>=118&&p.y<625){
  for(int i=0;i<4;i++){
   const float y=118.f+i*129.f;
   if(p.y>=y+6&&p.y<y+34){showWaveSelectMenu(i);return;}
  }
 }
 if(tab==0&&p.x>=20&&p.x<186&&p.y>121&&p.y<624){
  selectedOsc=juce::jlimit(0,3,(int)((p.y-121)/129.f));
  showStatus(tr("Oscillateur sélectionné : ","Selected oscillator: ")+
     juce::String::charToString((juce_wchar)('A'+selectedOsc)));
  repaint();return;
 }
 if(tab==0&&p.x>=278&&p.x<956&&p.y>=156&&p.y<612){
  rememberUndo();
  selectedParameter=serenity::position;
  draggedParameter=serenity::position;
  startValue=synth.getValue(serenity::position);
  return;
 }
 if(tab==0&&p.x>186&&p.x<253&&p.y>121&&p.y<624){
  int which=juce::jlimit(0,3,(int)((p.y-121)/129.f));
  const int levels[]={serenity::oscALevel,serenity::oscBLevel,serenity::oscCLevel,serenity::oscDLevel};
  const bool enabled=synth.getValue(levels[which])>.001f;
  rememberUndo();
  synth.setValue(levels[which],enabled?0.f:.65f);
  showStatus(juce::String("OSC ")+juce::String::charToString((juce_wchar)('A'+which))+
             (enabled?" OFF":" ON"));return;
 }
 if(tab==1){
  if(p.x>=23&&p.x<=226&&p.y>=648&&p.y<=684){importWavetableAudio(selectedOsc);return;}
  if(p.x>=236&&p.x<=424&&p.y>=648&&p.y<=684){rememberUndo();synth.setValue(serenity::oscAWave+selectedOsc,(float)serenity::waveCount);repaint();return;}
  if(p.x>=570&&p.x<=726&&p.y>=718&&p.y<=754){
   showOscTransformMenu(selectedOsc,true);return;
  }
  if(p.x>=862&&p.x<=1019&&p.y>=718&&p.y<=754){
   showOscTransformMenu(selectedOsc,false);return;
  }
  // Use the original HARMONICS panel for additive spectral editing.
  if(p.x>=1171&&p.x<=1468&&p.y>=161&&p.y<=374){
   if(e.mods.isRightButtonDown()){showWavetableMenu();return;}
   if(e.mods.isLeftButtonDown()){
    synth.setValue(serenity::oscAWave+selectedOsc,(float)serenity::waveCount);
    drawingHarmonic=true;drawCustomHarmonic(p);
   }
   return;
  }
  // OUTPUT WAVEFORM is edited in place, exactly within the original drawing pane.
  if(p.x>=1168&&p.x<=1472&&p.y>=465&&p.y<=611) {
   if(e.mods.isRightButtonDown()){showWavetableMenu();return;}
   if(!e.mods.isLeftButtonDown())return;
   synth.setValue(serenity::oscAWave+selectedOsc,(float)serenity::waveCount);
   drawingCustom=true;customDrawLastSample=-1;
   drawCustomPoint(p);return;
  }
  if(p.x>=465&&p.x<=1135&&p.y>=599&&p.y<=665){
   selectedCustomFrame=juce::jlimit(0,7,(int)((p.x-465.f)/84.f));
   synth.setValue(serenity::position,(float)selectedCustomFrame/7.f);
   customEditorTouched=true;repaint();return;
  }
  if(e.mods.isRightButtonDown()&&p.x>=462&&p.x<=1137&&p.y>=163&&p.y<=570) {
   showWavetableMenu();return;
  }
  if(p.x>20&&p.x<426&&p.y>153&&p.y<193){
   selectedOsc=juce::jlimit(0,3,(int)((p.x-20)/104.f));repaint();return;
  }
  if(p.x>24&&p.x<425&&p.y>=205&&p.y<572){
   int col=juce::jlimit(0,3,(int)((p.x-24)/103.5f));
   int row=juce::jlimit(0,3,(int)((p.y-205)/94.9f));
   const int waves[]={serenity::oscAWave,serenity::oscBWave,serenity::oscCWave,serenity::oscDWave};
   rememberUndo();
   synth.setValue(waves[selectedOsc],(float)(row*4+col));repaint();return;
  }
  if(p.x>=458&&p.x<1133&&p.y>=599&&p.y<=665){
   int frame=juce::jlimit(0,7,(int)((p.x-458)/84.f));
   synth.setValue(serenity::position,frame/7.f);repaint();return;
  }
 }
 if(tab==2){
  if(p.y>=624&&p.y<=656&&((p.x>=21&&p.x<=230)||(p.x>=768&&p.x<=972))){juce::StringArray choices;for(auto name:serenity::filterNames)choices.add(name);showChoiceMenu(p.x<500?serenity::filter1Type:serenity::filter2Type,choices);return;}
  if(p.x>=674&&p.x<=730&&p.y>=578&&p.y<=602){rememberUndo();synth.setValue(serenity::filter1Enabled,synth.getValue(serenity::filter1Enabled)>.5f?0.f:1.f);repaint();return;}
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
    if(localX>=698&&localX<=724){rememberUndo();synth.setRoute(slot,{});repaint();return;}
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
  rememberUndo();
  draggedParameter=param;startValue=synth.getValue(param);repaint();return;
 }
}
void SerenityEditor::mouseDrag(const juce::MouseEvent& e){
 if(drawingHarmonic){drawCustomHarmonic(logical(e.position));return;}
 if(drawingCustom){drawCustomPoint(logical(e.position));return;}
 if(sequenceDragLane>=0){editSequenceAt(logical(e.position),sequenceDragLane);return;}
 if(draggedParameter<0)return;
 const auto &spec=serenity::specs[(size_t)draggedParameter];
 auto p=logical(e.position);
 float change=(downPos.y-p.y)/245.f*(spec.hi-spec.lo)*(e.mods.isShiftDown() ? .12f:1.0f);
 synth.setValue(draggedParameter,juce::jlimit(spec.lo,spec.hi,startValue+change));
 showStatus(juce::String(spec.id)+" : "+juce::String(synth.getValue(draggedParameter),3));
}
void SerenityEditor::mouseUp(const juce::MouseEvent&){
 if(drawingCustom) synth.rebuildCustomFrame(selectedOsc,selectedCustomFrame);
 draggedParameter=-1;
 sequenceDragLane=-1;
 drawingCustom=false;
 drawingHarmonic=false;
 customDrawLastSample=-1;
}
void SerenityEditor::mouseDoubleClick(const juce::MouseEvent& e){
 int index=paramAt(logical(e.position));
 if(index>=0){synth.setValue(index,serenity::specs[(size_t)index].initial);
  showStatus(tr("Paramètre réinitialisé.","Parameter reset."));}
}
void SerenityEditor::mouseMove(const juce::MouseEvent& e){
 int i=paramAt(logical(e.position));
 if(i!=hoverParameter){hoverParameter=i;repaint();}
}
void SerenityEditor::mouseExit(const juce::MouseEvent&){if(hoverParameter>=0){hoverParameter=-1;repaint();}}
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
juce::Rectangle<int> SerenityEditor::footerRepaintBounds()const{return scaleRect(875,858,617,133).getSmallestIntegerContainer();}
void SerenityEditor::timerCallback(){
 const bool hadStatus=statusCountdown>0;if(hadStatus)statusCountdown--;
 auto follow=[](float prev,float input){return input>prev?input:std::max(input,prev*.88f-.0002f);};
 meterLeft=follow(meterLeft,synth.getChannelPeak(0));meterRight=follow(meterRight,synth.getChannelPeak(1));
 // Full repaint remains necessary for host parameter automation. JUCE clips the
 // dynamic plots to their reserved regions and does not create child windows.
 repaint();
}


void SerenityEditor::paintSequencer(juce::Graphics& g) {
 if(tab!=5 && tab!=6)return;
 const bool arp=tab==5;
 const bool enabled=synth.getSequenceMode()==(arp?1:2);
 if(!enabled && !sequenceTouched)return; // Original screenshot is left pixel-for-pixel at startup.
 const int bank=synth.getSequenceBank();
 const int length=synth.getSequenceLength();
 const int playing=synth.getSequencePlayingStep();
 const int first=sequencePage*16;
 const float xStart=86.f,cell=85.6f;
 const auto ink=juce::Colour(0xff09121d);
 const auto grid=juce::Colour(0xff263449);
 const auto normal=juce::Colour(0xff6ed5e4);
 const auto dim=juce::Colour(0xff2b5661);
 const auto mod=juce::Colour(0xffa883ed);
 auto drawLane=[&](float top,float bottom,float middle,int type){
  auto area=scaleRect(75,top,1390,bottom-top);
  g.setColour(ink);g.fillRect(area);
  g.setColour(grid);
  for(int q=0;q<=4;q++) {
   auto yy=(top+(bottom-top)*q/4.f)*getHeight()/canvasHeight;
   g.drawHorizontalLine((int)std::round(yy),area.getX(),area.getRight());
  }
  const auto midPixel=(middle*getHeight()/canvasHeight);
  g.setColour(juce::Colour(0xff49617a));
  g.drawHorizontalLine((int)std::round(midPixel),area.getX(),area.getRight());
  for(int i=0;i<16;i++){
   const int index=first+i;
   const auto step=synth.getSequenceStep(bank,index);
   const bool active=index<length;
   const float center=xStart+cell*i+cell*.5f;
   const float width=cell*.72f;
   const float drawX=center-width*.5f;
   float value=0;
   if(type==0)value=(float)step.pitch/24.f;
   if(type==1)value=step.velocity;
   if(type==2)value=step.modulation;
   const float vertical=(type==1) ? (bottom-top-16.f) : (bottom-top)*.46f;
   const float high=std::max(1.5f,std::abs(value)*vertical);
   const float baseline=type==1?bottom-4.f:middle;
   const float y=(type==1||value>=0)?baseline-high:baseline;
   g.setColour(active?(type==2?mod:normal):dim);
   g.fillRoundedRectangle(scaleRect(drawX,y,width,high),2.0f);
   if(active && playing==index && enabled){
    g.setColour(gold.withAlpha(.9f));
    g.drawRoundedRectangle(scaleRect(drawX-3.f,top+3.f,width+6.f,bottom-top-6.f),3.f,1.4f);
   }
   if(type==0 && active)
    drawText(g,juce::String(step.pitch>0?"+":"")+juce::String(step.pitch),drawX+12,y-22.f,48,20,muted,11);
  }
  if(length<=first) {
   g.setColour(dim.withAlpha(.55f));g.fillRect(area);
  }
 };
 if(arp){
  drawLane(365.f,622.f,494.f,0);
  // A live note-name/step-number strip replaces the frozen labels in the reference image.
  // It does not overlap the instructional text at the top of the original plot.
  g.setColour(ink);g.fillRect(scaleRect(76,623,1388,52));
  static const char* pitchNames[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
  for(int i=0;i<16;i++){
   const int index=first+i;
   const auto step=synth.getSequenceStep(bank,index);
   const int midi=juce::jlimit(0,127,60+step.pitch);
   const float x=xStart+cell*i+cell*.5f;
   const juce::Colour color=index<length?muted:dim;
   drawText(g,juce::String(pitchNames[midi%12])+juce::String(midi/12-1),
            x-23,626,46,19,color,10);
   drawText(g,juce::String(index+1),x-9,651,26,17,color,10);
  }
 }else{
  drawLane(326.f,440.f,385.f,0);
  drawLane(519.f,607.f,603.f,1);
  drawLane(705.f,796.f,751.f,2);
  drawText(g,tr("BANQUE ","BANK ")+juce::String(bank+1)+
           juce::String::fromUTF8(" · ")+juce::String(length)+tr(" PAS"," STEPS")+
           " ("+juce::String(sequencePage+1)+"/4)",
           1040,164,210,24,violet,11,true);
  if(sequencePage>0){
   // Only future pages replace the screenshot's static step numbers.
   g.setColour(ink);g.fillRect(scaleRect(75,798,1390,27));
   for(int i=0;i<16;i++)
    drawText(g,juce::String(first+i+1),xStart+cell*i+cell*.5f-13.f,803,30,18,muted,10);
  }
 }
 if(arp)drawText(g,juce::String(sequencePage+1)+"/4",1396,800,60,22,gold,12,true);
 if(enabled){
  const auto label=arp?tr("ARP : ACTIVÉ","ARP: ON"):tr("SEQ : ACTIVÉ","SEQ: ON");
  g.setColour(juce::Colour(0xff12404b));
  g.fillRoundedRectangle(scaleRect(25,183,148,30),14.f);
  drawText(g,label,40,183,120,30,aqua,12,true);
 }
}
bool SerenityEditor::editSequenceAt(juce::Point<float> p,int lane) {
 const int bank=synth.getSequenceBank();
 const int index=sequencePage*16+juce::jlimit(0,15,(int)((p.x-85.f)/85.6f));
 if(index>=64||p.x<76||p.x>1460)return false;
 const auto stepBefore=synth.getSequenceStep(bank,index);
 auto step=stepBefore;
 if(lane==0){
  const float baseline=(tab==5)?494.f:385.f;
  const float extent=(tab==5)?125.f:55.f;
  step.pitch=juce::jlimit(-24,24,(int)std::lround((baseline-p.y)/extent*24.f));
 }else if(lane==1){
  step.velocity=juce::jlimit(0.f,1.f,(603.f-p.y)/77.f);
 }else if(lane==2){
  step.modulation=juce::jlimit(-1.f,1.f,(751.f-p.y)/39.f);
 }else return false;
 synth.setSequenceStep(bank,index,step);
 sequenceTouched=true;
 repaint();
 return true;
}
void SerenityEditor::showSequenceRateMenu(){
 juce::PopupMenu m;
 m.addSectionHeader(tr("DIVISION RYTHMIQUE","STEP DIVISION"));
 for(int rate:{4,8,16,32})
  m.addItem(rate,juce::String("1/")+juce::String(rate),true,synth.getSequenceRate()==rate);
 juce::Component::SafePointer<SerenityEditor> safe(this);
 m.showMenuAsync(juce::PopupMenu::Options(),[safe](int choice){
  if(!safe||choice==0)return;
  safe->synth.setSequenceRate(choice);
  safe->showStatus(juce::String("1/")+juce::String(choice));
  safe->sequenceTouched=true;safe->repaint();
 });
}
void SerenityEditor::showSequenceBankMenu(){
 juce::PopupMenu m;
 m.addSectionHeader(tr("BANQUE DE PATTERNS","PATTERN BANK"));
 for(int i=0;i<4;i++)
  m.addItem(i+1,tr("Banque ","Bank ")+juce::String(i+1),true,synth.getSequenceBank()==i);
 juce::Component::SafePointer<SerenityEditor> safe(this);
 m.showMenuAsync(juce::PopupMenu::Options(),[safe](int choice){
  if(!safe||choice==0)return;
  safe->synth.setSequenceBank(choice-1);
  safe->sequenceTouched=true;safe->repaint();
 });
}
bool SerenityEditor::sequenceClick(juce::Point<float> p) {
 const bool arp=tab==5;
 if(p.x>=25&&p.x<174&&p.y>174&&p.y<229){
  const int mode=arp?1:2;
  synth.setSequenceMode(synth.getSequenceMode()==mode?0:mode);
  sequenceTouched=true;
  showStatus(arp?tr("Arpégiateur activé / désactivé","Arpeggiator toggled"):
                 tr("Séquenceur activé / désactivé","Sequencer toggled"));
  return true;
 }
 if(p.x>=196&&p.x<388&&p.y>=185&&p.y<238){showSequenceRateMenu();return true;}
 if(p.x>=414&&p.x<488&&p.y>=165&&p.y<244){
  juce::PopupMenu menu;
  for(int count:{8,16,32,64})
   menu.addItem(count,juce::String(count)+tr(" pas"," steps"),true,synth.getSequenceLength()==count);
  juce::Component::SafePointer<SerenityEditor> safe(this);
  menu.showMenuAsync(juce::PopupMenu::Options(),[safe](int count){
   if(!safe||count<=0)return;
   safe->synth.setSequenceLength(count);safe->sequencePage=0;
   safe->sequenceTouched=true;safe->repaint();
  });
  return true;
 }
 if(p.x>=497&&p.x<565&&p.y>=168&&p.y<242){
  synth.setSequenceGate(juce::jlimit(0.05f,1.f,synth.getSequenceGate()+.10f));
  sequenceTouched=true;showStatus(tr("Durée des pas : ","Step gate: ")+juce::String(synth.getSequenceGate(),2));return true;
 }
 if(p.x>=577&&p.x<655&&p.y>=168&&p.y<242){
  synth.setSequenceSwing(synth.getSequenceSwing()>=.48f?0.f:synth.getSequenceSwing()+.10f);
  sequenceTouched=true;showStatus("Swing : "+juce::String(synth.getSequenceSwing(),2));return true;
 }
 if(!arp&&p.x>=1250&&p.x<1375&&p.y>=159&&p.y<231){showSequenceBankMenu();return true;}
 // Right and left arrows in the three sequencer lane headers.
 if(!arp&&p.x>1390&&p.x<1482&&
    ((p.y>=261&&p.y<=305)||(p.y>=466&&p.y<=509)||(p.y>=640&&p.y<=681))){
  sequencePage=(sequencePage+(p.x<1438?3:1))%4;
  sequenceTouched=true;repaint();return true;
 }
 if(arp&&p.x>1350&&p.y>292&&p.y<330){sequencePage=(sequencePage+1)%4;sequenceTouched=true;repaint();return true;}
 const bool pitch=arp?(p.y>=356&&p.y<640):(p.y>=311&&p.y<450);
 const bool level=!arp&&p.y>=508&&p.y<614;
 const bool modulation=!arp&&p.y>=689&&p.y<807;
 if(p.x>75&&p.x<1463&&(pitch||level||modulation)){
  const int lane=pitch?0:(level?1:2);
  if(editSequenceAt(p,lane)){sequenceDragLane=lane;return true;}
 }
 if(arp&&p.y>741&&p.y<784&&p.x>=125&&p.x<1364){
  const int tool=juce::jlimit(0,8,(int)((p.x-125)/139.f));
  static const int shapes[9][8]={
   {0,12,24,12,0,12,24,12},{0,4,7,12,7,4,0,4},{0,3,7,12,7,3,0,3},
   {0,3,7,10,7,3,0,3},{0,4,7,11,7,4,0,4},{0,4,7,12,7,4,0,-5},
   {0,7,12,7,0,7,12,7},{0,5,7,12,7,5,0,5},{0,0,0,0,0,0,0,0}
  };
  const int bank=synth.getSequenceBank();
  for(int i=0;i<64;i++){auto v=synth.getSequenceStep(bank,i);v.pitch=shapes[tool][i%8];synth.setSequenceStep(bank,i,v);}
  sequenceTouched=true;repaint();return true;
 }
 if(arp&&p.y>786&&p.y<828&&p.x>=125&&p.x<812){
  const int edit=juce::jlimit(0,4,(int)((p.x-125)/140.f));
  const int bank=synth.getSequenceBank();
  std::array<serenity::SequenceStep,64> copy{};
  for(int i=0;i<64;i++)copy[(size_t)i]=synth.getSequenceStep(bank,i);
  for(int i=0;i<64;i++){
   auto v=copy[(size_t)i];
   if(edit<=1)v.pitch=juce::jlimit(-24,24,v.pitch+(edit==0?-12:12));
   else if(edit==2)v=copy[(size_t)((i+1)%64)];
   else if(edit==3)v=copy[(size_t)((i+63)%64)];
   else v.pitch=juce::Random::getSystemRandom().nextInt(49)-24;
   synth.setSequenceStep(bank,i,v);
  }
  sequenceTouched=true;repaint();return true;
 }
 return false;
}


// Wavetable drawing and operations live in the existing 2D/3D editor panels.
// The ten original skin images remain unchanged.
void SerenityEditor::drawCustomPoint(juce::Point<float> p){
 const int point=juce::jlimit(0,serenity::editableSamples-1,
  (int)std::lround((p.x-1171.f)/296.f*(serenity::editableSamples-1)));
 const float value=juce::jlimit(-1.f,1.f,(538.f-p.y)/66.f);
 if(customDrawLastSample<0){
  synth.setCustomPoint(selectedOsc,selectedCustomFrame,point,value);
 }else{
  const int start=customDrawLastSample,stop=point;
  const int steps=std::abs(stop-start);
  if(steps==0)synth.setCustomPoint(selectedOsc,selectedCustomFrame,point,value);
  else for(int n=0;n<=steps;n++){
   const float t=(float)n/steps;
   const int index=start+(int)std::lround((stop-start)*t);
   synth.setCustomPoint(selectedOsc,selectedCustomFrame,index,
      customDrawLastValue+(value-customDrawLastValue)*t);
  }
 }
 customDrawLastSample=point;customDrawLastValue=value;
 customEditorTouched=true;
 repaint();
}

void SerenityEditor::importWavetableAudio(int osc){
 juce::PopupMenu choices;
 choices.addSectionHeader(tr("TAILLE DU CYCLE SOURCE","SOURCE CYCLE LENGTH"));
 choices.addItem(1,tr("Un seul cycle : fichier entier","Single cycle: whole file"));
 for(int n:{256,512,1024,2048,4096})choices.addItem(n,juce::String(n)+tr(" échantillons par cycle"," samples per cycle"));
 juce::Component::SafePointer<SerenityEditor> safe(this);
 choices.showMenuAsync(juce::PopupMenu::Options(),[safe,osc](int choice){
  if(safe&&choice>0)safe->chooseWavetableFile(osc,choice==1?0:choice);
 });
}
void SerenityEditor::chooseWavetableFile(int osc,int cycleLength){
 fileChooser=std::make_unique<juce::FileChooser>(
  tr("Importer une table d'ondes WAV ou AIFF","Import WAV or AIFF wavetable"),juce::File{},"*.wav;*.aif;*.aiff");
 juce::Component::SafePointer<SerenityEditor> safe(this);
 fileChooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,
 [safe,osc,cycleLength](const juce::FileChooser& chooser){
  if(!safe)return;
  auto file=chooser.getResult();if(file==juce::File{})return;
  safe->rememberUndo();
  auto result=safe->synth.importWavetableFile(file,osc,cycleLength);
  if(result.failed()){safe->showStatus(result.getErrorMessage());return;}
  safe->selectedOsc=osc;safe->selectedCustomFrame=0;
  safe->synth.setValue(serenity::position,0.f);safe->customEditorTouched=true;
  safe->showStatus(safe->tr("Import réussi : 8 formes éditables. ","Imported: 8 editable frames. ")+file.getFileNameWithoutExtension());
  safe->repaint();
 });
}

void SerenityEditor::showWavetableMenu(){
 juce::PopupMenu menu;
 menu.addSectionHeader(tr("ÉDITION WAVETABLE","WAVETABLE EDITING"));
 menu.addItem(1,tr("Importer une wavetable WAV / AIFF...","Import WAV / AIFF wavetable..."));
 menu.addItem(2,tr("Activer la table personnalisée","Use custom wavetable"));
 menu.addItem(3,tr("Normaliser l'image sélectionnée","Normalize selected frame"));
 menu.addItem(4,tr("Supprimer l'offset continu","Remove DC offset"));
 menu.addItem(5,tr("Dupliquer vers l'image suivante","Duplicate to next frame"));
 menu.addItem(6,tr("Réinitialiser cette image","Reset this frame"));
 menu.addItem(7,tr("Exporter les huit images en WAV...","Export eight frames as WAV..."));
 juce::PopupMenu spectra;
 spectra.addItem(10,tr("Filtre spectral : 16 harmoniques","Spectral filter: 16 harmonics"));
 spectra.addItem(11,tr("Filtre spectral : 32 harmoniques","Spectral filter: 32 harmonics"));
 spectra.addItem(12,tr("Fondamentale seule","Fundamental only"));
 spectra.addItem(13,tr("Harmoniques impaires","Odd harmonics"));
 spectra.addItem(14,tr("Harmoniques paires","Even harmonics"));
 spectra.addItem(15,tr("Supprimer les 4 premières harmoniques","Remove first four harmonics"));
 spectra.addItem(16,tr("Rotation de phase : +90 degrés","Rotate spectral phase by +90 degrees"));
 spectra.addItem(17,tr("Lissage spectral : 64 harmoniques","Spectral smoothing: 64 harmonics"));
 spectra.addSeparator();
 spectra.addItem(18,tr("Morphing des images 1 à 8","Morph frames 1 to 8"));
 menu.addSubMenu(tr("TRAITEMENTS SPECTRAUX","SPECTRAL PROCESSING"),spectra);
 const int osc=selectedOsc,frame=selectedCustomFrame;
 juce::Component::SafePointer<SerenityEditor> safe(this);
 menu.showMenuAsync(juce::PopupMenu::Options(),[safe,osc,frame](int choice){
  if(!safe||choice<=0)return;
  if(choice==1){safe->importWavetableAudio(osc);return;}
  if(choice==7){
   safe->fileChooser=std::make_unique<juce::FileChooser>(
    safe->tr("Exporter ma wavetable","Export custom wavetable"),
    juce::File{},"*.wav");
   juce::Component::SafePointer<SerenityEditor> nested(safe);
   safe->fileChooser->launchAsync(
    juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles,
    [nested,osc](const juce::FileChooser& chooser){
     if(!nested)return;
     juce::File file=chooser.getResult();
     if(file==juce::File{})return;
     if(file.getFileExtension().isEmpty())file=file.withFileExtension(".wav");
     std::vector<unsigned char> bytes;
     bytes.reserve(44+serenity::editableFrames*serenity::editableSamples*2);
     auto push16=[&bytes](int x){bytes.push_back((unsigned char)(x&255));bytes.push_back((unsigned char)((x>>8)&255));};
     auto push32=[&bytes](unsigned x){for(int k=0;k<4;k++)bytes.push_back((unsigned char)((x>>(k*8))&255));};
     auto label=[&bytes](const char* s){for(int k=0;k<4;k++)bytes.push_back((unsigned char)s[k]);};
     const unsigned frames=serenity::editableFrames*serenity::editableSamples;
     label("RIFF");push32(36+frames*2);label("WAVE");
     label("fmt ");push32(16);push16(1);push16(1);push32(44100);push32(88200);push16(2);push16(16);
     label("data");push32(frames*2);
     for(int frame=0;frame<serenity::editableFrames;frame++)
      for(int i=0;i<serenity::editableSamples;i++){
       const float v=nested->synth.getCustomPoint(osc,frame,i);
       push16((int)(short)juce::jlimit(-32768,32767,(int)std::lround(v*32767.f)));
      }
     if(file.replaceWithData(bytes.data(),bytes.size()))
      nested->showStatus(nested->tr("Wavetable WAV exportée.","Wavetable WAV exported."));
     else nested->showStatus(nested->tr("Échec de l'export WAV.","WAV export failed."));
    });
   return;
  }
  switch(choice){
   case 2: break;
   case 3: safe->synth.normalizeCustomFrame(osc,frame);break;
   case 4: safe->synth.removeCustomDc(osc,frame);break;
   case 5: safe->synth.copyCustomFrame(osc,frame,(frame+1)%serenity::editableFrames);break;
   case 6: safe->synth.resetCustomFrame(osc,frame);break;
   case 10:safe->synth.transformCustomFrame(osc,frame,serenity::spectral::Transform::lowPass16);break;
   case 11:safe->synth.transformCustomFrame(osc,frame,serenity::spectral::Transform::lowPass32);break;
   case 12:safe->synth.transformCustomFrame(osc,frame,serenity::spectral::Transform::fundamental);break;
   case 13:safe->synth.transformCustomFrame(osc,frame,serenity::spectral::Transform::oddOnly);break;
   case 14:safe->synth.transformCustomFrame(osc,frame,serenity::spectral::Transform::evenOnly);break;
   case 15:safe->synth.transformCustomFrame(osc,frame,serenity::spectral::Transform::highPass4);break;
   case 16:safe->synth.transformCustomFrame(osc,frame,serenity::spectral::Transform::phaseQuarterTurn);break;
   case 17:safe->synth.transformCustomFrame(osc,frame,serenity::spectral::Transform::smooth64);break;
   case 18:safe->synth.morphCustomFrames(osc,0,7);break;
   default:return;
  }
  safe->synth.setValue(serenity::oscAWave+osc,(float)serenity::waveCount);
  safe->customEditorTouched=true;
  safe->repaint();
 });
}


void SerenityEditor::drawCustomHarmonic(juce::Point<float> p){
 // Left-hand dB labels and bottom 1/8/16/24/32 labels belong to the original skin.
 constexpr float left=1202.f,right=1466.f,top=166.f,bottom=355.f;
 const int harmonic=juce::jlimit(1,serenity::spectral::displayedHarmonics,
    1+(int)((juce::jlimit(left,right-.01f,p.x)-left)/
      ((right-left)/serenity::spectral::displayedHarmonics)));
 const float relativeDb=-60.f*juce::jlimit(0.f,1.f,(p.y-top)/(bottom-top));
 const auto spectrum=synth.getCustomHarmonics(selectedOsc,selectedCustomFrame);
 float peak=0.f;
 for(auto x:spectrum)peak=std::max(peak,x);
 const float reference=std::max(.05f,peak);
 const float target=(p.y>=bottom-2.f)?0.f:
   juce::jlimit(0.f,1.f,reference*std::pow(10.f,relativeDb/20.f));
 selectedHarmonic=harmonic;
 synth.setCustomHarmonic(selectedOsc,selectedCustomFrame,harmonic,target);
 customEditorTouched=true;
 repaint();
}
