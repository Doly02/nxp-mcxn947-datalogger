import re

INPUT_FILE = "lint.log"
OUTPUT_FILE = "lint_filtered_only_mandatory.log"

# Path To Application Source Files
TARGET_PATH_FRAGMENT = "application\\source\\".lower()

def is_relevant_file_line(line: str) -> bool:
    return line.lower().startswith("file ") and TARGET_PATH_FRAGMENT in line.lower()

def is_filtered_out(line: str) -> bool:
    """Vrací True, pokud má být řádek přeskočen (advisory nebo required)."""
    return "advisory" in line.lower() or "required" in line.lower()

def filter_lint_output():
    with open(INPUT_FILE, "r", encoding="utf-16") as fin:
        lines = fin.readlines()

    filtered = []
    i = 0
    while i < len(lines):
        line = lines[i]

        if is_relevant_file_line(line):
            block = [line]
            i += 1
            while i < len(lines):
                next_line = lines[i]
                if next_line.lower().startswith("file "):  # Begginign of Another Finding
                    break
                if not is_filtered_out(next_line):
                    block.append(next_line)
                i += 1
            if len(block) > 1:  # Ignere Block If The Block Don't Contains Another Content
                filtered.extend(block)
        else:
            i += 1

    with open(OUTPUT_FILE, "w", encoding="utf-8") as fout:
        fout.writelines(filtered)

    print(f"Done! Output Stored To: {OUTPUT_FILE}")

if __name__ == "__main__":
    filter_lint_output()
