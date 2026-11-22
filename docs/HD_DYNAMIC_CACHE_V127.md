# Serenity WAVETABLE v1.2.7 — caches spectraux conditionnels

## Objectif
Réduire le coût des notes modulées à 32 voix et quatre générateurs (unisson ×16), sans diminuer la polyphonie, la résolution ou la qualité sonore.

## Modifications
- Avant de préparer les plans fréquentiels, inspecter les routes actives vers la hauteur et les éventuels changements de note du séquenceur. Si le bloc comporte une route pitch ou si un pas actif du séquenceur peut transposer, ne pas construire de cache de hauteur pour les voix.
- Examiner le mode FM fréquentielle par oscillateur : FM BA / CA concerne le générateur A, FM CB le générateur B ; les oscillateurs C et D sans modulation de fréquence peuvent conserver leur pré-calcul.
- Ne pas préparer un plan pour un générateur silencieux.
- Préserver le chemin dynamique de v1.2.6 : fréquence, incrément de phase et interpolation spectrale restent calculés par échantillon lorsque nécessaire.
- La décision est prise une fois par bloc, sans allouer de mémoire et sans modifier le signal PCM ni les données des tables HD.

## Tests
- Régressions DSP ciblées : activation/désactivation de routes de pitch, morphing seul, séquenceur désactivé ou avec/sans transposition, FM en mode phase et en mode fréquence, destinations de modulation par oscillateur.
- Bilan A/B sur même runner contre le commit v1.2.6 `56ab424460f3b46e29eff885a15fcef820b09302`, en alternance référence/candidate/candidate/référence.
- Neuf configurations, dont 32 notes ×16 à 48 et 96 kHz, 32 notes ×16 avec FM fréquentielle, modulation matricielle PITCH/MORPH et transposition du séquenceur.
- Comparaison sample par sample des sorties stéréo, mesure des temps moyens et du percentile 99. Échec du build si la dérive sonore dépasse 0,0003 en amplitude absolue ou 0,001 en RMS relative.
- Confirmation des 27 scénarios fonctionnels, absence d'allocations C++ ordinaires dans le chemin audio, stabilité à quatre tables HD maximales.
- Les chiffres CPU des runners GitHub ne remplacent pas la validation sur interface audio et DAW réels.

## Conditions de livraison
Les deux builds macOS ARM64 (AU/VST3/Standalone) et Windows x64 (VST3/Standalone) et les comparaisons de fidélité doivent être verts avant d'annoncer la v1.2.7 comme validée. Les gains CPU sont rapportés tels que mesurés, y compris d'éventuelles régressions.
