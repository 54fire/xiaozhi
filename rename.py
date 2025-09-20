#!/usr/bin/env python3
import os
import re

files = os.listdir("music")
pattern = re.compile(r"(\d+)-(\d+)(\..+)?$")  # 匹配 1-1.jpg 这种

# 过滤并排序
matched_files = []
for f in files:
    m = pattern.match(f)
    if m:
        row, col, ext = int(m.group(1)), int(m.group(2)), m.group(3) or ""
        matched_files.append((row, col, ext, f))

# 按行列排序（先行再列）
matched_files.sort(key=lambda x: (x[0], x[1]))
old_dir = "music"
new_dir = "music_m4a"
# 重命名
for idx, (_, _, ext, old) in enumerate(matched_files):
    new = f"{idx}{ext}"
    print(f"{old} -> {new}")
    os.rename(old_dir + "/" + old, new_dir + "/" + new)
