# Serenity WAVETABLE v0.9.0 — Native UI: 3D, qualité visuelle et contrôles DSP

Cette version continue à employer exclusivement JUCE/C++17. Aucun PNG de capture écran n'est chargé ou intégré au plugin.

## Éditeur et oscillateurs : données réelles
- Les vues 3D de l'onglet OSCILLATEURS et de l'onglet ÉDITEUR WAVETABLE dessinent réellement les 8 frames des 4 oscillateurs à partir des tables PCM du moteur (prévisualisation mip 0).
- Dessin des couches en perspective, remplissage semi-transparent par frame, plan d'affichage de la position et mise en évidence de la couche sélectionnée.
- Les courbes 2D de chaque oscillateur, les miniatures des 16 wavetables et les 8 vignettes de frames utilisent les mêmes données du moteur.
- Le mode Custom lit directement les échantillons modifiés; les transformations WARP/SHAPE sont répercutées sur les courbes comme sur le signal.

## Commandes réellement activées
- 32 nouveaux paramètres APVTS (4 oscillateurs × octave, demi-tons, cents, panorama, warp amount, shape amount, warp mode, shape mode), sauvegardés dans les projets AU/VST3, avec automation et MIDI Learn.
- WARP : Off, Bend, PWM, Sync. SHAPE : Off, Saturate, Wavefold, Quantize. Ces traitements agissent réellement dans la synthèse; la visualisation appelle le même code DSP.
- Navigation native A/B/C/D, menus de table et de modes, glissement des potentiomètres, double clic pour réinitialiser, Shift pour réglage fin.
- En-tête : comparaison A/B de deux états complets, favoris, Annuler/Rétablir (32 états pour les interactions natives), raccourcis clavier Ctrl/Cmd+Z, Ctrl/Cmd+Shift+Z et Ctrl/Cmd+Y, FR/EN.
- Spectre et oscilloscope : lecture de 256 échantillons du bus stéréo de sortie, avec FFT 512 points calculée seulement sur le thread UI, aucune allocation graphique sur le thread audio.

## Design et ergonomie
- Finitions métalliques davantage travaillées : gradients bleu graphite, reflets de biseaux, grain discret, LED des commandes.
- Reprise des dimensions et zones actives de l'écran OSCILLATEURS, sans aucun PNG importé.
- Message d'état déplacé entre la barre de modulation et le pied de page pour éliminer son chevauchement.

## Tests
- Analyse et reconstruction FFT déjà testées depuis la v0.5.
- Nouveau test pur C++ de WARP/PWM/SYNC/SHAPE, valeurs limites et vrais échantillons des tables factory.
- Test JUCE : l'image de l'oscillateur est issue de l'audio PCM, WARP/SHAPE modifient les données, les réglages persistent lors de sauvegarde/restauration.
- Builds macOS ARM64 AU/VST3/Standalone et Windows x64 VST3/Standalone, avec vrais artefacts GitHub.

## Limites connues et prudence
- Les dix écrans sont natifs mais la fidélité pixel à pixel aux références nécessite encore une passe manuelle fine.
- Certains modules visibles des autres pages, et plusieurs commandes avancées non exposées par le moteur, restent à implémenter; une commande affichée n'est pas une preuve qu'un moteur DSP existe.
- Les nouvelles transformations non linéaires nécessitent encore des mesures audio anti-aliasing dédiées, notamment lors de modulation extrême.
- Les sauvegardes A/B et Undo/Redo sont locales à l'éditeur; pas encore un historique global des automations du DAW.
- Les binaires ne sont pas signés/notarisés.

Règles : aucun chevauchement, pas de symboles à la place des lettres, français UTF-8, aucune capture de fond, aucune utilisation de PluginMaker.ai.
