#!/usr/bin/env python3
"""Paired CPU measurement and sample-by-sample audio fidelity on the same runner."""
import csv
import math
import pathlib
import platform
import statistics
import struct
import subprocess
import sys

def executable(build, name):
    for path in (build/name, build/"Release"/name, build/(name+".exe"), build/"Release"/(name+".exe")):
        if path.is_file():
            return path.resolve()
    raise FileNotFoundError(name + " not found in " + str(build))

def row_data(csv_path):
    with csv_path.open(newline="", encoding="utf-8") as f:
        return {row["scenario"]: row for row in csv.DictReader(f)}

def audio_delta(a, b):
    x=a.read_bytes()
    y=b.read_bytes()
    if len(x)!=len(y) or len(x)<128 or len(x)%4:
        raise ValueError(f"Audio buffers have mismatched sizes: {a}, {b}")
    samples=len(x)//4
    u=struct.unpack("<"+str(samples)+"f", x)
    v=struct.unpack("<"+str(samples)+"f", y)
    err=sum((p-q)**2 for p,q in zip(u,v))
    signal=sum(p*p for p in u)
    return max(abs(p-q) for p,q in zip(u,v)),math.sqrt(err/max(signal,1e-30)),samples

def main():
    if len(sys.argv)!=2: raise SystemExit("Usage: compare_hd_ab.py <build-directory>")
    build=pathlib.Path(sys.argv[1]).resolve()
    base=executable(build, "serenity_hd_ab_baseline")
    candidate=executable(build, "serenity_hd_ab_candidate")
    # Alternating order reduces systematic thermal / load / startup bias.
    order=["baseline","candidate","candidate","baseline"]
    programs={"baseline":base,"candidate":candidate}
    trials=[]
    for number,variant in enumerate(order):
        prefix=build/("hd-ab-trial-"+str(number)+"-"+variant)
        subprocess.run([str(programs[variant]),str(prefix)],check=True,timeout=240)
        for scenario,row in row_data(pathlib.Path(str(prefix)+".csv")).items():
            trials.append({"trial":number,"variant":variant,**row})
    cases=sorted(set(r["scenario"] for r in trials))
    with (build/"hd-ab-trials.csv").open("w",newline="",encoding="utf-8") as f:
        writer=csv.DictWriter(f,fieldnames=list(trials[0].keys()))
        writer.writeheader();writer.writerows(trials)
    summary=[]
    for case in cases:
        baseline=[r for r in trials if r["scenario"]==case and r["variant"]=="baseline"]
        new=[r for r in trials if r["scenario"]==case and r["variant"]=="candidate"]
        if len(baseline)!=2 or len(new)!=2:
            raise AssertionError("missing A/B trials: "+case)
        def med(rows,col): return statistics.median(float(row[col]) for row in rows)
        a=med(baseline,"mean_ms")
        b=med(new,"mean_ms")
        before_p99=med(baseline,"p99_ms")
        after_p99=med(new,"p99_ms")
        budget=med(new,"deadline_ms")
        max_error,rel_error,samples=audio_delta(
            build/("hd-ab-trial-0-baseline-"+case+".f32"),
            build/("hd-ab-trial-1-candidate-"+case+".f32"))
        if not math.isfinite(max_error) or not math.isfinite(rel_error):
            raise AssertionError("Non-finite A/B audio comparison")
        summary.append({
            "scenario":case,"baseline_mean_ms":round(a,6),
            "candidate_mean_ms":round(b,6),
            "cpu_change_pct":round((b/a-1)*100,2),
            "baseline_mean_budget_pct":round(100*a/budget,2),
            "candidate_mean_budget_pct":round(100*b/budget,2),
            "baseline_p99_ms":round(before_p99,6),
            "candidate_p99_ms":round(after_p99,6),
            "candidate_p99_budget_pct":round(100*after_p99/budget,2),
            "audio_max_absolute_error":round(max_error,9),
            "audio_relative_rms_error":round(rel_error,9),
            "audio_samples_compared":samples,
            "quality_pass":max_error<=0.0003 and rel_error<=0.001
        })
    with (build/"hd-ab-summary.csv").open("w",newline="",encoding="utf-8") as f:
        writer=csv.DictWriter(f,fieldnames=list(summary[0].keys()))
        writer.writeheader();writer.writerows(summary)
    lines=[
        "# Serenity HD v1.2.9 — paired A/B benchmark",
        "",
        f"Runner: {platform.system()} {platform.machine()}. Pinned baseline: v1.2.8 commit 5b27c97b48f005bfd7f6eae79347d30e4352f575.",
        "A/B/A/B order: baseline, candidate, candidate, baseline, on the same runner, same source benchmark, identical high-density HD tables and MIDI notes.",
        "CPU numbers are medians of two trials for each version. Shared-runner scheduling and thermal variation remain sources of measurement noise.",
        "A negative change indicates less CPU time. Fidelity compares the complete sequence of rendered interleaved stereo blocks, sample for sample.",
        "",
        "| Scenario | Baseline mean % budget | New mean % budget | Mean CPU change | New p99 % budget | Max audio delta |",
        "|---|---:|---:|---:|---:|---:|",
    ]
    for s in summary:
        lines.append(f"| {s['scenario']} | {s['baseline_mean_budget_pct']:.2f}% | {s['candidate_mean_budget_pct']:.2f}% | {s['cpu_change_pct']:+.2f}% | {s['candidate_p99_budget_pct']:.2f}% | {s['audio_max_absolute_error']:.8f} |")
    lines.append("")
    lines.append("Quality tolerance: max absolute sample difference <= 0.0003, relative RMS difference <= 0.001.")
    lines.append("CI success does not certify 32-note ×16 unison as meeting real-time deadlines. See CSV for outliers and actual block times.")
    (build/"hd-ab-summary.md").write_text("\n".join(lines)+"\n",encoding="utf-8")
    print("\n".join(lines))
    if not all(s["quality_pass"] for s in summary):
        raise AssertionError("New engine failed same-runner sample-by-sample quality regression")

if __name__=="__main__":
    main()
