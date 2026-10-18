#!/usr/bin/env python3
"""Generates the ISA definitions from docs/instructions.json.

Outputs (all rendered through the Jinja2 templates in tools/templates):
  rtl/isa.sv                        SystemVerilog package
  include/isa/operator.hpp          C++ Operator enum and field layout tables
  docs/instruction_formats.md       opcode and bit layout tables

Use --check to fail when the checked in outputs are stale.
"""
import argparse
import json
import math
import re
import sys
from pathlib import Path

from jinja2 import Environment, FileSystemLoader, StrictUndefined

ROOT = Path(__file__).resolve().parent.parent
SPEC = ROOT / "docs" / "instructions.json"
TEMPLATES = Path(__file__).resolve().parent / "templates"
OUTPUTS = {
    "isa.sv.j2": ROOT / "rtl" / "isa.sv",
    "operator.hpp.j2": ROOT / "include" / "isa" / "operator.hpp",
    "instruction_formats.md.j2": ROOT / "docs" / "instruction_formats.md",
}


def clog2(v):
    return max(1, math.ceil(math.log2(v)))


def camel(snake):
    return "".join(p.capitalize() for p in snake.split("_"))


def snake_upper(camel_name):
    return re.sub(r"(?<=[a-z0-9])(?=[A-Z])", "_", camel_name).upper()


def to_sv(expr):
    """Translates a width expression into a SystemVerilog constant expression."""
    expr = re.sub(r"\b[a-z][a-z0-9_]*\b", lambda m: m.group(0).upper(), expr)
    expr = expr.replace("CLOG2(", "$clog2(")
    return re.sub(r"MAX\(([^,()]+),\s*([^()]+)\)", r"((\1) > (\2) ? (\1) : (\2))", expr)


class Spec:
    def __init__(self, data):
        self.data = data
        self.values = dict(data["parameters"])
        self.values["special_count"] = len(data["special_registers"])
        self.exprs = {k: k for k in self.values}
        for name, expr in data["derived"].items():
            self.values[name] = self.evaluate(expr)
        self.word = self.values["word_size"]
        self.op_bits = self.values["op_size"]

    def evaluate(self, expr):
        return int(eval(str(expr), {"__builtins__": {}}, {**self.values, "clog2": clog2, "max": max}))

    def resolve_format(self, name, fmt):
        fields = []
        if fmt["opcode"]:
            fields.append({"name": "op", "width": "op_size", "type": "op_e", "comment": "operation code"})
        fields.extend(dict(f) for f in fmt["fields"])
        for f in fields:
            if not f.get("fill"):
                f["bits"] = self.evaluate(f["width"])
                f["sv_width"] = to_sv(str(f["width"]))
        used = sum(f["bits"] for f in fields if not f.get("fill"))
        fill_bits = self.word - used
        if fill_bits < 0:
            raise ValueError(f"format {name} is {used} bits wide, the word is {self.word}")
        resolved, lsb = [], 0
        for f in fields:
            if f.get("fill"):
                if fill_bits == 0:
                    continue
                f["bits"] = fill_bits
                others = " + ".join(g["sv_width"] for g in fields if not g.get("fill"))
                f["sv_width"] = f"WORD_SIZE - ({others})"
            elif fmt.get("fill") is False:
                pass
            f["lsb"] = lsb
            lsb += f["bits"]
            resolved.append(f)
        if lsb != self.word:
            raise ValueError(f"format {name} is {lsb} bits wide, expected {self.word}")
        return {"name": name, "type": f"{name}_t", "camel": camel(name), "comment": fmt["comment"],
                "opcode": fmt["opcode"], "fields": resolved}

    def build(self):
        formats = [self.resolve_format(n, f) for n, f in self.data["formats"].items()]
        names = {f["name"] for f in formats}
        seen = {}
        instructions = []
        for i in self.data["instructions"]:
            if i["format"] not in names:
                raise ValueError(f"{i['name']} uses unknown format {i['format']}")
            if i["opcode"] in seen:
                raise ValueError(f"opcode {i['opcode']:#x} used by {seen[i['opcode']]} and {i['name']}")
            if i["opcode"] >= (1 << self.op_bits):
                raise ValueError(f"opcode of {i['name']} does not fit in {self.op_bits} bits")
            seen[i["opcode"]] = i["name"]
            instructions.append({**i, "sv": "OP_" + snake_upper(i["name"]),
                                 "hex": f"{i['opcode']:0{(self.op_bits + 3) // 4}X}"})
        special = [
            {"name": r["name"], "sv": "SPECIAL_" + snake_upper(r["name"]), "cpp": camel(r["name"]),
             "index": i, "comment": r.get("comment", "")}
            for i, r in enumerate(self.data["special_registers"])
        ]
        return {
            "parameters": [{"name": k, "sv": k.upper(), "value": v} for k, v in self.data["parameters"].items()],
            "special": special,
            "special_count": len(special),
            "derived": [{"name": k, "sv": k.upper(), "expr": e, "sv_expr": to_sv(e), "value": self.values[k]}
                        for k, e in self.data["derived"].items()],
            "enums": [{"name": k, **v} for k, v in self.data["enums"].items()],
            "formats": formats,
            "instructions": instructions,
            "op_bits": self.op_bits,
            "hex_digits": (self.op_bits + 3) // 4,
            "word": self.word,
        }


def render(model):
    env = Environment(loader=FileSystemLoader(TEMPLATES), undefined=StrictUndefined,
                      trim_blocks=True, lstrip_blocks=True, keep_trailing_newline=True)
    env.filters["camel"] = camel
    return {tpl: env.get_template(tpl).render(**model) for tpl in OUTPUTS}


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--check", action="store_true", help="fail if the generated files are out of date")
    args = parser.parse_args()
    model = Spec(json.loads(SPEC.read_text())).build()
    stale = []
    for tpl, text in render(model).items():
        path = OUTPUTS[tpl]
        if args.check:
            if not path.exists() or path.read_text() != text:
                stale.append(path)
        else:
            path.write_text(text)
    if stale:
        print("out of date (run tools/generate_isa.py):", *map(str, stale), sep="\n  ")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
