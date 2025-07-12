import struct
import zlib
import sys

POLY = 0xEDB88320
START_XOR = 0xFFFFFFFF

# Forward and reverse lookup tables
crc_table = [0] * 256
rev_table = [0] * 256

def init_tables():
    global crc_table, rev_table
    for i in range(256):
        fwd = i
        rev = i << 24
        for _ in range(8):
            if fwd & 1:
                fwd = (fwd >> 1) ^ POLY
            else:
                fwd >>= 1

            if rev & 0x80000000:
                rev = ((rev ^ POLY) << 1) | 1
            else:
                rev <<= 1
            rev &= 0xFFFFFFFF
        crc_table[i] = fwd
        rev_table[i] = rev

def fix_crc32(data: bytearray, fixpos: int, wantcrc: int):
    """Patch 4 bytes at fixpos so the CRC32 of the whole data equals wantcrc."""
    length = len(data)
    if fixpos + 4 > length:
        raise ValueError("Fix position out of range")

    # Forward CRC up to fixpos
    crc = START_XOR
    for i in range(fixpos):
        crc = (crc >> 8) ^ crc_table[(crc ^ data[i]) & 0xFF]

    # Write temporary value (not used directly)
    data[fixpos:fixpos+4] = struct.pack("<I", crc)

    # Reverse CRC from end to fixpos
    crc = wantcrc ^ START_XOR
    for i in reversed(range(fixpos, length)):
        b = data[i]
        index = (crc >> 24) & 0xFF
        crc = ((crc << 8) & 0xFFFFFFFF) ^ rev_table[index] ^ b

    # Write final patch value
    data[fixpos:fixpos+4] = struct.pack("<I", crc)

    # finally we return the patched CRC in order to patch the elf with the same
    return crc

def patch_firmware_info(elf_path, length_val, crc_val):
    with open(elf_path, 'rb+') as f:
        elf = f.read()

        # Validate ELF header
        if elf[:4] != b'\x7fELF':
            raise RuntimeError("Not a valid ELF file")

        if elf[4] != 1:
            raise NotImplementedError("Only 32-bit ELF files are supported")

        # ELF header fields
        e_shoff = struct.unpack_from('<I', elf, 0x20)[0]
        e_shentsize = struct.unpack_from('<H', elf, 0x2E)[0]
        e_shnum = struct.unpack_from('<H', elf, 0x30)[0]
        e_shstrndx = struct.unpack_from('<H', elf, 0x32)[0]

        # Locate section header string table
        shstr_offset = struct.unpack_from('<I', elf, e_shoff + e_shstrndx * e_shentsize + 0x10)[0]

        # Find `.firmware_info` section
        for i in range(e_shnum):
            sh_offset = e_shoff + i * e_shentsize
            sh_name_offset = struct.unpack_from('<I', elf, sh_offset)[0]
            section_offset = struct.unpack_from('<I', elf, sh_offset + 0x10)[0]
            section_size = struct.unpack_from('<I', elf, sh_offset + 0x14)[0]

            # Read section name
            end = elf.find(b'\x00', shstr_offset + sh_name_offset)
            name = elf[shstr_offset + sh_name_offset:end].decode()

            if name == '.firmware_info':
                print(f"Found .firmware_info at file offset 0x{section_offset:X} (size {section_size} bytes)")

                # Offsets in struct: length @ 4, crc32 @ 12
                if section_size < 16:
                    raise RuntimeError(".firmware_info section too small")

                # Patch length
                f.seek(section_offset + 4)
                f.write(struct.pack('<I', length_val))

                # Patch CRC
                f.seek(section_offset + 12)
                f.write(struct.pack('<I', crc_val))

                print(f"Patched length = 0x{length_val:08X}, crc32 = 0x{crc_val:08X}")
                return

        raise RuntimeError("Could not find .firmware_info section")

if __name__ == "__main__":
    # filename without extension, we then patch ".bin" and ".elf"
    filename = sys.argv[1]

    firmware_info = 0x2a0
    len_offset = firmware_info + 4
    patch_offset = firmware_info + 12

    init_tables()

    with open(filename + ".bin", "r+b") as f:
        buf = bytearray(f.read())
        f.close()
    # from magic number
    desired_crc =  int.from_bytes(buf[firmware_info:firmware_info + 4], byteorder='little', signed=False)

    # store len
    buf[len_offset:len_offset+4] = struct.pack("<I", len(buf))

    written_crc = fix_crc32(buf, patch_offset, desired_crc)

    result = zlib.crc32(buf) & 0xFFFFFFFF
    print(f"Patched CRC: {result:08X}")
    assert result == desired_crc

    with open(filename + ".bin", "wb") as f:
        f.write(buf)
        f.close()

    # now patch elf
    #    patch_elf_at_addr(filename + ".elf", 0x08002A0,len(buf), result)
    print(f"written_crc: {written_crc:08X}");
    patch_firmware_info(filename + ".elf",len(buf),written_crc)
