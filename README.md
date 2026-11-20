# Serenity WAVETABLE — Liberty Music Software

**État : code source de développement 0.1 (non commercial, non validé en DAW).**

Projet natif **C++17 / JUCE 8.0.10**, conçu pour macOS AU/VST3/Standalone et Windows VST3/Standalone.
Les dix captures fournies constituent la **référence graphique de départ** : mêmes proportions, mêmes panneaux, même palette, mêmes dix onglets. Le code embarque ces captures telles quelles comme calques de référence et superpose les premières commandes réelles. Il ne s'agit **pas encore** d'une interface entièrement redessinée en composants natifs. Certaines commandes visibles sur les captures n'ont pas encore de fonction.

## Déjà dans le code

- Moteur DSP compilé et testé sans JUCE : quatre générateurs de formes d'ondes à tables interpolées, 16 familles × 8 frames, mipmaps harmoniques pour limiter l'aliasing, unison réglable jusqu'à 16 oscillateurs par générateur, jusqu'à 32 notes, stéréo.
- Enveloppe d'amplitude ADSR et six enveloppes de modulation simples. Une base de huit sorties LFO est disponible, sans éditeur graphique complet.
- Deux étages de filtrage passe-bas, routage série/parallèle, réglages de coupure, mélange et activation du filtre 2. Les 15 types annoncés dans le cahier des charges ne sont **pas** implémentés.
- Matrice 64 routages réellement prise en compte dans le DSP pour hauteur, position de table, cutoff, gain et panoramique. Éditeur sur trois pages de 24 emplacements.
- Saturation douce, délai stéréo simple et réverbération algorithmique simple. **Pas 30 effets livrés**.
- 100 **définitions de factory presets** (12+15+10+12+10+8+10+8+8+7), distinctes et générées de manière déterministe. Elles ne sont pas encore individuellement écoutées, équilibrées ni validées comme presets commerciaux.
- Navigateur de presets : catégories, recherche, sélection, favoris. Export/import de l'état du plugin dans un fichier `.serenitypreset`.
- Commandes natives interactives initiales : navigation des dix onglets, oscillateurs A/B/C/D marche/arrêt, choix de formes d'ondes, position de table, knobs principaux, filtres et macros partiellement reliées au DSP. Glisser vertical, molette, Maj+glisser fin, double-clic réinitialisation.
- FR/EN (FR par défaut), navigation et panneaux principaux traduits. **La traduction intégrale de tous les libellés incrustés dans les dix captures reste à réaliser en composants natifs.**
- MIDI Learn sur les paramètres déclarés : clic droit sur un paramètre pris en charge → « Apprentissage MIDI CC » → mouvement de contrôleur CC. Mapping sauvegardé dans l'état de session.
- Restauration via APVTS : paramètres, preset courant, FR/EN, favoris, matrice et mappings MIDI.
- Test compilé sous Linux pour le cœur DSP (sans plugin host) et workflow GitHub Actions prêt à tester les formats.

## Compiler

Le dépôt doit être accessible à JUCE 8.0.10 par Internet, ou disposer d'une copie locale de JUCE.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

Avec un JUCE local :

```bash
cmake -S . -B build -DJUCE_SOURCE_DIR=/chemin/vers/JUCE
```

Pour les tests DSP sans JUCE (vérifiés ici) :

```bash
cmake -S . -B build-dsp -DSERENITY_BUILD_PLUGIN=OFF -DBUILD_TESTING=ON
cmake --build build-dsp
ctest --test-dir build-dsp --output-on-failure
```

## Règles de développement

1. Ne jamais remplacer les captures par un autre design sans accord. Migrer progressivement vers des éléments natifs reproduisant ces écrans.
2. Aucun chevauchement, aucun glyphe corrompu, aucune zone tronquée, textes FR accentués et anglais vérifiés.
3. Ne jamais annoncer « 30 effets », « éditeur WAV », « 64 pas » ou « 100 presets finalisés » tant que chaque fonction n'est pas réellement câblée, testée et validée à l'écoute.
4. Aucun traitement bloquant ni allocation dynamique volontaire dans `Engine::render()`.
5. Pas d'installateur commercial, signature Apple ou notarisation fournis dans cette version.

Voir [`docs/ETAT_ET_SUITE.md`](docs/ETAT_ET_SUITE.md) pour le détail des modules encore nécessaires.
