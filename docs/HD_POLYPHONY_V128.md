# SERENITY WAVETABLE v1.2.8 — PITCH/MORPH, séquenceur et haute polyphonie

## Objectifs intangibles

32 notes, 4 oscillateurs simultanés, jusqu'à 16 oscillateurs d'unisson par générateur, 256 trames HD de 2 048 échantillons, 11 niveaux spectraux, 100 presets. Aucun changement visuel, de paramètres hôte ou de sérialisation.

## Correctifs de traitement

1. Préparation des fréquences HD persistante d'un bloc à l'autre avec clé d'invalidation : identité/âge de la note, hauteur d'oscillateur, désaccordage, modulation statique, fréquence d'échantillonnage, nombre d'oscillateurs d'unisson, pas de séquence et présence d'une table HD. Si ces valeurs sont identiques, les 16 plans sont réutilisés au lieu d'être reconstruits.
2. Les sources de matrice de modulation VÉLOCITÉ, NOTE et MOLETTE sont constantes pour la voix pendant un bloc : leur offset de hauteur est préparé sans altération de l'ordre de calcul de l'ancienne matrice.
3. Les sources d'enveloppe et LFO vers PITCH interdisent tout cache spectral de hauteur ; elles conservent la fréquence et la sélection de niveaux anti-aliasing calculées à chaque échantillon.
4. Les pas transposés du séquenceur sont autorisés à utiliser un plan préparé pour leur hauteur courante ; une transition survenant au milieu du bloc active immédiatement le chemin dynamique jusqu'au prochain bloc (aucun pic de recalcul synchrone).
5. Le facteur `commonPitch` de hauteur du séquenceur est recalculé uniquement lorsque la note du pas change, pas à chaque échantillon.
6. Le mode FM fréquentielle conserve un chemin dynamique pour les oscillateurs effectivement concernés, et les autres oscillateurs restent éligibles au cache.

## Validation et limites

- Comparaisons A/B exécutées sur le même runner contre la v1.2.7, SHA `ad9a0dc6a130106ef507daa4cbef26821479db71`.
- Onze scénarios à 32 notes et quatre oscillateurs, dont unisson ×4/×16, 48/96 kHz, deux filtres, FM, PITCH/MORPH, séquenceur transposé, LFO de hauteur et vélocité/suivi de clavier.
- Comparaison sur **tous les blocs audio mesurés** : dérive autorisée ≤0,0003 en amplitude absolue et ≤0,001 en RMS relative. Erreur au-delà des limites = build rouge.
- CTest et rapports CPU/percentiles doivent être verts sur macOS ARM64 et Windows x64 avant de déclarer les artefacts livrés.
- Aucun résultat GitHub Actions ne certifie à lui seul l'absence de décrochages audio sur matériel réel : il faut poursuivre les validations en DAW.

Les éventuelles régressions de CPU doivent être rapportées même lorsque les builds sont verts.
