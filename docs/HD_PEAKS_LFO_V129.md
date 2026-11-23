# SERENITY WAVETABLE v1.2.9 — AUDIO REALTIME PEAK CONTROL

## Invariants

32 notes simultanées, quatre oscillateurs, 16 unités d'unisson par oscillateur, quatre tables HD à 256 formes de 2048 échantillons, 11 niveaux spectraux, 100 presets. Aucun changement d'interface, de paramètres hôte, de formats AU/VST3/Standalone ni de compatibilité de projet.

## Changements testés

1. Scanner les 64 routes de modulation une fois par bloc, construire un tableau préalloué de pointeurs vers les routes valides, et conserver leur ordre source. Au lieu de répéter 64 tests conditionnels par note et par échantillon, itérer uniquement sur les routes actives.
2. Pour une modulation de pitch issue uniquement des LFO sans fade et éventuellement de la molette, partager les calculs exponentiels de hauteur entre les notes, mais pas l'état ni le résultat d'une modulation par voix. Avec fades, enveloppes, suivi de clavier ou vélocité, préserver l'ancienne voie individuelle.
3. Pré-calculer les durées double précision du séquenceur pour les pas pairs/impairs lors de `prepare` et `setPattern` ; la formule et la gestion du swing restent inchangées.
4. **Correction après mesures du build n°140 :** le plafonnement à seize groupes note/oscillateur par bloc ralentissait le séquenceur sous unisson ×16 (macOS jusqu'à +39 % en temps moyen, Windows +6 à +7 %). Ce plafonnement est retiré : tous les plans invalidés sont construits dès le début du bloc, puis réutilisés dans le chemin rapide. L'optimisation ultérieure des pics doit se concentrer sur le coût propre des voix et non sur une limitation artificielle de la préparation.
5. Garder toutes les phases, formes d'onde, niveaux d'unisson et routages de modulation à leur précision d'origine.

## Tests de non-régression

- Test `serenity_sequence_tests` compare 162 000 appels au nouveau séquenceur et à un modèle indépendant du moteur v1.2.8, sur trois fréquences d'échantillonnage, six configurations de tempo, de division, de gate et de swing.
- 15 scénarios A/B compilés contre v1.2.8 commit `5b27c97b48f005bfd7f6eae79347d30e4352f575`, exécutés sur le même runner GitHub en alternance référence-candidate-candidate-référence.
- Les 15 scénarios comprennent x16 48 kHz / buffer 64, séquenceur transposé normal et rapide, FM dynamique, pitch/morph, modulations LFO à fade, deux LFO de pitch, clavier et 96 kHz.
- Comparaison intégrale de tous les échantillons audio stéréo : maximum absolu <= 0,0003 ; écart RMS relatif <= 0,001. Build rouge si une régression sonore échoue.
- Conserver les contrôles HD habituels (27 scénarios, allocations ordinaires vérifiées, 4 tables complètes), mesurer CPU moyen, p99, pics et dépassements.
- Aucun résultat sur runner hébergé ne prouve la tenue audio sur ordinateur personnel ; les essais sur DAW et interface audio restent une condition distincte.

La validation est limitée aux builds et aux mesures réellement passées, sans déclaration automatique de temps réel atteint.

## Révision de validation

Le premier essai v1.2.9 (build n°140) a réussi les contrôles de fidélité mais régressait sur les séquences à ×16. Une correction ciblée retire le plafonnement des caches avant la publication finale. Les chiffres ci-dessus concernent le premier essai et ne doivent pas être utilisés pour annoncer les performances de la révision. L'ensemble des builds macOS/Windows et le benchmark A/B doivent être relancés et lus de nouveau.
