#!/usr/bin/env python3
"""Lightweight consistency checks for the GDPS Full Rater source tree."""
from __future__ import annotations
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)

try:
    mod = json.loads((ROOT / "mod.json").read_text(encoding="utf-8"))
    difficulties = json.loads((ROOT / "difficulties.json").read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError) as exc:
    fail(f"Could not read project JSON: {exc}")

cpp = (ROOT / "src/DevRateStarsPopup.cpp").read_text(encoding="utf-8")
php_set = (ROOT / "server-addon/cvolton/database/gdpsFullRaterSet.php").read_text(encoding="utf-8")
php_get = (ROOT / "server-addon/cvolton/database/gdpsFullRaterGet.php").read_text(encoding="utf-8")
hook = (ROOT / "src/hooks/LevelInfoLayer.cpp").read_text(encoding="utf-8")
utils = (ROOT / "src/utils/Utils.cpp").read_text(encoding="utf-8")

json_tiers = difficulties.get("tiers", [])
json_keys = [item.get("key") for item in json_tiers]
if len(json_keys) != len(set(json_keys)):
    fail("Duplicate difficulty keys in difficulties.json")

ui_keys = set(re.findall(r'\{"([a-z0-9-]+)",\s*"[^"]+",\s*"difficulty_[^"]+"\}', cpp))
if set(json_keys) != ui_keys:
    fail(f"UI keys differ from difficulties.json: json={set(json_keys)}, ui={ui_keys}")

match = re.search(r"\$tiers\s*=\s*array\((.*?)\);", php_set, re.S)
if not match:
    fail("PHP tier allow-list was not found")
php_keys = set(re.findall(r"'([a-z0-9-]+)'\s*=>", match.group(1)))
expected_php = set(json_keys) - {"unrated"}
if php_keys != expected_php:
    fail(f"PHP keys differ from UI keys: php={php_keys}, expected={expected_php}")

for required in ("gdpsFullRaterGet.php", "gdpsFullRaterSet.php", "GJP2", "commandRate"):
    haystack = cpp + php_set + php_get
    if required not in haystack:
        fail(f"Required API/auth reference missing: {required}")

if "Rate Button Warning" not in hook or "onSaveTier" not in cpp:
    fail("M336 warning or custom-tier save action missing")
if ".supabase.co" not in utils:
    fail("Backend guard for accidental Supabase URL configuration missing")
if not (ROOT / "LICENSE-M336").exists():
    fail("M336 license notice missing")

print(f"OK: {mod.get('id')} {mod.get('version')}")
print(f"OK: {len(json_tiers)} UI difficulty entries; PHP allow-list matches")
print("OK: M336 warning, PHP GET/SET endpoints, and GDPS auth references present")
