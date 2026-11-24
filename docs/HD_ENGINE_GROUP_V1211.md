# SERENITY WAVETABLE v1.2.11 — vraie comparaison groupée à 32 notes

## Exigences préservées

- Polyphonie 32 notes MIDI, 4 oscillateurs simultanés, jusqu'à 16 unités d'unisson par oscillateur.
- Tables HD complètes de 256 formes et 2048 échantillons par forme ; 11 niveaux anti-aliasing. Pas de baisse de fidélité ou de modulation.
- Aucun changement de l'interface JUCE, des presets, des identifiants hôte ou des formats AU/VST3/Standalone.

## Trois modes indépendants dans le moteur réel

Le mode 0 appelle exactement l'ancien lecteur scalaire v1.2.10. En mode 1, les oscillateurs HD à hauteur stable sont lus quatre par quatre, en regroupant les accès mémoire mais en conservant chaque niveau spectral, fraction d'échantillon et phase. En mode 2, seul le calcul d'interpolation utilise des opérations vectorielles NEON/SSE2 ; les lectures mémoire restent des gathers scalaires, car les phases sont différentes.

Tous les résultats des groupes sont ensuite accumulés **dans le même ordre** que le moteur original. Les phases, modulations, panoramiques, filtres, enveloppes, séquences et effets continuent de suivre leurs voies de production. En modulation dynamique LFO ou FM, chaque oscillateur garde son lecteur scalaire existant.

L'activation est contrôlée par `Engine::setHdUnisonGroupMode(0|1|2)` pour les tests natifs ; le plugin JUCE ne modifie pas cette valeur et reste en mode scalaire par défaut jusqu'à validation des rapports.

## Tests haute exigence

- 32 000 comparaisons de groupes de quatre phases/positions/mips : sortie groupée scalaire **identique exactement** au chemin individuel ; SIMD tolérance 2e−6.
- Benchmark **sur le vrai `Engine::render`** en conditions 32 notes et quatre générateurs : chaque mode exécuté deux fois dans l'ordre 0,1,2,2,1,0 sur le même runner.
- Scénarios : 48 kHz buffer 128, 96 kHz buffer 256, morphing, séquenceur transposé, unisson ×4, LFO dynamique en témoin, warp de phase. Résultats des trois modes dans `hd-unison-engine-benchmark.csv`, avec CPU moyen, p99, pic, dépassements et précision numérique.
- Référence inter-version : 18 cas A/B comparant v1.2.10 (commit `b6a9382bc3d7f8ed3c2b7228936b1f8f4353cd07`) et la v1.2.11, sur chaque plateforme.
- Toute différence de fidélité au-delà des tolérances provoque un build rouge. Un ratio SIMD favorable au microbenchmark uniquement ne justifie **jamais** son activation dans le moteur.

## Décision prévue

En fonction des mesures du moteur complet, activer uniquement les chemins qui sont **simultanément** plus rapides, sans régression des pics significative et numériquement fidèles. Si le groupement ou le SIMD ralentit Windows x64 ou ARM NEON, laisser la voie concernée désactivée dans le plugin de production et conserver l'étude et les tests pour une optimisation future.
