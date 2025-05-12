import os

def is_code_line(line):
    line = line.strip()
    return line and not line.startswith('//') and not line.startswith('/*') and not line.startswith('*')

def count_code_lines_in_file(path):
    count = 0
    no_comment_count = 0
    in_block_comment = False
    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            count += 1
            line = line.strip()
            if in_block_comment:
                if '*/' in line:
                    in_block_comment = False
                continue
            if line.startswith('/*'):
                in_block_comment = True
                continue
            if is_code_line(line):
                no_comment_count += 1
    return [count, no_comment_count]

def count_lines_in_dir(root):
    total = 0
    no_comment_total = 0
    for dirpath, _, filenames in os.walk(root):
        for fname in filenames:
            if fname.endswith(('.c', '.cpp', '.h')):
                full_path = os.path.join(dirpath, fname)
                lines, no_comment_lines = count_code_lines_in_file(full_path)
                print(f"{full_path}: {lines} lines, {no_comment_lines} no comment lines")
                total += lines
                no_comment_total += no_comment_lines
    print(f"\nTotal code lines: {total} {no_comment_total}")

if __name__ == "__main__":
    count_lines_in_dir(".")  # 当前目录