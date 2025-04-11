## 📦 gps_timefix v1.0.0 — Initial public release

This is the first public release of the `gps_timefix` module for Earthworm.

### 🔧 Features

- Reads `TYPE_TRACEBUF2` messages from a shared memory ring
- Replaces faulty GPS-based timestamps with local system time
- Maintains continuity per channel (STA.CHAN.NET.LOC)
- Avoids gaps and overlaps
- Outputs corrected data to a separate ring

### 📄 Files included

- `gps_timefix.c` — Core source code
- `makefile.unix` — Linux Makefile
- `gps_timefix.d` — Example configuration file
- `README.md`, `LICENSE`, `VERSION`, `changelog.txt`

### 🛠 Compatibility

- Earthworm under Linux environments
- Tested with `sniffwave` and real-time tracebuf2 streams

---

**Author:** Ricardo Sifon  
**License:** MIT  
**Repository:** [https://github.com/ricardosifon/gps_timefix](https://github.com/ricardosifon/gps_timefix)

Feel free to fork, contribute or suggest improvements. This module is intended to support robust seismic data acquisition in environments with unreliable GPS time.
