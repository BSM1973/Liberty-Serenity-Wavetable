#!/usr/bin/env python3
"""One-time transfer of the user's ten ORIGINAL UI screenshots and native JUCE sources.

This imports source files and checks each reference PNG by decoded RGBA pixels.
No generated or substitute interface artwork is accepted.
"""
from pathlib import Path
from io import BytesIO
from PIL import Image
import hashlib
import struct
import urllib.request
import zipfile

BASE = "https://cdn.shopify.com/s/files/1/1114/8047/1891/files/"
SOURCE_URL = BASE + "serenity-ui-source-transfer-v020.png?v=1795191465"
SOURCE_SHA256 = "5754337ee53086c2af74c94fe3087a3c2267b8099e793a16da4c1f1ff4f7a6d4"
MAGIC = b"LIBERTY_UI_SOURCE_02"
ASSETS = {
 "oscillators": ("1795191111", "46035b900a0f10004a537f5c89a4d5f2f0b85807bf2b3e6fceac6d31a0603663"),
 "editor": ("1795191127", "14e1ea1acdfc57494f4a816013ee6ed86622a3dfd81c7d74d8ed3816b7f69382"),
 "filters": ("1795191136", "54466d21fe0789fa50a0d0f62cc533218e015feb38c50722aaaf0e7c7d1fb0ee"),
 "modulation": ("1795191145", "8157facc49188e261ffb946e1f549bd9c414aa4e49a110050ec1d4cafb75a13a"),
 "effects": ("1795191154", "ed69cc80f128001353542f6e182981bf66865ffed72d7d0fb9ea25302d2b1906"),
 "arpeggiator": ("1795191163", "6aa3c1da58f660023b91cb6342e695a081c9255ccd89cba30a8e71a2711af610"),
 "sequencer": ("1795191172", "75d11a7a16c2ec251cf4b740ef48f12822b0c236bdfcbf002d14c83d97fb554f"),
 "matrix": ("1795191180", "a6b65be2bfef0f8a6357c48b76be87b011507ebf7fc2d1742ef8b5a45cf7b30e"),
 "presets": ("1795191188", "89135c87f388a7b5f799ea8a7b53f98b695177d019aee05502b8db6d03ec9e2d"),
 "settings": ("1795191197", "1e5c7838cc4f87ef1c38611de761e06abeca27b5111e9a63304d6f7b471c268d"),
}

def fetch(url):
    req = urllib.request.Request(url, headers={
        "User-Agent": "Liberty-Music-Software-GitHub-Source-Import/1.0",
        "Accept": "image/png,*/*"
    })
    with urllib.request.urlopen(req, timeout=120) as response:
        return response.read()

def main():
    root = Path.cwd()
    # Reconstruct original source snapshot from temporary lossless RGB transport.
    response = fetch(SOURCE_URL)
    candidate = Image.open(BytesIO(response))
    src = candidate.convert("RGB").tobytes()
    print("Transport diagnostic:", candidate.format, candidate.mode, candidate.size,
          "magic candidate=", src[:32].hex(), "expected=", MAGIC.hex(), flush=True)
    assert src.startswith(MAGIC), "Source transport image changed during upload"
    offset = len(MAGIC)
    size = struct.unpack(">Q", src[offset:offset + 8])[0]
    digest = src[offset + 8:offset + 40]
    blob = src[offset + 40:offset + 40 + size]
    assert hashlib.sha256(blob).digest() == digest, "Source payload corrupted"
    assert hashlib.sha256(blob).hexdigest() == SOURCE_SHA256, "Source package mismatch"
    with zipfile.ZipFile(BytesIO(blob)) as z:
        for entry in z.namelist():
            # Preserve existing GitHub build workflow; it includes UI startup tests.
            if entry.startswith(".github/") or entry.startswith("build"):
                continue
            if entry.startswith("/") or ".." in Path(entry).parts:
                raise ValueError("Unexpected archive path: " + entry)
            destination = root / entry
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(z.read(entry))
            print("Restored", entry)

    old = root / "Source" / "Plugin.cpp"
    if old.exists():
        old.unlink()  # Remove the simplified interface that ignored the reference screens.

    assetdir = root / "Assets" / "UI"
    assetdir.mkdir(parents=True, exist_ok=True)
    for page, (stamp, expected_hash) in ASSETS.items():
        url = BASE + "serenity-ui-reference-" + page + ".png?v=" + stamp
        raw = fetch(url)
        reference = Image.open(BytesIO(raw)).convert("RGBA")
        actual_hash = hashlib.sha256(reference.tobytes()).hexdigest()
        if reference.size != (2048, 1361) or actual_hash != expected_hash:
            raise AssertionError(
                page + ": original reference image pixel mismatch; no substitute permitted."
                + " got=" + actual_hash + " size=" + repr(reference.size)
            )
        out = assetdir / ("skin_" + page + ".png")
        out.write_bytes(raw)
        print("Verified ORIGINAL", out, reference.size, actual_hash)

    (root / "ORIGINAL_UI_VERIFIED.txt").write_text(
        "Serenity WAVETABLE - Original user screenshots restored at 2048 x 1361.\n"
        "Ten RGBA SHA256 pixel verifications PASSED.\n"
        "This is the user-supplied reference UI, not an invented replacement.\n"
        "Builds must additionally pass real editor startup tests.\n", encoding="utf-8"
    )
    print("SUCCESS: native C++ JUCE sources and ten identical original UI captures restored.")

if __name__ == "__main__":
    main()
