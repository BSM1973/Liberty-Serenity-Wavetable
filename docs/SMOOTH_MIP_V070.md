# Serenity WAVETABLE v0.7.0 — Smooth spectral transitions

## Respect des écrans d’origine
- Les dix fichiers PNG du dossier `Assets/UI` restent strictement identiques.
- Pas de modification des libellés, positions, formes d'onde visibles ni des onglets.
- Le code demeure en C++17 / JUCE, sans PluginMaker.ai.

## DSP livré dans cette étape
- Transitions entre les 9 niveaux spectraux anti-aliasing des wavetables personnalisées.
- Fondu préventif entre les deux niveaux adjacents dans les 20 % de fréquence situés **avant** chaque seuil harmonique.
- Interpolation smoothstep (pente nulle au départ et à l'arrivée) pour éviter l'ancienne commutation brusque.
- Les deux tables partagent la même référence de phase FFT, avec interpolation identique entre les 8 frames pour les quatre oscillateurs.
- La table la plus riche est entièrement éteinte avant le franchissement du seuil de sécurité 0,48 × fréquence d'échantillonnage.
- Correction de la comparaison de fréquence au seuil pour éviter les erreurs d'arrondi flottant.
- Aucune allocation, serrure, FFT ni création de ressources dans `render()`.

## Tests automatisés
- Continuité des échantillons de part et d'autre de chaque seuil : 8 transitions × 64 phases × 5 fréquences d'échantillonnage.
- Fondu progressif et monotone ; aucune contribution de la table brillante au-delà de son seuil.
- Vérification spectrale que la 90e harmonique est supprimée dans la plage qui la ferait se replier.
- Interpolation des frames et contrôle des hauteurs MIDI 24 à 127.
- Taux contrôlés : 44,1 / 48 / 88,2 / 96 / 192 kHz.
- Reprise de tous les tests précédents, dont démarrage de l'éditeur JUCE et navigation des dix onglets, sur macOS ARM64 et Windows x64.
- Assertions des tests DSP maintenues actives dans les builds Release.

## Limites explicites
- Ce traitement concerne les wavetables personnalisées. Les autres moteurs DSP et les 30 effets restent à finaliser.
- Le fondu entre mipmaps ne remplace pas un suréchantillonnage 2×/4×/8×.
- Les changements du contenu des tables pendant une note ne bénéficient pas encore d'un fondu entre anciennes et nouvelles tables.
- Un test automatisé ne prouve pas un test d'écoute A/B sur ordinateur réel.
- Les artefacts de développement AU, VST3 et Standalone ne sont pas encore signés ou notarisés.
