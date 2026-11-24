# SERENITY WAVETABLE v1.2.10 — lecture HD et faisabilité SIMD

## Objectifs sans compromis

32 notes, 4 générateurs, unisson x16, 256 formes par banque, 2048 échantillons par forme, 11 niveaux spectraux, 100 presets, formats AU/VST3/Standalone. Aucun changement de configuration, d'interface, de qualité ou de sérialisation.

## Chemin audio optimisé

1. `oscillatorWarpPhase` produit déjà une phase en [0, 1] ; utiliser directement cette phase dans `HdTable::samplePreparedSpectralWrapped`. Le cas limite où l'arrondi donne exactement 1.0 est ramené à 0.0 comme précédemment. Les méthodes publiques acceptant une phase quelconque continuent à la normaliser.
2. La variante `prepareSpectralNear` conserve le dernier niveau spectral et recherche localement les frontières. Son résultat est exactement égal à `prepareSpectral` sur les tests, mais l'essai du build n°142 a montré une forte régression CPU pour les modulations LFO/FM (surcoût de stockage et de contrôle par unité d'unisson). **Elle n'est donc pas activée dans le moteur**. Le chemin dynamique garde la préparation spectrale de référence, avec l'optimisation de phase normalisée.
3. La fréquence, le désaccordage, la phase et la forme restent propres aux 16 oscillateurs, avec toutes les interpolations temporelles, entre formes et entre niveaux toujours actives.

## Mesures et SIMD

- Test de 50 000 fréquences, phases et positions plus toutes les transitions des 11 niveaux et 11 points de départ : parité **exacte** à la fois des sélections spectrales et des samples normalisés.
- Étude NEON/SSE2 isolée du moteur : 4 phases différentes, chargements dispersés scalaires, interpolation vectorisée, comparaison de sortie à 2e-6, puis microbenchmark du coût total. SIMD seulement évalué, non activé dans le moteur.
- Benchmark A/B alterné référence/candidate/candidate/référence face au commit v1.2.9 `5970109079840506cfc308a3828cfd25e3c4f02a`, sur un même runner par plateforme. 18 scénarios dont 32 notes x16 à 48/96 kHz, buffers 64/128/256, PITCH/MORPH, FM, LFO, séquenceur et phase WIDTH.
- Comparaison de **tous** les échantillons stéréo ; seuils de régression 0.0003 en crête absolue et 0.001 en RMS relatif. Les tests existants contrôlent aussi les 27 cas HD et l'absence d'allocations C++ ordinaires dans le moteur.
- Les rapports incluent CPU moyen, percentile 99, délai des blocs et temps du prototype SIMD. Les chiffres des runners GitHub ne constituent pas une qualification temps réel sur l'ordinateur du musicien.

Ne pas considérer des gains CPU comme acquis tant que les deux rapports complets n'ont pas été lus. Les artefacts doivent être téléchargeables après double build vert.

## Décision après première mesure (build n°142)

Les 18 comparaisons audio ont donné un écart nul sur les deux plateformes. Mais la stratégie de mip dynamique avec cache s'est révélée trop coûteuse : jusqu'à +60,81 % sous LFO 96 kHz sur macOS et +40,34 % sur Windows. L'optimisation est donc retirée du moteur avant livraison finale, sans réduire la fidélité.

**SIMD prototype isolé :** le microbenchmark du premier essai donne ARM NEON 6,08 ms contre 13,59 ms en scalaire (rapport 0,448), tandis que Windows SSE2 donne 23,33 ms contre 12,41 ms en scalaire (rapport 1,880). Cette différence signifie qu'un déploiement SIMD indistinct sur les deux plateformes serait imprudent ; aucune voie SIMD n'est activée dans le moteur de production. Ces nombres caractérisent uniquement le microbenchmark, pas une accélération globale de la synthèse.

Le build de révision est nécessaire pour valider la sélection spectrale de référence et mesurer les performances finales à unisson ×16.
