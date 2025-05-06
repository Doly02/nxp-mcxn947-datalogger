import re

INPUT_FILE = "lint.log"
OUTPUT_FILE = "lint_filtered.log"

# Path To Application Source Files
TARGET_PATH = r"application\\source\\"

def is_relevant_file_line(line: str) -> bool:
    return re.search(rf"File .*{TARGET_PATH}.*\.c", line, re.IGNORECASE) is not None

def filter_lint_output():
    with open(INPUT_FILE, "r", encoding="utf-16") as fin:
        lines = fin.readlines()

    filtered = []
    capture = False

    for i, line in enumerate(lines):
        if is_relevant_file_line(line):
            capture = True
            filtered.append(line)
            continue

        if capture:
            # Adds Another Linkes, To Get To The End of Finding (Empty Line or Another Finding)
            if line.strip() == "" or line.startswith("File "):
                capture = False
            else:
                filtered.append(line)

    with open(OUTPUT_FILE, "w", encoding="utf-8") as fout:
        fout.writelines(filtered)

    print(f"Done! Output Stored To: {OUTPUT_FILE}")

if __name__ == "__main__":
    filter_lint_output()
