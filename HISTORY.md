# Project history

This repository was rebuilt on 2026-10-22. The working tree was imported as one
commit on top of the existing 2026-07-23 history. The dates below come from the
author's session archives and build folders, not from git.

| Date | What happened |
|---|---|
| 2026-07-19 | First working UI pass: skin, star field, fader, VST3 skeleton |
| 2026-07-20 | GitHub repository created |
| 2026-07-23 | UI series 01–14 committed: cockpit frame, fader, response plot, three overlays, correction curve lock |
| 2026-08 – 2026-09 | Work moved to a separate product; this plugin was frozen |
| 2026-10-15 | HONESTMIX Connect line started (UI-01 / UI-03) |
| 2026-10-16 | Headphone acceptance pass, Connect 0.2.1, macOS installer |
| 2026-10-22 | Public source at 0.6.12; measurement data restricted to the 517 cleared profiles |

## Why there is no commit-per-day history

Honest answer: git does not carry the July–October development record.

The source tree was copied between machines, and the copy reset every file's
modification time to 2026-10-20. Reconstructing a commit per day from those
timestamps would have meant inventing dates, so it was not done. What exists is
what actually happened:

- `41a7011` and its 13 ancestors — the 2026-07-23 UI series, committed the day
  it was written.
- `9ddbb09` — the 2026-10-22 import of the 0.6.12 tree.
- `873162b` — the same day, restricting measurement data to the profiles cleared
  for redistribution (see [DATA-RIGHTS.md](DATA-RIGHTS.md)).

A reader who wants to judge activity should look at the content, the test suite
and the data-rights boundary rather than at a commit graph that would have been
easy to fabricate and hard to verify.
