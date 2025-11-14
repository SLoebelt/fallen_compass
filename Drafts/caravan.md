Sehr gut, das macht das Ganze noch klarer und „spielbarer“ im Kopf 🔧
Ich baue deine Ergänzungen direkt ins Konzept ein und formuliere es so, dass wir daraus später 1:1 ein GDD ableiten können.

---

## 1. Überblick – Wander-Enklave (aktualisiertes Konzept)

**Arbeitstitel:** _Wander-Enklave_

Du führst eine kleine Gruppe von 5–10 Personen durch eine gefährliche Welt. Ihr bewegt euch als Konvoi über eine **verdeckte Weltkarte**, schlagt unterwegs Lager auf, trefft andere Gruppen, Tiere und Siedlungen und geratet gelegentlich in Kämpfe.

Der Spieler **spielt immer den aktuellen Gruppenführer**.
Die anderen Gruppenmitglieder handeln in Echtzeit entsprechend der **tagesaktuellen Rolle**, die du ihnen gibst.

Kern-Säulen:

- **Drei Gameplay-Phasen**

  - _Kampf_ – rundenbasiert
  - _Wanderung_ – Echtzeit (ähnlich Wartales/Project Zomboid)
  - _Lager/Dorf/POI_ – Echtzeit in begrenztem Bereich (Einzelkarte/Level)

- **Tagesrollen & Persönlichkeit**

  - Rollen werden **tagesweise** vergeben (nicht dauerhaft).
  - Persönlichkeit & Fähigkeiten bestimmen, wer für welche Rolle geeignet ist.
  - Falsche Zuordnung → schlechtere Effektivität, Stimmungseinbrüche, Beziehungsschäden.

- **Dialog- & Eventsystem**

  - Dialogfenster für Gespräche mit Gruppenmitgliedern, NPCs, Dorfleuten.
  - Eventfenster für interne Gruppenereignisse, Random-Events, Begegnungen.

- **Erkundung & Fog of War**

  - Karte ist anfangs komplett verdeckt.
  - Bewegen des Konvois & Expeditionen im Lager decken Stück für Stück die Welt auf.

---

## 2. Die drei Gameplay-Phasen

### 2.1 Kampf – rundenbasiert

- Wird ausgelöst durch:

  - Überfälle, Jagden, Arena-Kämpfe, scheiternde Verhandlungen, bestimmte Events.

- Läuft in **Runden**:

  - Jede Figur führt Aktionen aus (Bewegen, Angreifen, Fähigkeiten einsetzen).
  - Positionierung, Initiative, Reichweite & Gelände spielen eine Rolle.

- Fähigkeiten hängen ab von:

  - Rolle (z. B. Vorhut, Heiler, Tieraufseher),
  - Ausrüstung,
  - evtl. Traits/Erfahrung.

**Permadeath:**

- Stirbt eine Figur, bleibt sie tot.
- Stirbt der Gruppenführer, musst du **einen neuen Anführer aus der Gruppe bestimmen**.

---

### 2.2 Wanderung – Echtzeit (Overworld)

- Übergeordnete Ansicht (z. B. isometrische oder „Weltkarte“-Ansicht).

- Du steuerst den Konvoi in **Echtzeit** über Pfade, Wege, Gelände.

- Während der Wanderung:

  - Ressourcen ticken (Proviant, Wasser, ggf. Ausdauer der Tiere).
  - Zufalls- und geplante Events können auftauchen:

    - Begegnung mit anderen Gruppen/NPCs
    - Sichtung von Tieren (Jagdchance/Gefahr)
    - Umgebungshinweise (z. B. Rauch in der Ferne, Spuren im Boden)

- **Fog of War:**

  - Bereiche, die du noch nicht besucht hast, sind verdeckt.
  - Bewegung deckt einen Radius um den Konvoi auf.

---

### 2.3 Lager / Dorf / POI – Echtzeit auf lokaler Karte

Wenn du lagerst oder einen Ort betrittst (Dorf, Lager, POI), wechselst du auf eine **lokale Map**:

- Begrenzter Bereich einsehbar (z. B. dein Camp, Dorfplatz, Arena, Marktplatz).

- Echtzeit-Steuerung:

  - Du bewegst den Gruppenführer.
  - Andere Gruppenmitglieder agieren halb-autonom nach ihren Rollen (oder auf Befehl).

- Im **Lager**:

  - Tagesrollen festlegen.
  - Aufgaben: Holz sammeln, Wasser holen, Tiere versorgen, Wache halten, reparieren.
  - Expeditionstrupps planen (Zielbereich auf der Overworld wählen, Proviant & Ausrüstung zuweisen).

- In **Dörfern/POIs**:

  - Taverne: Dialoge, Gerüchte, mögliche Rekruten, soziale Events.
  - Marktplatz: Handel mit Waren, Waffen, Rüstungen, Werkzeug, Proviant.
  - Arena: optionale Kämpfe (Belohnungen, Risiko).
  - Lager anderer Gruppen: Handel, Bündnisse, Überfälle oder ignorieren.

---

## 3. Rollen & Tageszuweisung

### 3.1 Rollen sind **tagesweise**, nicht dauerhaft

Am Beginn eines Tages (oder Lagers) legst du fest, wer welche Aufgabe übernimmt, z. B.:

- **Vorhut** (Scouting, erhöhte Chance, Gefahren früh zu sehen)
- **Nachhut** (Sicherung, weniger Überfälle von hinten)
- **Tieraufseher** (Handling von Zug-/Packtieren, höhere Reiseeffizienz, weniger Verluste)
- **Proviantverwalter/Essenstransport** (weniger Verschwendung, besseres Rations-Management)
- **Wache** (Lager-Sicherheit, geringere Überfallchance im Camp)
- **Heiler** (bessere Regeneration, Behandlung von Verletzungen/Krankheiten)
- **Handwerker** (Reparaturen, simples Crafting/Upgrades)

### 3.2 Persönlichkeit & Fähigkeiten beeinflussen Eignung

Jede Figur hat:

- **Traits** (z. B. fleißig/faul, ängstlich/mutig, empathisch/hart, diszipliniert/chaotisch).
- **Fähigkeitenwerte** (z. B. Tiere, Medizin, Handwerk, Wahrnehmung).

Zuweisung einer Rolle:

- **Gute Eignung** (Traits + Skills passen):

  - Hohe Effektivität (z. B. bessere Reisegeschwindigkeit, bessere Ressourcenausbeute, geringeres Risiko).
  - Positive Stimmung („Ich fühle mich gebraucht.“).

- **Mittelmäßige Eignung**:

  - Normale Effektivität, keine großen Moodänderungen.

- **Schlechte Eignung** (z. B. ängstlicher Typ als Vorhut, fauler an harter Arbeit):

  - Schlechtere Effektivität (Fehler, Pannen, Risiken steigen).
  - Stimmung sinkt → Figur ist frustriert/genervt.
  - **Beziehung zum Gruppenführer leidet** („Der weiß gar nicht, was ich kann/will.“).

Das schafft einen Spannungsbogen:
Manchmal MUSST du einen schlechten Fit wählen (weil niemand anderes kann) und trägst dann die sozialen & effektiven Konsequenzen.

---

## 4. Dialoge & Events

### 4.1 Dialogfenster

- Klassische Dialogboxen mit:

  - Figurenporträt, Name, Stimmungshinweis.
  - Text + Auswahlantworten für den Gruppenführer.

- Einsatzbereiche:

  - Gespräche mit Gruppenmitgliedern im Lager/Unterwegs/Dorf.
  - Verhandlungen/Handel mit NPCs.
  - Sidequests/Storyentscheidungen.

Entscheidungen in Dialogen:

- beeinflussen **Mood**,
- verändern **Beziehung** zwischen Figur und Anführer,
- können **Traits verstärken oder abschwächen** (mit zunehmendem Fortschritt).

### 4.2 Eventfenster

- Kurzformat für kleinere Vorfälle:

  - „X und Y hatten Streit um Z.“
  - „Tier ist verletzt.“
  - „Unbekannte Spuren nahe des Lagers.“

- Du triffst 1–3 Entscheidungen:

  - „Ignorieren“, „Schlichten“, „Bestrafen“, „Belohnen“, etc.

- Diese Events sind an Traits, Rollen, Mood und manchmal aktuelle Ressourcenlage geknüpft.

---

## 5. Erkundung & Expeditionen (Fog of War)

- **Gesamte Karte** ist zu Beginn verborgen.
- **Konvoi-Bewegung**:

  - Deckt einen Sicht-Radius um den Konvoi herum auf.

- **Expeditionen vom Lager**:

  - Im Lager kannst du 1–3 Figuren auf Erkundung schicken.
  - Du klickst auf einen Zielbereich auf der Karte und planst eine Route / ein Ziel.
  - Expeditionen verbrauchen Proviant & Ausrüstung.
  - Sie decken weiter entfernte Bereiche auf, finden Ressourcen, Hinweise, Orte – oder geraten in Gefahr (Events/Kämpfe).

Expeditionen sind ein wichtiger Hebel für:

- Risiko/Ertrag-Entscheidungen.
- Charakterentwicklung (wer stellt sich der Gefahr, wer versagt, wer wächst daran?).

---

Damit sind deine neuen Punkte vollständig im Konzept verankert:

- **Rollen = täglich neu vergeben, mit Einfluss von Persönlichkeit & Fähigkeiten**
- **3 klar getrennte Phasen: rundenbasierter Kampf, Echtzeit-Wanderung, Echtzeit-Lager/POI**
- **Dialog- & Eventsystem als zentrales Tool für Story, Stimmung & Beziehungen**
- **Vollständige Fog-of-War-Erkundung via Konvoi + Expeditionen**

Wenn du magst, kann ich jetzt im nächsten Schritt direkt das **vollumfängliche Game Design Document** für _Wander-Enklave_ schreiben – mit:

- Feature-Liste pro System
- klarer Scope-Definition für Phase 1 (Prototyp), Phase 2 (Demo) und Phase 3 (Early Access)
- und einer sinnvollen, schlanken ersten Version, die du in Unreal Schritt für Schritt umsetzen kannst.
