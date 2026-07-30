# Apps in CrosspointTH 1.6

The Apps menu is deliberately isolated from the reader, boot, display, HAL, partition, and OTA paths. All app data is
stored on the SD card. The apps use the existing physical-button mapping and e-paper theme metrics.

## Clock and Thai calendar

- Shows the device clock and a monthly calendar.
- Thai UI uses Thai month/day labels and Buddhist Era years (`ค.ศ. + 543`). Other languages use Gregorian years.
- Left/Right changes month; Confirm returns to the current month.
- If the time is not valid, sync it under Settings before using the calendar.

## Weather

- Uses the account-free Open-Meteo current-weather API over HTTPS.
- Left/Right selects one of eight Thai locations; Confirm refreshes explicitly.
- The last successful result is cached at `/.crosspoint/weather-v1.bin` on SD and Wi-Fi is switched off on exit when
  Weather started it.
- No location, account, or IP-address data is sent anywhere except the selected coordinates in the Open-Meteo request.

## Pomodoro

- Default cycle: 25-minute focus, 5-minute short break, and a 15-minute long break every four sessions.
- Confirm starts/pauses/resumes; Right skips to the next phase.
- The display refreshes only when the shown minute changes. Auto-sleep is blocked only while the timer is running.

## Flashcards

Create `/flashcards` on the SD card and copy UTF-8 CSV decks into it. The first row must contain either `front,back` or
`front_content,back_content`. For example:

```csv
front,back
สวัสดี,hello
ขอบคุณ,thank you
"หนึ่ง, สอง, สาม","one, two, three"
```

Quoted commas and doubled quotes are supported. A line may contain at most 767 bytes, a deck at most 2,000 cards, and
up to 64 decks are listed. Cards are streamed from SD rather than loaded as a whole deck. Review state is stored beside
the deck as `<deck>.csv.srs`; deleting that file resets progress. Do not edit a deck while reviewing it.

The four ratings are Again, Hard, Good, and Easy. Scheduling uses a compact SM-2-style interval/ease record and writes
only the reviewed card's small fixed-size record.

## Games

- 2048: directional buttons move the board.
- Sudoku: move the cursor and Confirm cycles the selected editable cell.
- Minesweeper: Confirm reveals; hold Confirm to place or remove a flag.
- Caro: move the cursor and Confirm places X; the lightweight opponent then places O.
- Thai Draughts (หมากฮอสไทย): eight pieces per side, mandatory forward captures, multi-capture,
  promotion, and long-range kings that land immediately behind a captured piece. Confirm selects and moves a piece.

These games intentionally avoid animation, networking, saved replays, and large assets to suit the ESP32-C3 and e-paper
display. They are casual implementations rather than tournament-grade chess or puzzle engines.

## Calendar and weather sleep screen

Select `Calendar + Weather` under the Sleep Screen setting to retain a dashboard during deep sleep. It shows the time,
Thai Buddhist calendar when the interface language is Thai, battery percentage, and the last cached weather reading.
On X3, the device wakes briefly at five-minute clock boundaries, redraws without starting Wi-Fi, and immediately returns
to deep sleep. Refresh Weather before sleeping when current data is important. X4 keeps the dashboard static because its
battery latch removes MCU power during deep sleep.
