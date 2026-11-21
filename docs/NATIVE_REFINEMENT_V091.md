# Serenity WAVETABLE v0.9.1 — Rendu natif unique

Correctif de la v0.9.0 après inspection des captures produites par le build macOS.

La version v0.8.2 utilisait encore un second rendu, `paintEditor()`, `paintCustomWaveform()` et `paintCustomHarmonics()`, superposé au nouveau moteur graphique lorsque la table Custom était sélectionnée.

Ces trois chemins de dessin ont été supprimés (déclarations et implémentations). Le seul dessin 3D, l'oscilloscope 2D et le graphique harmonique sont maintenant ceux du moteur natif `SerenityNativeUI.cpp`. Il échantillonne les données PCM, y compris les nouvelles transformations, sans aucune superposition.

Ce correctif est strictement graphique; les paramètres audio, la sauvegarde et la navigation restent identiques à v0.9.0. Les tests CI génèrent des captures de contrôle mais ces PNG ne font pas partie du plugin.

Critères : aucun doublon de courbes 3D, pas de curseur incrusté, pas d'image fixe, AU/VST3/Standalone et tests verts avant livraison.
