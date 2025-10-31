#!/usr/bin/env python3
import argparse, re, os, sys

def parse_dimacs_header(lines):
    hdr_idx = None
    nvars = nclauses = None
    for i, ln in enumerate(lines):
        ln = ln.strip()
        if ln.startswith('p cnf'):
            parts = ln.split()
            if len(parts) >= 4:
                try:
                    nvars = int(parts[2])
                    nclauses = int(parts[3])
                    hdr_idx = i
                    break
                except ValueError:
                    pass
    if hdr_idx is None:
        raise ValueError("DIMACS header ('p cnf ...') not found.")
    return hdr_idx, nvars, nclauses

def read_assignments_file(path):
    """읽은 모델 텍스트에서 v 라인들의 리터럴을 모아 0에서 종료."""
    lits = []
    with open(path, 'r') as f:
        for raw in f:
            s = raw.strip()
            if not s:
                continue
            # 허용: 맨 앞에 'v '가 있을 수도 있고 없을 수도
            if s.startswith('v '):
                s = s[2:].strip()
            # 라인 안의 토큰들을 전부 스캔
            for tok in s.split():
                # 첫 0에서 종료
                if tok == '0':
                    return lits
                # 숫자만 취급 (부호 포함)
                if re.fullmatch(r'-?\d+', tok):
                    v = int(tok)
                    if v == 0:
                        return lits
                    lits.append(v)
                # 그 외 토큰은 무시 (예: stray 'v')
    return lits

def main():
    ap = argparse.ArgumentParser(
        description="Append unit clauses from a model text (v ... 0) to a DIMACS CNF and fix header counts."
    )
    ap.add_argument("-i", "--input",  required=True, help="Input CNF file (DIMACS)")
    ap.add_argument("-a", "--assign", required=True, help="Assignments text file (lines starting with 'v', terminated by 0)")
    ap.add_argument("-o", "--output", required=True, help="Output CNF file path")
    ap.add_argument("--dedup", action="store_true", help="Remove duplicate unit clauses (same literal)")
    args = ap.parse_args()

    # 읽기
    with open(args.input, 'r') as f:
        cnf_lines = f.readlines()

    hdr_idx, nvars, nclauses = parse_dimacs_header(cnf_lines)

    # 모델 파싱
    lits = read_assignments_file(args.assign)
    if not lits:
        print("No literals parsed from assignments file (no '0' terminator or empty).", file=sys.stderr)
        sys.exit(1)

    # 중복 제거 옵션
    if args.dedup:
        # 같은 literal 중복 제거 (순서 유지)
        seen = set()
        deduped = []
        for L in lits:
            if L not in seen:
                deduped.append(L)
                seen.add(L)
        lits = deduped

    # 헤더 갱신
    maxvar_in_units = max(abs(L) for L in lits) if lits else 0
    new_nvars    = max(nvars, maxvar_in_units)
    new_nclauses = nclauses + len(lits)

    # 새 헤더 라인 작성
    new_header = f"p cnf {new_nvars} {new_nclauses}\n"

    # 본문 그대로 두고 헤더만 치환
    out_lines = cnf_lines[:]
    out_lines[hdr_idx] = new_header

    # 유닛 클로즈 추가
    # 주석으로 출처 한 줄
    #out_lines.append(f"c appended {len(lits)} unit clauses from {os.path.basename(args.assign)}\n")
    for L in lits:
        out_lines.append(f"{L} 0\n")

    # 저장
    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    with open(args.output, 'w') as f:
        f.writelines(out_lines)

    print(f"✅ Wrote '{args.output}' with header 'p cnf {new_nvars} {new_nclauses}' "
          f"(added {len(lits)} unit clauses).")

if __name__ == "__main__":
    main()