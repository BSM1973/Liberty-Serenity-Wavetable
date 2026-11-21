# SERENITY WAVETABLE v0.8.0 — Reconstruction native de l'interface

## Correction de conception fondamentale
Jusqu'à la v0.7.0, le plugin chargeait dix captures d'écran complètes comme images de fond. Les boutons ajoutés étaient superposés et le pointeur de la souris présent dans les captures pouvait donc être visible.

**À partir de v0.8.0, les PNG d'origine ne sont plus chargés ni compilés dans le plugin.** Ils restent dans `Assets/UI` comme références visuelles, hors du binaire. Les dix pages sont peintes en direct par `Source/SerenityNativeUI.cpp`, sans aucune capture raster à l'écran.

## Éléments reconstitués
- Bandeau SERENITY WAVETABLE by LIBERTY, sélection de presets, A/B, navigation, LEARN, volume, voix et vu-mètres.
- Dix onglets à leurs emplacements de référence.
- Panneaux métalliques graphite/bleu nuit, contours, séparateurs, éclairages cyan/violet/doré, boutons, sélecteurs, potentiomètres, valeurs, graduations et typographie.
- OSCILLATEURS : quatre oscillateurs, formes d'onde calculées, 3D wavetable native, position, niveaux, unisson, modulation croisée, visualisation 2D.
- ÉDITEUR WAVETABLE : bibliothèque de seize tables, huit images, représentation 3D et spectre, plus les commandes d'édition déjà intégrées.
- FILTRES : réponse fréquentielle native actualisée depuis les paramètres.
- MODULATION : enveloppe et LFO dessinés, 6 sélecteurs ENV et 8 LFO.
- EFFETS : chaîne, panneaux de contrôles et courbes dessinés par JUCE.
- ARPÉGIATEUR et SÉQUENCEUR : barres MIDI calculées depuis les patterns du moteur.
- MATRICE : 64 routages paginés (affichage des 24 premières lignes de la page de base).
- PRESETS, RÉGLAGES et pied de page macros/roues/mesures.

## Fonctions d'interface et tests
- L'affichage n'utilise plus `BinaryData::skin_*.png` ni `drawImageWithin`.
- La taille native 1492×991 est ajustée à la surface disponible sur les petits écrans.
- Textes français UTF-8 et anglais, avec sélecteur FR/EN.
- Le test JUCE crée, peint et distingue les dix écrans ; il vérifie également qu'un changement de niveau OSC A modifie le rendu (impossible avec une simple image figée).
- La chaîne Actions compile AU/VST3/Standalone sur macOS ARM64 et VST3/Standalone sous Windows x64.

## Transparence
- Les dix pages disposent désormais d'un rendu natif, mais la correspondance visuelle pixel par pixel avec les dix captures n'est pas encore certifiée. Comparaison à l'œil et correction fine de chaque page nécessaires.
- Certaines commandes visibles correspondant à des fonctions avancées encore absentes du DSP sont des contrôles graphiques de référence et ne sont pas encore connectées.
- Les courbes de synthèse utilisent les états de paramètres et, selon les panneaux, des représentations calculées ; les analyseurs audio live complets restent à développer.
- Les builds verts ne remplacent pas un test d'usage dans Ableton Live, Logic, Bitwig ou d'autres DAW.

Les règles restent : zéro raster des captures dans le binaire, aucun curseur visible, zéro chevauchement, aucun caractère français corrompu, conservation des dix structures visuelles d'origine.
