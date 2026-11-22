# Serenity WAVETABLE v1.2.5 — A/B CPU control à haute polyphonie

## Objectifs

- 32 notes MIDI, quatre oscillateurs, unisson 4× et 16×, aucune baisse de résolution : tables 256 × 2048, 11 niveaux spectraux, 100 presets et identifiants d'automatisation intacts.
- Réduire le nombre de fonctions transcendantes dans la lecture d'un ensemble massif de voix sans modifier les filtres, les enveloppes, les effets ni la modulation.

## Optimisations

- Facteurs de désaccordage des oscillateurs, gains de panorama et gains d'unisson préparés une fois par bloc.
- Facteur exponentiel de la hauteur des oscillateurs partagé entre voix lorsque la modulation de hauteur propre à une voix est nulle. Les modulations par voix utilisent toujours le calcul d'origine.
- Position et pointeurs des images HD préparés une fois par bloc pour les voix dont la position de table d'ondes n'est pas modulée. Les autres voix conservent leur interpolation individuelle.
- Fréquence et lissage des huit LFO calculés une fois par bloc. Les phases et la sortie des LFO continuent d'évoluer à chaque échantillon.

## Validation A/B sur le même runner

Chaque runner macOS ARM64 et Windows x64 construit le benchmark indépendant contre le code v1.2.4 épinglé par commit et la version candidate, puis lance la séquence référence–candidate–candidate–référence.

Scénarios : 32 notes, quatre oscillateurs, 48 kHz / buffer 256 / unisson 4, 48 kHz / buffer 128 / unisson 16, 96 kHz / buffer 256 / unisson 16, et deux filtres actifs à 48 kHz / buffer 256 / unisson 4.

Le programme compare les échantillons stéréo pour une excitation déterministe, mesure les durées moyennes et les p99, puis publie `hd-ab-summary.md`, `hd-ab-summary.csv` et les essais bruts. Seuils de régression sonore : écart absolu ≤0,0003 et erreur RMS relative ≤0,001. Une régression numérique échoue au build. Les pics et délais sur runners hébergés ne certifient pas la tenue en DAW réel.

Un gain relatif mesuré sous forte charge est attendu mais **ne doit jamais être annoncé avant que les rapports soient lus**. Les sessions longues sur interface matérielle et écoute A/B restent obligatoires.
