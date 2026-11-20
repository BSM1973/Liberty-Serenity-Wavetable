# Serenity WAVETABLE v0.4.0 — Éditeur de wavetables

Conserve les 10 images originales 2048 × 1361 comme fond d'interface.

## Fonctionnel et connecté au DSP
- Quatre oscillateurs possèdent chacun une wavetable personnalisée, composée de 8 images de 512 points.
- Clic et dessin à la souris dans le panneau OUTPUT WAVEFORM de l'écran original WAVETABLE EDITOR.
- Sélection directe des huit images via les huit boutons existants.
- Visualisation 2D et 3D de la table personnalisée, uniquement lorsqu'elle est sélectionnée.
- Morphing sonore entre images ; la forme dessinée remplace réellement l'oscillateur dans le moteur audio.
- Clic droit sur l'affichage pour importer un WAV mono/stéréo, exporter les 8 images en WAV PCM 16 bits.
- Normalisation, suppression d'offset continu, duplication d'une image vers la suivante, remise à zéro.
- Sauvegarde des quatre tables personnalisées dans l'état AU/VST3 et dans les presets enregistrés.
- Modification en direct via un tampon atomique lu seulement lors du changement, sans allocation dans le thread audio.

## Connu et non encore implémenté
- Import/export de formats propriétaires de tables tiers
- Dessin spectral/harmonique par FFT avec édition de phase
- Tables allant jusqu'à 256 frames (8 frames réellement implémentées pour le moment)
- Multi-mip custom/anti-aliasing spectral complet, resynthèse à partir de longues sources WAV
- 30 effets et architecture commerciale complète non encore disponibles

## Garantie de conception
Les fichiers `Assets/UI/skin_*.png` restent inchangés. Aucune nouvelle interface ni élément de décoration n'a été substitué aux captures. Le rendu dynamique n'apparaît que quand l'utilisateur sélectionne une table personnalisée.
