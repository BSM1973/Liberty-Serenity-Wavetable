# Serenity WAVETABLE — Suivi de réalisation (20 novembre 2026)

## Conception visuelle : référence contractuelle

Les 10 PNG originaux de l'utilisateur (2048×1361) sont dans `Assets/UI`. Ils servent de calques visuels de référence aux dix pages :

1. OSCILLATORS
2. WAVETABLE EDITOR
3. FILTERS
4. MODULATION
5. EFFECTS
6. ARPEGGIATOR
7. SEQUENCER
8. MATRIX
9. PRESETS
10. SETTINGS

Contrat d'interface : géométrie, couleurs, typographies, sections, logique de navigation conservées ; pas d'images alternatives inventées ; aucun chevauchement ; aucun glyphe corrompu. Remplacer progressivement les captures par une vraie interface vectorielle/juce interactive, **en gardant exactement cette direction**, avec animation et valeurs en temps réel.

## Ce qui est vraiment en place

| Domaine | Statut | Précision |
|---|---|---|
| Source C++ JUCE AU/VST3/Standalone | Code écrit, non compilé ici | JUCE indisponible dans le conteneur de validation |
| Moteur sonore autonome | Test validé | Génère de l'audio sans JUCE |
| 4 générateurs | Base fonctionnelle | 16 types de tables × 8 frames ; D n'a pas encore de lecture de sample |
| Unison | Base fonctionnelle | 1 à 16, contrôle limité dans l'UI actuelle |
| Polyphonie | Base fonctionnelle | Jusqu'à 32 notes, vol de voix simples |
| Dual filters | Base fonctionnelle | Passe-bas seulement (pente approximative), série/parallèle |
| Six enveloppes | Partiel | ADSR simple réutilisée ; pas six DAHDSR indépendantes avec courbes |
| Huit LFO | Partiel | Sorties sinusoïdales, pas les autres formes/modes |
| 64 routages | Partiel mais câblé au DSP | Destinations de base, pas toutes les destinations demandées |
| Effets | 3 modules audio simplifiés | Saturation, délai, réverbération ; 27+ traitements à développer |
| Arpégiateur | Non implémenté | Écran présent comme référence visuelle seulement |
| Séquenceur 64 pas | Non implémenté | Écran présent comme référence visuelle seulement |
| Presets d'usine | 100 définitions, non finalisées | Génération déterministe ; nécessitent programmation et écoute individuelles |
| Navigateur | Partiellement interactif | Recherche, catégories, favoris, importer/exporter l'état |
| FR/EN | Partiel | Navigation et commandes nouvelles, pas toutes les inscriptions des captures |
| MIDI Learn CC | Implémentation initiale | Mapping par clic droit, réception MIDI CC et persistance ; audit DAW requis |
| AU/VST3 validation | Non effectuée | Workflow CI fourni mais non exécuté sur macOS/Windows ici |

## Étapes indispensables avant une commercialisation

1. Compiler AU/VST3/Standalone sur macOS ARM64 et VST3/Standalone sur Windows, lever les erreurs, tester l'ouverture dans plusieurs DAW.
2. Convertir chaque contrôle représenté sur les dix captures en véritable composant interactif JUCE au plus près de son emplacement de référence ; faire disparaître les éléments visuellement figés, les curseurs de captures et les miniatures d'écran parasites.
3. Compléter le moteur de table : 256 frames, interpolation haute qualité, import WAV/glisser-déposer, édition par dessin/harmoniques/FFT, multi-samples et sauvegarde.
4. Ajouter les quinze algorithmes réels de chaque filtre, résonance, drive et enveloppes indépendantes.
5. Compléter les six enveloppes DAHDSR, huit LFOs multi-formes, toutes les sources/destinations et des modulations sur chaque paramètre ; actualiser les arcs de modulation.
6. Implémenter chaque effet (30) avec bypass, wet/dry, routage, réordonnancement, gestion des queues et tests audio.
7. Programmer arpégiateur avancé et séquenceur polyphonique 64 pas, sorties MIDI si compatibles et synchronisation DAW.
8. Finaliser mono/legato/portamento, aftertouch polyphonique, sustain, Scala et microtuning.
9. Traduire 100 % des éléments visuels : FR par défaut, bascule EN, accents et symboles vérifiés sur chaque onglet.
10. Écouter, corriger, documenter et équilibrer un par un les 100 presets ; contrôle du loudness, CPU, oversampling, anti-aliasing et gestion des clics.
11. Intégrer les builds, tests automatisés à 44,1/48/88,2/96/192 kHz, signatures Apple, notarisation et véritables installateurs via la chaîne de publication Liberty après validation.

Ce projet est une **base de développement démontrable**, pas une livraison commerciale terminée.
