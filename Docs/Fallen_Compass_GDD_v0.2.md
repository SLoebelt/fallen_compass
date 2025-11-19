# Expeditionsspiel – Game Design Document v0.2

_Stand: 14.11.2025 – Unreal Engine 5.7 (geplanter Full Release im Entwicklungszeitraum) – Hybrid-Ansatz: Blueprints + gezieltes C++_

---

## 1. Vision & Elevator Pitch

### 1.1 Elevator Pitch (Arbeitsfassung)

Du spielst einen **Entdecker**, der Expeditionen in unkartierte Regionen plant, ausrüstet und anführt.  
Im **Büro** bereitest du am Kartentisch Route, Crew und Ausrüstung vor. Auf einer **3D-Overworld in Top-Down-Perspektive** führst du deinen Konvoi durch unbekanntes Gelände, deckst die Karte auf und gehst Risiken ein. In **Lagern, Dörfern und Points of Interest** verwaltest du Ressourcen, Moral und Verwundete.  
Konflikte werden in **rundenbasierten, gridbasierten Taktik-Kämpfen** ausgetragen. Erfolg und Scheitern fühlen sich bewusst **ambivalent** an: Manchmal ist es ein Sieg, nur lebend zurückzukehren.

### 1.2 Ton & Kernfantasie

- Harte, aber faire **Expeditions-Fantasie**: Planung, Entbehrung, Risiko.
- Fokus auf **Crew, Erlebnisse und Karte**, nicht nur auf Loot.
- Entscheidungen haben **langfristige Konsequenzen**:
  - Tote Crewmitglieder fehlen bei späteren Expeditionen.
  - Reputation bei Auftraggebern beeinflusst zukünftige Chancen.
  - Die Weltkarte bleibt persistent und erzählt die Geschichte vergangener Versuche.

### 1.3 Technik, Engine & Steuerung

**Technik**

- Umsetzung als **Hybrid aus C++ und Blueprints**.
- **C++ ist immer eine Option**, wenn dadurch Gameplay-Systeme oder Performance-kritische Funktionen
  - schneller,
  - stabiler oder
  - besser wartbar
    werden (z. B. Pathfinding, komplexe Simulation, speicherintensive Systeme, Metasysteme wie Routenrisiko).
- Blueprints bleiben primär für **Content-nahe Logik** (UI, einfache Skripte, Events, Prototyping).
- Es gibt **kein starres „Blueprint First“-Dogma** – stattdessen einen pragmatischen Ansatz:
  > „Was technisch sinnvoll ist, bekommt C++; der Rest bleibt agil in Blueprints.“

**Engine**

- Projektstart auf **Unreal Engine 5.7 (Full Release)**.
- Ziel: möglichst lange auf einer **stabilen Major-Version** bleiben, um größeren Migrationsaufwand während der Early-Access-Phase zu vermeiden.
- Nutzung der UE5-Features (Lumen, Nanite, World Partition etc.) nur dort, wo sie **Workflow und Performance klar verbessern**.

**Steuerung – Überblick**

- **Büro (Kartentisch / Meta-Ebene)**
  - Perspektive: **First Person** (siehe 4.1).
  - Bewegung im kleinen Büro via **WASD + Maus**, Interaktionen per **Mausklick** (E/F optional).
- **Overworld**
  - 3D-Welt in **Top-Down-Sicht**.
  - **Linksklick**: bewegt den **Konvoi** zum Zielpunkt (Pathfinding über NavMesh).
  - **Rechtsklick**: löst **Interaktionen** am Ziel aus (POI betreten, mit NPC sprechen, Kontextaktionen).
  - **Kamera**:
    - Bewegung: z. B. über **WASD**, Middle-Mouse-Pan oder Edge-Scrolling.
    - **Zoomweite und Sicht nach vorne/hinten** hängen von der eingesetzten Crew ab (Späher / Vorhut / Nachhut).
- **Lager / POI / Dörfer (lokale Szenen)**
  - Darstellung: **Top-Down / leicht isometrisch**.
  - **Point & Click** zur Steuerung von Figuren/Aufgaben.
  - **WASD** dient ausschließlich einer **begrenzten Kamerabewegung**, um den Ausschnitt besser einsehen zu können.
- **Kämpfe (rundenbasiert, gridbasiert)**
  - Eigenes Level, **rundenbasiert auf Tiles**.
  - **Point & Click** für Einheiten (Bewegung, Aktionen).
  - **WASD/Maus** nur für Kamerabewegung/-rotation.
  - Das **„Advanced Turn Based Tile Toolkit“** wird als mögliche technische Grundlage für das Kampfsystem evaluiert und bei Einsatz über eigene Blueprints/C++-Code an die Bedürfnisse des Spiels angepasst.

---

## 2. Setting & Progression

### 2.1 Welt & Zeit (Platzhalter)

- Konkreter historischer Kontext, Region und Tonalität (realistisch vs. leicht fantastischer Einschlag) werden noch genauer definiert.
- Wichtig: Die Welt sollte **Exploration, Entbehrung und logistische Herausforderungen** glaubhaft tragen.

### 2.2 Expeditionstypen & Ziele (Platzhalter)

- **Primärziele** (z. B. bestimmtes Gebiet erreichen, Artefakt bergen, Auftrag erfüllen).
- **Sekundärziele/Nebenaufgaben** (z. B. zusätzliche Kartensegmente aufdecken, Kontakte knüpfen, Proben sammeln).
- **Langfristige Metaziele**:
  - Aufbau von Reputation.
  - Aufbau eines Kartenarchivs.
  - Freischalten neuer Startpunkte, Routen oder Ausrüstung.

### 2.3 Erfolg & Scheitern einer Expedition

- **Erfolg / Scheitern ist bewusst subjektiv** und wird sowohl mechanisch als auch emotional bewertet.
- Beispiele:
  - Eine Expedition kann sich wie ein **Erfolg** anfühlen, wenn die **gesamte Crew lebend** zurückkehrt,
    auch wenn nur ein Teil der Nebenziele erfüllt und nur wenige Kartenteile aufgedeckt wurden.
  - Eine Expedition, in der das **Primärziel** erreicht wird, aber **erfahrene Crewmitglieder sterben**,
    kann sich wie eine **Niederlage** anfühlen (Verlust von Expertise, Auswirkungen auf Moral/Stimmung,
    langfristige Konsequenzen im Meta-Spiel).
- Das Spiel spiegelt diese Ambivalenz durch:
  - **Abschlussberichte**, die nicht nur Primär-/Sekundärziele, sondern auch
    - Crewverluste, Verletzungen,
    - erbeutete Ressourcen,
    - Kartenerkundung,
    - Moral/Stimmung
      abbilden.
  - Meta-Systeme, bei denen der Spieler bewusst abwägen muss:
    - „Primärziel durchdrücken“ vs. „Crew schonen / Risiko reduzieren“.

---

## 3. Spielstruktur & Ebenen

### 3.1 Büro / Heimatbasis & Routenplanung

- Die Heimatbasis/Büro-Ebene ist die **Meta-Schicht**:
  - Verwaltung von **Auftraggebern, Reputation und Finanzen**.
  - Zusammenstellung der **Crew** und Auswahl von **Ausrüstung/Tieren/Wagen**.
  - **Kartentisch** als diegetische UI für Weltkarte, bekannte Regionen und Routenplanung.

#### 3.1.1 In-World Main Menu (Design-Update: November 2025)

- Das Spiel startet **direkt im Büro-Level (L_Office)** in einem **Main Menu-Zustand**:
  - Eine **statische oder langsam bewegte Kamera** (MenuCamera) fokussiert den Schreibtisch.
  - Eine **UI-Overlay** (WBP_MainMenu) zeigt:
    - Spiellogo (diegetisch oder als Overlay)
    - "New Legacy" (neue Expedition starten)
    - "Continue" (letzten Speicherstand laden, deaktiviert wenn keine Speicherstände vorhanden)
    - "Load Save Game" (Speicherstand-Auswahl öffnen)
    - "Options" (Einstellungen)
    - "Quit" (Spiel beenden)
  - **Atmosphärische Details**: Staubpartikel, flackernde Kerzen, Umgebungsgeräusche (Uhr, Wind, raschelndes Papier).
- Bei Klick auf **"New Legacy"**:
  - Sanfter **Kameraübergang** (2 Sekunden, kubisches Easing) von MenuCamera zur First-Person-Kamera.
  - Spieler-Charakter wird gespawnt/aktiviert.
  - UI wird entfernt, Eingabe aktiviert.
- Die **Tür im Büro** kehrt zum Main Menu-Zustand zurück:
  - Fade to Black (1-2 Sekunden).
  - Level-Reload (L_Office).
  - Initialisierung im Main Menu-Zustand (MenuCamera + WBP_MainMenu).
  - Gameplay-Zustand wird gelöscht (Position, Inventar etc.).
- **Vorteile**: Immersiv, keine Ladebildschirme beim Start, atmosphärische Einführung, nahtloser Übergang zwischen Meta-Ebene und Gameplay.

#### 3.1.2 Kartentisch – Interaktive Planungsoberfläche (Design-Update: November 2025)

Der **Kartentisch** ist das zentrale Interface für Expeditionsplanung im Büro. Der Tisch zeigt mehrere **physische Objekte**, die jeweils unterschiedliche Meta-Funktionen repräsentieren. Jedes Objekt ist direkt **anklickbar in der First-Person-Perspektive** (via Raycast/Interaktion).

**Interaction Pattern:**

- Spieler kann sich frei im Büro bewegen (First Person, WASD + Maus).
- Bei **Klick auf ein Objekt** auf dem Tisch:
  - Kamera **blendet zum Objekt** (2s, cubic easing, ähnlich wie Table View aus Week 1).
  - Falls benötigt, erscheint ein **UI-Widget** über/neben dem Objekt.
  - Spieler interagiert mit UI (Maus + Tastatur).
  - **ESC** oder "Zurück"-Button schließt Widget und blendet Kamera zurück zu First Person.

**Objekte auf dem Kartentisch:**

1. **Logbuch (Expedition Reports)**

   - **Physisches Objekt**: Aufgeschlagenes Buch mit Ledereinband, sichtbare Schrift/Skizzen.
   - **Funktion**: Archiv abgeschlossener Expeditionen.
   - **UI-Widget**: `WBP_ExpeditionLog`
     - Liste aller vergangenen Expeditionen (Datum, Ziel, Erfolg/Misserfolg).
     - Detailansicht: Crew-Verluste, Kartendaten, Ressourcenverbrauch, narrative Zusammenfassung.
   - **Gameplay-Zweck**: Lernen aus Fehlern, Nachschlagen von Routendaten, narrative Persistenz.

2. **Weltkarte (Route Planning)**

   - **Physisches Objekt**: Große Karte mit Gewichten an den Ecken, Kompass, Stift/Lineal daneben.
   - **Funktion**: Planung der kompletten Expeditionsroute.
   - **UI-Widget**: `WBP_RouteMap` (Hauptfunktion, siehe 3.1.3)
     - **Zweigeteilte Planung**:
       - **A) Anreise zum Expeditionsstartpunkt** (z. B. Schiff zur Küste, Wagen zum Fluss).
       - **B) Expeditionsgebiet** (wählbare vordefinierte Region/Kontinent für die 3D-Overworld-Expedition).
     - Zeigt **Fog-of-War**: Bereits erkundete Gebiete sichtbar, unbekannte Regionen verhüllt.
     - **Kalkulationswidget** (integriert oder als Sidebar):
       - Geschätzter Bedarf: Proviant, Ressourcen, Transportkosten (Schiffe, Wagen).
       - Risikostufe (niedrig/mittel/hoch, farbcodiert).
       - Gesamtkosten in Geld.
   - **Gameplay-Zweck**: Strategische Vorbereitung, Ressourcenabschätzung, Risikoabwägung.

3. **Briefe & Dokumente (Messages & Contracts)**

   - **Physisches Objekt**: Stapel Umschläge, versiegelte Briefe, Pergamente.
   - **Funktion**: Kommunikation mit Auftraggebern, Crew-Bewerbungen, Bestellungen.
   - **UI-Widget**: `WBP_MessagesHub`
     - **Tabs/Kategorien**:
       - **Aufträge**: Verfügbare Expeditionsaufträge von Auftraggebern (Belohnung, Ziel, Frist).
       - **Bewerbungen**: Crew-Mitglieder, die anheuern wollen (Fähigkeiten, Kosten).
       - **Bestellungen**: Formular zum Bestellen von Ausrüstung, Tieren, Proviant (Lieferzeit, Kosten).
     - Jede Kategorie zeigt **Liste von Einträgen**, auswählbar für Details/Aktionen.
   - **Gameplay-Zweck**: Auftragswahl, Crew-Rekrutierung, Logistikplanung.

4. **Kompass (Expedition Start Trigger)**
   - **Physisches Objekt**: Messingkompass, liegt auf Kartenstapel oder direkt auf der Tischplatte.
   - **Funktion**: Expedition **final starten**.
   - **Interaction**: Klick auf Kompass → **Bestätigungsdialog**:
     - "Expedition beginnen? [Routenübersicht, Kosten, Crew]"
     - Buttons: "Ja, aufbrechen" / "Abbrechen".
   - Bei Bestätigung:
     - **Fade to Black** (Kameraübergang).
     - **Lade Overworld-Level** mit gewählter Region und Route.
     - Expedition startet.
   - **Gameplay-Zweck**: Klare Schwelle ("Point of No Return"), emotionaler Moment, diegetische Trigger-Metapher.

---

#### 3.1.3 Routenplanung bis zum Expeditionsstartpunkt (Detaillierte Mechanik)

Die Routenplanung über die **Weltkarte** (Objekt 2) erfolgt in **zwei Phasen**:

**Phase A: Anreise zum Expeditionsstartpunkt**

- Wähle **Transportmittel und Route** von der Heimatbasis zum Start der Overworld-Expedition:
  - Beispiele: Schiff zur Küste, Flussdampfer, Wagen über bekannte Straßen.
- Diese Phase:
  - Kostet **Geld** (Charterkosten, Mautgebühren).
  - Hat ein **Risiko** (Wetter, Piraten, mechanische Ausfälle).
  - Wird **nicht im 3D-Overworld gespielt**, sondern abstrakt abgehandelt:
    - Nach Expeditionsstart: kurze **Event-Sequenz** oder **Zusammenfassung**.
    - Negative Events (Verzögerung, Ressourcenverlust) werden **vor Ankunft am Startpunkt** abgewickelt.

**Phase B: Expeditionsgebiet & Startpunkt**

- Wähle **vordefinierte Region/Kontinent** für die 3D-Expedition (z. B. "Dschungelküste", "Gebirgskette Nord").
- Wähle **Startpunkt innerhalb dieser Region**:
  - Muss an bekannter/zugänglicher Stelle liegen (Küste, Fluss, Rand erforschter Gebiete).
- **Fog-of-War**:
  - Bereits bereiste Gebiete sind auf der Karte **sichtbar und detailliert** (Dörfer, Landmarken, Ressourcenpunkte).
  - Unerkundete Gebiete sind **verhüllt/abstrakt** (grauer Nebel, grobe Geländetypen).
- **Kalkulationswidget** zeigt:
  - Geschätzter **Proviantbedarf** (basierend auf Crew-Größe, erwarteter Dauer).
  - **Ressourcenbedarf** (Werkzeuge, Medizin, Munition).
  - **Risikostufe** (Gelände, bekannte Gefahren, Jahreszeit).
  - **Transportkosten** (für Phase A).

**Routenrisiko & Kosten-Abschätzung:**

- Bereits bekannte Routen:
  - Kosten und Risiko sind **genau kalkulierbar**.
  - Niedrigeres Risiko für negative Events.
- Unbekannte Routen:
  - Kosten nur **grob geschätzt** (größere Spanne).
  - Höheres Risiko, aber potentiell **größere Entdeckungsgewinne** (neue Karten, Ressourcen, Geheimnisse).

**Gameplay-Abwägung:**

- Sichere, teure Anreise vs. riskante, günstige Route.
- Bekannte Region (planbar) vs. unbekannte Region (risikoreich, aber belohnend).
- Ressourcen-Puffer einplanen vs. knappes Budget maximieren.

---

### 3.2 Feldspiel: Overworld, Lager, Kampf

#### 3.2.1 Übersicht

- Das Feldspiel gliedert sich in drei Ebenen:
  1. **Overworld** – 3D, Top-Down, Echtzeit, Konvoi-Bewegung.
  2. **Lokale Szenen** – Lager, Dörfer, POIs mit Aufgaben & Interaktionen.
  3. **Kampf-Level** – runden- & gridbasierte Taktik-Kämpfe.

#### 3.2.2 Overworld (3D, Echtzeit)

- Die **Overworld** ist:
  - eine **3D-Welt in Top-Down-Perspektive**,
  - wird in **Echtzeit** simuliert,
  - zeigt den **Konvoi / die Expedition als sichtbare Figuren, Tiere, Wagen und Assets**, nicht nur als Marker.
- Der Spieler steuert die Expedition **per Maus** durch das Gelände:
  - **Linksklick auf den Boden**: Konvoi bewegt sich per Pathfinding an diese Position.
  - **Rechtsklick auf Objekte/POIs**: Interaktionen (z. B. Lager aufschlagen, Dorf betreten, Event auslösen).
- **Kamera & Sichtweite**:
  - Kamera bewegt sich unabhängig vom Konvoi (z. B. via WASD, Middle-Mouse oder Edge-Scrolling).
  - **Zoom und Blickreichweite** nach vorne/hinten werden von **Spähtrupps/Vorhut/Nachhut** beeinflusst:
    - Mehr/bessere Späher → weiter nach vorne sehen, Gefahren früher erkennen.
    - Vernachlässigte Sicherung → eingeschränkte Sicht, höheres Risiko von Überraschungen.
- **Karte / Fog-of-War**:
  - Die Weltkarte ist ein **separates Widget**, das aktiv geöffnet wird (kein permanentes Mini-Map-HUD).
  - Mit jeder Bewegung in **unbekanntes Gebiet** wird die Karte **weiter aufgedeckt**.
  - Bereits bekannte Regionen zeigen Details (Dörfer, Geländeformen, Wege), unbekannte Regionen bleiben abstrahiert/verhüllt.

#### 3.2.3 Lokale Szenen: Lager / POI / Dörfer

- Lokale Szenen sind eigene **Level** mit klar begrenzter Spielfläche:
  - Lagerplätze,
  - Points of Interest (Ruinen, Tempel, Höhlen etc.),
  - Dörfer / Siedlungen.
- **Zeitlogik**:
  - Die Welt bleibt zwar optisch in **Echtzeit** (Animationen, Effekte), aber die **ingame Zeit läuft nur in bestimmten Situationen** weiter, z. B.:
    - beim **Tag beenden**, nachdem Aufgaben zugewiesen wurden,
    - bei expliziten Aktionen, die Zeit kosten (z. B. längere Reparaturen, Rituale, Handwerk).
- **Bewegungsbegrenzung**:
  - Der spielbare Bereich ist **auf die Szene begrenzt** (Lager, Dorf, POI).
  - Es gibt **keine freie Bewegung darüber hinaus**; das Level muss aktiv **verlassen** werden (Rückkehr in die Overworld).
- **Steuerung**:
  - **Point & Click** für Einheiten / Interaktionen.
  - **WASD** (oder Edge-Scrolling) für begrenzte **Kamerabewegung**, um das Geschehen zu überblicken.

#### 3.2.4 Kampf – runden- & gridbasiert

- Kämpfe finden in **eigenen Levels** statt, die aus der Overworld oder lokalen Szenen heraus geladen werden.
- Der Kampf ist:
  - **rundenbasiert**,
  - auf einem klar definierten **Grid** (Quadrat oder Hex – hängt u. a. von der finalen Toolkit-Integration ab).
- **Zeitstillstand**:
  - Während des Kampfes **verstreicht keine Overworld-Zeit**:
    - Globale Zeit / Expeditionstag ist im Kampf **eingefroren**.
    - Kämpfe sind planbar und nicht an Echtzeitdruck gekoppelt.
- **Steuerung**:
  - **Point & Click** auf Gridfelder für Bewegung und Aktionen.
  - Kamera über **WASD / Maus** verschieb- und drehbar, ohne Einfluss auf die Einheitensteuerung.
- **Technik-Hinweis**:
  - Das „Advanced Turn Based Tile Toolkit“ kann als **Framework** für Grid, Pathfinding, Zugreihenfolge, Sichtlinien etc. dienen und wird über eigene Systeme (Statuswerte, Moral, Ausdauer, spezielle Expedition-Skills) erweitert.

#### 3.2.5 Zeitsprünge & Rasten

- **Zeitsprünge** (z. B. über Nacht) können **bewusst vom Spieler ausgelöst** werden.
- Die Dauer des Zeitsprungs beeinflusst:
  - **Regeneration** (Gesundheit, Ausdauer, Moral),
  - **Fortschritt von Aufgaben** (Holzfällen, Handwerk, Forschung, Reparaturen),
  - **Verbrauch von Ressourcen** (Proviant, Brennstoff, Verbandsmaterial),
  - ggf. **Risiko von Überfällen** oder Ereignissen während der Rast.
- Designziel: Eine **bewusste Entscheidung**, wie viel Zeit man investiert, um die Crew zu stabilisieren, vs. das Risiko, dass sich **äußere Bedingungen** verändern (Wetter, Gegnerbewegungen, Fristen von Auftraggebern).

---

## 4. Perspektiven, Tod & Schwierigkeit

### 4.1 Perspektiven (Büro, Overworld, Lager) & Tod des Entdeckers

**Büro / Heimatbasis**

- Perspektive: **First Person**.
- Der Spieler kann sich innerhalb eines **kleinen Büros** bewegen.
- Interaktionspunkte:
  - **Kartentisch** (Routenplanung, Expeditionsübersicht),
  - evtl. **Ablage/Archiv** (Berichte, Karten),
  - später zusätzliche Räume/Funktionen (z. B. Trophäen, Forschungsecke) im Verlauf des Early Access.

**Overworld**

- Top-Down-3D-Ansicht auf die **komplette Expedition**.
- **Jede Figur, jedes Tier, jeder Wagen und relevante Assets** sind sichtbar – der Konvoi ist **kein abstrakter Marker**, sondern physisch auf der Karte präsent.
- Kamera- und Steuerungsdetails siehe 3.2.2.

**Lager / POI / Dörfer**

- Ebenfalls **Top-Down-Darstellung**, Referenz z. B. „Wartales“.
- Lager als konkrete Szene:
  - Zelte, Feuerstelle, Werkbänke, Tiere, Vorratsstapel.
  - Klar erkennbare **Arbeits- und Ruhebereiche** (z. B. Jagdtrupp, Holzfäller, Handwerker, Wachposten).

**Tod des Entdeckers & Schwierigkeitsmodi**

- **Hardcore-Modus**
  - Stirbt der Entdecker (z. B. im Kampf oder durch ein Event), gilt dies als **Permadeath**.
  - Die laufende Expedition **scheitert endgültig**.
  - Der Spielstand/Spielverlauf wird so behandelt, dass der Entdecker **nicht weitergespielt** werden kann (Details: exakte Savegame-Behandlung noch zu definieren).
- **Casual-Modus**
  - Stirbt der Entdecker, **scheitert die Expedition**, aber die **Welt bleibt persistent** und der Entdecker lebt weiter.
  - **Harte Konsequenzen**:
    - **Verlust von Ansehen** bei Auftraggebern / Fraktionen.
    - Teile oder die gesamte **Crew können den Dienst quittieren** (z. B. traumatisiert, Vertrauensbruch).
    - **Ausrüstung, die mit auf Expedition war, gilt als vollständig verloren**.
    - **Keine Bezahlung / Belohnung** für diese Expedition.
  - Der Entdecker kehrt im Meta-Spiel (Büro) zurück:
    - Er trägt die **Niederlage und deren Folgen** weiter (z. B. Ruf-Mali, evtl. psychische/„narrative“ Narben).
    - Eine neue Expedition kann gestartet werden – mit geschwächter Ausgangslage.

---

### 4.2 Routen, Reisen & Risiken

#### 4.2.4 Routenrisiko & negative Events

- Jede geplante Route bzw. jeder Transportweg besitzt einen **Risikowert** (oder mehrere Teilwerte, z. B. Terrain, Wetter, Feindaktivität).
- **Negative Events** (z. B. Ressourcenverlust, Verletzungen, Verzögerungen) basieren auf diesem Risikowert:
  - Höheres Risiko = höhere **Wahrscheinlichkeit** bzw. Schwere negativer Ereignisse.
- Ablauf:
  - Risiko wird während der Planung visualisiert (z. B. Farbcode, Symbole, kurze Tooltipps).
  - Nach der **Ankunft am Startort** oder am Ende eines Reiseabschnitts werden auf Basis des Risikowerts die eingetretenen **negativen Events ermittelt** und dem Spieler präsentiert:
    - verlorene Ressourcen (Proviant, Tiere, Ausrüstung),
    - verletzte oder erschöpfte Crewmitglieder,
    - beschädigte Ausrüstung / Wagen,
    - Zeitverlust (Verspätung am Ziel).
- Ziel: Routenplanung soll eine **strategische Abwägung** zwischen
  - Sicherheit,
  - Geschwindigkeit und
  - Ressourcenverbrauch
    sein und sich spürbar unterschiedlich anfühlen (sicherer Umweg vs. riskante Abkürzung).

---

### 4.9 Ressourcen in der 3D-Welt

- Ressourcen werden nicht nur abstrakt über Events/Belohnungen, sondern auch **direkt in der 3D-Welt** gesammelt, abgebaut oder hergestellt.
- Beispiele:
  - **Jagd**:
    - In der Overworld (sichtbare Tierherden) oder als **Aufgabe im Lager** („Jagdtrupp aussenden“).
    - Im Kampf kann es **Events mit Tieren** geben, bei denen nach einem Sieg Fleisch/Felle erbeutet werden.
  - **Holzfällen / Rohstoffabbau**:
    - Als **Aufgabe im Lager** (Crewmitglieder werden zu Bäumen / Ressourcenpunkten geschickt).
    - Zeitgebunden, mit Risiko (z. B. Überfälle während der Arbeit).
  - **Handwerk & Produktion**:
    - Im Lager am Abend (z. B. Ausrüstung reparieren, neue Gegenstände herstellen, Medizin mischen).
    - Fortschritt ist an **Zeitsprünge / Rasten** gebunden (siehe 3.2.5).
- Die Verfügbarkeit und Effizienz solcher Aufgaben hängt von:
  - **Vorkommen in der Umgebung** (z. B. Wald vs. Steppe),
  - **Fähigkeiten und Anzahl der zugewiesenen Crewmitglieder**,
  - **Ausrüstung** (Werkzeuge, Jagdwaffen, Werkbänke).

---

## 5. Offene Designfragen (Arbeitsstand)

Die folgenden Punkte sind bewusst noch offen und werden später konkretisiert. Sie stehen **nicht** im Widerspruch zum bestehenden GDD, sondern markieren Stellen für vertiefte Ausarbeitung.

1. **Grid-Typ im Kampf**
   - Toolkit unterstützt sowohl **Quadrat- als auch Hex-Grids**.
   - Entscheidungskriterien:
     - Lesbarkeit bei vielen Einheiten.
     - Passung zum Terrain (z. B. organische vs. klar strukturierte Maps).
     - Gewünschte Bewegungsfreiheit (Diagonalen, Flankierung).
   - Aktueller Status: **„Noch zu entscheiden“**; Prototypen mit beiden Varianten sind denkbar.
2. **Kernressourcen-Fokus**
   - Es soll 1–2 **Schlüsselressourcen** geben, die alle Entscheidungen „erden“ (z. B. Proviant + Moral / Proviant + Medizin / Proviant + Munition).
   - Weitere Ressourcen bauen darauf auf (Spezialausrüstung, Tauschgüter etc.).
   - Aktueller Status: Kandidaten sind identifiziert, finale Auswahl und Gewichtung stehen noch aus.
3. **Detailtiefe der Büro-Ebene**
   - Welche zusätzlichen Interaktionen neben Kartentisch und Berichten (z. B. Forschungsabteilung, Diplomatie mit Auftraggebern, Museums-/Trophäenraum)?
   - Wie stark sollen Spieler\*innen zwischen Expeditionen „im Büro“ beschäftigt sein vs. schnell neue Expedition starten?

---

## 6. Technische Implementierung (Status)

Dieser Abschnitt dokumentiert die bereits implementierten technischen Systeme und Klassenstrukturen.

### 6.1 Expedition System (Task 1)

Das Expeditions-System verwaltet die Daten und den Zustand von Expeditionen.

- **`UFCExpeditionData` (C++ Data Asset)**

  - Erbt von `UPrimaryDataAsset`.
  - Definiert statische Daten einer Expedition: Name, Schwierigkeitsgrad, Basiskosten (Supplies), mögliche Belohnungen.
  - Dient als Konfigurationsbasis für Designer im Editor.

- **`UFCExpeditionManager` (C++ Game Instance Subsystem)**
  - Erbt von `UGameInstanceSubsystem`.
  - Verwaltet die **aktive Expedition** zur Laufzeit.
  - Methoden: `StartExpedition`, `CompleteExpedition`, `GetActiveExpedition`, `IsExpeditionActive`.
  - Persistenz über Levelwechsel hinweg (da GameInstance-gebunden).

### 6.2 Interaction System (Task 2)

Das Interaktionssystem für den Kartentisch basiert auf einem Interface-Ansatz, um C++-Logik mit Blueprint-Flexibilität zu verbinden.

- **`IFCTableInteractable` (C++ Interface)**

  - Definiert die Schnittstelle für alle interaktiven Objekte am Tisch.
  - Methoden (BlueprintNativeEvent):
    - `OnTableObjectClicked`: Reagiert auf Mausklicks.
    - `GetCameraTargetTransform`: Liefert die Kameraposition für den Zoom.
    - `CanInteract`: Prüft Interaktionsbedingungen.
    - `GetWidgetClass`: Liefert die zugehörige Widget-Klasse (falls vorhanden).

- **`BP_TableObject` (Blueprint Base Class)**

  - Implementiert `IFCTableInteractable`.
  - Enthält Basiskomponenten: `StaticMesh` (Visual), `SceneComponent` (CameraTarget), `SphereComponent` (Collision).
  - Dient als Parent für alle spezifischen Tischobjekte.

- **Implementierte Objekte (Child Blueprints)**
  - `BP_TableObject_Map`: Öffnet die Karte (WBP_MapTable).
  - `BP_TableObject_Logbook`: Öffnet das Logbuch (WBP_ExpeditionLog).
  - `BP_TableObject_Letters`: Öffnet den Nachrichten-Hub (WBP_MessagesHub).
  - `BP_TableObject_Compass`: Startet die Expedition (Trigger).
