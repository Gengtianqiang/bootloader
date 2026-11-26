"""Offline course tools. Standard library only; never builds or contacts a board."""
from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
from pathlib import Path
import shutil
import struct
import sys
import zlib

ROOT = Path(__file__).resolve().parents[1]
HEADER = struct.Struct("<4sHHIIII8s")
MAGIC = b"OTA1"


@dataclass(frozen=True)
class Layout:
    flash_base: int
    flash_size: int
    boot_size: int
    ram_base: int
    ram_size: int

    @property
    def app_base(self) -> int:
        return self.flash_base + self.boot_size

    @property
    def app_size(self) -> int:
        return self.flash_size - self.boot_size

    @classmethod
    def read(cls, path: Path) -> "Layout":
        data = json.loads(path.read_text(encoding="utf-8"))
        layout = cls(int(data["flash_base"], 0), data["flash_size_kib"] * 1024,
                     data["boot_size_kib"] * 1024, int(data["ram_base"], 0),
                     data["ram_size_kib"] * 1024)
        if (data["mcu"] != "STM32F407ZET6" or layout.flash_base != 0x08000000
                or layout.flash_size != 512 * 1024 or layout.ram_base != 0x20000000
                or layout.ram_size != 128 * 1024
                or data["boot_size_kib"] not in (16, 32, 48, 64, 128, 256, 384)
                or data["hse_hz"] != 8000000 or data["sysclk_hz"] != 168000000):
            raise ValueError("unsupported board/layout; port hardware settings first")
        return layout


def check_vectors(image: bytes, base: int, size: int, layout: Layout) -> tuple[int, int]:
    if len(image) < 8:
        raise ValueError("image does not contain two vector words")
    if len(image) > size:
        raise ValueError("image exceeds its Flash partition")
    stack, reset = struct.unpack_from("<II", image)
    if not layout.ram_base < stack <= layout.ram_base + layout.ram_size or stack % 8:
        raise ValueError("initial MSP must be 8-byte aligned in ordinary SRAM (end allowed)")
    entry = reset & ~1
    if not reset & 1 or not base <= entry < base + size:
        raise ValueError("Reset_Handler must be a Thumb address inside this Flash partition")
    return stack, reset


def make_package(image: bytes, layout: Layout, version: int) -> bytes:
    check_vectors(image, layout.app_base, layout.app_size, layout)
    # A structurally plausible vector alone is insufficient: the entry must
    # also be backed by bytes in this received raw image.
    _, reset = struct.unpack_from("<II", image)
    if (reset & ~1) - layout.app_base + 2 > len(image):
        raise ValueError("Reset_Handler points beyond received image bytes")
    if not 0 <= version <= 0xFFFFFFFF:
        raise ValueError("image version must fit uint32")
    crc = zlib.crc32(image) & 0xFFFFFFFF
    return HEADER.pack(MAGIC, 1, HEADER.size, layout.app_base,
                       len(image), crc, version, bytes(8)) + image


def verify_package(package: bytes, layout: Layout) -> tuple[dict, bytes]:
    if len(package) < HEADER.size:
        raise ValueError("truncated OTA header")
    magic, schema, header_size, base, length, crc, version, reserved = HEADER.unpack_from(package)
    if magic != MAGIC or schema != 1 or header_size != HEADER.size or reserved != bytes(8):
        raise ValueError("unsupported or corrupt OTA header")
    if base != layout.app_base:
        raise ValueError("package load address does not match current App partition")
    image = package[HEADER.size:]
    if len(image) != length:
        raise ValueError("payload length does not match header")
    if zlib.crc32(image) & 0xFFFFFFFF != crc:
        raise ValueError("payload CRC32 mismatch")
    # Reuse the complete length, vector and entry-presence checks.
    make_package(image, layout, version)
    return {"schema": schema, "load_address": f"0x{base:08X}",
            "image_length": length, "crc32": f"0x{crc:08X}", "version": version}, image


def inspect_elf(blob: bytes, layout: Layout) -> dict:
    """Read ELF32 little-endian ARM section headers without external packages."""
    if len(blob) < 52:
        raise ValueError("truncated ELF header")
    fields = struct.unpack_from("<16sHHIIIIIHHHHHH", blob)
    ident, elf_type, machine = fields[:3]
    if ident[:7] != b"\x7fELF\x01\x01\x01" or machine != 40 or elf_type != 2:
        raise ValueError("expected an ELF32 little-endian ARM executable")
    shoff, shentsize, shnum, shstrndx = fields[6], fields[11], fields[12], fields[13]
    if shentsize != 40 or not 0 < shstrndx < shnum or shoff + shentsize * shnum > len(blob):
        raise ValueError("invalid/truncated ELF section table")
    sections = [struct.unpack_from("<10I", blob, shoff + i * shentsize) for i in range(shnum)]
    names_header = sections[shstrndx]
    names = blob[names_header[4]:names_header[4] + names_header[5]]
    report = []
    vectors = None
    for section in sections:
        name_offset, kind, _, address, offset, size = section[:6]
        name = names[name_offset:].split(b"\x00", 1)[0].decode("ascii", errors="replace")
        if name in (".isr_vector", ".text", ".rodata", ".data", ".bss", ".heap"):
            report.append({"section": name, "address": f"0x{address:08X}", "size": size})
        if name == ".isr_vector":
            if kind != 1 or size < 8 or offset + size > len(blob):
                raise ValueError("invalid vector section")
            vectors = (address, blob[offset:offset + size])
    if vectors is None:
        raise ValueError("ELF has no .isr_vector section")
    address, data = vectors
    if address == layout.flash_base:
        image_kind, region_size = "bootloader", layout.boot_size
    elif address == layout.app_base:
        image_kind, region_size = "app", layout.app_size
    else:
        raise ValueError("vector table is not at a configured image base")
    stack, reset = check_vectors(data, address, region_size, layout)
    return {"format": "ELF32 ARM", "image": image_kind, "elf_entry": f"0x{fields[4]:08X}",
            "vector_base": f"0x{address:08X}", "initial_msp": f"0x{stack:08X}",
            "reset_handler": f"0x{reset:08X}", "sections": report}


def doctor(layout: Layout) -> dict:
    tools = {name: shutil.which(name) for name in
             ("cmake", "ninja", "arm-none-eabi-gcc", "arm-none-eabi-readelf", "arm-none-eabi-gdb")}
    required = ("CMakeLists.txt", "cmake/arm-none-eabi.cmake", "linker/firmware.ld.in",
                "platform/stm32f407/system_stm32f4xx.c", "startup/startup_stm32f40_41xxx.s",
                "third_party/stm32f4/CMSIS/Include/core_cm4.h",
                "third_party/stm32f4/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c")
    missing_files = [name for name in required if not (ROOT / name).is_file()]
    return {"python": sys.version.split()[0], "tools_on_path": tools,
            "missing_course_files": missing_files,
            "boot_base": f"0x{layout.flash_base:08X}", "boot_bytes": layout.boot_size,
            "app_base": f"0x{layout.app_base:08X}", "app_bytes": layout.app_size,
            "note": "Checks paths/layout only; does not compile or verify board wiring/tool versions."}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--layout", type=Path, default=ROOT / "config/flash_layout.json")
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("doctor", help="check paths and partition arithmetic; no build")
    inspect = sub.add_parser("inspect", help="inspect ELF, raw App BIN, or OTA package")
    inspect.add_argument("input", type=Path)
    pack = sub.add_parser("pack", help="wrap App BIN in a 32-byte CRC32 header")
    pack.add_argument("input", type=Path)
    pack.add_argument("output", type=Path)
    pack.add_argument("--version", type=int, required=True)
    verify = sub.add_parser("verify", help="check package metadata, bounds, vectors, and CRC32")
    verify.add_argument("input", type=Path)
    args = parser.parse_args(argv)
    try:
        layout = Layout.read(args.layout)
        if args.command == "doctor":
            report = doctor(layout)
            print(json.dumps(report, indent=2, ensure_ascii=False))
            return int(bool(report["missing_course_files"]) or
                       any(path is None for path in report["tools_on_path"].values()))
        blob = args.input.read_bytes()
        if args.command == "pack":
            package = make_package(blob, layout, args.version)
            with args.output.open("xb") as output:
                output.write(package)
            report, _ = verify_package(package, layout)
            report["output"] = str(args.output)
        elif args.command == "verify" or blob.startswith(MAGIC):
            report, _ = verify_package(blob, layout)
        elif blob.startswith(b"\x7fELF"):
            report = inspect_elf(blob, layout)
        else:
            stack, reset = check_vectors(blob, layout.app_base, layout.app_size, layout)
            report = {"format": "raw App BIN (address assumed from layout)", "bytes": len(blob),
                      "initial_msp": f"0x{stack:08X}", "reset_handler": f"0x{reset:08X}"}
        print(json.dumps(report, indent=2, ensure_ascii=False))
        return 0
    except (OSError, ValueError, KeyError, TypeError, struct.error) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
