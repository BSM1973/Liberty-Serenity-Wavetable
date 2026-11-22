# Serenity WAVETABLE 1.2.6 — planification fréquentielle HD

Cette livraison optimise uniquement la lecture haute polyphonie des tables d'ondes et les calculs de niveau spectral.

**Aucune réduction** des 32 voix MIDI, du nombre de voix d'unisson (×16 sur chacun des 4 oscillateurs), de la résolution 2048 et des 256 formes disponibles par table, ni des 11 niveaux anti-aliasing.

## Principe

À l'entrée d'un bloc audio, pour chaque voix active, chaque oscillateur et chaque unité d'unisson :
- Préparer sa fréquence exacte selon le calcul de hauteur de la v1.2.5.
- Préparer son incrément de phase.
- Déterminer son niveau spectral et le facteur d'interpolation vers le niveau adjacent à partir de cette fréquence.
- Stocker les plans dans une mémoire appartenant au moteur ; aucune allocation dynamique dans render().

Le chemin pré-calculé est utilisé uniquement lorsque la modulation de hauteur est nulle, que le séquenceur ne transpose pas la note et que le mode FM fréquentielle n'ajoute pas de modulation. Tous les autres cas conservent les calculs de fréquence et de bande à chaque échantillon. Les transitions de morphing et la position 3D des formes restent indépendantes du cache de fréquence.

## Exigences de test

- 30 000 comparaisons numériques déterministes entre `HdTable::samplePrepared` et `HdTable::samplePreparedSpectral` avec des fréquences, phases, positions et fréquences d'échantillonnage variables ; différence maximale 1e−7.
- Benchmark C++ compilé contre la v1.2.5 (SHA 1721a7506adc1028eb3a816a59f431bb72c86287) et la version courante sur **le même runner**, à 32 notes, 4 oscillateurs, 48 kHz ou 96 kHz, unisson ×4 / ×16 et deux filtres actifs.
- Deux tests supplémentaires déclenchent respectivement la FM de fréquence et la modulation matricielle de hauteur / morphing ; l'optimisation ne doit pas changer leur son.
- Échantillons stéréo A/B comparés avec tolérance absolue maximale 0,0003 et erreur RMS relative maximale 0,001.
- CI rouge si la fidélité régresse ; les rapports de performance et les tests DSP doivent être disponibles en artefacts.
- Les mesures sur runners hébergés ne permettent pas de certifier l'absence de décrochages sur l'ordinateur personnel ; essais DAW et écoute musicale nécessaires.
