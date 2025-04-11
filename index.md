---
title: gps_timefix
---

# gps_timefix — Earthworm Module

**Author:** Ricardo Sifon  
**Version:** 1.0.0  
**License:** MIT  
**Repository:** [GitHub](https://github.com/ricardosifon/gps_timefix)

---

`gps_timefix` is an Earthworm module that corrects `TYPE_TRACEBUF2` messages affected by GPS failure by replacing timestamps with continuous local system time.

### 🔧 Features

- Replaces `starttime` and `endtime` in tracebuf2 messages
- Maintains continuity per channel
- Outputs corrected messages to a new shared ring
- Fully compatible with Earthworm on Linux

---

### 📦 Resources

- [README](https://github.com/ricardosifon/gps_timefix/blob/master/README.md)
- [Download Release](https://github.com/ricardosifon/gps_timefix/releases)
- [LICENSE](https://github.com/ricardosifon/gps_timefix/blob/master/LICENSE)
