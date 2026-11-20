# Serenity WAVETABLE — Séquenceur et arpégiateur (v0.3.1)

Cette étape conserve les dix captures d'interface originales, sans nouvelle conception graphique.

## Relié au moteur audio
- Quatre banques de 64 pas au maximum, pages de 16 pas dans l'interface
- Lecture des hauteurs en demi-tons relatifs à la note tenue
- Véritable piste de vélocité avec silence lorsque la valeur vaut zéro
- Modulation par pas de la fréquence de filtre
- Gate, swing et divisions synchronisables au tempo fourni par le DAW (120 BPM si indisponible)
- Réenclenchement des enveloppes sur les pas joués et lissage des ouvertures/fermetures
- Edition par clic/glissement sur les graphiques d'origine
- Sauvegarde et rappel des quatre banques de patterns dans l'état AU/VST3

## Validations automatiques
- Test du générateur de pas indépendant du DAW
- Vérification des 64 pas, notes et silences, swing, gate, mode OFF
- Lecture MIDI et audio dans le vrai processeur JUCE
- Sauvegarde/relecture de la banque 4 et du pas 64
- Démarrage et rendu des dix interfaces originales

## Encore en développement
- MIDI note output (selon hôte), export MIDI par glisser-déposer
- Sequencing polyphonique indépendant par note tenue
- Probability, ratcheting, slide, microtiming et chaînage avancé
- Edition avancée de wavetables et 30 effets visés par le cahier des charges
- Validation d'écoute individualisée des 100 presets factory

Tous les contrôles annoncés comme avancés dans les captures ne sont pas encore fonctionnels. Le build vert prouve la compilation et les tests spécifiés, pas la conformité commerciale complète.
