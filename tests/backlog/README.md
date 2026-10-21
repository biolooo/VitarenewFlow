# Backlog checks

From the repository root, in a Linux host environment with GCC, zlib headers and
UBSan:

```sh
bash tests/backlog/run-host.sh
python3 scripts/check-component-packages.py
python3 -m unittest discover -s tests/backlog -p 'test_*.py'
```

The banner/library test links the actual `banner_format.c` and `library_model.c`.
It uses only synthetic compressed textures and names. A missing-cache assertion
does not exercise the GPU/player fallback: real display quality, worker timing,
automatic banner generation, a full 1024-title scan and console FPS remain
separate hardware checks. The 1024-entry test checks distinct model cover paths.

The package checker reads local manifests and payloads only. It checks exact
manifest sizes/SHA256, bounded local basenames, unique SD targets, expected HTTPS
URL mapping and named nonempty source/notice companions. It does not inspect or
approve licenses, corresponding-source completeness or binary reproducibility.
Its error-path tests mutate only temporary synthetic data.
