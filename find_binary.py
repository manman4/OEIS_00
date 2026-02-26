import os

IGNORED_FILES = {".DS_Store"}
IGNORED_DIRS = {".git"}
IGNORED_EXTENSIONS = {".txt", ".pdf"}


def is_binary(path):
    try:
        with open(path, "rb") as f:
            chunk = f.read(1024)
            return b"\x00" in chunk
    except Exception:
        return False


def find_binary_files(root):
    for root_dir, dirs, files in os.walk(root):

        # ディレクトリ除外
        dirs[:] = [d for d in dirs if d not in IGNORED_DIRS]

        for name in files:
            # ファイル名除外
            if name in IGNORED_FILES:
                continue

            # 拡張子除外
            ext = os.path.splitext(name)[1].lower()
            if ext in IGNORED_EXTENSIONS:
                continue

            path = os.path.join(root_dir, name)
            if os.path.isfile(path) and is_binary(path):
                print(path)


if __name__ == "__main__":
    find_binary_files(".")