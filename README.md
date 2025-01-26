# Saturn Save Tools

A set of command-line tools for managing SEGA Saturn single save files directly from the PC. (Windows/Linux/macOS)
- `ss-converter`
- `sgm-unpacker`
- `sst`

## Saturn Save Converter Tool

This command-line tool converts single Saturn save files to `BUP` format (Pseudo Saturn Kai) and SAROO format (`SSAVERAW` single save).

### Supported formats

- `.BUP`: Pseudo Saturn Kai
- `.bin`: SAROO (SSAVERAW)
- `.XML`: SS Backup RAM Parser
- `.B64`: Save Game Manager
- `.CMS`: PC Comms Link

### Output formats

- `.BUP`: Pseudo Saturn Kai (Default)
- `.bin`: SAROO single save

### Usage

```
SEGA Saturn save converter 0.1.0 - (c) 2025 by Bucanero

USAGE: ./ss-converter filename [-s]

Opt:	-s	Convert to SAROO single save format
```

**Example:**

```
% ./ss-converter HANGONGP_01.bin

SEGA Saturn save converter 0.1.0 - (c) 2025 by Bucanero

[*] Loading HANGONGP_01.bin...
    > Type:	 SAROO (SSAVERAW)

[*] Exporting HANGONGP_01.BUP...
    > Format  :	 BUP (Pseudo Saturn Kai)
    > Filename:	 HANGONGP_01
    > Comment :	 HANGONGP95
    > Language:	 JP
    > Date:    	 1997-07-13 01:17
    > Size:    	 1692 bytes

[i] Successfully exported save file.
```

---

## SGM Unpacker Tool

This command-line tool extracts single Saturn save files packed in `SGM` archives created with [Save Game Manager](http://www.rockin-b.de/saturn-savegamemanager.html).

### Usage

```
SEGA Saturn SGM save unpacker 0.1.0 - (c) 2025 by Bucanero

USAGE: ./sgm-unpacker filename
```

**Example:**

```
% ./sgm-unpacker SGMSEGARALL

SEGA Saturn SGM save unpacker 0.1.0 - (c) 2025 by Bucanero

[*] Unpacking SGMSEGARALL...
[i] Packed Files: 2
    > SEGARALLY_0.BUP 	 1994-01-01 00:49 	 5056 bytes 	 OK!
    > SEGARALLY_1.BUP 	 1994-01-01 00:20 	 62464 bytes 	 OK!
[i] Successfully Unpacked 2 files.
```

## Building the source code

```
make
```

## Credits

- XML save format - from SS Backup RAM Parser by [hitomi2500](https://github.com/hitomi2500/ss-save-parser)
- BUP save format - from Pseudo Saturn Kai by [cafe-alpha](https://github.com/cafe-alpha/pskai_wtfpl)
- SAROO save format - from SAROO by [tpunix](https://github.com/tpunix/SAROO)
- B64/SGM save formats - from Save Game Manager by [Rockin'-B](http://www.rockin-b.de/saturn-savegamemanager.html)

## License

This software is licensed under GNU GPLv3, please review the [LICENSE](https://github.com/bucanero/saturn-save-tools/blob/main/LICENSE)
file for further details. No warranty provided.
