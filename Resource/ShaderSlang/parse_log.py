#!/usr/bin/env python3
"""解析log文件，统计ValueParam参数名出现次数"""

import re
import sys
from collections import Counter

def parse_log(log_file):
    """解析log文件，提取并统计参数名"""
    pattern = re.compile(r'PrepareValueParameters - ValueParam\[\d+\]: (\w+)')
    counter = Counter()

    with open(log_file, 'r', encoding='utf-8') as f:
        for line in f:
            match = pattern.search(line)
            if match:
                param_name = match.group(1)
                counter[param_name] += 1

    return counter

def main():
    if len(sys.argv) < 2:
        print("用法: python parse_log.py <log文件路径>")
        sys.exit(1)

    log_file = sys.argv[1]
    counter = parse_log(log_file)

    if not counter:
        print("未找到匹配的参数")
        return

    # 按出现次数降序排列
    print(f"{'参数名':<50} {'次数':>8}")
    print("-" * 60)
    for name, count in counter.most_common():
        print(f"{name:<50} {count:>8}")

    print("-" * 60)
    print(f"{'总计不同参数':<50} {len(counter):>8}")
    print(f"{'总计出现次数':<50} {sum(counter.values()):>8}")

if __name__ == "__main__":
    main()
