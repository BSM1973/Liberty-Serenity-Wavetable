# Serenity WAVETABLE v0.5.0 — Édition FFT des harmoniques

Cette étape ajoute de vrais traitements spectraux à la table personnalisée de la v0.4.1, sans remplacer les dix images originales de l'interface.

## Traitements implémentés
- FFT radix-2 512 échantillons et reconstruction inverse, réelle, sans allocation dynamique.
- Analyse des 32 premières harmoniques de chacune des 8 images personnalisées des 4 oscillateurs.
- Cliquer et glisser directement sur les barres du panneau HARMONICS existant pour modifier une amplitude spectrale tout en conservant la phase.
- Les images de la table et le signal audio utilisent les échantillons reconstruits, pas une animation de démonstration.
- Menu contextuel : fondamentales seules, harmoniques paires/impaires, filtres spectraux 16/32/64, coupe-bas harmonique, rotation de phase de +90°.
- Morphing automatique entre la première et la dernière image, calculé et enregistré sur les images intermédiaires.
- Rappel de chaque harmonique modifiée dans l'état des projets DAW, grâce au mécanisme de la v0.4.1.

## Tests de régression
- FFT et FFT inverse 512 points, reconstruction exacte de sinus.
- Harmoniques FFT modifiées et préservation de la fondamentale.
- Filtrage des coefficients réels, rotation de phase, interpolation des images.
- Tests d'ouverture des 10 onglets, rendu de l'écran WAVETABLE EDITOR.
- Persistance et lecture audio dans le processeur JUCE; compilation AU/VST3/Standalone.

## Limites explicites
- Le panneau HARMONICS donne 32 amplitudes individuelles. Il ne constitue pas encore un éditeur spectral de phases indépendantes par harmonique.
- Les wavetables personnalisées restent sur 8 images x 512 points par oscillateur, et non 256 images.
- Les formes personnalisées n'ont pas encore de pyramide de bandes anti-aliasing complète pour les notes aiguës.
- Les 30 effets et d'autres exigences de la version commerciale restent à implémenter.

Critère permanent : aucune modification des dix captures `Assets/UI/skin_*.png`, aucun chevauchement ni remplacement de caractères français par des symboles.
