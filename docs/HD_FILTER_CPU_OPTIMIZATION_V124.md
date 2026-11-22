# Serenity WAVETABLE v1.2.4 — optimisation des coefficients de filtres

Objectif : réduire le coût CPU des 32 notes / 4 oscillateurs / unisson ×4 et ×16 **sans réduire la polyphonie, la fidélité ou les paramètres automatiques**.

## Changements DSP

1. `FilterChannel::runPrepared` reçoit le coefficient TPT de coupure `g` et de résonance `k`. Le traitement audio est identique à `run`; les deux voies stéréo utilisent la même paire `g,k`, mais leurs états SVF et leur signal restent indépendants.
2. La résonance des filtres 1 et 2 ne varie pas à l'intérieur d'un bloc et `k` est calculé une fois par bloc.
3. Filtre 1 : si le cutoff n'est pas lié à la note, l'enveloppe ou la vélocité, et sans contribution de modulation matricielle vers ce cutoff, calculer `tan` une seule fois par échantillon pour toutes les voix. Dès qu'une modulation individuelle intervient, calculer `g` pour chaque voix.
4. Filtre 2 : si aucune modulation note/enveloppe/vélocité n'intervient, `g` est calculé une seule fois par bloc ; sinon, calcul individuel par voix. Pas de partage abusif entre les notes.
5. Aucune modification des fréquences de coupure, des 15 modes de filtres, du gain, de la saturation, du routage série/parallèle ni des coefficients spectraux HD.

## Tests et validation

- Test de parité de l'ancien `FilterChannel::run` contre `runPrepared` : 15 types × 8 configurations × 2 048 échantillons, erreur maximale admise 1e-6 ; les deux traitements possèdent des états SVF indépendants.
- Scénarios de performances hérités de v1.2.3 : 32 notes, quatre oscillateurs, unisson ×1, ×4, ×16 ; 44,1/48/96 kHz ; buffers 64/128/256/512 ; quatre tables HD 256 × 2 048.
- Quatre scénarios additionnels : deux filtres actifs, soit cutoff partagé soit modulation individuelle, à 32 notes × unisson ×4 et ×16.
- Comparez les colonnes mean_budget_pct et p99_budget_pct de v1.2.4 contre le build n°132 de v1.2.3 **sur la même plateforme**. L'ordonnancement des runners GitHub peut varier et brouiller les comparaisons.
- Les tests statiques et CI ne certifient ni l'absence de craquements sur machine cible ni la qualité subjective des presets : écoute A/B et sessions longues en DAW restent nécessaires.

Le code est gelé pour ce lot tant que la parité numérique, les CTest et les deux builds ne sont pas verts.
