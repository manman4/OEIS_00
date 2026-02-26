import os
import sys

def is_binary(path):
    try:
        with open(path, "rb") as f:
            magic = f.read(4)

        # ELF
        if magic == b"\x7fELF":
            return True

        # Mach-O
        if magic in (
            b"\xfe\xed\xfa\xce",
            b"\xfe\xed\xfa\xcf",
            b"\xca\xfe\xba\xbe",
        ):
            return True

        return False
    except Exception:
        return False


def find_compiled_binaries(root):
    for root_dir, _, files in os.walk(root):
        for name in files:
            path = os.path.join(root_dir, name)
            if os.path.isfile(path) and os.access(path, os.X_OK):
                if is_binary(path):
                    print(path)


find_compiled_binaries(".")