# Serenity WAVETABLE — Suivi de réalisation (20 novembre 2026)

## Référence visuelle
Les dix captures originales sont la référence stricte : oscillateurs, éditeur wavetable, filtres, modulation, effets, arpégiateur, séquenceur, matrice, presets et réglages. Ne pas inventer d'images alternatives. Aucun chevauchement, caractère corrompu ni élément inaccessible. Maintenir la palette bleu nuit/graphite, turquoise, violet et or.

## État du paquet de développement v0.1
Le ZIP source préparé contient un moteur DSP autonome, un projet CMake/JUCE, des tests, les 10 captures et une première interface interactive partielle. Ce paquet **n'est pas encore entièrement publié dans ce dépôt GitHub**.

| Module | État du paquet v0.1 |
| --- | --- |
| DSP wavetable | Cœur autonome testé, 16 familles × 8 frames |
| Polyphonie | Base jusqu'à 32 notes |
| Oscillateurs | Base de 4 générateurs |
| Filtres | Passe-bas simplifiés, série/parallèle |
| Enveloppes/LFO | Versions partielles |
| Matrice | 64 emplacements, 5 destinations DSP de base |
| Effets | Saturation, délai et réverbération simples |
| Arpégiateur | Non implémenté |
| Séquenceur 64 pas | Non implémenté |
| Presets | 100 définitions programmatiques, non validées individuellement |
| MIDI Learn | Implémentation initiale, validation DAW nécessaire |
| FR/EN | Navigation initiale, traduction complète non réalisée |
| AU/VST3 | Compilation/validation DAW non effectuées |

## Avant publication commerciale
1. Transférer tous les fichiers sources et ressources PNG du paquet v0.1 dans GitHub.
2. Exécuter les builds AU/VST3/Standalone macOS et VST3/Standalone Windows, corriger les erreurs et produire de vrais artefacts.
3. Remplacer les captures statiques par des composants JUCE interactifs, fidèles à la référence pixel par pixel.
4. Compléter synthèse, import/édition de wavetable, filtres, enveloppes, LFOs, matrice, trente effets, arpégiateur et séquenceur 64 pas.
5. Finaliser les traductions FR/EN et vérifier les accents, symboles, tailles et alignements sur les dix onglets.
6. Écouter, équilibrer, corriger et valider les 100 presets un par un.
7. Tester DAW, performances, MIDI Learn, sauvegarde/rappel, puis réaliser signature et notarisation macOS.

**Ne pas présenter les fonctionnalités non développées comme finalisées.**
