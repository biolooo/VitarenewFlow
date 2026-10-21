# Preserved release fixtures

Imported byte-for-byte from the local 0.12.33 source archive during migration.
Historical .sh/.py files may contain legacy paths or generators: do not run
them indiscriminately. Use `bash scripts/verify-local.sh` from the repository.
The runner copies this checkout into fresh SDK staging and runs selected existing
fixtures. Source anchors reject drift in copied component/launch functions.
Other copied fixture logic is historical and is not full hardware coverage.
TWL tests mock NAND operations; no console or real NAND is accessed.
Private music fixtures are excluded. Native build and host checks do not establish
HOME/POWER, audio output, cartridge or DS compatibility on a console.
