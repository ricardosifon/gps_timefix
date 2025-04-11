# gps_timefix — Earthworm Module for Timestamp Correction

`gps_timefix` is an Earthworm module that corrects `TYPE_TRACEBUF2` messages with faulty or missing timestamps (commonly due to GPS failure). It replaces the `starttime` and `endtime` fields with locally generated system time, maintaining continuity and avoiding gaps or overlaps.

---

## 📌 Project Presentation

**Author:** RICARDO SIFON  
**License:** MIT  
**Repository:** [https://github.com/ricardosifon/gps_timefix](https://github.com/ricardosifon/gps_timefix)

### 🔧 What does it do?

- Reads `TYPE_TRACEBUF2` messages from a shared memory ring
- Detects channel identity (STA.CHAN.NET.LOC)
- Replaces timestamps with local system time
- Ensures continuity per channel (no gaps or overlaps)
- Outputs corrected messages to a new ring

Tested using `sniffwave` and multiple real-time data streams.

---

## 🔨 Installation

1. Copy `gps_timefix.c` to:
   ```
   src/diagnostic_tools/gps_timefix/
   ```

2. Copy `makefile.unix` into the same folder.

3. Compile:
   ```bash
   make -f makefile.unix
   ```

4. The binary will be placed in:
   ```
   bin/gps_timefix
   ```

---

## ⚙️ Configuration (`gps_timefix.d`)

Example configuration file:

```ini
MyModuleId          MOD_GPS_TIMEFIX
RingName            GPS_IN_RING
OutRingName         GPS_OUT_RING
HeartBeatInterval   30
LogFile             2

GetEventsFrom       INST_WILDCARD MOD_WILDCARD TYPE_TRACEBUF2
```

Place this file in:
```
run/gps_timefix.d
```

Update `earthworm.d`:
```ini
Ring    GPS_IN_RING     512
Ring    GPS_OUT_RING    512

Module  MOD_GPS_TIMEFIX "gps_timefix"
```

And `startstop`:
```ini
Module  gps_timefix  Start  /home/admin/earthworm/bin/gps_timefix  /home/admin/earthworm/run/gps_timefix.d
```

---

## 🔍 Verification

Use `sniffwave` to inspect corrected output:
```bash
sniffwave -r GPS_OUT_RING
```

---

## 🛠 Dependencies

Link the following Earthworm libraries:
- `logit`
- `kom`
- `transport`
- `util`

With:
```bash
-lkom -llogit -ltransport -lutil -lm
```

---

## 📄 License

MIT License — see `LICENSE` file.

---

## 📋 Changelog

See `changelog.txt` for detailed version history.

---

## 🤝 Contributing

Suggestions, improvements, and pull requests are welcome!
Feel free to fork and contribute.

---

## 📣 Community

If you are part of the Earthworm development community or users group, feel free to test, reuse, and share feedback. This module is intended to support robust seismic data acquisition in environments where GPS-based timekeeping is unreliable.

Thanks for checking it out!

— Ricardo Sifon
