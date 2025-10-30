#!/usr/bin/env python3
import json
import argparse
import sys
import math

# How to use:
# cnf=16c5482d8e658b54e20d59cfd4b1d588-two-trees-511v.sanitized
# python init_generate/init_generate.py -i /home/curie/DiffSAT/dataset/sat_cnf_2024_satisfiable/$cnf/results_3SAT/ranked_by_confidence_0.txt -o /home/curie/kissat/init_generate/0.txt 
# ./build/kissat --init-phase-file=/home/curie/kissat/init_generate/0.txt /home/curie/DiffSAT/dataset/sat_cnf_2024_satisfiable/$cnf/$cnf.cnf
def main():
    parser = argparse.ArgumentParser(
        description="init-phase text generator"
    )
    parser.add_argument("--input", "-i", required=True, help="Input confidence file path")
    parser.add_argument("--output", "-o", required=True, help="Output init phase file path")
    parser.add_argument("--topk", type=float, default=None,
                        help="Top-k variables")
    args = parser.parse_args()

    input_path = args.input
    output_path = args.output

    try:
        with open(input_path, "r") as f:
            entries = [json.loads(line) for line in f if line.strip()]
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

    if not entries:
        print("Error", file=sys.stderr)
        sys.exit(1)

    max_index = max(e["index"] for e in entries)
    phases = ["d"] * (max_index + 1)
    num_vars = max_index + 1
    if args.topk is not None:
        k = math.ceil(num_vars * args.topk)
        if k == 0 and args.topk > 0:
            k = 1
        entries = entries[:k]
    
    for e in entries:
        idx = e.get("index")
        val = e.get("assignment")
        if idx is None or val not in (0, 1):
            print(f"Error: {e}", file=sys.stderr)
            continue
        phases[idx] = str(val)

    try:
        with open(output_path, "w") as f:
            f.write(" ".join(phases))
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

    print(f"Done! '{output_path}' -> ({len(phases)} entries)")
    print(f"   ex: {output_path} → '1 0 d d 1 ...'")

if __name__ == "__main__":
    main()