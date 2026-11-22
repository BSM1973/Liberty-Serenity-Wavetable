# Serenity WAVETABLE — Liberty Music Software

Version 1.2.7 — WAVETABLE HD ; import WAV/AIFF à taille de cycle explicite, basé sur la version 1.0.0 (build 126 réussi). Le contrôle utilisateur de la version précédente est de 10/11 critères ; le dernier critère non validé, non identifié, est reporté à la demande du propriétaire. La compilation et les tests de cette nouvelle version doivent être validés séparément.

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

## Optimisation v1.2.2

Les mesures HD QC du build n°130 montrent des dépassements du budget audio à forte polyphonie, surtout avec beaucoup de voix d'unisson. v1.2.2 optimise les recherches d'échantillons HD, supprime des opérations transcendantes répétées dans le bouclage d'unisson et conserve les paramètres, données PCM, niveaux spectraux et algorithmes de filtrage. Les benchmarks du build n°131 devront confirmer le gain ou révéler des régressions. Les timings des runners GitHub ne garantissent pas la tenue des deadlines sur les machines des utilisateurs.

## Optimisation v1.2.4 — coefficients des filtres

Lorsque le cutoff du filtre 1 est partagé entre les voix, le coefficient de filtre TPT est calculé une seule fois par échantillon, au lieu d'être recalculé par voix et par canal. Pour le filtre 2, si le cutoff ne dépend pas des voix, les coefficients sont calculés une seule fois par bloc. Dès qu'un suivi de note, une enveloppe ou la vélocité modifie un filtre, son coefficient individuel reste calculé à chaque voix (mais est partagé entre ses canaux stéréo). La résonance est constante dans un bloc audio et ne nécessite pas de recalcul par voix.

Le traitement de saturation, les états de filtre, les quinze types, les réponses à la modulation, l'architecture HD, les 32 notes et les seize voix d'unisson restent inchangés. Le test DSP compare les quinze types de filtres avec l'ancienne implémentation, huit configurations de saturation, résonance et fréquence, sur 2 048 échantillons chacune. Les performances sont relevées dans les CSV habituels, avec des configurations supplémentaires à deux filtres actifs.

## Performance haute polyphonie v1.2.5

À 32 notes, quatre oscillateurs et unisson 4× ou 16×, le moteur met en cache les coefficients de désaccordage et de panoramique par bloc, mutualise le facteur de hauteur MIDI par oscillateur et par échantillon quand la modulation de hauteur individuelle est nulle, calcule les incréments des huit LFO par bloc et partage la sélection de formes HD entre les voix lorsque leur position n'est pas modulée individuellement. Les chemins avec modulation individuelle restent inchangés. Aucun niveau spectral, forme, preset, polyphonie, modulation ou fréquence d'échantillonnage n'a été retiré.

Le workflow construit les deux mêmes benchmarks C++ contre le DSP de référence v1.2.4 (commit épinglé) et le DSP actuel, puis les exécute en alternance sur le même runner macOS/Windows. Il publie les temps moyens, les percentiles 99 et les différences d'échantillons stéréo de sortie. Il refuse une dérive de fidélité supérieure aux seuils fixés. Ces comparaisons réduisent l'effet des différences entre runners, sans garantir le temps réel sur ordinateur personnel.

## Optimisation HD v1.2.6 : fréquence et bande spectrale préparées par voix

Lorsque la hauteur d'une note reste stable, la fréquence des 1 à 16 oscillateurs d'unisson, leur incrément de phase et leur niveau de filtrage anti-aliasing sont préparés une fois au début du bloc audio pour chaque voix et générateur. Les données préparées résident dans le moteur, pas sur la pile du callback audio. Les 256 formes, les 2048 échantillons, les 11 niveaux spectraux et les quatre oscillateurs restent inchangés.

En cas de pitch bend par modulation, transposition du séquenceur, ou FM de fréquence non nulle, le moteur utilise immédiatement l'ancien chemin dynamique, qui recalcule les fréquences et le niveau spectral à chaque échantillon. Les modifications de morphing des tables et les 15 modes de filtre ne sont pas modifiés.

Le nouveau banc A/B compare la v1.2.6 à la v1.2.5 sur chaque runner macOS/Windows, avec des scénarios supplémentaires de FM dynamique et de modulation matricielle. Il vérifie la précision des échantillons audio, les temps moyens, les pics p99 et la polyphonie. Aucun gain n'est considéré comme acquis avant lecture des rapports.

## Optimisation v1.2.7 — supprimer les caches inutiles

Le moteur v1.2.6 préparait les fréquences et niveaux spectraux HD pour les notes actives même lorsque la modulation de hauteur ou la transposition du séquenceur empêchaient ensuite leur utilisation. La v1.2.7 calcule l'éligibilité au cache une seule fois par bloc, avant de parcourir les 32 notes. L'activation d'une route de matrice vers PITCH ou la présence d'une transposition de séquence désactive la préparation ; en FM fréquentielle, seuls les oscillateurs effectivement modulés sont exclus du cache.

Les traitements à hauteur variable continuent à calculer les fréquences et les niveaux spectraux à chaque échantillon. La fidélité du moteur, les 100 presets, les 32 notes, les quatre oscillateurs, l'unisson ×16, les 256 formes, les 2 048 échantillons et l'interface ne sont pas modifiés.

Le benchmark A/B épinglé à v1.2.6 comprend désormais des charges supplémentaires à unisson ×16 avec FM, matrice PITCH/MORPH et transposition du séquenceur. Le build échoue si les comparaisons d'échantillons sortent de la tolérance de qualité ; les rapports CPU sont publiés indépendamment des variations des runners GitHub.

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
