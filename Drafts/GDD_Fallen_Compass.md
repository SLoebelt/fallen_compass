# Game Design Document – _Arbeitstitel: „Fallen Compass – An Explorer’s Legacy“_

---

## 1. Vision & High-Level

### 1.1 Elevator Pitch

Du bist ein wenig respektierter Entdecker in einer fiktiven Welt am Übergang vom späten Mittelalter ins Zeitalter der großen Entdeckungen.
Mit begrenzten Mitteln stellst du Expeditionen zusammen, planst Routen und Ziele, führst eine kleine Gruppe von Spezialisten durch gefährliche, unerforschte Regionen – und versuchst, mit Ruhm, Funden und Erfolgen deinen Ruf und deine Karriere aufzubauen.

Jede Expedition ist riskant:

- Wer stirbt, bleibt tot.
- Stirbt dein Entdecker, ist deine **Karriere vorbei**.

---

### 1.2 Genre & Kernfantasie

- **Genre-Mix**

  - Singleplayer
  - Narrative Expedition-Sim
  - Light-Management & Rollenverteilung
  - Rundenbasierte Taktik-Kämpfe (selten, aber wichtig)
  - Exploration mit Fog of War

- **Kernfantasie**v

  - Du bist ein unterschätzter Entdecker, der:

    - Expeditionen plant wie ein Stratege,
    - unterwegs als Gruppenführer Verantwortung trägt,
    - mit menschlichen Schwächen und Stärken seiner Crew umgehen muss,
    - Stück für Stück unbekannte Regionen kartiert und dabei Geschichten schreibt.

---

### 1.3 Plattform, Engine, Steuerung

- **Plattform:** PC (Steam, später ggf. GOG/Epic)
- **Technik:** Umsetzung als Hybrid aus C++ und Blueprints.

C++ ist immer eine Option, wenn dadurch Gameplay-Systeme oder Performance-kritische Funktionen schneller, stabiler oder besser wartbar werden (z. B. Pathfinding, komplexe Simulation, speicherintensive Systeme).

Blueprints bleiben primär für Content-nahe Logik (UI, einfache Skripte, Events, Prototyping), aber es gibt kein striktes „Blueprint First“-Dogma. Stattdessen: pragmatischer Ansatz – „das, was technisch sinnvoll ist“.

- **Engine:** Projektstart auf Unreal Engine 5.7. Nutzung der UE5-Features (Lumen, Nanite, World Partition) nur dort, wo sie Performance und Workflow klar verbessern.

- **Steuerung:**

  - Overworld: 3D-Welt in Top-Down-Perspektive (Referenzen: Wartales, Project Zomboid). Konkretes Eingabeschema (Point & Click / WASD / Hybrid) ist noch in Evaluierung, siehe Designnotizen zur Overworld-Steuerung.

  - Lokale Szenen & Kämpfe: Darstellung top-down / isometrisch. Point & Click zur Steuerung der Figuren. WASD dient ausschließlich der begrenzten Kamerabewegung, um den Levelausschnitt besser einsehen zu können (Pan/leichter Scroll), nicht zur Figurensteuerung.

---

## 2. Design-Pfeiler

1. **Kleine Gruppe, große Bedeutung**
   Wenige Figuren (5–10), jede mit Persönlichkeit und echter Bedeutung.

2. **Planung & Verantwortung statt Dauer-Action**
   Langsamer, überlegter Spielfluss. Entscheidungen zählen mehr als Reflexe.

3. **Risikoreiche Expeditionen als „Runs“**
   Jede Expedition ist eine in sich geschlossene Reise – mit Hin- und Rückweg. Erfolg / Scheitern ist bewusst subjektiv und wird sowohl spielmechanisch als auch emotional bewertet.

   - Eine Expedition kann sich wie ein Erfolg anfühlen, wenn die gesamte Crew lebend zurückkehrt, auch wenn nur ein Teil der Neben­ziele erfüllt und nur wenige Kartenteile aufgedeckt wurden.

   - Eine Expedition, in der das Primärziel erreicht wird, aber erfahrene Crewmitglieder sterben, kann sich wie eine Niederlage anfühlen (Verlust von Expertise, Auswirkungen auf Moral/Stimmung, langfristige Konsequenzen im Meta-Spiel).

4. **Charakterbindung + Permadeath**
   Du hängst an deinen Leuten – und spürst Verluste.

5. **Wachsende Karriere in einer glaubwürdigen Welt**
   Späte Mittelalter-/Frühneuzeit-Vibes, technische Entwicklung, Geld & Ruf entscheiden, was möglich ist.

---

## 3. Spielstruktur & Loops

### 3.1 Oberer Meta-Loop (Karriere)

Wiederkehrender Ablauf:

1. **Planungsphase im Büro**

   - Expedition auswählen (Vertrag, Ziel, Auftraggeber).
   - Crew zusammenstellen.
   - Ausrüstung & Proviant bestimmen.
   - Startregion & grobe Route wählen. Die Routenplanung beginnt im Büro am Kartentisch.
     - Von hier aus wird die Route bis zum eigentlichen Expeditionsstartpunkt (z. B. Küste, befahrbarer Fluss, Rand einer Region) geplant.
     - Begrenzung durch Kartenwissen:
       - Vom Kartentisch aus können keine Details in unerkundeten Regionen geplant werden – diese Bereiche sind auf der Karte verborgen.
       - Erst wenn in einer Region bereits eine Expedition stattgefunden hat und Kartendaten vorliegen, können Route und Ressourcenaufwand tiefer in diese Region hinein geplant werden (z. B. bekannte Pässe, Dörfer, Ressourcen­punkte).

2. **Expedition (Hinweg)**

   - Reise von Startregion ins Landesinnere / zum Ziel-POI.
   - Unterwegs: Wanderung, Lagerphasen, Kämpfe, Events.

3. **Ziel-POI erreichen**

   - Hauptziel ausführen (z. B. Artefakt bergen, Gebiet kartieren, Person finden).
   - Optional: zusätzliche Erforschung der Umgebung.

4. **Rückweg**

   - Zurück zum Startgebiet/Schiff/Hub.
   - Ressourcenmanagement, Verluste, zusätzliche Funde.

5. **Rückkehr ins Büro**

   - Auswertung, Belohnungen, Konsequenzen.
   - Ruf verbessern, neue Verträge/Förderer/Forschung freischalten.
   - Crew-Entwicklung, ggf. neue Rekruten.

6. **Nächste Expedition planen**

   - Loop beginnt von vorne.

---

### 3.2 Innerer Expedition-Loop

Innerhalb einer Expedition:

1. **Tag Start**

   - Du bist im Lager/aktuellen Standort.
   - Rollen für den Tag vergeben.
   - Ziele festlegen (Richtung, POIs, Zwischenlager).

2. **Wanderung (Echtzeit, Overworld)**

   - Die Overworld ist:
     - eine 3D-Welt in Top-Down-Perspektive,
     - wird in Echtzeit simuliert,
     - zeigt den Konvoi / die Expedition als sichtbare Figuren, Tiere, Wagen und Assets, nicht nur als Marker.
   - Der Spieler steuert die Expedition durch das Gelände (konkrete Eingabevariante siehe separate Vorschläge zur Steuerung).
   - Späher / Vorhut / Nachhut beeinflussen:
     - den Zoom der Kamera (wie weit nach vorne/hinten und zur Seite sichtbar ist),
     - die Sichtweite in Bewegungsrichtung (Frühwarnzeit für Gefahren/Events),
     - ggf. die Größe des Bereichs, der auf der Karte aufgedeckt wird.
   - Die Weltkarte ist ein separates Widget, das aktiv geöffnet werden muss.
   - Mit jeder Bewegung in unbekanntes Gebiet wird die Karte weiter aufgedeckt (Fog-of-War / Exploration).
   - In bereits bekannten Regionen sind Details (Dörfer, Geländeformen, Wege) sichtbar; unbekannte Regionen bleiben abstrahiert.
   - Zufalls- & Script-Events treten auf.

3. **Lokale Szenen (Lager/POI/Dorf)**

   - Lokale Szenen sind eigene Level mit klar begrenzter Spielfläche:
     - Lagerplätze
     - Points of Interest (Ruinen, Tempel, Höhlen etc.)
     - Dörfer / Siedlungen
   - Zeitlogik:
     - Die Karte bleibt zwar in Echtzeit darstellbar (Animationen, Effekte), aber die ingame Zeit läuft nur in bestimmten Situationen weiter, z. B.:
       - beim Tag beenden, nachdem Aufgaben zugewiesen wurden,
       - bei expliziten Aktionen, die Zeit kosten (z. B. längere Reparaturen, Ritual, Handwerk).
   - Bewegungsbegrenzung:
     - Der spielbare Bereich ist auf die Szene begrenzt (Lager, Dorf, POI).
     - Es gibt keine freie Bewegung darüber hinaus; das Level muss aktiv verlassen werden (Rückkehr in Overworld).
   - Steuerung:
     - Point & Click für Einheiten.
     - WASD (oder Edge-Scrolling) für begrenzte Kamerabewegung.
   - Interaktionen, Aufgaben, Dialoge.
   - Optional Mini-Expeditionen von dort aus.

4. **Kampf (wenn ausgelöst)**

   - Kämpfe finden in eigenen Levels statt, die aus der Overworld oder lokalen Szenen heraus geladen werden.
   - Der Kampf ist:
     - rundenbasiert,
     - auf einem klar definierten Grid (Hex oder Quadrat, wird noch festgelegt).
   - Während des Kampfes verstreicht keine Overworld-Zeit:
     - Globale Zeit / Expeditionstag ist im Kampf eingefroren.
     - Dadurch sind Kämpfe planbar und nicht an Echtzeitdruck gekoppelt.
   - Steuerung:
     - Point & Click auf Gridfelder für Bewegung und Aktionen.
     - Kamera über WASD / Maus verschieb- und drehbar, aber ohne Einfluss auf die Einheitensteuerung.

5. **Tagesende**

   - Zeitsprünge (z. B. über Nacht) können manuell durch den Spieler ausgelöst werden.
   - Die Dauer des Zeitsprungs bestimmt:
     - Regeneration (Gesundheit, Ausdauer, Moral)
     - Fortschritt von Aufgaben (z. B. Holzfällen, Handwerk, Forschung)
     - Verbrauch von Ressourcen (Proviant, Brennstoff, ggf. Risiko von Angriffen während der Rast).
   - Designziel: Eine bewusste Entscheidung, wie viel Zeit man investiert, um die Crew zu stabilisieren, mit dem Risiko, dass sich weltweite Situationen verschieben (Wetter, Gegnerbewegungen, Aufträge).

---

### 3.3 Tagesstruktur

- **Ein Tag ≈ ~15 Minuten Echtzeit** (ohne Kämpfe/POI-Aufenthalte).
- Kehren regelmäßig wiederkehrende „Zyklen“:

  - Morgens: Rollen zuweisen, grobe Planung.
  - Tagüber: Wanderung + Events.
  - Abends: Lager/POI, Aufgaben, Dialoge, ggf. Kampf.

Kämpfe & POIs verlängern den Tag in Realzeit, aber Ingame bleibt die Struktur klar.

---

## 4. Systems Design

### 4.1 Spielerrolle & Perspektiven

- Spieler ist **immer der aktuelle Entdecker/Gruppenführer**.
- Büro / Heimatbasis
  - Perspektive: First Person.
  - Der Spieler kann sich nur im kleinen Büro bewegen und dort mit dem Kartentisch und weiteren Objekten interagieren (z. B. Akten, Ausrüstung, Expeditionsberichte).
  - Im Verlauf des Early Access können zusätzliche Interaktionsmöglichkeiten im Büro freigeschaltet werden (z. B. Trophäenraum, Kartenarchiv, Forschungstisch).
- Overworld
  - Top-Down-3D-Ansicht auf die komplette Expedition.
  - Jede Figur, jedes Tier, jeder Wagen und relevante Assets sind sichtbar – der Konvoi ist kein abstrakter Marker, sondern physisch auf der Karte präsent.
- Lager / POI / Dörfer
  - Ebenfalls Top-Down-Darstellung, ähnlich Referenz „Wartales“.
  - Das Lager wird als konkrete Szene abgebildet (Zelte, Feuerstelle, Werkbänke, Tiere, Lagerbestände).
  - Fokus auf Lesbarkeit von Aufgaben (wer macht was, wo im Lager).
- Tod des Entdeckers
  - Hardcore-Modus:
    - Stirbt der Entdecker (z. B. im Kampf oder durch ein Event), gilt dies als Permadeath.
    - Die Expedition ist damit endgültig gescheitert; der Spielstand wird entsprechend behandelt (Details: siehe Schwierigkeits- und Meta-Design).
  - Casual-Modus:
    - Die Expedition scheitert bei Tod des Entdeckers, aber der Spielstand wird nicht dauerhaft zerstört.
    - Konkrete Ausgestaltung (schwerwiegende Konsequenzen vs. „Traum“-Interpretation) siehe Vorschläge im Abschnitt „Offene Design-Entscheidungen“ – noch nicht final.

---

### 4.2 Planungsphase im Büro (Hub)

**Ort:** Kleines Büro/Kontor in einer Hafenstadt / Küstenstadt.

Elemente:

1. **Kartentisch**

   - Übersicht über bekannte Welt/Regionen.
   - Markierung:

     - Entdeckte Regionen.
     - Bekannte POIs.
     - Offene Verträge/Aufträge.

2. **Depeschen / Schriftverkehr**

   - UI, in dem Briefe/Mitteilungen dargestellt werden:

     - Bewerbungen von Rekruten.
     - Angebote/Anfragen von Investoren/Förderern.
     - Händlerangebote.
     - Regierungs-/Gildenaufträge (Verträge).

3. **Expeditionsplanung**

   - Expedition wählen:

     - Ziel (POI/Region), Auftraggeber, Belohnung, Risiko.

   - Crew auswählen aus:

     - vorhandenen Veteranen.
     - neuen Rekruten (mit Kosten).

   - Proviant/Ausrüstung:

     - Budget begrenzt durch Geld/Förderer.
     - Auswahl: Essen, Wasser, Medizin, Werkzeuge, Waffen, Tiere, Transportmittel.

4. **Startpunkt & Anreise**

   - Startregion wählen (z. B. bestimmter Küstenabschnitt/Flusshafen).
   - Bestimmen, mit welchem Transport (Schiff, Boot, Wagenkolonne → später nur abstrahiert).
   - Jede geplante Route bzw. jeder Transportweg besitzt einen Risikowert (oder mehrere Teilwerte, z. B. Terrain, Wetter, Feindaktivität).
   - Negative Events (z. B. Ressourcenverlust, Verletzungen, Verzögerungen) basieren auf diesem Risikowert:
     - Höheres Risiko = höhere Wahrscheinlichkeit bzw. Schwere negativer Ereignisse.
   - Ablauf:
     - Risiko wird während der Planung angezeigt (z. B. durch Farbcode oder Symbole).
     - Nach der Ankunft am Startort oder Abschnittsende werden auf Basis des Risikowerts die eingetretenen negativen Events ermittelt und dem Spieler präsentiert:
       - verlorene Ressourcen (Proviant, Tiere, Ausrüstung),
       - verletzte oder erschöpfte Crewmitglieder,
       - beschädigte Ausrüstung / Wagen,
       - Zeitverlust.
   - Ziel: Routenplanung soll eine strategische Abwägung zwischen Sicherheit, Geschwindigkeit und Ressourcenverbrauch sein.
   - **Anreise wird per Ladebildschirm simuliert**, keine aktive Seefahrt nötig.

---

### 4.3 Overworld-Exploration (Wanderung)

- Darstellung:

  - Karte mit 3D-Gelände oder stilisierter 3D/2D-Overworld.
  - Konvoi als Einheit (ein Icon oder eine kleine Gruppe).

- Mechaniken:

  - **Bewegung in Echtzeit** mit optionaler Pause-Funktion.
  - Ressourcenverbrauch pro Strecke/Zeit:

    - Proviant
    - Wasser
    - ggf. Tierkraft/Kondition.

  - **Fog of War**:

    - Unbekannte Regionen sind verdeckt.
    - Sichtbereich um Konvoi herum wird sichtbar.
    - Aufgedeckte Gebiete bleiben erkennbar.

- Overworld-Events:

  - Begegnung mit:

    - anderen Gruppen/Konvois (Handel, Gefahr).
    - Tieren (Jagdchance/Kampf).
    - Spuren/Indizien von POIs oder Gefahren.

  - Wetter:

    - Regen, Sturm, Hitze → Modifikatoren auf Reise, Risiko, Krankheiten (spätere Erweiterung).

---

### 4.4 Lager / POI / Dörfer (lokale Szenen)

**Lager:**

- Kleine, begrenzte 3D-Map:

  - Zelte, Feuerstelle, Wagen, Tiere.

- Funktionen:

  1. **Tagesrollen zuweisen**

     - Für den kommenden Tag.

  2. **Aufgaben vergeben**

     - Sammeln (Holz/Wasser), Reparaturen, Wache, Kochen.

  3. **Mini-Expeditionen**

     - 1–3 Figuren werden ausgewählt, bekommen Proviant & Ausrüstung.
     - Zielbereiche auf Overworld werden markiert.
     - Nach einer Zeit + Event-Checks kommen sie (hoffentlich) zurück.

- Tagesabschluss:

  - „Schlafen gehen“/Tag beenden → Zeitsprung, Nacht-Events, Erholung & Ressourcenverfall.

**Dörfer/Lager anderer Gruppen:**

- Lokale Map mit:

  - Taverne → Dialoge, Gerüchte, Rekruten.
  - Marktplatz → Handel (Nahrung, Ausrüstung, Munition, Tiere).
  - Lagerplatz anderer Gruppen → Diplomatie, Handel, Bedrohungen.

**POIs (Ziele):**

- Spezielle Orte:

  - Ruinen, Tempel, alte Festungen, Minen, verlassene Lager, mysteriöse Orte (ohne Magie, aber „unheimlich/ungewöhnlich“).

- Hauptziel:

  - Artefakte finden, Gebiete kartieren, vermisste Personen suchen, Beweisstücke bringen etc.

---

### 4.5 Rollen & Tageszuweisung

Rollen sind **tagesbasiert**, nicht permanent.

Beispiele:

- Vorhut (Scouting, Gefahr früh erkennen)
- Nachhut (Rückenschutz)
- Tieraufseher (Tiere, Zugleistung, Umgang mit Panik)
- Proviantverwalter (rationiert effizient)
- Wache (Perimeter, Nachtwache)
- Heiler (Verletzungen/Krankheiten)
- Handwerker (Reparaturen, Crafting, Fallen)

**Einfluss von Persönlichkeit & Fähigkeiten:**

- Jede Figur hat:

  - **Traits** (mutig/ängstlich, fleißig/faul, empathisch/hart, diszipliniert/chaotisch, etc.)
  - **Fähigkeiten-Stats** (Tiere, Medizin, Handwerk, Wahrnehmung, Kampfarten, etc.)

- Wenn Rolle gut passt:

  - Gute Effizienz (weniger Ressourcenverbrauch, bessere Ergebnisse).
  - Mood (Stimmung) steigt oder bleibt stabil.
  - Beziehung zum Entdecker verbessert sich.

- Wenn Rolle schlecht passt:

  - Erhöhtes Risiko (Unfälle, Fehler, verpasste Chancen).
  - Mood sinkt.
  - Beziehung zum Entdecker verschlechtert sich.

---

### 4.6 Persönlichkeit, Stimmung & Beziehungen

**Stufe 1 (MVP für Prototyp):**

- 2–3 Traits pro Figur.
- Einfacher Mood-Wert (negativ/neutral/positiv).
- Events ändern Mood & ggf. einfache Konsequenzen.

**Stufe 2 (Demo/EA):**

- Mehr Traits.
- Mood mit mehreren Stufen (depressiv, gereizt, neutral, zufrieden, euphorisch).
- Verhalten & Erfolgschancen variieren leicht.
- Dialogtexte & Eventausgänge hängen stärker von Traits/Mood ab.

**Stufe 3 (später):**

- Einfache Beziehungswerte zwischen Figuren.
- Spezielle Events abhängig von Trait + Beziehung + Situation.
- Langfristige Entwicklung (Traumata, Abstumpfung, Idealisierung des Entdeckers etc.).

---

### 4.7 Dialog- & Event-System

**Dialogfenster:**

- Für:

  - Gespräche mit Crew-Mitgliedern.
  - NPCs in Dörfern/Lagern.
  - Auftraggebern/Förderern.

- Mehrfachauswahl-Antworten:

  - beeinflussen Mood, Beziehung, manchmal Traits.
  - entscheiden über Quest-/Event-Ausgänge.

**Eventfenster (Kurzform):**

- Kleine textbasierte Ereignisse mit 1–3 Optionen.

  - z. B. „Streit im Lager“, „Tier verletzt“, „verdächtige Geräusche nachts“.

- Eingebettet in Wanderung/Lager-Phase.

---

### 4.8 Kampf-System

- **Rundenbasiert** (taktisch, übersichtlich).
- Auslöser:

  - Räuber/Überfälle, aggressive Tiere, Arena-Kämpfe, bestimmte POI-Events.

- Mechaniken (erste Version):

  - Grid/Hex-Map.
  - Einfache Aktionen: Bewegen, Standardangriff, 1–2 Fähigkeiten, Defensive Aktion.
  - Initiative-Reihenfolge oder Phasen-System (z. B. alle Spieler, dann Gegner).

**Ausrüstung & Waffen:**

- Frühe Stufen:

  - Knüppel, Speere, einfache Schwerter, Bögen, Armbrüste.

- Mit Fortschritt & Geld:

  - Primitive Schusswaffen (Hakenbüchse, Arkebuse, frühe Pistolen).
  - Später ggf. leichte Kanonen für spezielle Szenen/Events (späteres Feature).

**Permadeath & Konsequenzen:**

- Tod ist permanent.
- Krankheiten/Verletzungen können persistieren (z. B. Narben, Einschränkungen).
- Verlust von Spezialisten kann die Effizienz der nächsten Tage/Expeditionen stark beeinflussen.

---

### 4.9 Ressourcen & Ökonomie

**Wichtige Ressourcen:**

- Proviant (Essen)
- Wasser
- Medizin
- Ausrüstung (Werkzeuge, Zelte, Reparaturmaterialien)
- Waffen & Munition
- Tiere (Zug- & Packtiere)
- Geld

Ressourcen können nicht nur abstrakt (über Events oder Belohnungen), sondern auch direkt in der 3D-Welt gesammelt, abgebaut oder hergestellt werden.

Beispiele:

- Jagd:
  - In der Overworld (sichtbare Tierherden) oder als Aufgabe im Lager („Jagdtrupp aussenden“).
  - Im Kampf kann es Events mit Tieren geben, bei denen nach einem Sieg Fleisch/Felle erbeutet werden.
- Holzfällen / Rohstoffabbau:
  - Als Aufgabe im Lager (Crewmitglieder werden zu Bäumen / Ressourcenpunkten geschickt).
  - Zeitgebunden, mit Risiko (z. B. Überfälle während Arbeit).
- Handwerk & Produktion:
- Im Lager am Abend (z. B. Ausrüstung reparieren, neue Gegenstände herstellen, Medizin mischen).
- Fortschritt ist an Zeitsprünge / Rasten gebunden.

Die Verfügbarkeit und Effizienz solcher Aufgaben hängt ab von:

- Vorkommen in der Umgebung (z. B. Wald vs. Steppe),
- Fähigkeiten und Anzahl der zugewiesenen Crewmitglieder,
- Ausrüstung (Werkzeuge, Jagdwaffen, Werkbänke).

**Geldquellen:**

- Vertragsbelohnungen
- Handel (Funde verkaufen)
- Arena-Kämpfe/Wetten (später optional)
- Plünderungen (moralisch fragwürdig, Konsequenzen!)

**Meta-Progression:**

- Ruf bei:

  - Förderern
  - Fraktionen
  - Gesellschaft (Zeitungen, Chronisten etc.)

- Forschung:

  - bessere Ausrüstung
  - effizientere Routen
  - neue Regionen/POIs

---

## 5. Welt & Setting

- Fiktive Welt, inspiriert von:

  - Spätes Mittelalter → Übergang zur frühen Neuzeit.
  - Gesellschaften mit starkem Standesdenken, Kirchenkraft, Gilden, frühen Nationalstaaten.

- Technologien:

  - Handwerk, Segel- und Ruder-Schiffe, Wagen, einfache Mechanik.
  - Schießpulver im Kommen: anfangs seltene, teure Schusswaffen.

- Keine Magie, keine klassischen Fantasy-Rassen – aber:

  - Viel Raum für kulturelle Vielfalt (andere Religionen, Stämme, Reiche).

---

## 6. Scope für Phasen (Feature-MVPs)

### Phase 1 – Prototyp (1–2 Monate)

**Ziel:** Ein kurzer, spielbarer Expeditions-Run (1 Region, 1 Ziel, ca. 2–3 Ingame-Tage).

**Must-Haves:**

- Büro/Planung (ein Raum + UI für 1 Vertrag, 3–4 Rekruten, einfache Ausrüstungswahl).
- Overworld:

  - Eine Region (Küste + etwas Inland).
  - Fog of War + Konvoi-Bewegung.

- Lager:

  - Simple Campszene.
  - Tagesrollen (rudimentär, 3–4 Rollen).

- Events:

  - 10–15 einfache Random-/Script-Events.

- Kampf:

  - Basic rundenbasiertes System mit 2 Waffenarten (Nahkampf + Bogen).

- 4–6 Charaktere mit Traits Stufe 1 + Mood einfach.

---

### Phase 2 – Demo (bis ca. Monat 6)

**Ziel:** Öffentliche Demo, in der man mehrere Expeditionen machen kann, mit Wiedererkennungswert & Charakterbindung.

**Must-Haves:**

- Mehrere Verträge/Ziele.
- 2–3 Regionen (unterschiedliche Biome).
- 2–3 Dörfer, 1–2 spezielle POIs mit kleinen Geschichten.
- Rollen-System erweitert (mehr Rollen, stärkere Effekte).
- Persönlichkeit Stufe 2 (mehr Traits, Mood mit Stufen, spürbare Unterschiede).
- Mehr Events (30–50), einige klein verknüpft.
- Kampf mit 4–5 Waffentypen (inkl. erster Schusswaffen).
- Simple Forschung/Meta-Fortschritt.

---

### Phase 3 – Early Access (Monat 6–9 als Ziel)

**Ziel:** Solide Kampagne/„Karriere“, viele mögliche Runs, gute Wiederspielbarkeit.

**Must-Haves (on top):**

- Deutlich mehr Content: Regionen, POIs, Events, Waffen, Verträge.
- Ausgebautes Forschungs- & Förderer-System.
- Balancing-Schleifen mit Feedback.
- Mehr Variation bei Expeditionen (Spezialkontrakte, schwierige Distanzmissionen).
- Persönlichkeit Stufe 2 stabil und sichtbar integriert.

---

Die späteren Phasen (Full Release, DLCs) sind dann v. a. **Content + Vertiefung**.
