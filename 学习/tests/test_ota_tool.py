"""Exercise actual package/parser failure cases; no firmware compilation."""
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
import zlib

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from ota_tool import HEADER, ROOT, Layout, check_vectors, inspect_elf, make_package, verify_package


class PackageTests(unittest.TestCase):
    def setUp(self):
        self.layout = Layout.read(ROOT / "config/flash_layout.json")
        # Parser fixture only: not executable firmware, never written/flashed.
        self.image = struct.pack("<II", 0x20020000, self.layout.app_base + 9) + bytes(56)

    def test_roundtrip_preserves_every_byte_and_version(self):
        metadata, image = verify_package(make_package(self.image, self.layout, 42), self.layout)
        self.assertEqual(image, self.image)
        self.assertEqual(metadata["version"], 42)
        self.assertEqual(metadata["image_length"], len(self.image))

    def test_crc_known_vector_matches_documented_crc32(self):
        self.assertEqual(zlib.crc32(b"123456789") & 0xFFFFFFFF, 0xCBF43926)

    def test_payload_bit_flip_rejected(self):
        package = bytearray(make_package(self.image, self.layout, 1))
        package[-1] ^= 1
        with self.assertRaisesRegex(ValueError, "CRC32"):
            verify_package(package, self.layout)

    def test_truncated_or_extra_payload_rejected(self):
        package = make_package(self.image, self.layout, 1)
        for malformed in (package[:-1], package + b"x", package[:12]):
            with self.subTest(length=len(malformed)), self.assertRaises(ValueError):
                verify_package(malformed, self.layout)

    def test_wrong_load_address_rejected(self):
        package = bytearray(make_package(self.image, self.layout, 1))
        struct.pack_into("<I", package, 8, self.layout.flash_base)
        with self.assertRaisesRegex(ValueError, "load address"):
            verify_package(package, self.layout)

    def test_bad_schema_or_reserved_bytes_rejected(self):
        package = make_package(self.image, self.layout, 1)
        for offset in (0, 4, 6, 31):
            changed = bytearray(package)
            changed[offset] ^= 1
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                verify_package(changed, self.layout)

    def test_ram_end_is_valid_initial_stack(self):
        stack, _ = check_vectors(self.image, self.layout.app_base, self.layout.app_size, self.layout)
        self.assertEqual(stack, 0x20020000)

    def test_invalid_stacks_rejected(self):
        for stack in (0xFFFFFFFF, 0x20000000, 0x20020008, 0x20010004, 0x10010000):
            image = struct.pack("<I", stack) + self.image[4:]
            with self.subTest(stack=stack), self.assertRaises(ValueError):
                make_package(image, self.layout, 1)

    def test_thumb_and_partition_checks(self):
        for entry in (self.layout.app_base + 8, self.layout.flash_base + 9, 0x08080001):
            image = self.image[:4] + struct.pack("<I", entry) + self.image[8:]
            with self.subTest(entry=entry), self.assertRaises(ValueError):
                make_package(image, self.layout, 1)

    def test_reset_entry_must_exist_in_received_file(self):
        image = self.image[:4] + struct.pack("<I", self.layout.app_base + 0x101) + self.image[8:]
        with self.assertRaisesRegex(ValueError, "beyond received"):
            make_package(image, self.layout, 1)

    def test_oversize_rejected(self):
        with self.assertRaisesRegex(ValueError, "exceeds"):
            make_package(self.image + bytes(self.layout.app_size), self.layout, 1)

    def test_invalid_partition_boundary_rejected(self):
        data = json.loads((ROOT / "config/flash_layout.json").read_text())
        data["boot_size_kib"] = 40
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "layout.json"
            path.write_text(json.dumps(data))
            with self.assertRaises(ValueError):
                Layout.read(path)

    def test_elf_vector_section_is_read_without_external_tools(self):
        names = b"\x00.shstrtab\x00.isr_vector\x00"
        vector_offset = 52 + len(names)
        table_offset = vector_offset + len(self.image)
        header = struct.pack("<16sHHIIIIIHHHHHH", b"\x7fELF\x01\x01\x01" + bytes(9),
                             2, 40, 1, self.layout.app_base + 9, 0, table_offset,
                             0, 52, 0, 0, 40, 3, 1)
        sections = bytes(40)
        sections += struct.pack("<10I", 1, 3, 0, 0, 52, len(names), 0, 0, 1, 0)
        sections += struct.pack("<10I", 11, 1, 2, self.layout.app_base,
                                vector_offset, len(self.image), 0, 0, 512, 0)
        report = inspect_elf(header + names + self.image + sections, self.layout)
        self.assertEqual(report["image"], "app")
        self.assertEqual(report["vector_base"], f"0x{self.layout.app_base:08X}")

    def test_truncated_elf_rejected(self):
        with self.assertRaisesRegex(ValueError, "truncated"):
            inspect_elf(b"\x7fELF", self.layout)


if __name__ == "__main__":
    unittest.main()
