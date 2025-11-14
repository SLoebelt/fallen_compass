8-Wochen-Plan für Phase 1 (Prototyp)

inkl. Marketplace-Empfehlungen & Level-Konzept

Ziel von Phase 1 (8 Wochen)

Ein spielbarer Prototyp mit:

Büro/Planungsszene

Einer kleinen Testregion (Overworld)

Einem einfachen Lager-Level

Basic-Rollenvergabe

4–6 Charakteren (wie oben)

~10 Events (siehe oben)

Einfache rundenbasierte Kämpfe (Räuber/Tiere)

Und: Du bleibst unter ~100–150 € Asset-Kosten für diese Phase (2 Monate).

Empfohlene Asset-Basis (Phase 1)

Priorität 1 – Innenräume (Büro & Lager-Hütten)

Low Poly Medieval Interior and Constructions – modularer, topdown-tauglicher Innenraum-/Tavernen-/Möbelpack inkl. Waffen.

Eignet sich perfekt, um:

dein Büro/Kontor zu bauen,

einfache Innenräume/Lager zu gestalten.

Priorität 2 – Außenumgebung (Dorf/Küstenregion)

Medieval Village Environment – realistisches mittelalterliches Dorf-/Umgebungsset, UE5-kompatibel.

Gut für:

erste Dorf-/POI-Szenen,

kleine Stücke Overworld (du kannst Teilareale für Prototyp verwenden).

Charaktere:
Für Phase 1 kannst du:

zunächst mit UE-Mannequins oder MetaHumans arbeiten (minimal),

Charakter-Packs wie Medieval Characters oder NPC Characters Female (Pack) später für Phase 2 einkaufen, wenn der Prototyp sitzt.

Preise schwanken und sind oft rabattiert; viele mittelalterliche Packs liegen grob im Bereich 15–60 € pro Pack, abhängig von Umfang und Sale.

Budget-Vorschlag für Phase 1:

Monat 1:

1x Low Poly Medieval Interior and Constructions

Monat 2:

1x Medieval Village Environment

Damit bist du voraussichtlich im Rahmen von ~80–120 € in zwei Monaten (abhängig von Sales/Steuern).

Level-/Map-Konzept für Phase 1

1. Büro/Planungszimmer (Hub-Level)

Kleine Interior-Map:

Schreibtisch, Kartentisch, Regale, Fenster mit Blick auf die Stadt/Hafen (Fake-BG).

Interaktive Hotspots:

Kartentisch → öffnet Overworld-/Vertrags-UI.

Briefstapel → öffnet Depeschen/Vertrags-/Rekruten-UI.

Tür → „Start Expedition“ → Ladebildschirm → Overworld.

Für Phase 1 reicht ein Raum + UI völlig.

2. Overworld-Testkarte (Region 01 – Küstenstreifen & Inland)

Simple, eher kleine Map:

Küstenlinie / Flussmündung

Ein Waldstreifen

1 Hügel / Landmarke

Konvoi als Icon, der:

sich auf Pfaden oder frei über Gelände bewegt,

Fog-of-War im Umkreis aufdeckt.

In Phase 1 reicht eine kompakte Region (z. B. 512x512m oder ähnlich) zum Testen des Loops.

3. Lager-Level (Camp)

Klein, rundum begrenzt:

2–3 Zelte, 1 Wagen, 1 Feuerstelle, 1 Tierbereich.

Interaktive Spots:

Feuer → Tagesende/Schlafen.

Wagen/Tiere → Logistik/Transport-UI.

Zelt → Rollenvergabe/Party-UI.

Du kannst in Phase 1 nur ein Lager-Layout haben und später varianten hinzufügen.

4. Einfaches Dorf-/POI-Level (optional gegen Ende Phase 1)

Kleines Dorfstück:

1 Platz mit Taverne & Marktstand (Innenräume optional leer).

Nutzung:

Für einen oder zwei Events (z. B. „Kranker im Dorf“, „zwielichtiger Händler“).

Kann aus Medieval Village Environment modular zusammengebaut werden.

Wenn Zeit knapp wird, kannst du das Dorf auch auf Phase 2 verschieben.

5. Kampfarena (kleine Taktik-Map)

Sehr simple Kampfarena:

Ein Waldrand oder Lichtung, ein paar Steine/Bäume als Deckung.

Wird reused für:

Räuberüberfall

Jagd-Event

Layout: klein halten, Fokus auf Funktionsfähigkeit des Turn-Systems, nicht auf Optik.

8-Wochen-Plan (konkret)

Woche 1 – Setup & Büro-Greybox

UE5.6 Projekt aufsetzen, Source Control einrichten.

Büro-Level grob in Greybox bauen (Wände, Tisch, Dummy-Meshes).

Erste Kamerasteuerung & Interaktion (z. B. E drücken → UI auf).

Woche 2 – Daten & Grund-UI

Data-Strukturen für:

Charakter (Name, Traits, Stats, Inventar).

Vertrag/Expedition.

Event (Text, Optionen, Outcomes).

Simple UI-Prototypen:

Charakterliste.

Vertragsliste.

Basic-Dialogfenster.

Woche 3 – Overworld-Prototyp & Fog of War

Overworld-Testmap blockout (Landschaft + 1–2 Landmarken).

Konvoi-Icon + Klick-Bewegung in Echtzeit.

Simpler Fog-of-War (z. B. Material/Decal oder per Grid getrackt).

Mini-Prototyp: Aus Büro → Overworld laden → hin und her laufen.

Hier kannst du Low Poly Medieval Interior and Constructions holen und Büro hübscher machen, wenn du magst.

Woche 4 – Lager-Level & Tagesrolle-MVP

Camp-Level blockout (Feuer, Zelte, Wagen, Tiere).

Wechsel von Overworld → Lager und zurück.

Tagesrollen-System MVP:

Rollen als enum + UI,

Zuweisung pro Charakter,

einfacher Effekt (z. B. log-Ausgabe „+10% Holz gesammelt“).

Woche 5 – Eventsystem & 6–8 der Eventcards umsetzen

Generisches Eventsystem:

Event-Datenstruktur → UI → Verarbeiten von Outcomes.

6–8 der oben gelisteten Events implementieren (Mood, Ressourcen, Flags).

Erste Pass-Integration in Wanderung/Lager (z. B. Timer/Trigger).

Hier kannst du Medieval Village Environment holen und schon mal ein kleines Dorfstück / zusätzliche Szenerie für Tests bauen.

Woche 6 – Basic Kampf-MVP

Kleine Arena-Map bauen.

Turn-Order-System (z. B. simple Reihenfolge: alle Spieler, dann alle Gegner).

Grundaktionen:

Bewegen, einfacher Angriff, End Turn.

Ein Gegnertyp (Räuber oder Wolf).

Anbindung an Events:

Z. B. „Nächtliche Geräusche“ → Kampf starten.

Woche 7 – Charakter-Templates & Mood/Traits-Hooks

Die 6 Starter-Charaktere als Data-Einträge anlegen.

Traits & Mood erste Auswirkungen:

z. B. Mood beeinflusst Event-Outcomes leicht.

Traits beeinflussen Chancen/Outcomes bei bestimmten Events.

Basic-Visualisierung im UI (Icons/Farbcodes).

Woche 8 – Stabilisierung & Vertikalschnitt

Crash-/Bugfixing, kleine UX-Verbesserungen.

Ziel: „Vertical Slice“:

Büro → Expedition planen → Overworld laufen → Lager aufschlagen → 1–2 Events → 1 Kampf → zurück ins Büro (auch wenn Rückweg noch gecheatet abgekürzt wird).

Interne Test-Session: 2–3 Leute spielen lassen, Feedback zu Flow & Verständlichkeit holen.
