# Serenity WAVETABLE — Liberty Music Software

Version 1.2.1 — WAVETABLE HD ; import WAV/AIFF à taille de cycle explicite, basé sur la version 1.0.0 (build 126 réussi). Le contrôle utilisateur de la version précédente est de 10/11 critères ; le dernier critère non validé, non identifié, est reporté à la demande du propriétaire. La compilation et les tests de cette nouvelle version doivent être validés séparément.

## Plateformes et formats

- macOS Apple Silicon : AU, VST3, Standalone.
- Windows x64 : VST3, Standalone.
- C++17, JUCE 8.0.10.

## Fonctions présentes

- Quatre générateurs, tables d'ondes interpolées, 16 familles de tables d'usine, édition de tables personnalisées, unison, warp, shape, FM et modulation en anneau.
- Deux filtres à variables d'état, 15 modes chacun, routage série/parallèle.
- Six enveloppes DAHDSR avec boucle et courbe, huit LFO avec synchronisation et fondu.
- Matrice de modulation de 64 emplacements, macros, arpégiateur et séquenceur jusqu'à 64 pas.
- Effets : saturation, EQ, compression, chorus/flanger/phaser, délai, réverbération stéréo FDN et traitements de modulation.
- 100 presets d'usine, navigateur, recherche, favoris, import/export de l'état.
- Interface native JUCE à dix pages, français/anglais, ajustement proportionnel à l'écran, sans ascenseur. Les dix images de référence sont conservées dans Assets/UI et ne servent pas de fonds statiques.
- MIDI Learn sur paramètres pris en charge, sauvegarde des paramètres et des mappings.

## Importer ses tables d'ondes

Dans WAVETABLE EDITOR, sélectionner A, B, C ou D, puis IMPORTER WAV / AIFF. Choisir le fichier, ou le déposer directement sur la page. Le menu affiche le nombre de formes et les tailles de cycle compatibles avec sa longueur. Choisir un cycle unique ou 256/512/1024/2048/4096 échantillons par cycle selon la source. Les fichiers mono/stéréo conservent de 1 à 256 formes, chacune à 2048 échantillons ; les cycles de taille différente sont rééchantillonnés. Morphing continu, 11 niveaux anti-aliasing calculés hors callback. Les tables HD sont sauvegardées dans le projet et exportables en WAV flottant 32 bits. Les outils d'édition 512 points ne sont pas appliqués directement aux tables HD : conversion explicite et annulable vers huit formes via le menu contextuel. Les anciennes tables et presets restent au format historique. La table personnalisée est activée et enregistrée dans l'état du plugin. TABLE PERSONNELLE permet de revenir à la table importée après sélection d'une table d'usine. Limite HD : 256 formes, cycle source jusqu'à 4096 échantillons, longueur multiple de la taille de cycle ; pas de détection automatique de hauteur ni d'import de formats propriétaires.

## Validation et distribution

Les tests automatiques DSP et UI sont exécutés sur les deux plateformes. Les archives de plugins et les captures natives sont publiées par GitHub Actions. Le build vert ne constitue pas une certification de fidélité pixel par pixel, d'absence de tout défaut graphique ou de compatibilité exhaustive avec tous les DAW. La validation auditive individuelle des presets et la signature/notarisation commerciale ne sont pas attestées. Les archives actuelles sont des binaires non signés, pas des installateurs commerciaux.

## Compiler

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

Pour JUCE local : ajouter `-DJUCE_SOURCE_DIR=/chemin/vers/JUCE` à la configuration. Pour les tests DSP seuls : `-DSERENITY_BUILD_PLUGIN=OFF`.

## Règles permanentes

Zéro chevauchement, aucun caractère corrompu, accents français corrects, aucune commande trompeuse, fenêtre complète sans ascenseur, fidélité aux références, aucune régression sonore. Artefacts téléchargeables après validation des builds.

Voir [l'historique de validation visuelle](docs/VISUAL_ACCEPTANCE_V0132.md).
