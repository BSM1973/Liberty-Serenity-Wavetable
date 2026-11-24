# SERENITY WAVETABLE v1.2.10 — lecture HD et faisabilité SIMD

## Objectifs sans compromis

32 notes, 4 générateurs, unisson x16, 256 formes par banque, 2048 échantillons par forme, 11 niveaux spectraux, 100 presets, formats AU/VST3/Standalone. Aucun changement de configuration, d'interface, de qualité ou de sérialisation.

## Chemin audio optimisé

1. `oscillatorWarpPhase` produit déjà une phase en [0, 1] ; utiliser directement cette phase dans `HdTable::samplePreparedSpectralWrapped`. Le cas limite où l'arrondi donne exactement 1.0 est ramené à 0.0 comme précédemment. Les méthodes publiques acceptant une phase quelconque continuent à la normaliser.
2. Pour les voies de hauteur dynamique, conserver seulement l'indice du niveau spectral précédent. `prepareSpectralNear` compare les frontières ascendantes/descendantes à partir de ce niveau. Il produit exactement le même indice et le même facteur `smoothstep` que `prepareSpectral`, y compris aux frontières.
3. La fréquence, le désaccordage, la phase et la forme restent propres aux 16 oscillateurs, avec toutes les interpolations temporelles, entre formes et entre niveaux toujours actives.

## Mesures et SIMD

- Test de 50 000 fréquences, phases et positions plus toutes les transitions des 11 niveaux et 11 points de départ : parité **exacte** à la fois des sélections spectrales et des samples normalisés.
- Étude NEON/SSE2 isolée du moteur : 4 phases différentes, chargements dispersés scalaires, interpolation vectorisée, comparaison de sortie à 2e-6, puis microbenchmark du coût total. SIMD seulement évalué, non activé dans le moteur.
- Benchmark A/B alterné référence/candidate/candidate/référence face au commit v1.2.9 `5970109079840506cfc308a3828cfd25e3c4f02a`, sur un même runner par plateforme. 18 scénarios dont 32 notes x16 à 48/96 kHz, buffers 64/128/256, PITCH/MORPH, FM, LFO, séquenceur et phase WIDTH.
- Comparaison de **tous** les échantillons stéréo ; seuils de régression 0.0003 en crête absolue et 0.001 en RMS relatif. Les tests existants contrôlent aussi les 27 cas HD et l'absence d'allocations C++ ordinaires dans le moteur.
- Les rapports incluent CPU moyen, percentile 99, délai des blocs et temps du prototype SIMD. Les chiffres des runners GitHub ne constituent pas une qualification temps réel sur l'ordinateur du musicien.

Ne pas considérer des gains CPU comme acquis tant que les deux rapports complets n'ont pas été lus. Les artefacts doivent être téléchargeables après double build vert.
