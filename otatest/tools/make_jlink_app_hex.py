"""Package the linked App and its Step2 launch record in one SWD HEX file."""

import argparse
from pathlib import Path
import struct
import zlib


APP_BASE = 0x08008000
RECORD_ADDRESS = 0x0801FFF0
RECORD_MAGIC = 0x31474D49  # IMG1 on the little-endian target
SRAM_BASE = 0x20000000
SRAM_END = 0x20020000


def image_record(image):
    size = len(image)
    if not 8 <= size <= RECORD_ADDRESS - APP_BASE:
        raise ValueError("App size exceeds the run partition or lacks a vector table")

    stack, reset = struct.unpack_from("<II", image)
    reset_address = reset & ~1
    if (not SRAM_BASE < stack <= SRAM_END or stack % 8 or not reset & 1
            or not APP_BASE + 8 <= reset_address < APP_BASE + size):
        raise ValueError("Invalid App vectors. Build the App-Debug preset.")

    return struct.pack("<IIII", size, zlib.crc32(image), size ^ 0xFFFFFFFF, RECORD_MAGIC)


def hex_record(address, kind, data):
    record = struct.pack(">BHB", len(data), address, kind) + data
    return ":" + (record + bytes([-sum(record) & 0xFF])).hex().upper()


def image_records(image, commit):
    upper = None
    for start, data in ((APP_BASE, image), (RECORD_ADDRESS, commit)):
        for offset in range(0, len(data), 16):
            address = start + offset
            if address >> 16 != upper:
                upper = address >> 16
                yield hex_record(0, 4, struct.pack(">H", upper))
            yield hex_record(address & 0xFFFF, 0, data[offset:offset + 16])
    yield hex_record(0, 1, b"")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input_bin", type=Path)
    parser.add_argument("output_hex", type=Path)
    args = parser.parse_args()
    if args.input_bin.resolve() == args.output_hex.resolve():
        parser.error("Choose a separate output path")
    try:
        image = args.input_bin.read_bytes()
        commit = image_record(image)
        args.output_hex.write_text("\n".join(image_records(image, commit)) + "\n",
                                   encoding="ascii")
    except (OSError, ValueError) as error:
        parser.exit(1, f"App HEX: {error}\n")
    print(f"Prepared {args.output_hex} ({len(image)} App bytes plus {len(commit)} record bytes)")


if __name__ == "__main__":
    main()
